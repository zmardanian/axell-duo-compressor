"""Refine the Measured opto law with the onset flare (DECISIONS D28): fits onset gain / fast / slow time constants and the
release split, starting from la2a_measured_model.json, on the calibration blocks only (minutes 3-7, blocks 0, 4, ...).
Objective = GR envelope error (10 ms frames) + deviation of the MEAN onset trajectory (2.5 ms frames, first 30 ms after
every level jump > 10 dB inside the calibration blocks) from the hardware's mean trajectory. The mean matters: single
onsets in 2.5 ms frames of noise jitter by several dB (measurement noise), the systematic shape is in the mean.
Evaluation on held-out audio (everything else).

    python3 la2a_onset_fit.py out.json
"""
import json, os, subprocess, sys
from concurrent.futures import ThreadPoolExecutor
import numpy as np
from scipy.optimize import minimize
import la2a_compare as L

RENDER = os.environ.get("DUO_RENDER_FIT", L.RENDER)
G0 = -0.38
base = json.load(open(os.path.join(os.path.dirname(__file__), "la2a_measured_model.json")))["model"]
NAMES = ["onsetGain", "onsetFastSec", "onsetSlowSec", "fastShare", "fastReleaseSec", "slowMinSec"]
FREE = int(os.environ.get("ONSET_FREE", "6"))   # 3 = only the flare; the release law stays as calibrated in 2.0
W_ONSET = float(os.environ.get("ONSET_WEIGHT", "1.0"))
LOG = {"onsetFastSec", "onsetSlowSec", "fastReleaseSec", "slowMinSec"}
BOUNDS = {"onsetGain": (0, 6), "onsetFastSec": (0.001, 0.05), "onsetSlowSec": (0.005, 0.3), "fastShare": (0.05, 0.95),
          "fastReleaseSec": (0.005, 0.5), "slowMinSec": (0.1, 5)}
F10, F25 = int(0.01 * L.FS), int(0.0025 * L.FS)
CAL = (180, 420)

data = []
for idx, name in L.PAIRS:
    limit, pr = L.settings(name)
    x, y = L.load(idx, name)
    data.append((idx, limit, pr, x, y))


def render(x, limit, pr, m, tag):
    os.makedirs(L.WORK, exist_ok=True)
    fi, fo = f"{L.WORK}/of_{tag}_in.f32", f"{L.WORK}/of_{tag}_out.f32"
    if not os.path.exists(fi):
        x.astype(np.float32).tofile(fi)
    p = {"fetOn": 0, "optoOn": 1, "optoMode": limit, "optoPR": pr, "colorPct": 0, "scHpfHz": 20, "osLog2": 0}
    p.update({f"m.{k}": v for k, v in m.items()})
    subprocess.run([RENDER, fi, fo, str(L.FS), "1"] + [f"{k}={v}" for k, v in p.items()], check=True, stderr=subprocess.DEVNULL)
    return np.fromfile(fo, np.float32)


def onsets(lx, mask):
    return [i for i in range(2, len(lx) - 13) if mask[i] and lx[i] - lx[i - 1] > 10 and lx[i] > -25
            and np.all(np.abs(np.diff(lx[i:i + 12])) < 3)]


def errors(x, y, z, sel10, sel25):
    lx, ly, lz = L.frames_db(x, F10), L.frames_db(y, F10), L.frames_db(z, F10)
    n = min(len(lx), len(lz))
    env = ((lz[:n] - lx[:n] + G0) - (ly[:n] - lx[:n]))[sel10[:n] & (lx[:n] > -45)]
    ax, ay, az = L.frames_db(x, F25), L.frames_db(y, F25), L.frames_db(z, F25)
    hs, ms = [], []
    for i in onsets(ax, sel25):
        h = (ay - ax)[i - 1:i + 13]; m = (az - ax)[i - 1:i + 13]
        hs.append(h - h[0]); ms.append(m - m[0])
    return env, (np.array(hs), np.array(ms))


def blocks(n, step, phase):
    t = np.arange(n) * step / L.FS
    blk = np.floor((t - CAL[0]) / 20).astype(int) % 4
    inside = (t >= CAL[0]) & (t < CAL[1])
    return inside & (blk == phase) if phase is not None else ~(inside & (blk == 0))


def to_model(u):
    m = dict(base)
    for k, v in zip(NAMES[:FREE], u):
        v = float(np.exp(v)) if k in LOG else float(v)
        lo, hi = BOUNDS[k]
        m[k] = min(max(v, lo), hi)
    m["slowMaxSec"] = max(m["slowMaxSec"], m["slowMinSec"])
    return m


pool = ThreadPoolExecutor(3)


def objective(u):
    m = to_model(u)
    def one(d):
        idx, limit, pr, x, y = d
        xs, ys = x[CAL[0] * L.FS:CAL[1] * L.FS], y[CAL[0] * L.FS:CAL[1] * L.FS]
        z = render(xs, limit, pr, m, f"c{idx}")
        n10, n25 = len(xs) // F10, len(xs) // F25
        t10 = np.floor(np.arange(n10) * F10 / L.FS / 20).astype(int) % 4 == 0
        t25 = np.floor(np.arange(n25) * F25 / L.FS / 20).astype(int) % 4 == 0
        return errors(xs, ys, z, t10, t25)
    res = list(pool.map(one, data))
    env = np.concatenate([a for a, _ in res])
    hs = np.concatenate([b[0] for _, b in res]); ms = np.concatenate([b[1] for _, b in res])
    return float(np.sqrt(np.mean(env ** 2)) + W_ONSET * np.sqrt(np.mean((ms.mean(0) - hs.mean(0)) ** 2)))


def held_out(m):
    out = {}
    def one(d):
        idx, limit, pr, x, y = d
        z = render(x, limit, pr, m, f"h{idx}")
        e, (hs, ms) = errors(x, y, z, blocks(len(x) // F10, F10, None), blocks(len(x) // F25, F25, None))
        return float(np.sqrt(np.mean(e ** 2))), float(np.sqrt(np.mean((ms.mean(0) - hs.mean(0)) ** 2)))
    for d, r in zip(data, pool.map(one, data)):
        out[d[0]] = {"env_10ms_db": r[0], "mean_onset_traj_db": r[1]}
    return out


if __name__ == "__main__":
    u0 = [0.0, np.log(0.008), np.log(0.04), base["fastShare"], np.log(base["fastReleaseSec"]), np.log(base["slowMinSec"])][:FREE]
    print(f"2.0 Measured law: objective {objective(u0):.3f}", flush=True)
    best = None
    for start in (u0, [1.5] + list(u0[1:])):
        r = minimize(objective, start, method="Nelder-Mead", options={"maxfev": 350, "xatol": 1e-3, "fatol": 1e-3, "adaptive": True})
        print(f"  start done: {r.fun:.3f}", flush=True)
        if best is None or r.fun < best.fun:
            best = r
    m = to_model(best.x)
    res = {"model": m, "objective": best.fun, "held_out": held_out(m), "held_out_2_0": held_out(to_model(u0))}
    print(json.dumps(res, indent=1), flush=True)
    json.dump(res, open(sys.argv[1], "w"), indent=1)
