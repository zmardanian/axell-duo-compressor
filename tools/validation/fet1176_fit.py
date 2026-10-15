"""Which FET parameters would make the stage follow the hardware 1176LN (ALL, 800 us, 1100 ms) best?
Fits input gain, knee and attack/release (inside the knob ranges) on 100 calibration notes, per ratio law,
then checks the held-out notes. Diagnostic for DECISIONS D24 (does the model need a change?)."""
import numpy as np
from scipy.optimize import minimize
import fet1176_compare as M

notes = M.notes()
cal, ev = notes[0::2][::6], notes[1::2][::3]
def score(items, v, ratio, tag):
    ind, knee, atk, rel = v
    e, _ = M.evaluate(items, ind, tag, fetRatio=ratio, fetKneeDb=float(np.clip(knee, 0, 24)),
                      fetAtkMs=float(np.clip(atk, 0.02, 0.8)), fetRelMs=float(np.clip(rel, 50, 1100)))
    return float(e.mean())
for ratio, name in ((4, "ALL"), (3, "20:1"), (1, "8:1"), (0, "4:1")):
    best = None
    for start in ([2.5, 2.0, 0.8, 1100], [6.0, 10.0, 0.4, 1100]):
        r = minimize(lambda v: score(cal, v, ratio, "fitc"), start, method="Nelder-Mead",
                     options={"xatol": 0.05, "fatol": 0.003, "maxiter": 60})
        if best is None or r.fun < best.fun:
            best = r
    print(f"{name:5s}: in {best.x[0]:+.2f} dB, knee {np.clip(best.x[1],0,24):.1f} dB, attack {np.clip(best.x[2],.02,.8)*1000:.0f} us, "
          f"release {np.clip(best.x[3],50,1100):.0f} ms | cal {best.fun:.3f} dB, held-out {score(ev, best.x, ratio, 'fite'):.3f} dB", flush=True)
