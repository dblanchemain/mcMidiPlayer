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
import threading
import math
import platform
import queue
import argparse
import numpy as np

_parser = argparse.ArgumentParser(add_help=False)
_parser.add_argument('--max-ports', type=int, default=16)
_args, _ = _parser.parse_known_args()
MAX_PORTS_CFG = _args.max_ports

PLATFORM = platform.system()
USE_JACK  = PLATFORM == 'Linux'

try:
    import soundfile as sf
    import sounddevice as sd
except ImportError as e:
    sys.stdout.write(json.dumps({'type': 'error', 'message': str(e)}) + '\n')
    sys.stdout.flush()
    sys.exit(1)

if USE_JACK:
    try:
        import jack
    except ImportError:
        USE_JACK = False

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

    def load(self):
        if not self.file or not os.path.exists(self.file):
            event_queue.put({'type': 'load_error', 'id': self.id,
                             'message': 'Fichier introuvable'})
            return False
        try:
            data, sr = sf.read(self.file, always_2d=True, dtype='float32')
            with self.lock:
                self.data = data
                self.sr   = sr
                self._env_cache     = None
                self._env_cache_key = None
            event_queue.put({'type': 'loaded', 'id': self.id,
                             'channels': data.shape[1], 'frames': data.shape[0], 'sr': sr})
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

    def start(self, velocity=127):
        with self.lock:
            if self.data is None:
                return
            env   = self._get_envelope()
            voice = Voice(self.data, env, self, velocity)
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


def run_jack():
    client    = jack.Client('mcMidiPlayer', no_start_server=True)
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
        process_commands()


def run_sounddevice():
    SR    = 44100
    BLOCK = 1024

    def audio_callback(outdata, frames, time_info, status):
        outdata[:] = 0.0
        n_out = outdata.shape[1]
        bufs  = [np.zeros(frames, dtype=np.float32) for _ in range(n_out)]
        for track in snapshot_tracks():
            track.render_all(bufs, frames, n_out)
        for ch in range(n_out):
            outdata[:, ch] += bufs[ch]

    try:
        info  = sd.query_devices(kind='output')
        n_out = min(info.get('max_output_channels', 2), MAX_PORTS_CFG)
    except Exception:
        n_out = 2

    try:
        stream = sd.OutputStream(samplerate=SR, channels=n_out,
                                 blocksize=BLOCK, dtype='float32',
                                 callback=audio_callback)
    except Exception as e:
        emit({'type': 'error', 'message': f'sounddevice indisponible : {e}'})
        return

    with stream:
        emit({'type': 'ready', 'backend': 'sounddevice', 'maxPorts': n_out})
        process_commands()


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
            track.start(velocity)

        elif cmd == 'stop':
            track = get_or_create(row_id)
            if not track.one_shot:
                track.stop()


if __name__ == '__main__':
    threading.Thread(target=read_stdin,    daemon=True).start()
    threading.Thread(target=event_emitter, daemon=True).start()
    if USE_JACK:
        try:
            run_jack()
        except Exception as e:
            emit({'type': 'error', 'message': f'JACK indisponible ({e}), bascule sur sounddevice'})
            run_sounddevice()
    else:
        run_sounddevice()
