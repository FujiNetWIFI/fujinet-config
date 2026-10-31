#!/usr/bin/env python3
"""sndcheck.py -- the sounds in a MAME -wavwrite recording (o2sim O2SIM_WAV).

    sndcheck.py FILE.wav [FROM_FRAME [TO_FRAME]]

One line a sound: the frame it starts on (60 a second from the recording's
start), its length in frames, its loudest frequency and its level. Silence
of more than a frame ends a sound; a change of pitch splits one (ALERT's
two notes show as two lines, the second starting where the first ends).
"""
import sys
import wave

import numpy as np

STEP = 1 / 240                  # a window: a quarter frame
QUIET = 0.02                    # of full scale


def main(path, lo=0, hi=None):
    w = wave.open(path)
    rate, ch, width = w.getframerate(), w.getnchannels(), w.getsampwidth()
    raw = np.frombuffer(w.readframes(w.getnframes()), dtype="<i%d" % width)
    x = raw.reshape(-1, ch).mean(axis=1) / float(1 << (8 * width - 1))
    n = int(rate * STEP)
    wins = len(x) // n
    rms = np.array([np.sqrt(np.mean(x[i * n:(i + 1) * n] ** 2)) for i in range(wins)])
    out = []
    i = 0
    while i < wins:
        if rms[i] < QUIET:
            i += 1
            continue
        j = i
        while j < wins and (rms[j] >= QUIET or (j + 4 < wins and rms[j + 1:j + 5].max() >= QUIET)):
            j += 1
        seg = x[i * n:j * n]
        # split on a change of pitch: each frame's peak frequency
        fr = int(rate / 60)
        peaks = []
        for k in range(0, len(seg) - fr + 1, fr):
            f = np.abs(np.fft.rfft(seg[k:k + fr] * np.hanning(fr)))
            f[:3] = 0
            peaks.append(np.argmax(f) * rate / fr)
        if not peaks:
            peaks = [0]
        start = 0
        for k in range(1, len(peaks) + 1):
            if k == len(peaks) or abs(peaks[k] - peaks[start]) > 0.15 * peaks[start] + 30:
                a = i * n + start * fr
                b = min(i * n + k * fr, j * n)
                if b > a:
                    f = np.abs(np.fft.rfft(x[a:b] * np.hanning(b - a)))
                    f[:3] = 0
                    hz = np.argmax(f) * rate / (b - a)
                    lvl = np.sqrt(np.mean(x[a:b] ** 2))
                    out.append((a / rate * 60, (b - a) / rate * 60, hz, lvl))
                start = k
        i = j
    for t, d, hz, lvl in out:
        if t < lo or (hi is not None and t > hi):
            continue
        print("frame %7.1f  %4.1f frames  %6.0f Hz  level %.3f" % (t, d, hz, lvl))


if __name__ == "__main__":
    a = sys.argv[1:]
    main(a[0], float(a[1]) if len(a) > 1 else 0, float(a[2]) if len(a) > 2 else None)
