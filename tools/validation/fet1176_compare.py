"""Axell Duo Compressor - FET stage vs a hardware 1176LN (limiter section of a Universal Audio 6176 channel
strip). Data: Martinez Ramirez, Benetos, Reiss, "Deep Learning for Black-Box Modeling of Audio Effects",
Appl. Sci. 10(2) 638, 2020; Zenodo 3562442, CC BY 4.0. 1186 dry guitar/bass notes (IDMT-SMT-Audio-Effects) and
the same notes through the hardware at one setting: attack 800 us, release 1100 ms, input 4, output 7, ratio ALL.

Every file is peak-normalised, so absolute levels are lost. Compared is the gain trajectory over each note:
g(t) = frame level of the output - frame level of the input (5 ms frames, dB), up to a constant per note
(removed). The unknown level into the hardware is one global input gain, fitted on the even-numbered notes;
the odd-numbered notes are the held-out evaluation.

    python3 fet1176_compare.py [--json out.json]
Environment: DUO_RENDER, MDPI_DIR (folder with dry/ and limiter/), WORK.
"""
import glob, json, os, subprocess, sys
import numpy as np
from scipy.io import wavfile
from scipy.optimize import minimize_scalar

RENDER = os.environ.get("DUO_RENDER", os.path.expanduser("~/axell-duo-compressor/build/duo_render"))
DATA = os.environ.get("MDPI_DIR", os.path.expanduser("~/axell-duo-data/mdpi-1176"))
WORK = os.environ.get("WORK", "/tmp/duo-1176")
FS = 44100
F = int(0.005 * FS)


def notes():
    out = []
    for f in sorted(glob.glob(f"{DATA}/dry/*.wav")):
        b = os.path.basename(f)[:-4]
        g = f"{DATA}/limiter/{b}_limiter.wav"
        if os.path.exists(g):
            out.append((b, f, g))
    return out


def load(f):
    _, x = wavfile.read(f)
    return x.astype(np.float32) / 32768.0


def frames_db(x):
    m = len(x) // F
    return 10 * np.log10(np.mean(x[: m * F].astype(np.float64).reshape(m, F) ** 2, 1) + 1e-20)


def render_batch(xs, params, tag):
    """renders notes one after another with 4 s of silence between (state settles), returns per-note outputs"""
    os.makedirs(WORK, exist_ok=True)
    gap = np.zeros(4 * FS, np.float32)
    seq, pos = [], []
    p = 0
    for x in xs:
        seq += [x, gap]
        pos.append(p)
        p += len(x) + len(gap)
    fi, fo = f"{WORK}/{tag}_in.f32", f"{WORK}/{tag}_out.f32"
    np.concatenate(seq).tofile(fi)
    subprocess.run([RENDER, fi, fo, str(FS), "1"] + [f"{k}={v}" for k, v in params.items()], check=True,
                   stderr=subprocess.DEVNULL)
    z = np.fromfile(fo, np.float32)
    return [z[q:q + len(x)] for q, x in zip(pos, xs)]


def traj_error(x, y, z):
    """dB RMS between hardware and model gain trajectories (per-note constant removed), active frames"""
    lx, ly, lz = frames_db(x), frames_db(y), frames_db(z)
    act = lx > lx.max() - 40
    gh, gm = (ly - lx)[act], (lz - lx)[act]
    d = (gm - gm.mean()) - (gh - gh.mean())
    return float(np.sqrt(np.mean(d ** 2))), float(np.sqrt(np.mean((gh - gh.mean()) ** 2)))


def model_params(in_db, **kw):
    p = {"fetOn": 1, "optoOn": 0, "fetRatio": 4, "fetAtkMs": 0.8, "fetRelMs": 1100, "fetInDb": in_db, "fetOutDb": 0,
         "colorPct": 30, "scHpfHz": 20, "osLog2": 1}
    p.update(kw)
    return p


def evaluate(items, in_db, tag, **kw):
    xs = [load(f) for _, f, _ in items]
    ys = [load(g) for _, _, g in items]
    zs = render_batch(xs, model_params(in_db, **kw), tag)
    e = [traj_error(x, y, z) for x, y, z in zip(xs, ys, zs)]
    return np.array([a for a, _ in e]), np.array([b for _, b in e])


if __name__ == "__main__":
    all_notes = notes()
    cal, ev = all_notes[0::2], all_notes[1::2]
    print(f"{len(all_notes)} note pairs: {len(cal)} calibration, {len(ev)} held-out", flush=True)
    sub = cal[::4]  # fit on a quarter of the calibration notes (speed), check on all of them
    f = lambda d: float(np.mean(evaluate(sub, d, "fit")[0]))
    r = minimize_scalar(f, bounds=(-12.0, 30.0), method="bounded", options={"xatol": 0.1})
    in_db = float(r.x)
    e_cal, _ = evaluate(cal, in_db, "cal")
    e_ev, base_ev = evaluate(ev, in_db, "ev")
    res = {"input_gain_db": in_db, "cal_err_db_mean": float(e_cal.mean()), "eval_err_db_mean": float(e_ev.mean()),
           "eval_err_db_median": float(np.median(e_ev)), "eval_err_db_p90": float(np.percentile(e_ev, 90)),
           "baseline_no_dynamics_db_mean": float(base_ev.mean()), "notes_eval": len(ev)}
    for k, v in res.items():
        print(f"  {k}: {v:.3f}" if isinstance(v, float) else f"  {k}: {v}")
    if "--json" in sys.argv:
        json.dump(res, open(sys.argv[sys.argv.index("--json") + 1], "w"), indent=1)
