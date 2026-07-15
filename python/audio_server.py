#!/usr/bin/env python3
"""
mcMidiPlayer — audio_server.py
Lecture multicanalien polyphonique via JACK (Linux) ou sounddevice (autres OS).
Protocole : JSON line-delimited sur stdin/stdout.
License: GPL-3.0-or-later — D.Blanchemain
"""

import sys
import os
import json
import signal
import threading
import math
import platform
import queue
import argparse
import contextlib
import concurrent.futures
import numpy as np

_parser = argparse.ArgumentParser(add_help=False)
_parser.add_argument('--max-ports', type=int, default=16)
_args, _ = _parser.parse_known_args()
MAX_PORTS_CFG = _args.max_ports

_current_device   = None  # int ou None (None = défaut / JACK)
_current_channels = None  # int ou None (None = auto)

PLATFORM = platform.system()
USE_JACK  = PLATFORM == 'Linux'

try:
    import soundfile as sf
    import sounddevice as sd
except ImportError as e:
    sys.stdout.write(json.dumps({'type': 'error', 'message': str(e)}) + '\n')
    sys.stdout.flush()
    sys.exit(1)

try:
    from math import gcd
    from scipy.signal import resample_poly as _resample_poly
    def _resample(data, from_sr, to_sr):
        if from_sr == to_sr:
            return data
        g = gcd(int(from_sr), int(to_sr))
        return _resample_poly(data, int(to_sr) // g, int(from_sr) // g,
                              axis=0).astype(np.float32)
except ImportError:
    def _resample(data, from_sr, to_sr):
        return data  # pas de scipy : lecture sans resampling

OUTPUT_SR = None  # taux d'échantillonnage de sortie, fixé au démarrage du backend

if USE_JACK:
    try:
        import jack
    except ImportError:
        USE_JACK = False

# ── Greffons FX Faust (traitement au Note On, via dawdreamer) ─────────────────

# En exécutable PyInstaller gelé, __file__ pointe vers le dossier d'extraction
# temporaire (sys._MEIPASS) et non l'emplacement réel de l'exécutable ; le
# catalogue faust_fx/ est copié par electron-forge à côté de l'exécutable
# (extraResource: 'python'), donc on résout depuis sys.executable dans ce cas.
_FX_BASE_DIR = os.path.dirname(sys.executable) if getattr(sys, 'frozen', False) \
    else os.path.dirname(os.path.abspath(__file__))
FX_DIR = os.path.join(_FX_BASE_DIR, 'faust_fx')
FX_VELOCITY_BUCKETS = (16, 32, 48, 64, 80, 96, 112, 127)

FAUST_FX_ERROR = None

# dawdreamer/libfaust (JIT LLVM) doit toujours être utilisé depuis le MÊME
# thread OS — pas seulement "pas en même temps" : un simple Lock ne suffit
# pas, un deuxième thread qui l'utilise après coup (même sans chevauchement
# avec le premier) reste bloqué indéfiniment. Chaque touche lance pourtant
# son propre thread de rendu FX (_start_fx_render) ; on route donc tout
# appel natif vers un unique thread pool à un seul worker, réutilisé pour
# toute la durée de vie du process.
_faust_pool = concurrent.futures.ThreadPoolExecutor(max_workers=1, thread_name_prefix='faust')

try:
    import dawdreamer as _daw
    HAVE_FAUST_FX = True
except ImportError as e:
    _daw = None
    HAVE_FAUST_FX = False
    FAUST_FX_ERROR = f'{e} (python={sys.executable})'


@contextlib.contextmanager
def _suppress_native_stderr():
    """dawdreamer/libfaust écrivent parfois des avertissements bénins
    (ex: nombre de canaux) directement sur le fd stderr natif ; on les
    masque pour ne pas les faire remonter comme erreurs côté renderer."""
    try:
        stderr_fd = sys.stderr.fileno()
        saved_fd  = os.dup(stderr_fd)
        devnull   = os.open(os.devnull, os.O_WRONLY)
    except Exception:
        yield
        return
    try:
        os.dup2(devnull, stderr_fd)
        yield
    finally:
        os.dup2(saved_fd, stderr_fd)
        os.close(devnull)
        os.close(saved_fd)


def _process_channel_through_fx(mono, sr, fx_chain, velocity):
    """Fait passer un signal mono (un canal du fichier N-canaux) à travers
    la chaîne de greffons Faust définie côté mcMidiKeyboard, avec les
    paramètres proportionnels à la vélocité. Limitation v1 : le rendu
    conserve la longueur d'origine (pas d'extension de queue de reverb/delay
    au-delà de la fin du fichier), pour rester compatible avec l'enveloppe de
    fade calculée sur cette longueur."""
    n = len(mono)
    if n == 0 or not fx_chain or not HAVE_FAUST_FX:
        return mono
    return _faust_pool.submit(
        _process_channel_through_fx_on_faust_thread, mono, sr, fx_chain, velocity, n
    ).result()


def _process_channel_through_fx_on_faust_thread(mono, sr, fx_chain, velocity, n):
    duration  = n / sr
    vel_scale = max(0.0, min(1.0, velocity / 127.0))

    with _suppress_native_stderr():
        engine   = _daw.RenderEngine(sr, 512)
        src_data = np.stack([mono, mono]).astype(np.float32)  # duplication mono→2ch
        src      = engine.make_playback_processor('src', src_data)
        graph      = [(src, [])]
        prev       = 'src'
        any_stage  = False

        for i, stage in enumerate(fx_chain):
            dsp_name = stage.get('dsp')
            if not dsp_name:
                continue
            dsp_path = os.path.join(FX_DIR, dsp_name)
            if not os.path.isfile(dsp_path):
                continue
            proc_name = f'fx{i}'
            fp = engine.make_faust_processor(proc_name)
            fp.set_dsp(dsp_path)
            if not fp.compile():
                continue
            bounds = {p['name']: (p['min'], p['max']) for p in fp.get_parameters_description()}
            for pname, pcfg in (stage.get('params') or {}).items():
                if pname not in bounds:
                    continue
                pmin, pmax = bounds[pname]
                # La vélocité interpole depuis pmin (vel=0, état "au repos" du
                # paramètre) vers la valeur éditée (vel=127), plutôt que de
                # multiplier la valeur absolue : sinon, pour un pmin > 0 (ex.
                # fréquence de filtre 20-20000 Hz), vel_scale faible écrase la
                # valeur sous pmin et le clamp la fige, rendant la vélocité
                # sans effet perceptible sur ce paramètre.
                base = max(pmin, min(pmax, pmin + (float(pcfg.get('value', 0.0)) - pmin) * vel_scale))
                if pcfg.get('automate'):
                    pts = pcfg.get('points') or []
                    if len(pts) < 2:
                        v0  = float(pcfg.get('value', 0.0))
                        pts = [{'t': 0.0, 'v': v0}, {'t': 1.0, 'v': float(pcfg.get('valueEnd', v0))}]
                    pts = sorted(pts, key=lambda p: p['t'])
                    xs    = np.array([p['t'] for p in pts], dtype=np.float32)
                    ys    = pmin + (np.array([p['v'] for p in pts], dtype=np.float32) - pmin) * vel_scale
                    t     = np.linspace(0.0, 1.0, n, dtype=np.float32)
                    curve = np.clip(np.interp(t, xs, ys), pmin, pmax)
                    fp.set_automation(pname, curve.astype(np.float32))
                else:
                    fp.set_parameter(pname, base)
            graph.append((fp, [prev]))
            prev      = proc_name
            any_stage = True

        if not any_stage:
            return mono
        if not engine.load_graph(graph) or not engine.render(duration):
            return mono
        out = engine.get_audio()

    if out.shape[1] < n:
        pad = np.zeros(n - out.shape[1], dtype=np.float32)
        return np.concatenate([out[0], pad])
    return out[0, :n].astype(np.float32)

# ── Communication JSON ────────────────────────────────────────────────────────

def emit(obj):
    sys.stdout.write(json.dumps(obj) + '\n')
    sys.stdout.flush()

cmd_queue   = queue.Queue()
event_queue = queue.Queue()

def event_emitter():
    while True:
        try:
            emit(event_queue.get(timeout=1))
        except queue.Empty:
            pass

def read_stdin():
    for line in sys.stdin:
        line = line.strip()
        if line:
            try:
                cmd_queue.put(json.loads(line))
            except json.JSONDecodeError:
                pass

RELEASE_FRAMES = 2205  # ~50 ms à 44100 Hz

class Voice:
    __slots__ = ('data', 'envelope', 'track', 'vel_scale', 'n_ch', 'n_frames',
                 'pos', 'active', 'releasing', 'release_pos')

    def __init__(self, data, envelope, track, velocity=127):
        self.data        = data
        self.envelope    = envelope
        self.track       = track   # ref dynamique pour le gain en temps réel
        self.vel_scale   = max(0.0, min(1.0, velocity / 127.0))
        self.n_ch        = data.shape[1]
        self.n_frames    = data.shape[0]
        self.pos         = 0
        self.active      = True
        self.releasing   = False
        self.release_pos = 0

    def release(self):
        if not self.releasing:
            self.releasing   = True
            self.release_pos = 0

    def render(self, out_buffers, frames, max_ch):
        if not self.active:
            return False
        remaining = self.n_frames - self.pos
        if remaining <= 0:
            self.active = False
            return False
        chunk_len = min(frames, remaining)
        chunk     = self.data[self.pos: self.pos + chunk_len]
        env_chunk = self.envelope[self.pos: self.pos + chunk_len]
        amp       = self.track.gain_linear() * self.vel_scale

        if self.releasing:
            r_remaining = max(RELEASE_FRAMES - self.release_pos, 0)
            fade_len    = min(chunk_len, r_remaining)
            if r_remaining > 0:
                fade = np.linspace(1.0, 0.0, r_remaining, dtype=np.float32)
                env_chunk = env_chunk.copy()
                env_chunk[:fade_len] *= fade[:fade_len]
            if chunk_len > r_remaining:
                chunk_len = fade_len if fade_len > 0 else 0
                self.active = False
            self.release_pos += chunk_len

        n_ch = min(self.n_ch, max_ch)
        for ch in range(n_ch):
            out_buffers[ch][:chunk_len] += chunk[:chunk_len, ch] * env_chunk[:chunk_len] * amp

        self.pos += chunk_len
        if self.pos >= self.n_frames:
            self.active = False
        return self.active


class Track:
    def __init__(self, row_id):
        self.id        = row_id
        self.file      = ''
        self.gain      = 1.0
        self.volume    = 1.0   # multiplicateur linéaire direct (knob volume)
        self.fade_type = 'l'
        self.fade_in   = 0.1
        self.fade_out  = 0.1
        self.data      = None
        self.sr        = 44100
        self.one_shot  = False
        self.lock      = threading.Lock()
        self.voices    = []
        self._env_cache     = None
        self._env_cache_key = None
        self.fx_chain        = []    # [{dsp, params:{name:{value,automate,points:[{t,v}]}}}]
        self.fx_cache        = {}    # {vélocité_palier: np.ndarray (frames, ch)}
        self.fx_state        = 'idle'  # idle|rendering|ready|error
        self._fx_chain_json  = '[]'

    def load(self):
        if not self.file or not os.path.exists(self.file):
            event_queue.put({'type': 'load_error', 'id': self.id,
                             'message': 'Fichier introuvable'})
            return False
        try:
            data, sr = sf.read(self.file, always_2d=True, dtype='float32')
            if OUTPUT_SR and sr != OUTPUT_SR:
                data = _resample(data, sr, OUTPUT_SR)
                sr   = OUTPUT_SR
            with self.lock:
                self.data = data
                self.sr   = sr
                self._env_cache     = None
                self._env_cache_key = None
                self.fx_cache       = {}
            event_queue.put({'type': 'loaded', 'id': self.id,
                             'channels': data.shape[1], 'frames': data.shape[0], 'sr': sr})
            if self.fx_chain and HAVE_FAUST_FX:
                self._start_fx_render()
            return True
        except Exception as e:
            event_queue.put({'type': 'load_error', 'id': self.id, 'message': str(e)})
            return False

    def _build_envelope(self):
        n  = len(self.data)
        fi = min(int(self.fade_in  * self.sr), n)
        fo = min(int(self.fade_out * self.sr), n - fi)
        env = np.ones(n, dtype=np.float32)
        if fi > 0:
            env[:fi] = self._fade_curve(np.linspace(0, 1, fi, dtype=np.float32))
        if fo > 0:
            env[n - fo:] *= self._fade_curve(np.linspace(1, 0, fo, dtype=np.float32))
        return env

    def _fade_curve(self, t):
        ft = self.fade_type
        if ft == 'q': return np.sin(t * (np.pi / 2))
        if ft == 'h': return np.sin(t * np.pi)
        if ft == 't': return t
        if ft == 'l': return np.log1p(t * (math.e - 1))
        if ft == 'p': return 1 - (1 - t) ** 2
        return t

    def _get_envelope(self):
        key = (len(self.data), self.sr, self.fade_in, self.fade_out, self.fade_type)
        if self._env_cache_key != key:
            self._env_cache     = self._build_envelope()
            self._env_cache_key = key
        return self._env_cache

    def gain_linear(self):
        db = (self.gain - 1) * 2.0
        return 10 ** (db / 20.0) * self.volume

    # ── Greffons FX Faust ──────────────────────────────────────────────────

    def set_fx_chain(self, fx_chain):
        """Met à jour la chaîne FX ; relance le précalcul par palier de
        vélocité si elle a effectivement changé."""
        new_json = json.dumps(fx_chain or [], sort_keys=True)
        if new_json == self._fx_chain_json:
            return
        self._fx_chain_json = new_json
        self.fx_chain = fx_chain or []
        with self.lock:
            self.fx_cache = {}
        if self.fx_chain and self.data is not None and HAVE_FAUST_FX:
            self._start_fx_render()
        else:
            self.fx_state = 'idle'

    def _start_fx_render(self):
        self.fx_state = 'rendering'
        event_queue.put({'type': 'fx_state', 'id': self.id, 'state': 'rendering'})
        threading.Thread(target=self._render_fx_buckets, daemon=True).start()

    def _render_fx_buckets(self):
        with self.lock:
            data  = self.data
            sr    = self.sr
            chain = list(self.fx_chain)
        if data is None:
            return
        try:
            n_frames, n_ch = data.shape
            new_cache = {}
            for vel in FX_VELOCITY_BUCKETS:
                out = np.zeros((n_frames, n_ch), dtype=np.float32)
                for ch in range(n_ch):
                    out[:, ch] = _process_channel_through_fx(data[:, ch], sr, chain, vel)
                new_cache[vel] = out
            with self.lock:
                if list(self.fx_chain) == chain:   # pas changé pendant le calcul
                    self.fx_cache = new_cache
                    self.fx_state = 'ready'
            event_queue.put({'type': 'fx_ready', 'id': self.id})
        except Exception as e:
            self.fx_state = 'error'
            event_queue.put({'type': 'fx_error', 'id': self.id, 'message': str(e)})

    def start(self, velocity=127):
        with self.lock:
            if self.data is None:
                return
            data = self.data
            if self.fx_chain and self.fx_cache:
                bucket  = min(FX_VELOCITY_BUCKETS, key=lambda b: abs(b - velocity))
                fx_data = self.fx_cache.get(bucket)
                if fx_data is not None:
                    data = fx_data
            env   = self._get_envelope()
            voice = Voice(data, env, self, velocity)
            self.voices.append(voice)

    def stop(self):
        with self.lock:
            for v in self.voices:
                v.release()

    def stop_hard(self):
        with self.lock:
            for v in self.voices:
                v.active = False
            self.voices.clear()

    def render_all(self, out_buffers, frames, max_ch):
        with self.lock:
            had_voices = bool(self.voices)
            for v in self.voices:
                v.render(out_buffers, frames, max_ch)
            self.voices = [v for v in self.voices if v.active]
            if had_voices and not self.voices:
                event_queue.put({'type': 'voice_end', 'id': self.id})


tracks      = {}
tracks_lock = threading.Lock()

def get_or_create(row_id):
    with tracks_lock:
        if row_id not in tracks:
            tracks[row_id] = Track(row_id)
        return tracks[row_id]

def snapshot_tracks():
    with tracks_lock:
        return list(tracks.values())

# ── Polyphonie globale ────────────────────────────────────────────────────────

_max_voices = 0  # 0 = illimité

def _count_voices():
    return sum(
        sum(1 for v in t.voices if v.active)
        for t in snapshot_tracks()
    )

def _steal_voice():
    """Relâche la voix la plus avancée pour libérer une place."""
    best_v, best_pos = None, -1
    for t in snapshot_tracks():
        with t.lock:
            for v in t.voices:
                if v.active and not v.releasing and v.pos > best_pos:
                    best_pos, best_v = v.pos, v
    if best_v:
        best_v.release()


def run_jack():
    global OUTPUT_SR
    client    = jack.Client('mcMidiPlayer', no_start_server=True)
    OUTPUT_SR = client.samplerate
    MAX_PORTS = MAX_PORTS_CFG
    out_ports = [client.outports.register(f'out_{i+1}') for i in range(MAX_PORTS)]

    @client.set_process_callback
    def process(frames):
        buffers = [np.frombuffer(p.get_buffer(), dtype=np.float32) for p in out_ports]
        for b in buffers:
            b[:] = 0.0
        for track in snapshot_tracks():
            track.render_all(buffers, frames, MAX_PORTS)

    @client.set_shutdown_callback
    def shutdown(status, reason):
        emit({'type': 'error', 'message': f'JACK shutdown : {reason}'})

    with client:
        try:
            targets = client.get_ports('system:playback_.*', is_input=True)
            for i, t in enumerate(targets):
                if i < MAX_PORTS:
                    client.connect(out_ports[i], t)
        except Exception:
            pass

        emit({'type': 'ready', 'backend': 'jack', 'maxPorts': MAX_PORTS})
        return process_commands()


def run_sounddevice():
    global OUTPUT_SR
    device = _current_device
    try:
        info = sd.query_devices(device, 'output') if device is not None else sd.query_devices(kind='output')
        SR   = int(info.get('default_samplerate', 48000))
    except Exception:
        SR   = 48000
    OUTPUT_SR = SR
    BLOCK = 1024

    def audio_callback(outdata, frames, time_info, status):
        outdata[:] = 0.0
        n_out = outdata.shape[1]
        bufs  = [np.zeros(frames, dtype=np.float32) for _ in range(n_out)]
        for track in snapshot_tracks():
            track.render_all(bufs, frames, n_out)
        for ch in range(n_out):
            outdata[:, ch] += bufs[ch]

    if _current_channels is not None:
        n_out = min(_current_channels, MAX_PORTS_CFG)
    else:
        try:
            info  = sd.query_devices(device, 'output') if device is not None else sd.query_devices(kind='output')
            n_out = min(info.get('max_output_channels', 2), MAX_PORTS_CFG)
        except Exception:
            n_out = 2

    try:
        stream = sd.OutputStream(device=device, samplerate=SR, channels=n_out,
                                 blocksize=BLOCK, dtype='float32',
                                 callback=audio_callback)
    except Exception as e:
        emit({'type': 'error', 'message': f'sounddevice indisponible : {e}'})
        return

    with stream:
        emit({'type': 'ready', 'backend': 'sounddevice', 'maxPorts': n_out})
        return process_commands()


def process_commands():
    while True:
        try:
            msg = cmd_queue.get(timeout=1)
        except queue.Empty:
            continue

        cmd    = msg.get('cmd')
        row_id = msg.get('id')

        if cmd == 'quit':
            break

        elif cmd == 'set_polyphonie':
            global _max_voices
            _max_voices = max(0, int(msg.get('value', 0)))

        elif cmd == 'update':
            track = get_or_create(row_id)
            old_file = track.file
            track.gain      = float(msg.get('gain', 1))
            track.fade_type = msg.get('fadeType', 'l')
            track.fade_in   = float(msg.get('fadeIn',  0.05))
            track.fade_out  = float(msg.get('fadeOut', 0.1))
            track.one_shot  = bool(msg.get('oneShot', False))
            with track.lock:
                track._env_cache_key = None
            track.set_fx_chain(msg.get('fx', []))
            new_file = msg.get('file', '')
            if new_file and new_file != old_file:
                track.file = new_file
                threading.Thread(target=track.load, daemon=True).start()

        elif cmd == 'set_gain':
            track = get_or_create(row_id)
            track.volume = float(msg.get('gain', 1))

        elif cmd == 'remove':
            with tracks_lock:
                t = tracks.pop(row_id, None)
            if t:
                t.stop_hard()

        elif cmd == 'play':
            track    = get_or_create(row_id)
            velocity = int(msg.get('velocity', 127))
            if _max_voices > 0 and _count_voices() >= _max_voices:
                _steal_voice()
            track.start(velocity)

        elif cmd == 'stop':
            track = get_or_create(row_id)
            if not track.one_shot:
                track.stop()

        elif cmd == 'list_devices':
            try:
                devices  = sd.query_devices()
                hostapis = sd.query_hostapis()
                _, default_out = sd.default.device
                dlist = []
                for i, d in enumerate(devices):
                    if d['max_output_channels'] < 1:
                        continue
                    api_idx  = d.get('hostapi', 0)
                    api_name = hostapis[api_idx]['name'] if api_idx < len(hostapis) else ''
                    dlist.append({
                        'index':               i,
                        'name':                d['name'],
                        'max_output_channels': d['max_output_channels'],
                        'default_samplerate':  d['default_samplerate'],
                        'is_default':          i == default_out,
                        'hostapi':             api_name,
                    })
                emit({'type': 'devices', 'list': dlist})
            except Exception as ex:
                emit({'type': 'error', 'message': str(ex)})

        elif cmd == 'set_device':
            global _current_device, _current_channels
            dev = msg.get('device')
            ch  = msg.get('channels')
            _current_device   = int(dev) if dev is not None else None
            _current_channels = int(ch)  if ch  is not None else None
            return 'restart'


if __name__ == '__main__':
    signal.signal(signal.SIGTERM, lambda *_: cmd_queue.put({'cmd': 'quit'}))

    threading.Thread(target=read_stdin,    daemon=True).start()
    threading.Thread(target=event_emitter, daemon=True).start()
    while True:
        if USE_JACK and _current_device is None:
            try:
                result = run_jack()
            except Exception as e:
                emit({'type': 'error', 'message': f'JACK indisponible ({e}), bascule sur sounddevice'})
                result = run_sounddevice()
        else:
            result = run_sounddevice()
        if result != 'restart':
            break
