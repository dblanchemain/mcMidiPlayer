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

const THEME_KEY = 'mcMidiPlayer.theme';

function applyTheme(name) {
  const theme = THEMES[name] || THEMES['Marine'];
  const root = document.documentElement;
  for (const [prop, val] of Object.entries(theme.vars)) {
    root.style.setProperty(prop, val);
  }
  localStorage.setItem(THEME_KEY, name);
  const sel = document.getElementById('themeSelect');
  if (sel) sel.value = name;
}

function initThemes() {
  const sel = document.getElementById('themeSelect');
  if (!sel) return;

  // Remplir le select
  for (const [key, t] of Object.entries(THEMES)) {
    const opt = document.createElement('option');
    opt.value = key;
    opt.textContent = t.label;
    sel.appendChild(opt);
  }

  sel.addEventListener('change', () => applyTheme(sel.value));

  // Appliquer le thème sauvegardé ou Marine par défaut
  const saved = localStorage.getItem(THEME_KEY) || 'Marine';
  applyTheme(saved);
}
