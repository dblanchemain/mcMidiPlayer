# mcMidiPlayer

Lecteur audio multicanal piloté par un fichier MIDI : chaque piste MIDI déclenche les échantillons d'un « interprète », avec bascule de banques de sons en cours de lecture (Program Change ou marker texte `bank2`, `bank3`…).

## Installation

### 1. Node.js et dépendances JS

```bash
npm install
```

### 2. Python (serveur audio)

Le serveur audio (`python/audio_server.py`) tourne via JACK sous Linux et via sounddevice (PortAudio) sous Windows/macOS, avec resampling automatique (scipy) si le fichier ne correspond pas au taux d'échantillonnage de la sortie.

**Linux**

```bash
sudo apt install jackd2 libportaudio2          # JACK + fallback PortAudio
pip3 install jack sounddevice soundfile numpy scipy
# Greffons FX Faust définis via mcMidiKeyboard (optionnel — sans ce module, les FX sont ignorés) :
pip3 install dawdreamer
```

JACK est utilisé en priorité ; si le serveur JACK n'est pas démarré (ou `jack-client` absent), bascule automatique sur sounddevice.

**Windows**

```bash
pip3 install sounddevice soundfile numpy scipy
pip3 install dawdreamer   # optionnel — greffons FX Faust
```

**macOS**

```bash
brew install portaudio
pip3 install sounddevice soundfile numpy scipy
pip3 install dawdreamer   # optionnel — greffons FX Faust
```

> Ces dépendances Python ne sont nécessaires que pour lancer depuis les sources (`npm start`) ou sur un paquet Linux (.deb/.rpm/.zip), où `audio_server.py` est exécuté via `python3` du système. Sur les paquets Windows/macOS publiés en release, le serveur audio est figé en exécutable autonome (PyInstaller) et n'a besoin d'aucun Python installé.

## Démarrage

```bash
npm start
```

## Build / paquets distribuables

```bash
npm run make
```

Sur Windows et macOS, `audio_server.py` doit être figé en exécutable avant le build packagé :

```bash
pip install pyinstaller sounddevice soundfile numpy scipy dawdreamer
pyinstaller --onefile --clean --noconfirm \
  --exclude-module jack \
  --collect-submodules scipy.signal \
  --hidden-import sounddevice \
  --collect-all dawdreamer \
  --distpath python \
  python/audio_server.py
```

La CI GitHub Actions (`.github/workflows/build.yml`) effectue cette étape et publie automatiquement les binaires Linux/macOS/Windows à chaque push sur `master`.

## Utilisation

Préparer un dossier `partition_midi` contenant :

- un fichier `.mid` — la piste 0 porte le tempo et les markers globaux, chaque piste suivante correspond à un interprète ;
- pour chaque interprète, un ou plusieurs fichiers `<nom>_bank<N>.json` décrivant la/les banque(s) de sons (les interprètes sont ordonnés par tri numérique de `<nom>`).

Puis dans l'application : **Ouvrir dossier** → **▶ Lire**.

### Format `<nom>_bank<N>.json`

```json
{
  "nbCanaux": 2,
  "polyphonie": 8,
  "keys": [
    {
      "key": 60,
      "file": "/chemin/vers/fichier.wav",
      "gain": 1,
      "fadeType": "l",
      "fadeIn": 0.05,
      "fadeOut": 0.1,
      "oneShot": false,
      "fx": [
        {
          "dsp": "delay.dsp",
          "params": {
            "/DELAY/0x00/delay":    { "value": 200 },
            "/DELAY/0x00/feedback": { "value": 50, "automate": true, "points": [{ "t": 0, "v": 50 }, { "t": 1, "v": 10 }] }
          }
        }
      ]
    }
  ]
}
```

- `key` : numéro de note MIDI (0-127)
- `fadeType` : `q` (qt sinusoïde), `h` (demi-sinus), `t` (linéaire), `l` (logarithmique), `p` (parabolique inversée)
- `nbCanaux` / `polyphonie` : la valeur la plus haute trouvée parmi toutes les banques de tous les interprètes est appliquée globalement
- `fx` (optionnel) : chaîne de greffons Faust telle que définie par **mcMidiKeyboard** (bouton FX de son interface — voir son README pour l'éditer). mcMidiPlayer se contente de la lire et de l'appliquer à la lecture, sans interface d'édition. Chaque greffon référence un fichier `.dsp` du dossier `python/faust_fx/` et ses paramètres, identifiés par leur chemin Faust complet. Pour chaque paramètre, `value` est la valeur statique appliquée quand `automate` est faux ou absent ; avec `automate: true`, le paramètre suit une courbe définie par `points` (breakpoints `{t, v}`, `t` ∈ [0,1] = position relative dans la durée de l'échantillon), interpolée linéairement. La valeur réellement appliquée est toujours proportionnelle à la vélocité de la note (`× vélocité/127`). Le rendu a lieu au chargement du son (pas en temps réel) et nécessite `dawdreamer` ; sans ce module, le champ `fx` est ignoré et le son joue sans traitement.

### Bascule de banque

Pendant la lecture, un marker texte `bank2`, `bank3`… (piste globale ou piste de l'interprète) ou un Program Change passe l'interprète concerné à la banque correspondante, sans coupure des notes en cours.

## Architecture

- `main.js` — processus principal Electron (spawn du serveur audio, dialogues fichiers/dossiers via IPC)
- `renderer.js` — interface, parseur MIDI, ordonnancement du schedule, gestion des banques et du thème
- `python/audio_server.py` — serveur audio (JACK sur Linux, sounddevice ailleurs), protocole JSON ligne par ligne sur stdin/stdout, rendu des greffons FX Faust (`dawdreamer`) si un son en définit
- `python/faust_fx/` — greffons Faust (`.dsp`) référencés par le champ `fx` des banques, partagés avec mcMidiKeyboard
- `themes.js` — thèmes d'interface (popup Préférences)

## License

GPL-3.0-or-later
