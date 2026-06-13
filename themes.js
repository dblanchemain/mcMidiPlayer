// themes.js — Définition et application des thèmes mcMidiPlayer
// License: GPL-3.0-or-later — D.Blanchemain

const THEMES = {
  Marine: {
    label: 'Marine',
    vars: {
      '--bg':      '#1e1e2e',
      '--surface': '#2a2a3e',
      '--border':  '#44475a',
      '--text':    '#cdd6f4',
      '--accent':  '#7aa2f7',
      '--red':     '#f38ba8',
      '--green':   '#a6e3a1',
      '--muted':   '#6c7086',
    },
  },
  Sombre: {
    label: 'Sombre',
    vars: {
      '--bg':      '#111111',
      '--surface': '#1e1e1e',
      '--border':  '#333333',
      '--text':    '#d4d4d4',
      '--accent':  '#569cd6',
      '--red':     '#f44747',
      '--green':   '#6a9955',
      '--muted':   '#666666',
    },
  },
  Marbré: {
    label: 'Marbré',
    vars: {
      '--bg':      '#ddd5c5',
      '--surface': '#ece6d8',
      '--border':  '#b8b0a0',
      '--text':    '#1a1510',
      '--accent':  '#c86810',
      '--red':     '#b83020',
      '--green':   '#507838',
      '--muted':   '#786858',
    },
  },
  Clair: {
    label: 'Clair',
    vars: {
      '--bg':      '#f0f0f0',
      '--surface': '#ffffff',
      '--border':  '#cccccc',
      '--text':    '#1e1e1e',
      '--accent':  '#2563eb',
      '--red':     '#dc2626',
      '--green':   '#16a34a',
      '--muted':   '#888888',
    },
  },
  Boisé: {
    label: 'Boisé',
    vars: {
      '--bg':      '#1a1f16',
      '--surface': '#252b1f',
      '--border':  '#3a4a30',
      '--text':    '#c8d8b4',
      '--accent':  '#8fbc5a',
      '--red':     '#e07050',
      '--green':   '#6dbf67',
      '--muted':   '#5a7050',
    },
  },
};

const _THEME_STATE_KEY = 'mcMidiPlayer.themeState';
const _THEMES_PERSO_KEY = 'mcMidiPlayer.themes.perso';
const _THEME_KEY_LEGACY = 'mcMidiPlayer.theme';

let _themeCourant = null;

function _themeAppliquer(vars) {
  const root = document.documentElement;
  for (const [k, v] of Object.entries(vars)) {
    if (k.startsWith('--')) root.style.setProperty(k, v);
  }
}

function applyTheme(name) {
  const theme = THEMES[name] || THEMES['Marine'];
  _themeCourant = Object.assign({}, theme.vars, { _preset: name });
  _themeAppliquer(_themeCourant);
  localStorage.setItem(_THEME_STATE_KEY, JSON.stringify(_themeCourant));
}

function initThemes() {
  try {
    const raw = localStorage.getItem(_THEME_STATE_KEY);
    if (raw) {
      _themeCourant = JSON.parse(raw);
      _themeAppliquer(_themeCourant);
      return;
    }
    const oldName = localStorage.getItem(_THEME_KEY_LEGACY);
    if (oldName && THEMES[oldName]) {
      applyTheme(oldName);
      return;
    }
  } catch (_) {}
  applyTheme('Marine');
}

// ── Préférences : Thème ───────────────────────────────────────────────────────

function prefThemeRemplirUI() {
  const t = _themeCourant || Object.assign({}, THEMES['Marine'].vars, { _preset: 'Marine' });
  const get = (id, fallback) => { const el = document.getElementById(id); if (el) el.value = t[fallback] || ''; };
  get('prefTcBg',      '--bg');
  get('prefTcSurface', '--surface');
  get('prefTcBorder',  '--border');
  get('prefTcText',    '--text');
  get('prefTcAccent',  '--accent');
  get('prefTcRed',     '--red');
  get('prefTcGreen',   '--green');
  get('prefTcMuted',   '--muted');
  const preset = document.getElementById('prefThemePreset');
  if (preset) {
    const name = t._preset || 'perso';
    for (const opt of preset.options) opt.selected = (opt.value === name);
  }
  prefThemeRemplirSelectPerso();
}

function prefThemePresetChange(val) {
  if (val === 'perso') return;
  const th = THEMES[val];
  if (!th) return;
  _themeCourant = Object.assign({}, th.vars, { _preset: val });
  _themeAppliquer(_themeCourant);
  localStorage.setItem(_THEME_STATE_KEY, JSON.stringify(_themeCourant));
  prefThemeRemplirUI();
}

function prefThemeCustomChange() {
  const v = (id) => { const el = document.getElementById(id); return el ? el.value : ''; };
  _themeCourant = {
    _preset:    'perso',
    '--bg':      v('prefTcBg'),
    '--surface': v('prefTcSurface'),
    '--border':  v('prefTcBorder'),
    '--text':    v('prefTcText'),
    '--accent':  v('prefTcAccent'),
    '--red':     v('prefTcRed'),
    '--green':   v('prefTcGreen'),
    '--muted':   v('prefTcMuted'),
  };
  const preset = document.getElementById('prefThemePreset');
  if (preset) preset.value = 'perso';
  _themeAppliquer(_themeCourant);
  localStorage.setItem(_THEME_STATE_KEY, JSON.stringify(_themeCourant));
}

function prefThemeReset() {
  applyTheme('Marine');
  prefThemeRemplirUI();
}

function prefThemesPersoListe() {
  try { return JSON.parse(localStorage.getItem(_THEMES_PERSO_KEY) || '[]'); }
  catch { return []; }
}

function prefThemeRemplirSelectPerso() {
  const sel = document.getElementById('prefThemesSauvegardes');
  if (!sel) return;
  const liste = prefThemesPersoListe();
  sel.innerHTML = '<option value="">— choisir —</option>' +
    liste.map(t => `<option value="${t._nom}">${t._nom}</option>`).join('');
}

function prefThemeSauvegarderPerso() {
  const nomEl = document.getElementById('prefThemeSaveName');
  const nom = nomEl ? nomEl.value.trim() : '';
  if (!nom) { alert('Saisissez un nom pour ce thème.'); return; }
  const liste = prefThemesPersoListe();
  const entry = Object.assign({}, _themeCourant || {}, { _nom: nom, _preset: 'perso' });
  const idx = liste.findIndex(t => t._nom === nom);
  if (idx >= 0) liste[idx] = entry; else liste.push(entry);
  localStorage.setItem(_THEMES_PERSO_KEY, JSON.stringify(liste));
  prefThemeRemplirSelectPerso();
  const sel = document.getElementById('prefThemesSauvegardes');
  if (sel) sel.value = nom;
}

function prefThemeChargerPerso() {
  const sel = document.getElementById('prefThemesSauvegardes');
  if (!sel || !sel.value) return;
  const t = prefThemesPersoListe().find(th => th._nom === sel.value);
  if (!t) return;
  _themeCourant = Object.assign({}, t);
  _themeAppliquer(_themeCourant);
  localStorage.setItem(_THEME_STATE_KEY, JSON.stringify(_themeCourant));
  prefThemeRemplirUI();
}

function prefThemeSupprimerPerso() {
  const sel = document.getElementById('prefThemesSauvegardes');
  if (!sel || !sel.value) return;
  const liste = prefThemesPersoListe().filter(t => t._nom !== sel.value);
  localStorage.setItem(_THEMES_PERSO_KEY, JSON.stringify(liste));
  prefThemeRemplirSelectPerso();
}

function prefThemeExporter() {
  const liste = prefThemesPersoListe();
  if (!liste.length) { alert('Aucun thème personnalisé à exporter.'); return; }
  const blob = new Blob([JSON.stringify(liste, null, 2)], { type: 'application/json' });
  const url  = URL.createObjectURL(blob);
  const a    = document.createElement('a');
  a.href = url; a.download = 'themes.json'; a.click();
  URL.revokeObjectURL(url);
}

function prefThemeImporter() {
  const el = document.getElementById('prefThemeImportFile');
  if (el) { el.value = ''; el.click(); }
}

function prefThemeImporterFichier(input) {
  const file = input.files[0];
  if (!file) return;
  const reader = new FileReader();
  reader.onload = function(e) {
    let importes;
    try { importes = JSON.parse(e.target.result); }
    catch { alert('Fichier JSON invalide.'); return; }
    if (!Array.isArray(importes)) { alert('Format invalide : tableau JSON attendu.'); return; }
    const liste = prefThemesPersoListe();
    let ajouts = 0, remplacements = 0;
    for (const t of importes) {
      if (!t._nom) continue;
      const idx = liste.findIndex(x => x._nom === t._nom);
      if (idx >= 0) { liste[idx] = t; remplacements++; } else { liste.push(t); ajouts++; }
    }
    localStorage.setItem(_THEMES_PERSO_KEY, JSON.stringify(liste));
    prefThemeRemplirSelectPerso();
    alert(`${ajouts} thème(s) ajouté(s), ${remplacements} remplacé(s).`);
  };
  reader.readAsText(file);
}
