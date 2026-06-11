// mcMidiPlayer — renderer
// License: GPL-3.0-or-later — D.Blanchemain

// ── État ──────────────────────────────────────────────────────────────────────

let schedule      = [];   // [{ sec, cmd, id, velocity? }]
let totalDuration = 0;
let playTimeouts  = [];   // IDs setTimeout pour les événements MIDI
let playInterval  = null; // ID setInterval pour la progression
let playOffset    = 0;    // position de départ (secondes)
let playT0        = 0;    // performance.now() au moment du Play
let isPlaying     = false;
let isPaused      = false;

let audioReady        = false;
let audioReadyCb      = null;
let trackDots         = {};   // id → élément .track-dot
let activeTrackIds    = new Set();
let interpVolumes     = [];   // volume par interprète (0..2, défaut 1)
let interpKeyData     = [];   // [{id, origGain}] par interprète
let loadPending       = 0;    // fichiers WAV en cours de chargement
let loadResolved      = 0;    // fichiers WAV chargés ou en erreur

// ── Volume knob ───────────────────────────────────────────────────────────────

const VOL_MIN = 0;
const VOL_MAX = 2;

function volToDeg(vol) {
  return (vol - 1) * 135;
}

function setInterpVolume(interpIdx, vol) {
  interpVolumes[interpIdx] = vol;
  for (const { id } of interpKeyData[interpIdx] ?? []) {
    window.api.sendAudio({ cmd: 'set_gain', id, gain: vol });
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
  } else if (msg.type === 'loaded') {
    const dot = trackDots[msg.id];
    if (dot) { dot.className = 'track-dot ready'; }
    loadResolved++;
    checkAllLoaded();
  } else if (msg.type === 'load_error') {
    const dot = trackDots[msg.id];
    if (dot) { dot.className = 'track-dot error'; dot.title = msg.message; }
    loadResolved++;
    checkAllLoaded();
  }
});

function checkAllLoaded() {
  if (loadPending === 0) return;
  if (loadResolved >= loadPending) {
    enableTransport(true);
    setStatus(`Prêt`, 'ok');
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
  // Normalise le buffer reçu (Buffer node, Uint8Array, ArrayBuffer, ou {type:'Buffer',data:[]})
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
  if (hdrLen > 6) p += hdrLen - 6; // sauter les octets d'entête supplémentaires

  const tracks = [];

  // Parcourir tous les chunks présents (ignore les chunks non-MTrk)
  while (p + 8 <= total) {
    const magic    = rd32();
    const chunkLen = rd32();
    const chunkEnd = Math.min(p + chunkLen, total);

    if (magic !== 0x4d54726b) { // chunk inconnu : sauter
      p = chunkEnd;
      continue;
    }

    const evs = [];
    let tick = 0, rs = 0;

    while (p < chunkEnd) {
      tick += rdVL();
      if (p >= chunkEnd) break;

      let st = u8[p];
      if (st & 0x80) {
        p++;
        if (st < 0xf0) rs = st; // running status uniquement pour les messages voix
      } else {
        st = rs; // running status : pas d'incrément de p
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
      } else if (type === 0xc0 || type === 0xd0) {
        safe8();
      } else if (st === 0xff) {
        const mt = safe8(), ml = rdVL();
        if (mt === 0x51 && ml === 3) {
          evs.push({ tick, type: 'tempo', tempo: (safe8() << 16) | (safe8() << 8) | safe8() });
        } else if (mt === 0x03) { // track name
          evs.trackName = new TextDecoder().decode(u8.slice(p, Math.min(p + ml, chunkEnd)));
          p = Math.min(p + ml, chunkEnd);
        } else {
          p = Math.min(p + ml, chunkEnd);
        }
        if (mt === 0x2f) break; // end-of-track
      } else if (st === 0xf0 || st === 0xf7) {
        p = Math.min(p + rdVL(), chunkEnd);
      } else {
        p++; // octet inconnu : avancer
      }
    }

    p = chunkEnd;
    tracks.push(evs);
  }

  return { format, ppq, tracks };
}

// ── Construction du schedule de lecture ──────────────────────────────────────

function buildSchedule(midi, interpMaps) {
  let tempo = 500000;
  for (const ev of (midi.tracks[0] || [])) {
    if (ev.type === 'tempo') { tempo = ev.tempo; break; }
  }
  const tickSec = tick => (tick * tempo) / (midi.ppq * 1e6);

  const events = [];
  for (let t = 1; t < midi.tracks.length; t++) {
    const keyMap = interpMaps[t - 1];
    if (!keyMap) continue;
    for (const ev of midi.tracks[t]) {
      if (ev.type !== 'on' && ev.type !== 'off') continue;
      const id = keyMap.get(ev.note);
      if (!id) continue;
      const entry = { sec: tickSec(ev.tick), cmd: ev.type === 'on' ? 'play' : 'stop', id };
      if (ev.type === 'on') entry.velocity = ev.vel;
      events.push(entry);
    }
  }
  events.sort((a, b) => a.sec - b.sec);
  return events;
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
  trackDots     = {};
  activeTrackIds.clear();
  interpVolumes = [];
  interpKeyData = [];
  loadPending   = 0;
  loadResolved  = 0;

  const list = document.getElementById('trackList');
  list.innerHTML = '';
  setMidiStatus('MIDI: —');
  setStatus('Chargement…');

  const entries = await window.api.listFolder(folder);

  const jsonEntries = entries
    .filter(e => e.name.match(/_interp\d+\.json$/))
    .sort((a, b) => a.name.localeCompare(b.name, undefined, { numeric: true }));
  const midEntry = entries.find(e => e.name.endsWith('.mid'));

  if (!jsonEntries.length) { setStatus('Aucun fichier *_interp*.json trouvé', 'err'); return; }
  if (!midEntry)           { setStatus('Aucun fichier .mid trouvé', 'err'); return; }

  // Lire les JSON
  const interps = [];
  let nbCanaux = 2;
  for (const je of jsonEntries) {
    const text = await window.api.readTextFile(je.full);
    if (!text) { setStatus(`Erreur lecture ${je.name}`, 'err'); return; }
    try {
      const data = JSON.parse(text);
      nbCanaux = Math.max(nbCanaux, data.nbCanaux ?? 2);
      interps.push({ name: je.name.replace('.json', ''), keys: data.keys || [] });
    } catch(e) {
      setStatus(`JSON invalide : ${je.name}`, 'err'); return;
    }
  }

  // Parser le MIDI d'abord pour récupérer les noms de pistes
  const midBuf = await window.api.readBinaryFile(midEntry.full);
  if (!midBuf) { setStatus('Erreur lecture fichier MIDI', 'err'); return; }

  let midi;
  try {
    midi = parseMidi(midBuf);
  } catch(e) {
    setStatus(`MIDI invalide : ${e.message}`, 'err'); return;
  }

  // Redémarrer le serveur audio avec le bon nombre de canaux
  audioReady = false;
  window.api.restartAudio(nbCanaux);
  setStatus(`audio: démarrage (${nbCanaux} canaux)…`);
  await waitAudioReady();

  // Enregistrer les pistes dans l'audio server + construire les keyMaps
  const interpMaps = [];
  for (let i = 0; i < interps.length; i++) {
    const midiTrackName = midi.tracks[i + 1]?.trackName ?? '';
    const keyMap = new Map();
    interpVolumes[i] = 1;
    interpKeyData[i] = [];
    for (const k of interps[i].keys) {
      const id       = `t${i + 1}_${k.key}`;
      const origGain = k.gain ?? 1;
      window.api.sendAudio({
        cmd:      'update',
        id,
        file:     k.file,
        gain:     origGain,
        fadeType: k.fadeType ?? 'l',
        fadeIn:   k.fadeIn   ?? 0.05,
        fadeOut:  k.fadeOut  ?? 0.1,
        oneShot:  k.oneShot  ?? false,
      });
      loadPending++;
      keyMap.set(k.key, id);
      trackDots[id] = null;
      interpKeyData[i].push({ id, origGain });
    }
    interpMaps.push(keyMap);
    addTrackRow(i, interps[i].name, midiTrackName, interps[i].keys.length, nbCanaux);
    for (const k of interps[i].keys) {
      const id  = `t${i + 1}_${k.key}`;
      const dot = document.querySelector(`.track-dot[data-interp="${i}"]`);
      if (dot) trackDots[id] = dot;
    }
  }

  schedule = buildSchedule(midi, interpMaps);
  if (!schedule.length) { setStatus('Aucun événement MIDI à jouer', 'err'); return; }

  totalDuration = (schedule.at(-1).sec ?? 0) + 1;
  updateProgress(0);
  setMidiStatus(`MIDI: ${midi.tracks.length - 1} piste(s), ${schedule.length} év.`);
  setStatus(`Chargement des sons… (0 / ${loadPending})`);
  // Le transport est activé par checkAllLoaded() quand tous les fichiers sont chargés
}

// ── UI interprètes ────────────────────────────────────────────────────────────

function addTrackRow(idx, jsonName, midiName, nKeys, nbCanaux) {
  const li  = document.createElement('li');
  li.className = 'track-item';

  const dot = document.createElement('span');
  dot.className = 'track-dot loading';
  dot.dataset.interp = String(idx);
  li.appendChild(dot);

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

  const ch = document.createElement('span');
  ch.className = 'track-ch';
  ch.textContent = `${nbCanaux} ch`;
  li.appendChild(ch);

  const info = document.createElement('span');
  info.className = 'track-info';
  info.textContent = `${nKeys} son(s)`;
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
      const msg = { cmd: ev.cmd, id: ev.id };
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
  const sent = new Set();
  for (const ev of schedule) {
    if (!sent.has(ev.id)) {
      window.api.sendAudio({ cmd: 'stop', id: ev.id });
      sent.add(ev.id);
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

// ── Seek via clic sur la barre de progression ─────────────────────────────────

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
