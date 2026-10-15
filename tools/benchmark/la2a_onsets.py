"""Onset behaviour vs the hardware LA-2A: at every abrupt level jump (+10 dB or more within 5 ms frames) in the
noise-burst sections of the SignalTrain recordings, the gain trajectory over the next 30 ms - hardware vs the cached
plugin renders of hw_fidelity.py (BENCH_WORK/la2a_renders). Gains are relative to the frame before the jump.

    python3 la2a_onsets.py
"""
import os, sys
import numpy as np
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "validation"))
import la2a_compare as L  # noqa: E402

FS = 44100
F = int(0.0025 * FS)  # 2.5 ms frames
cache = os.path.join(os.environ.get("BENCH_WORK", "/tmp/duo-bench"), "la2a_renders")
plugins = {"Axell Opto (Measured)": "ax", "UADx LA-2A (fitted knob)": "ub"}
T = 12  # frames after the jump (30 ms)
acc = {k: [] for k in ["hardware", *plugins]}
for idx, name in L.PAIRS:
    x, y = L.load(idx, name)
    lx, ly = L.frames_db(x, F), L.frames_db(y, F)
    zs = {k: L.frames_db(np.load(os.path.join(cache, f"{v}{idx}.npy")), F) for k, v in plugins.items()}
    start = int(300 * FS / F)  # noise-burst material
    jumps = [i for i in range(start + 2, len(lx) - T - 1)
             if lx[i] - lx[i - 1] > 10 and lx[i] > -25 and np.all(np.abs(np.diff(lx[i:i + T])) < 3)]
    for i in jumps:
        acc["hardware"].append((ly - lx)[i - 1:i + T] - (ly - lx)[i - 1])
        for k in plugins:
            z = zs[k]
            acc[k].append((z - lx[:len(z)])[i - 1:i + T] - (z - lx[:len(z)])[i - 1])
n = len(acc["hardware"])
print(f"{n} onsets (level jump > 10 dB to above -25 dBFS, steady for 30 ms afterwards)")
print("ms after jump  " + "  ".join(f"{(t) * 2.5:5.1f}" for t in range(T + 1)))
for k, v in acc.items():
    m = np.mean(v, axis=0)
    print(f"{k:24s}" + "  ".join(f"{g:+5.1f}" for g in m))
hw = np.mean(acc["hardware"], axis=0)
for k in plugins:
    d = np.mean(acc[k], axis=0) - hw
    print(f"{k:24s} RMS deviation from the hardware trajectory: {np.sqrt(np.mean(d ** 2)):.2f} dB")
