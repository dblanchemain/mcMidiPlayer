// mcMidiPlayer — main process
// License: GPL-3.0-or-later — D.Blanchemain

const { app, BrowserWindow, ipcMain, dialog } = require('electron');
const path = require('path');
const { spawn } = require('child_process');
const fs = require('fs');

let mainWindow;
let audioProcess = null;

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 700,
    height: 520,
    minWidth: 560,
    minHeight: 400,
    title: 'mcMidiPlayer',
    backgroundColor: '#1e1e2e',
    autoHideMenuBar: true,
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      contextIsolation: true,
      nodeIntegration: false,
    },
  });
  mainWindow.loadFile('index.html');
  mainWindow.on('closed', () => { stopAudioServer(); mainWindow = null; });
}

// ── Audio server ─────────────────────────────────────────────────────────────

function startAudioServer(maxPorts = 16) {
  if (audioProcess) return;
  const base      = app.isPackaged ? process.resourcesPath : __dirname;
  const pythonDir = path.join(base, 'python');

  let spawnCmd, spawnArgs;
  if (app.isPackaged && process.platform !== 'linux') {
    const exe = process.platform === 'win32' ? 'audio_server.exe' : 'audio_server';
    spawnCmd  = path.join(pythonDir, exe);
    spawnArgs = ['--max-ports', String(maxPorts)];
  } else {
    spawnCmd  = 'python3';
    spawnArgs = [path.join(pythonDir, 'audio_server.py'), '--max-ports', String(maxPorts)];
  }

  const proc = spawn(spawnCmd, spawnArgs, { stdio: ['pipe', 'pipe', 'pipe'] });
  audioProcess = proc;

  proc.stdout.on('data', data => {
    for (const line of data.toString().split('\n').filter(Boolean)) {
      try {
        const msg = JSON.parse(line);
        if (mainWindow) mainWindow.webContents.send('audio-event', msg);
      } catch (_) {}
    }
  });

  proc.stderr.on('data', d => {
    const txt = d.toString().trim();
    console.error('[audio]', txt);
    if (mainWindow) mainWindow.webContents.send('audio-event',
      { type: 'error', message: txt.split('\n').pop() });
  });

  proc.on('exit', code => {
    console.log('[audio] exit', code);
    if (audioProcess === proc) audioProcess = null;
  });
}

function stopAudioServer() {
  if (audioProcess) {
    sendToAudio({ cmd: 'quit' });
    audioProcess.kill();
    audioProcess = null;
  }
}

function sendToAudio(msg) {
  if (audioProcess && audioProcess.stdin.writable)
    audioProcess.stdin.write(JSON.stringify(msg) + '\n');
}

// ── IPC ──────────────────────────────────────────────────────────────────────

ipcMain.handle('open-folder-dialog', () =>
  dialog.showOpenDialog(mainWindow, {
    properties: ['openDirectory'],
    title: 'Ouvrir un dossier partition_midi',
  }).then(r => r.canceled ? null : r.filePaths[0])
);

ipcMain.handle('list-folder', (_, folderPath) => {
  try {
    return fs.readdirSync(folderPath)
      .map(name => ({ name, full: path.join(folderPath, name) }));
  } catch (_) { return []; }
});

ipcMain.handle('read-text-file', (_, filePath) => {
  try { return fs.readFileSync(filePath, 'utf8'); }
  catch (_) { return null; }
});

ipcMain.handle('read-binary-file', (_, filePath) => {
  try { return fs.readFileSync(filePath); }
  catch (_) { return null; }
});

ipcMain.on('audio-cmd', (_e, msg) => sendToAudio(msg));

ipcMain.on('restart-audio-server', (_e, maxPorts) => {
  stopAudioServer();
  startAudioServer(maxPorts || 16);
});

// ── Lifecycle ────────────────────────────────────────────────────────────────

app.whenReady().then(() => { createWindow(); startAudioServer(); });

app.on('window-all-closed', () => {
  stopAudioServer();
  if (process.platform !== 'darwin') app.quit();
});

app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0) createWindow();
});
