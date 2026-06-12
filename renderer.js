// mcMidiPlayer — renderer
// License: GPL-3.0-or-later — D.Blanchemain

// ── État ──────────────────────────────────────────────────────────────────────

let schedule      = [];   // [{ sec, cmd, interpIdx, key, velocity? } | { sec, cmd:'bank_switch', interpIdx }]
let totalDuration = 0;
let playTimeouts  = [];
let playInterval  = null;
let playOffset    = 0;
let playT0        = 0;
let isPlaying     = false;
let isPaused      = false;

let audioReady    = false;
let audioReadyCb  = null;

// Par interprète
let interpBanks   = [];   // [{ name, banks, bankIdx, activeSlot, loadedIds, activeKeyMap }]
let interpVolumes = [];   // volume (0..2) par interprète
let interpMuted   = [];   // mute par interprète
let interpDots    = {};   // interpIdx → élément .track-dot

// Chargement initial
let loadPending     = 0;
let loadResolved    = 0;
let loadingComplete = false;

// ── IDs de pistes ─────────────────────────────────────────────────────────────

function mkId(slot, interpIdx, key) {
  return `${slot}_i${interpIdx}_${key}`;
}

// ── Volume knob ───────────────────────────────────────────────────────────────

const VOL_MIN = 0;
const VOL_MAX = 2;

function volToDeg(vol) {
  return (vol - 1) * 135;
}

function setInterpVolume(interpIdx, vol) {
  interpVolumes[interpIdx] = vol;
  if (interpMuted[interpIdx]) return;
  const state = interpBanks[interpIdx];
  if (!state) return;
  for (const id of state.loadedIds[state.activeSlot] ?? []) {
    window.api.sendAudio({ cmd: 'set_gain', id, gain: vol });
  }
}

function setInterpMute(interpIdx, muted) {
  interpMuted[interpIdx] = muted;
  const gain = muted ? 0 : (interpVolumes[interpIdx] ?? 1);
  const state = interpBanks[interpIdx];
  if (!state) return;
  for (const id of state.loadedIds[state.activeSlot] ?? []) {
    window.api.sendAudio({ cmd: 'set_gain', id, gain });
  }
}

function initVolKnob(wrap, interpIdx) {
  const svg = wrap.querySelector('.vol-knob');
  const val = wrap.querySelector('.vol-val');
  let startY, startVol;

  svg.addEventListener('mousedown', e => {
    e.preventDefault();
    startY   = e.clientY;
    startVol = interpVolumes[interpIdx] ?? 1;

    const onMove = e2 => {
      const delta  = (startY - e2.clientY) * 0.02;
      const newVol = Math.round(Math.max(VOL_MIN, Math.min(VOL_MAX, startVol + delta)) * 10) / 10;
      setInterpVolume(interpIdx, newVol);
      svg.style.transform = `rotate(${volToDeg(newVol)}deg)`;
      val.textContent = newVol.toFixed(1);
    };
    const onUp = () => {
      document.removeEventListener('mousemove', onMove);
      document.removeEventListener('mouseup',   onUp);
    };
    document.addEventListener('mousemove', onMove);
    document.addEventListener('mouseup',   onUp);
  });

  svg.addEventListener('dblclick', () => {
    setInterpVolume(interpIdx, 1);
    svg.style.transform = `rotate(${volToDeg(1)}deg)`;
    val.textContent = '1.0';
  });
}

// ── Audio events ──────────────────────────────────────────────────────────────

window.api.onAudioEvent(msg => {
  if (msg.type === 'ready') {
    audioReady = true;
    setStatus(`audio: ${msg.backend} (${msg.maxPorts} ch)`, 'ok');
    if (audioReadyCb) { audioReadyCb(); audioReadyCb = null; }
  } else if (msg.type === 'error') {
    setStatus(`audio: ${msg.message}`, 'err');
  } else if ((msg.type === 'loaded' || msg.type === 'load_error') && !loadingComplete) {
    const m = msg.id?.match(/^([ab])_i(\d+)_/);
    if (m) {
      const dot = interpDots[parseInt(m[2])];
      if (dot) {
        if (msg.type === 'loaded') dot.className = 'track-dot ready';
        else { dot.className = 'track-dot error'; dot.title = msg.message ?? ''; }
      }
    }
    loadResolved++;
    checkAllLoaded();
  }
});

function checkAllLoaded() {
  if (loadPending === 0) return;
  if (loadResolved >= loadPending) {
    loadingComplete = true;
    enableTransport(true);
    setStatus('Prêt', 'ok');
  } else {
    setStatus(`Chargement des sons… (${loadResolved} / ${loadPending})`);
  }
}

function waitAudioReady(ms = 6000) {
  if (audioReady) return Promise.resolve();
  return new Promise(resolve => {
    audioReadyCb = resolve;
    setTimeout(() => { if (audioReadyCb) { audioReadyCb = null; resolve(); } }, ms);
  });
}

// ── Status ────────────────────────────────────────────────────────────────────

function setStatus(txt, cls = '') {
  const el = document.getElementById('statusBar');
  el.textContent = txt;
  el.className = 'status' + (cls ? ' ' + cls : '');
}

function setMidiStatus(txt) {
  document.getElementById('midiStatus').textContent = txt;
}

// ── MIDI parser (Type 0 / Type 1, robuste) ───────────────────────────────────

function parseMidi(raw) {
  let u8;
  if (raw instanceof Uint8Array)        u8 = raw;
  else if (raw instanceof ArrayBuffer)  u8 = new Uint8Array(raw);
  else if (raw && raw.type === 'Buffer') u8 = new Uint8Array(raw.data);
  else if (Array.isArray(raw))          u8 = new Uint8Array(raw);
  else throw new Error('Données MIDI illisibles');

  const total = u8.length;
  let p = 0;

  const safe8  = () => p < total ? u8[p++] : 0;
  const rd16   = () => (safe8() << 8) | safe8();
  const rd32   = () => ((safe8() << 24) | (safe8() << 16) | (safe8() << 8) | safe8()) >>> 0;
  const rdVL   = () => { let v = 0, b; do { b = safe8(); v = (v << 7) | (b & 0x7f); } while ((b & 0x80) && p < total); return v; };

  if (rd32() !== 0x4d546864) throw new Error('Fichier MIDI invalide (pas de MThd)');
  const hdrLen = rd32();
  const format  = rd16();
  const nTracks = rd16();
  const ppq     = rd16() & 0x7fff;
  if (hdrLen > 6) p += hdrLen - 6;

  const tracks = [];

  while (p + 8 <= total) {
    const magic    = rd32();
    const chunkLen = rd32();
    const chunkEnd = Math.min(p + chunkLen, total);

    if (magic !== 0x4d54726b) { p = chunkEnd; continue; }

    const evs = [];
    let tick = 0, rs = 0;

    while (p < chunkEnd) {
      tick += rdVL();
      if (p >= chunkEnd) break;

      let st = u8[p];
      if (st & 0x80) {
        p++;
        if (st < 0xf0) rs = st;
      } else {
        st = rs;
      }

      const type = st & 0xf0;

      if (type === 0x80) {
        const n = safe8(); safe8();
        evs.push({ tick, type: 'off', note: n });
      } else if (type === 0x90) {
        const n = safe8(), v = safe8();
        evs.push({ tick, type: v ? 'on' : 'off', note: n, vel: v });
      } else if (type === 0xa0 || type === 0xb0 || type === 0xe0) {
        safe8(); safe8();
      } else if (type === 0xc0) {
        const prog = safe8();
        evs.push({ tick, type: 'pc', program: prog });
      } else if (type === 0xd0) {
        safe8();
      } else if (st === 0xff) {
        const mt = safe8(), ml = rdVL();
        if (mt === 0x51 && ml === 3) {
          evs.push({ tick, type: 'tempo', tempo: (safe8() << 16) | (safe8() << 8) | safe8() });
        } else if (mt === 0x03) {
          evs.trackName = new TextDecoder().decode(u8.slice(p, Math.min(p + ml, chunkEnd)));
          p = Math.min(p + ml, chunkEnd);
        } else {
          p = Math.min(p + ml, chunkEnd);
        }
        if (mt === 0x2f) break;
      } else if (st === 0xf0 || st === 0xf7) {
        p = Math.min(p + rdVL(), chunkEnd);
      } else {
        p++;
      }
    }

    p = chunkEnd;
    tracks.push(evs);
  }

  return { format, ppq, tracks };
}

// ── Construction du schedule ──────────────────────────────────────────────────

function buildSchedule(midi, nInterps) {
  let tempo = 500000;
  for (const ev of (midi.tracks[0] || [])) {
    if (ev.type === 'tempo') { tempo = ev.tempo; break; }
  }
  const tickSec = tick => (tick * tempo) / (midi.ppq * 1e6);

  const events = [];
  for (let t = 1; t < midi.tracks.length; t++) {
    const interpIdx = t - 1;
    if (interpIdx >= nInterps) continue;
    for (const ev of midi.tracks[t]) {
      if (ev.type === 'on') {
        events.push({ sec: tickSec(ev.tick), cmd: 'play', interpIdx, key: ev.note, velocity: ev.vel });
      } else if (ev.type === 'off') {
        events.push({ sec: tickSec(ev.tick), cmd: 'stop', interpIdx, key: ev.note });
      } else if (ev.type === 'pc') {
        events.push({ sec: tickSec(ev.tick), cmd: 'bank_switch', interpIdx });
      }
    }
  }
  events.sort((a, b) => a.sec - b.sec);
  return events;
}

// ── Gestion des banks ─────────────────────────────────────────────────────────

function loadBankIntoSlot(interpIdx, bankIdx, slot) {
  const state = interpBanks[interpIdx];
  if (!state || bankIdx >= state.banks.length) return;
  const bankData = state.banks[bankIdx];
  const vol = interpMuted[interpIdx] ? 0 : (interpVolumes[interpIdx] ?? 1);
  const ids = new Set();
  for (const k of bankData.keys ?? []) {
    const id = mkId(slot, interpIdx, k.key);
    window.api.sendAudio({
      cmd:      'update',
      id,
      file:     k.file,
      gain:     k.gain     ?? 1,
      fadeType: k.fadeType ?? 'l',
      fadeIn:   k.fadeIn   ?? 0.05,
      fadeOut:  k.fadeOut  ?? 0.1,
      oneShot:  k.oneShot  ?? false,
    });
    if (vol !== 1) window.api.sendAudio({ cmd: 'set_gain', id, gain: vol });
    ids.add(id);
  }
  state.loadedIds[slot] = ids;
}

function switchInterpBank(interpIdx) {
  const state = interpBanks[interpIdx];
  if (!state) return;
  const nextBankIdx = state.bankIdx + 1;
  if (nextBankIdx >= state.banks.length) return;

  const prevSlot = state.activeSlot;
  const nextSlot = prevSlot === 'a' ? 'b' : 'a';

  state.bankIdx    = nextBankIdx;
  state.activeSlot = nextSlot;

  // Reconstruire la keyMap pour la nouvelle bank
  state.activeKeyMap.clear();
  for (const k of state.banks[nextBankIdx].keys ?? []) {
    state.activeKeyMap.set(k.key, mkId(nextSlot, interpIdx, k.key));
  }

  // Appliquer le volume courant au nouveau slot
  const vol = interpMuted[interpIdx] ? 0 : (interpVolumes[interpIdx] ?? 1);
  if (vol !== 1) {
    for (const id of state.loadedIds[nextSlot] ?? []) {
      window.api.sendAudio({ cmd: 'set_gain', id, gain: vol });
    }
  }

  // Libérer l'ancien slot
  for (const id of state.loadedIds[prevSlot] ?? []) {
    window.api.sendAudio({ cmd: 'remove', id });
  }
  state.loadedIds[prevSlot] = new Set();

  // Précharger la bank suivante dans le slot libéré
  const futureBankIdx = nextBankIdx + 1;
  if (futureBankIdx < state.banks.length) {
    loadBankIntoSlot(interpIdx, futureBankIdx, prevSlot);
  }
}

// ── Chargement de la partition ────────────────────────────────────────────────

async function openPartition() {
  const folder = await window.api.openFolder();
  if (!folder) return;
  await loadPartition(folder);
}

async function loadPartition(folder) {
  stopPlayback(true);
  schedule      = [];
  totalDuration = 0;
  interpBanks   = [];
  interpVolumes = [];
  interpMuted   = [];
  interpDots    = {};
  loadPending     = 0;
  loadResolved    = 0;
  loadingComplete = false;

  const list = document.getElementById('trackList');
  list.innerHTML = '';
  setMidiStatus('MIDI: —');
  setStatus('Chargement…');

  const entries = await window.api.listFolder(folder);
  const midEntry = entries.find(e => e.name.endsWith('.mid'));
  if (!midEntry) { setStatus('Aucun fichier .mid trouvé', 'err'); return; }

  // Scan *_bank<N>.json
  const bankPattern = /^(.+)_bank(\d+)\.json$/;
  const banksByName = new Map();
  for (const e of entries) {
    const m = e.name.match(bankPattern);
    if (!m) continue;
    const name = m[1];
    const num  = parseInt(m[2]);
    if (!banksByName.has(name)) banksByName.set(name, []);
    banksByName.get(name).push({ num, path: e.full });
  }

  // Compatibilité ancienne structure *_interp<N>.json → banque unique
  if (!banksByName.size) {
    const oldPattern = /^(.+_interp\d+)\.json$/;
    for (const e of entries) {
      const m = e.name.match(oldPattern);
      if (!m) continue;
      banksByName.set(m[1], [{ num: 1, path: e.full }]);
    }
  }

  if (!banksByName.size) { setStatus('Aucun fichier *_bank*.json trouvé', 'err'); return; }

  // Trier les interprètes par nom (ordre numérique)
  const sortedNames = [...banksByName.keys()].sort((a, b) =>
    a.localeCompare(b, undefined, { numeric: true }));

  // Lire tous les JSON (toutes les banks) pour connaître nbCanaux, polyphonie et les clés
  let nbCanaux = 2, polyphonie = 0;
  const allBankData = [];

  for (const name of sortedNames) {
    const banks = banksByName.get(name).sort((a, b) => a.num - b.num);
    const bankDataArr = [];
    for (const b of banks) {
      const text = await window.api.readTextFile(b.path);
      if (!text) { bankDataArr.push({ keys: [] }); continue; }
      try {
        const data = JSON.parse(text);
        nbCanaux  = Math.max(nbCanaux,  data.nbCanaux  ?? 2);
        polyphonie = Math.max(polyphonie, data.polyphonie ?? 0);
        bankDataArr.push({ keys: data.keys ?? [] });
      } catch (_) {
        bankDataArr.push({ keys: [] });
      }
    }
    allBankData.push(bankDataArr);
  }

  // Parser le MIDI
  const midBuf = await window.api.readBinaryFile(midEntry.full);
  if (!midBuf) { setStatus('Erreur lecture fichier MIDI', 'err'); return; }
  let midi;
  try { midi = parseMidi(midBuf); }
  catch (e) { setStatus(`MIDI invalide : ${e.message}`, 'err'); return; }

  // Redémarrer le serveur audio
  audioReady = false;
  window.api.restartAudio(nbCanaux);
  setStatus(`audio: démarrage (${nbCanaux} canaux)…`);
  await waitAudioReady();
  if (polyphonie > 0) window.api.sendAudio({ cmd: 'set_polyphonie', value: polyphonie });

  // Initialiser l'état par interprète
  for (let i = 0; i < sortedNames.length; i++) {
    const name       = sortedNames[i];
    const bankDataArr = allBankData[i];

    interpVolumes[i] = 1;
    interpMuted[i]   = false;

    const state = {
      name,
      banks:        bankDataArr,
      bankIdx:      0,
      activeSlot:   'a',
      loadedIds:    { a: new Set(), b: new Set() },
      activeKeyMap: new Map(),
    };
    interpBanks.push(state);

    // Bank 0 → slot A
    loadBankIntoSlot(i, 0, 'a');
    loadPending += bankDataArr[0]?.keys.length ?? 0;

    // Bank 1 → slot B (préchargement)
    if (bankDataArr.length > 1) {
      loadBankIntoSlot(i, 1, 'b');
      loadPending += bankDataArr[1]?.keys.length ?? 0;
    }

    // keyMap initiale = bank 0 slot A
    for (const k of bankDataArr[0]?.keys ?? []) {
      state.activeKeyMap.set(k.key, mkId('a', i, k.key));
    }

    const midiTrackName = midi.tracks[i + 1]?.trackName ?? '';
    addTrackRow(i, name, midiTrackName, bankDataArr[0]?.keys.length ?? 0, bankDataArr.length, nbCanaux);
  }

  schedule = buildSchedule(midi, sortedNames.length);
  if (!schedule.length) { setStatus('Aucun événement MIDI à jouer', 'err'); return; }

  const playEvents = schedule.filter(e => e.cmd !== 'bank_switch');
  totalDuration = (playEvents.at(-1)?.sec ?? 0) + 1;
  updateProgress(0);
  setMidiStatus(`MIDI: ${midi.tracks.length - 1} piste(s), ${schedule.length} év.`);

  if (loadPending > 0) {
    setStatus(`Chargement des sons… (0 / ${loadPending})`);
  } else {
    loadingComplete = true;
    enableTransport(true);
    setStatus('Prêt', 'ok');
  }
}

// ── UI interprètes ────────────────────────────────────────────────────────────

function addTrackRow(idx, jsonName, midiName, nKeys, nBanks, nbCanaux) {
  const li  = document.createElement('li');
  li.className = 'track-item';

  const dot = document.createElement('span');
  dot.className = 'track-dot loading';
  dot.dataset.interp = String(idx);
  li.appendChild(dot);
  interpDots[idx] = dot;

  const nameWrap = document.createElement('span');
  nameWrap.className = 'track-name-wrap';

  const nm = document.createElement('span');
  nm.className = 'track-name';
  nm.textContent = midiName || jsonName;
  nameWrap.appendChild(nm);

  if (midiName && midiName !== jsonName) {
    const sub = document.createElement('span');
    sub.className = 'track-sub';
    sub.textContent = jsonName;
    nameWrap.appendChild(sub);
  }
  li.appendChild(nameWrap);

  const muteLabel = document.createElement('label');
  muteLabel.className = 'mute-label';
  const muteBox = document.createElement('input');
  muteBox.type = 'checkbox';
  muteBox.className = 'mute-check';
  muteBox.title = 'Mute';
  muteBox.addEventListener('change', () => {
    setInterpMute(idx, muteBox.checked);
    li.classList.toggle('muted', muteBox.checked);
  });
  muteLabel.appendChild(muteBox);
  muteLabel.append(' Mute');
  li.appendChild(muteLabel);

  const ch = document.createElement('span');
  ch.className = 'track-ch';
  ch.textContent = `${nbCanaux} ch`;
  li.appendChild(ch);

  const info = document.createElement('span');
  info.className = 'track-info';
  info.textContent = nBanks > 1
    ? `${nKeys} son(s) · ${nBanks} banks`
    : `${nKeys} son(s)`;
  li.appendChild(info);

  // Knob volume
  const volWrap = document.createElement('div');
  volWrap.className = 'vol-knob-wrap';
  const NS = 'http://www.w3.org/2000/svg';
  const svg = document.createElementNS(NS, 'svg');
  svg.setAttribute('class', 'vol-knob');
  svg.setAttribute('width', '28');
  svg.setAttribute('height', '28');
  svg.setAttribute('viewBox', '0 0 28 28');
  svg.setAttribute('title', 'Volume (double-clic = 1.0)');
  svg.style.transform = `rotate(${volToDeg(1)}deg)`;
  const circle = document.createElementNS(NS, 'circle');
  circle.setAttribute('cx', '14'); circle.setAttribute('cy', '14'); circle.setAttribute('r', '12');
  circle.setAttribute('fill', '#2a2a3e'); circle.setAttribute('stroke', '#44475a'); circle.setAttribute('stroke-width', '2');
  const line = document.createElementNS(NS, 'line');
  line.setAttribute('x1', '14'); line.setAttribute('y1', '4');
  line.setAttribute('x2', '14'); line.setAttribute('y2', '11');
  line.setAttribute('stroke', '#7aa2f7'); line.setAttribute('stroke-width', '2.5'); line.setAttribute('stroke-linecap', 'round');
  svg.appendChild(circle); svg.appendChild(line);
  const volVal = document.createElement('span');
  volVal.className = 'vol-val';
  volVal.textContent = '1.0';
  volWrap.appendChild(svg); volWrap.appendChild(volVal);
  li.appendChild(volWrap);
  initVolKnob(volWrap, idx);

  document.getElementById('trackList').appendChild(li);
}

// ── Transport ─────────────────────────────────────────────────────────────────

function play() {
  if (!schedule.length || isPlaying) return;
  isPlaying = true;
  isPaused  = false;
  playT0    = performance.now();

  document.getElementById('btnPlay').disabled  = true;
  document.getElementById('btnPause').disabled = false;
  document.getElementById('btnStop').disabled  = false;

  for (const ev of schedule) {
    if (ev.sec < playOffset - 0.05) continue;
    const delay = Math.max(0, (ev.sec - playOffset) * 1000);
    playTimeouts.push(setTimeout(() => {
      if (ev.cmd === 'bank_switch') {
        switchInterpBank(ev.interpIdx);
        return;
      }
      const state = interpBanks[ev.interpIdx];
      if (!state || interpMuted[ev.interpIdx]) return;
      const id = state.activeKeyMap.get(ev.key);
      if (!id) return;
      const msg = { cmd: ev.cmd, id };
      if (ev.velocity != null) msg.velocity = ev.velocity;
      window.api.sendAudio(msg);
    }, delay));
  }

  playInterval = setInterval(() => {
    const elapsed = playOffset + (performance.now() - playT0) / 1000;
    updateProgress(elapsed);
    if (elapsed >= totalDuration) stopPlayback(true);
  }, 80);
}

function pause() {
  if (!isPlaying || isPaused) return;
  isPaused   = true;
  isPlaying  = false;
  playOffset = playOffset + (performance.now() - playT0) / 1000;
  clearAllTimers();
  stopAllTracks();

  document.getElementById('btnPlay').disabled  = false;
  document.getElementById('btnPause').disabled = true;
}

function stopPlayback(rewind = true) {
  isPlaying = false;
  isPaused  = false;
  if (rewind) playOffset = 0;
  clearAllTimers();
  stopAllTracks();
  if (rewind) updateProgress(0);

  const hasSchedule = schedule.length > 0;
  document.getElementById('btnPlay').disabled  = !hasSchedule;
  document.getElementById('btnPause').disabled = true;
  document.getElementById('btnStop').disabled  = true;
}

function clearAllTimers() {
  playTimeouts.forEach(clearTimeout);
  playTimeouts = [];
  if (playInterval) { clearInterval(playInterval); playInterval = null; }
}

function stopAllTracks() {
  for (const state of interpBanks) {
    if (!state) continue;
    for (const id of state.loadedIds[state.activeSlot] ?? []) {
      window.api.sendAudio({ cmd: 'stop', id });
    }
  }
}

// ── Progression ───────────────────────────────────────────────────────────────

function updateProgress(elapsed) {
  const pct = totalDuration > 0 ? Math.min(1, elapsed / totalDuration) : 0;
  document.getElementById('progressFill').style.width = (pct * 100).toFixed(2) + '%';
  document.getElementById('timeDisplay').textContent  = `${fmt(elapsed)} / ${fmt(totalDuration)}`;
}

function fmt(s) {
  const m = Math.floor(s / 60), sec = Math.floor(s % 60);
  return `${m}:${String(sec).padStart(2, '0')}`;
}

// ── Seek ──────────────────────────────────────────────────────────────────────

document.getElementById('progressBar').addEventListener('click', e => {
  if (!schedule.length) return;
  const rect = e.currentTarget.getBoundingClientRect();
  playOffset = Math.max(0, Math.min(totalDuration, (e.clientX - rect.left) / rect.width * totalDuration));
  updateProgress(playOffset);
  if (isPlaying) {
    clearAllTimers();
    stopAllTracks();
    isPlaying = false;
    play();
  }
});

// ── Helpers UI ────────────────────────────────────────────────────────────────

function enableTransport(on) {
  document.getElementById('btnPlay').disabled  = !on;
  document.getElementById('btnPause').disabled = true;
  document.getElementById('btnStop').disabled  = true;
}

// ── Bindings ──────────────────────────────────────────────────────────────────

document.getElementById('btnOpen').addEventListener('click',  openPartition);
document.getElementById('btnPlay').addEventListener('click',  play);
document.getElementById('btnPause').addEventListener('click', pause);
document.getElementById('btnStop').addEventListener('click',  () => stopPlayback(true));

enableTransport(false);
initThemes();
