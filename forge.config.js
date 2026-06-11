module.exports = {
  packagerConfig: {
    asar: true,
    extraResource: ['python', 'resources'],
    name: 'mcMidiPlayer',
    executableName: 'mc-midi-player',
    icon: 'resources/icon',
  },
  rebuildConfig: {},
  makers: [
    { name: '@electron-forge/maker-squirrel', config: {} },
    { name: '@electron-forge/maker-zip', platforms: ['darwin', 'linux', 'win32'] },
    { name: '@electron-forge/maker-deb', config: {} },
    { name: '@electron-forge/maker-rpm', config: {} },
  ],
  plugins: [
    { name: '@electron-forge/plugin-auto-unpack-natives', config: {} },
  ],
};
