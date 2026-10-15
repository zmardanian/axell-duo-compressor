"""Calibrate the FET's ALL ("all buttons") law on the hardware 1176LN recordings (Martinez Ramirez et al., one setting:
ALL, attack 800 us, release 1100 ms). Only the ALL-mode constants are fitted - the attack/release knobs stay at the
hardware's settings and the other ratios are not touched - plus the unknown recording level (input gain).
Calibration: every 4th even-numbered note; evaluation: all 593 odd-numbered notes (held out).

    python3 fet1176_all_fit.py out.json
"""
import json, sys
import numpy as np
from scipy.optimize import minimize
import fet1176_compare as M

NAMES = ["ratio", "knee", "thrOffsetDb", "attackMul", "releaseMul", "bumpOffsetDb", "bumpSlope", "bumpMaxDb"]
CLASSIC = [20.0, 2.0, 0.0, 0.6, 0.7, 10.0, 0.15, 6.0]
LOG = {"ratio", "attackMul", "releaseMul"}
BOUNDS = {"ratio": (2, 60), "knee": (0, 24), "thrOffsetDb": (-24, 12), "attackMul": (0.05, 4), "releaseMul": (0.05, 4),
          "bumpOffsetDb": (0, 30), "bumpSlope": (0, 0.9), "bumpMaxDb": (0, 24)}
notes = M.notes()
cal, ev = notes[0::2][::4], notes[1::2]


def to_all(u):
    m = {}
    for k, v in zip(NAMES, u[1:]):
        v = float(np.exp(v)) if k in LOG else float(v)
        lo, hi = BOUNDS[k]
        m[k] = min(max(v, lo), hi)
    return float(u[0]), m


def score(items, u, tag):
    ind, m = to_all(u)
    e, _ = M.evaluate(items, ind, tag, fetRatio=4, **{f"a.{k}": v for k, v in m.items()})
    return float(e.mean())


u0 = [2.5] + [np.log(v) if k in LOG else v for k, v in zip(NAMES, CLASSIC)]
c0 = score(cal, u0, "c")
print(f"classic ALL law: calibration {c0:.3f} dB", flush=True)
r = minimize(lambda u: score(cal, u, "c"), u0, method="Nelder-Mead",
             options={"maxfev": 700, "xatol": 1e-3, "fatol": 1e-3, "adaptive": True})
ind, m = to_all(r.x)
held = score(ev, r.x, "e")
held0 = score(ev, u0, "e")
res = {"input_db": ind, "all": m, "calibration_db": r.fun, "held_out_db": held, "classic_held_out_db": held0}
print(json.dumps(res, indent=1), flush=True)
json.dump(res, open(sys.argv[1], "w"), indent=1)
