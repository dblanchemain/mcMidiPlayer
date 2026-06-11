// preload.js — pont sécurisé entre renderer et main
const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('api', {
  openFolder:     ()    => ipcRenderer.invoke('open-folder-dialog'),
  listFolder:     (p)   => ipcRenderer.invoke('list-folder', p),
  readTextFile:   (p)   => ipcRenderer.invoke('read-text-file', p),
  readBinaryFile: (p)   => ipcRenderer.invoke('read-binary-file', p),
  sendAudio:      (msg) => ipcRenderer.send('audio-cmd', msg),
  restartAudio:   (n)   => ipcRenderer.send('restart-audio-server', n),
  onAudioEvent:   (cb)  => ipcRenderer.on('audio-event', (_e, msg) => cb(msg)),
});
