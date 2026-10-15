"""Final LA-2A validation report: the calibrated opto law vs the hardware on held-out audio, at the hardware's own
knob settings and with one fixed output gain (the hardware's quiet-signal gain, -0.38 dB) - nothing fitted per file.

    python3 la2a_report.py <joint_fit.json> [report.json]

Held out = everything except the calibration blocks (minutes 3-7, blocks 0, 4, 8, ... of 20 s).
GR envelope error on 50 ms frames, music (minutes 0-5) and noise-burst (minutes 5-15) material separately.
MAE / STFT / LUFS as defined by Steinmetz & Reiss 2021 (Table 1, LA-2A test set), on held-out 10 s segments.
"""
import json, sys
import numpy as np
import la2a_compare as L
import la2a_joint_fit as J

G0 = 10 ** (-0.38 / 20)
F = int(0.05 * L.FS)
fit = json.load(open(sys.argv[1]))
CAL_BLOCK = 20


def held_out_mask(n_samples, step):
    t = np.arange(n_samples) * step / L.FS
    blk = np.floor((t - J.CAL[0]) / CAL_BLOCK).astype(int) % 4
    return ~((t >= J.CAL[0]) & (t < J.CAL[1]) & (blk == 0))


def render(x, limit, pr, model, tag):
    p = {"fetOn": 0, "optoOn": 1, "optoMode": limit, "optoPR": pr, "colorPct": 0, "scHpfHz": 20, "osLog2": 2}
    if model:
        p.update({f"m.{k}": v for k, v in model.items()})
    return L.render(x, p, tag)


rows = {}
for idx, name in L.PAIRS:
    limit, pr = L.settings(name)
    x, y = L.load(idx, name)
    lx, ly = L.frames_db(x, F), L.frames_db(y, F)
    mask = held_out_mask(len(lx), F) & (lx > -45)
    music = np.arange(len(lx)) * F / L.FS < 300
    res = {}
    for label, model in (("calibrated", fit["model"]), ("1.x law, same knobs", None)):
        z = render(x, limit, pr, model, f"rep{idx}{label[:3]}") * G0
        lz = L.frames_db(z, F)
        d = (lz - lx) - (ly - lx)
        r = {"gr_env_music_db": float(np.sqrt(np.mean(d[mask & music] ** 2))),
             "gr_env_noise_db": float(np.sqrt(np.mean(d[mask & ~music] ** 2))),
             "gr_bias_db": float(np.median(d[mask]))}
        # published metrics on held-out 10 s segments
        S = 10 * L.FS
        mae, st, lu = [], [], []
        for i in range(0, len(x) - S + 1, S):
            seg_t = (i + S / 2) / L.FS
            if J.CAL[0] <= seg_t < J.CAL[1]:
                continue  # segment metrics skip the whole calibration section
            a, b = z[i:i + S], y[i:i + S]
            if np.sqrt(np.mean(b.astype(np.float64) ** 2)) < 1e-4:
                continue
            mae.append(np.mean(np.abs(a - b)))
            st.append(L.mrstft(a, b))
            lu.append(abs(L.lufs(a) - L.lufs(b)))
        r.update({"mae": float(np.mean(mae)), "stft": float(np.mean(st)), "lufs": float(np.mean(lu)), "segments": len(mae)})
        res[label] = r
        print(f"{name} [{label}]: GR env music {r['gr_env_music_db']:.2f} dB, noise {r['gr_env_noise_db']:.2f} dB, "
              f"bias {r['gr_bias_db']:+.2f} | MAE {r['mae']:.2e} STFT {r['stft']:.3f} LUFS {r['lufs']:.3f} ({r['segments']} seg)", flush=True)
    rows[name] = res

summary = {}
for label in ("calibrated", "1.x law, same knobs"):
    summary[label] = {k: float(np.mean([rows[n][label][k] for n in rows])) for k in ("gr_env_music_db", "gr_env_noise_db", "mae", "stft", "lufs")}
summary["published (Steinmetz & Reiss 2021, Table 1)"] = {"mae": [7.66e-3, 1.2e-1], "stft": [0.551, 0.796], "lufs": [0.361, 1.305]}
print(json.dumps(summary, indent=1))
if len(sys.argv) > 2:
    json.dump({"per_file": rows, "summary": summary, "model": fit["model"]}, open(sys.argv[2], "w"), indent=1)
