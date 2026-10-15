"""Calibrate the opto law (knob scale, ratio/knee per mode, light-cell timing) on the SignalTrain LA-2A
recordings - one parameter set for all three hardware settings (it is one unit).

Calibration data: minutes 3-7 of each file (music and noise bursts), 20 s blocks 0, 4, 8, ... of it.
Held out: everything else (blocks 2, 6, ... of minutes 3-7 and all of minutes 0-3 and 7-15), about 85 %.
Objective: gain-reduction envelope error (10 ms frames, dB RMS) with the hardware's quiet gain fixed (-0.38 dB).

    python3 la2a_joint_fit.py out.json
"""
import json, os, subprocess, sys
from concurrent.futures import ThreadPoolExecutor
import numpy as np
from scipy.optimize import minimize
import la2a_compare as L

RENDER = os.environ.get("DUO_RENDER_FIT", L.RENDER)
G0, F = -0.38, int(0.01 * L.FS)
BLOCK = int(20 * L.FS / F)
CAL = (3 * 60, 7 * 60)
NAMES = ["prDbPerUnit", "prOffsetDb", "ratioComp", "ratioLim", "kneeComp", "kneeLim",
         "attackSec", "fastReleaseSec", "fastShare", "slowMinSec", "slowMaxSec"]
CLASSIC = [0.4, 0.0, 3.0, 10.0, 12.0, 6.0, 0.0085, 0.040, 0.6, 0.5, 5.0]
LOG = {"attackSec", "fastReleaseSec", "slowMinSec", "slowMaxSec", "ratioComp", "ratioLim"}
BOUNDS = {"prDbPerUnit": (0.05, 1.0), "prOffsetDb": (-40, 30), "ratioComp": (1.2, 30), "ratioLim": (1.2, 60),
          "kneeComp": (0, 24), "kneeLim": (0, 24), "attackSec": (0.001, 0.05), "fastReleaseSec": (0.01, 0.5),
          "fastShare": (0.05, 0.95), "slowMinSec": (0.1, 5), "slowMaxSec": (0.5, 60)}

data = []
for idx, name in L.PAIRS:
    limit, pr = L.settings(name)
    x, y = L.load(idx, name)
    lx, ly = L.frames_db(x, F), L.frames_db(y, F)
    data.append((idx, name, limit, pr, x, lx, ly))


def to_model(u):
    m = {}
    for k, v in zip(NAMES, u):
        v = float(np.exp(v)) if k in LOG else float(v)
        lo, hi = BOUNDS[k]
        m[k] = min(max(v, lo), hi)
    m["slowMaxSec"] = max(m["slowMaxSec"], m["slowMinSec"])
    return m


def from_model(vals):
    return [np.log(v) if k in LOG else v for k, v in zip(NAMES, vals)]


def render(d, model, seconds, tag):
    idx, name, limit, pr, x, lx, ly = d
    a, b = seconds
    xs = x[a * L.FS: b * L.FS]
    os.makedirs(L.WORK, exist_ok=True)
    fi, fo = f"{L.WORK}/jf{idx}_{a}_{b}_in.f32", f"{L.WORK}/jf{idx}_{a}_{b}_{tag}_out.f32"
    if not os.path.exists(fi):
        xs.astype(np.float32).tofile(fi)
    p = {"fetOn": 0, "optoOn": 1, "optoMode": limit, "optoPR": pr, "colorPct": 0, "scHpfHz": 20, "osLog2": 0}
    p.update({f"m.{k}": v for k, v in model.items()})
    subprocess.run([RENDER, fi, fo, str(L.FS), "1"] + [f"{k}={v}" for k, v in p.items()], check=True,
                   stderr=subprocess.DEVNULL)
    return L.frames_db(np.fromfile(fo, np.float32), F)


def errors(d, lz, seconds, phases):
    idx, name, limit, pr, x, lx, ly = d
    a, b = seconds
    fa, fb = a * L.FS // F, a * L.FS // F + len(lz)
    gx, gy = lx[fa:fb], ly[fa:fb]
    blk = (np.arange(len(lz)) // BLOCK) % 4
    act = (gx > -45) & np.isin(blk, phases)
    return ((lz - gx + G0) - (gy - gx))[act]


pool = ThreadPoolExecutor(3)
evals = [0]


def objective(u):
    m = to_model(u)
    lzs = list(pool.map(lambda d: render(d, m, CAL, "c"), data))
    e = np.concatenate([errors(d, lz, CAL, [0]) for d, lz in zip(data, lzs)])
    evals[0] += 1
    return float(np.sqrt(np.mean(e ** 2)))


def held_out(m):
    """per file: error on everything outside the calibration blocks (full 15 min render)"""
    out = {}
    lzs = list(pool.map(lambda d: render(d, m, (0, 15 * 60 - 1), "h"), data))
    for d, lz in zip(data, lzs):
        idx, name, limit, pr, x, lx, ly = d
        n = len(lz)
        t = np.arange(n) * F / L.FS
        blk = ((t - CAL[0]) * L.FS / F // BLOCK).astype(int) % 4
        incal = (t >= CAL[0]) & (t < CAL[1]) & (blk == 0)
        act = (lx[:n] > -45) & ~incal
        e = ((lz - lx[:n] + G0) - (ly[:n] - lx[:n]))[act]
        out[name] = float(np.sqrt(np.mean(e ** 2)))
    return out


if __name__ == "__main__":
    u0 = from_model(CLASSIC)
    print(f"classic (1.x) model: calibration error {objective(u0):.3f} dB", flush=True)
    best = None
    for start in (u0, from_model([0.25, -8, 3, 4, 6, 6, 0.006, 0.06, 0.5, 1.0, 10.0])):
        r = minimize(objective, start, method="Nelder-Mead",
                     options={"maxfev": 900, "xatol": 1e-3, "fatol": 1e-3, "adaptive": True})
        print(f"  start done: {r.fun:.3f} dB after {evals[0]} renders", flush=True)
        if best is None or r.fun < best.fun:
            best = r
    m = to_model(best.x)
    res = {"model": m, "calibration_error_db": best.fun, "held_out_error_db": held_out(m),
           "classic_held_out_error_db": held_out(to_model(u0))}
    print(json.dumps(res, indent=1), flush=True)
    json.dump(res, open(sys.argv[1], "w"), indent=1)
