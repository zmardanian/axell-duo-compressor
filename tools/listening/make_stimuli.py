"""Build the blind-test stimuli (tools/listening/stimuli/): hardware recordings vs Axell Duo Compressor renders of
the same input, loudness-matched (BS.1770 integrated), 16-bit WAV, from parts of the recordings that were NOT
used for calibration.

    python3 make_stimuli.py <opto_model.json> <fet_input_gain_db> [outdir]

LA-2A: SignalTrain Test split (CC BY 4.0), excerpts from minutes 0-3 (music; calibration used minutes 3-7).
1176: Martinez Ramirez et al. (CC BY 4.0), three held-out (odd-numbered) notes per trial.
"""
import json, os, subprocess, sys
import numpy as np
from scipy.io import wavfile
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "validation"))
import la2a_compare as L
import fet1176_compare as M

RENDER = os.environ.get("DUO_RENDER", L.RENDER)
model = json.load(open(sys.argv[1]))["model"]
fet_in = float(sys.argv[2])
out = sys.argv[3] if len(sys.argv) > 3 else os.path.join(os.path.dirname(__file__), "stimuli")
os.makedirs(out, exist_ok=True)
FS = 44100
manifest = {"version": 1, "abx": [], "mushra": []}


def wav(name, x):
    x = np.asarray(x, np.float64)
    wavfile.write(os.path.join(out, name), FS, np.clip(np.round(x * 32767), -32768, 32767).astype(np.int16))
    return name


def match(x, ref):
    """gain x so its integrated loudness equals ref's; fade 20 ms in/out"""
    g = 10 ** ((L.lufs(ref) - L.lufs(x)) / 20)
    y = x * g
    f = int(0.02 * FS)
    ramp = np.sin(np.linspace(0, np.pi / 2, f)) ** 2
    y[:f] *= ramp; y[-f:] *= ramp[::-1]
    return y


def trial(names_and_signals):
    """one common gain per trial: about -20 LUFS, every version's peak <= -1 dBFS (no clipping cue)"""
    ref = names_and_signals[0][1]
    peak = max(np.abs(x).max() for _, x in names_and_signals)
    g = min(10 ** ((-20.0 - L.lufs(ref)) / 20), 10 ** (-1 / 20) / peak)
    return [wav(n, x * g) for n, x in names_and_signals]


def render_full(x, params, tag):
    fi, fo = f"{L.WORK}/st_{tag}_in.f32", f"{L.WORK}/st_{tag}_out.f32"
    os.makedirs(L.WORK, exist_ok=True)
    x.astype(np.float32).tofile(fi)
    subprocess.run([RENDER, fi, fo, str(FS), "1"] + [f"{k}={v}" for k, v in params.items()], check=True, stderr=subprocess.DEVNULL)
    z = np.fromfile(fo, np.float32).astype(np.float64)
    os.remove(fi); os.remove(fo)
    return z


# ---- LA-2A: excerpts with clear, varying gain reduction from minutes 0-3 of each file
picked = {235: [20, 95], 256: [40, 130], 259: [10, 70]}   # start seconds (music sections)
for idx, name in L.PAIRS:
    limit, pr = L.settings(name)
    x, y = L.load(idx, name)
    x3, y3 = x[: 180 * FS].astype(np.float64), y[: 180 * FS].astype(np.float64)
    base = {"fetOn": 0, "optoOn": 1, "optoMode": limit, "optoPR": pr, "colorPct": 0, "scHpfHz": 20, "osLog2": 2}
    hw_model = render_full(x3, {**base, **{f"m.{k}": v for k, v in model.items()}}, f"h{idx}")
    # the 1.x law at the Peak Reduction that gives the same median gain reduction on these 3 minutes
    F = int(0.05 * FS)
    lx, ly = L.frames_db(x3, F), L.frames_db(y3, F)
    act = lx > -45
    target = np.median((ly - lx)[act])
    lo, hi = 0.0, 100.0
    for _ in range(12):
        mid = 0.5 * (lo + hi)
        zc = render_full(x3, {**base, "optoPR": mid}, f"c{idx}")
        (lo, hi) = (mid, hi) if np.median((L.frames_db(zc, F) - lx)[act]) > target - 0.38 else (lo, mid)
    classic = render_full(x3, {**base, "optoPR": 0.5 * (lo + hi)}, f"c{idx}")
    for k, s in enumerate(picked[idx]):
        sl = slice(s * FS, (s + 7) * FS)
        hw = y3[sl]
        tid = f"la2a_{idx}_{k}"
        mode = "limit" if limit else "compress"
        trial([(f"{tid}_hw.wav", match(hw.copy(), hw)), (f"{tid}_model.wav", match(hw_model[sl], hw)),
               (f"{tid}_classic.wav", match(classic[sl], hw))])
        manifest["abx"].append({"id": tid, "group": "LA-2A", "label": f"Opto · {mode} · Peak Reduction {pr:g}",
                                "hw": f"{tid}_hw.wav", "model": f"{tid}_model.wav"})
        if k == 0:
            manifest["mushra"].append({"id": tid, "hw": f"{tid}_hw.wav",
                                       "conditions": [{"id": "axell_hardware_model", "file": f"{tid}_model.wav"},
                                                      {"id": "axell_1x_model", "file": f"{tid}_classic.wav"}]})

# ---- 1176: four trials of three held-out notes each (guitar and bass)
notes = M.notes()[1::2]
rng = np.random.default_rng(7)
bass = [n for n in notes if os.path.basename(n[1]).startswith("B")]
gtr = [n for n in notes if os.path.basename(n[1]).startswith("G")]
for k, pool in enumerate([bass, gtr, bass, gtr]):
    sel = [pool[i] for i in rng.choice(len(pool), 3, replace=False)]
    xs = [M.load(f) for _, f, _ in sel]
    ys = [M.load(g) for _, _, g in sel]
    zs = M.render_batch(xs, M.model_params(fet_in, colorPct=30, osLog2=2), f"st{k}")
    gap = np.zeros(int(0.25 * FS))
    hw = np.concatenate(sum(([y.astype(np.float64), gap] for y in ys), []))
    md = np.concatenate(sum(([z.astype(np.float64), gap] for z in zs), []))
    tid = f"fet1176_{k}"
    trial([(f"{tid}_hw.wav", match(hw.copy(), hw)), (f"{tid}_model.wav", match(md, hw))])
    manifest["abx"].append({"id": tid, "group": "1176", "label": f"FET · all buttons · {'bass' if k % 2 == 0 else 'guitar'} notes",
                            "hw": f"{tid}_hw.wav", "model": f"{tid}_model.wav"})

json.dump(manifest, open(os.path.join(out, "manifest.json"), "w"), indent=1)
print(f"{len(manifest['abx'])} ABX trials, {len(manifest['mushra'])} rating screens -> {out}")
