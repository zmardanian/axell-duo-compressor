"""Independent black-box audit of the built Axell Duo Compressor VST3 through a real VST3 host (DuoAuditHost), with
analysis written here from textbook definitions (numpy/scipy) - nothing uses the plugin's own code.
Exit code = number of failures. Environment: see lib.py; DUO_V1_VST3 = the Duo Vocal Comp 1.0.0 bundle (optional,
for the compatibility check)."""
import math, os, re, sys
import numpy as np
from scipy import signal as sg
from lib import *

results = []
def report(name, ok, detail):
    results.append((name, ok, detail))
    print(f"{'PASS' if ok else 'FAIL'}  {name}: {detail}", flush=True)

def noise(n, ch, seed=1, amp=0.25):
    return np.random.default_rng(seed).uniform(-amp, amp, (n, ch))

def lowpassed(x, fs, fc):
    return sg.sosfiltfilt(sg.butter(12, fc, fs=fs, output="sos"), x, axis=0)

def tone(f, amp, secs, fs, ch=2):
    t = np.arange(int(fs * secs)) / fs
    return np.repeat((amp * np.sin(2 * np.pi * f * t))[:, None], ch, axis=1)

def thdn_db(y, fs, f0):
    t = np.arange(len(y)) / fs
    A = np.stack([np.sin(2 * np.pi * f0 * t), np.cos(2 * np.pi * f0 * t), np.ones_like(t)], 1)
    c, *_ = np.linalg.lstsq(A, y, rcond=None)
    r = y - A @ c
    return 20 * np.log10(np.sqrt(np.mean(r ** 2)) / (np.hypot(c[0], c[1]) / np.sqrt(2)) + 1e-30)

def true_peak(y, up=32):
    """band-limited (FFT) interpolation of the whole signal: the ideal reconstruction's peak"""
    n = len(y)
    return float(np.abs(sg.resample(y, n * up)).max())

def db(a):
    return 20 * np.log10(max(a, 1e-30))

OFF = {"FET On": "Off", "Opto On": "Off", "Color": "0"}
V1 = os.environ.get("DUO_V1_VST3", os.path.expanduser(
    "~/duo-vocal-comp-release/DuoVocalComp-1.0.0-linux-x86_64/Duo Vocal Comp.vst3"))

# ---------------------------------------------------------------- 1. silence, transparency, reported latency
fs = 48000
runs, ins = [], {}
r = Run("sil", fs, sets={}); runs.append(r); ins["sil"] = np.zeros((fs, 2))
for os_ in ["1x", "2x", "4x"]:
    r = Run(f"tr{os_}", fs, sets={**OFF, "Oversampling": os_}); runs.append(r)
    ins[r.name] = lowpassed(noise(fs, 2, 3), fs, 20000)
out = run_all(runs, ins, jobname="a1")
report("silence in -> silence out (defaults)", np.abs(out["sil"]).max() == 0.0, f"max |y| {np.abs(out['sil']).max():.3g}")
for os_, lat_exp in [("1x", 0), ("2x", None), ("4x", None)]:
    rr = [r for r in runs if r.name == f"tr{os_}"][0]
    x, y, L = ins[rr.name], out[rr.name], rr.latency
    e = np.abs(y[L + 2000:] - x[2000:len(x) - L]).max()
    report(f"stages off, {os_}: output = input delayed by the reported latency ({L}) (<20 kHz)", db(e) < -100, f"max error {db(e):.1f} dBFS")

# ---------------------------------------------------------------- 2. Duo Vocal Comp 1.0.0 compatibility
if os.path.isdir(V1):
    cases = {
        "defaults @2x": {"Oversampling": "2x"},
        "rap lead-like": {"Oversampling": "2x", "FET Input": "1", "FET Output": "4", "FET Attack": "250", "FET Release": "120",
                          "Opto Peak Reduction": "16.5", "Opto Gain": "3.5", "Sidechain HPF": "100", "Color": "35", "Opto HF Emphasis": "20"},
        "all-buttons parallel, opto first, 1x": {"Oversampling": "1x", "FET Ratio": "All", "FET Input": "9", "Mix": "45", "Order": "Opto > FET",
                                                  "Opto Mode": "Limit", "Color": "60", "Stereo Link": "40", "Auto Makeup": "On"},
        "4x, 20:1, opto off": {"Oversampling": "4x", "FET Ratio": "20:1", "FET Attack": "20", "FET Input": "20", "Opto On": "Off", "Output": "-3"},
    }
    sig = lowpassed(noise(fs * 4, 2, 9, 0.6), fs, 6000) * np.repeat(
        (0.2 + 0.8 * np.abs(np.sin(2 * np.pi * 1.7 * np.arange(fs * 4) / fs)))[:, None], 2, 1)
    # a 1.x project stores every 1.x value and loads with Opto Model = Classic (state restore: plugin suite); on a
    # fresh 2.0 instance that means: Peak Reduction as the 1.x default unless set, Opto Model Classic
    for s_ in cases.values():
        s_.setdefault("Opto Peak Reduction", "30")
    r1 = [Run(f"v1_{i}", fs, sets=s) for i, s in enumerate(cases.values())]
    r2 = [Run(f"v2_{i}", fs, sets={**s, "Opto Model": "Classic"}) for i, s in enumerate(cases.values())]
    o1 = run_all(r1, {r.name: sig for r in r1}, jobname="v1", plugin=V1)
    o2 = run_all(r2, {r.name: sig for r in r2}, jobname="v2")
    for i, k in enumerate(cases):
        d = np.abs(o1[f"v1_{i}"] - o2[f"v2_{i}"]).max()
        report(f"1.0.0 session compatibility: '{k}' renders like Duo Vocal Comp 1.0.0",
               d == 0.0 and r1[i].latency == r2[i].latency, f"max |difference| {d:.3g}, latency {r1[i].latency} / {r2[i].latency}")
else:
    report("1.0.0 compatibility", False, f"v1 bundle not found at {V1}")

# ---------------------------------------------------------------- 3. Clean detector: bass distortion
runs, ins = [], {}
for f0 in (30.0, 100.0):
    for det in ("Vintage", "Clean"):
        r = Run(f"det{int(f0)}{det}", fs, ch=1, sets={"Opto On": "Off", "Color": "0", "FET Attack": "20", "FET Release": "50",
                                                     "FET Input": "12", "FET Detector": det})
        runs.append(r); ins[r.name] = tone(f0, 0.5, 3.0, fs, 1)
out = run_all(runs, ins, jobname="det")
for f0 in (30, 100):
    v = thdn_db(out[f"det{f0}Vintage"][int(1.5 * fs):int(2.9 * fs), 0], fs, f0)
    c = thdn_db(out[f"det{f0}Clean"][int(1.5 * fs):int(2.9 * fs), 0], fs, f0)
    report(f"Clean detector, {f0} Hz, 20 us / 50 ms, 4:1, ~6 dB GR: THD+N", c < -90, f"Clean {c:.1f} dB (Vintage {v:.1f} dB)")

# ---------------------------------------------------------------- 4. aliasing at the 4x default
fs2 = 44100
r = Run("alias", fs2, ch=1, sets={"Opto On": "Off", "FET Ratio": "20:1", "FET Input": "24", "Color": "100"})
y = run_all([r], {"alias": tone(15000, 0.708, 3.0, fs2, 1)}, jobname="al")["alias"][int(1.5 * fs2):int(2.9 * fs2), 0]
w = np.kaiser(len(y), 20); Y = np.abs(np.fft.rfft(y * w)) / w.sum() * 2; f = np.fft.rfftfreq(len(y), 1 / fs2)
mask = (f > 20) & np.all([np.abs(f - 15000 * k) > 60 for k in (1, 2, 3)], axis=0)
report("4x (default): worst non-harmonic component, 15 kHz -3 dBFS, 20:1 +24 dB, Color 100", db(Y[mask].max()) < -120,
       f"{db(Y[mask].max()):.1f} dBFS")

# ---------------------------------------------------------------- 5. true-peak limiter
for fsl in (44100, 48000):
    n = fsl * 6
    rng = np.random.default_rng(5)
    t = np.arange(n) / fsl
    beat = np.mod(t, 0.5)
    kick = np.exp(-beat * 18) * np.sin(2 * np.pi * (55 + 90 * np.exp(-beat * 40)) * beat)
    snare = np.exp(-np.mod(t + 0.25, 0.5) * 30) * rng.standard_normal(n) * 0.5
    hats = np.exp(-np.mod(t, 0.125) * 80) * rng.standard_normal(n) * 0.2
    full = np.c_[kick + snare + hats, kick + 0.9 * snare + 1.1 * hats] * 0.7
    limited = lowpassed(full, fsl, 0.47 * fsl)
    sets = {**OFF, "Limiter On": "On", "Limiter Ceiling": "-1", "Output": "12", "Mix": "100"}
    rs = [Run(f"lim{fsl}{k}", fsl, sets=sets) for k in ("bl", "fb", "fb1x")]
    rs[2].sets = {**sets, "Oversampling": "1x"}
    out = run_all(rs, {rs[0].name: limited, rs[1].name: full, rs[2].name: full}, jobname="lim")
    for r_, what in zip(rs, ("content <= 0.47 fs", "full band, 4x", "full band, 1x (no oversampling filter)")):
        y = out[r_.name][fsl:]
        sp = db(np.abs(y).max())
        tp = max(db(true_peak(y[:, c])) for c in range(2))
        if "0.47" in what:
            report(f"limiter -1 dBTP, drums +12 dB, {fsl} Hz, {what}: no sample and no true peak above the ceiling",
                   sp <= -1.0 and tp <= -1.0, f"sample peak {sp:.3f} dBFS, true peak {tp:.3f} dBTP (ideal reconstruction)")
        else:
            # documented bound (DECISIONS D21): content above 0.48 fs cannot be predicted exactly by a low-latency
            # interpolator; at 2x/4x the oversampling filters remove most of it (bound +0.4 dB), at 1x nothing does
            # (bound +0.7 dB). Sample peaks are always held.
            bound = 0.7 if "1x" in what else 0.4
            report(f"limiter -1 dBTP, drums +12 dB, {fsl} Hz, {what}: sample peak <= ceiling, true peak <= ceiling + {bound} dB (documented)",
                   sp <= -1.0 and tp <= -1.0 + bound, f"sample peak {sp:.3f} dBFS, true peak {tp:.3f} dBTP")

# ---------------------------------------------------------------- 6. lookahead: latency and overshoot
fs = 48000
x = np.zeros((fs, 2)); x[fs // 2 - 2400:] = tone(1000, 0.5, 1, fs)[: fs // 2 + 2400]
rs = [Run(f"la{v}", fs, sets={"Opto On": "Off", "FET Ratio": "20:1", "FET Attack": "800", "FET Input": "24", "Lookahead": str(v)})
      for v in (0, 2, 5)]
out = run_all(rs, {r.name: x for r in rs}, jobname="la")
base = rs[0].latency
for r_ in rs[1:]:
    v = float(r_.name[2:])
    report(f"lookahead {v:g} ms: reported latency = 4x latency + {v:g} ms", r_.latency == base + round(v * fs / 1000),
           f"{r_.latency} = {base} + {r_.latency - base}")
ov = []
for r_ in rs:
    y = out[r_.name][:, 0]
    ov.append(db(np.abs(y).max()) - db(np.abs(y[-fs // 10:]).max()))
report("lookahead 2 ms: onset overshoot of a 20:1 / 800 us FET reduced by > 10 dB", ov[1] < ov[0] - 10,
       f"overshoot {ov[0]:.1f} dB -> {ov[1]:.1f} dB (5 ms: {ov[2]:.1f} dB)")

# ---------------------------------------------------------------- 7. external side-chain through the host's bus
fs = 48000
track = tone(1000, 10 ** (-30 / 20), 3, fs)
key = tone(120, 10 ** (-3 / 20), 3, fs)
rs = [Run("scK", fs, sets={"Opto On": "Off", "Color": "0", "FET Key": "External"}),
      Run("scQ", fs, sets={"Opto On": "Off", "Color": "0", "FET Key": "External"}),
      Run("scI", fs, sets={"Opto On": "Off", "Color": "0", "FET Key": "Internal"})]
out = run_all(rs, {r.name: track for r in rs}, sidechains={"scK": key, "scQ": key * 0.001, "scI": key})
lv = {k: db(np.sqrt(2 * np.mean(out[k][-fs:, 0] ** 2))) for k in ("scK", "scQ", "scI")}
report("external key: loud key on the side-chain bus ducks the track", lv["scQ"] - lv["scK"] > 6,
       f"track {lv['scQ']:.2f} dBFS with a quiet key, {lv['scK']:.2f} dBFS with a loud key")
report("internal key: the side-chain bus is ignored", abs(lv["scI"] - lv["scQ"]) < 0.05, f"{lv['scI']:.2f} vs {lv['scQ']:.2f} dBFS")

# ---------------------------------------------------------------- 8. Mid/Side
mono = np.repeat(lowpassed(noise(fs * 2, 1, 4, 0.8), fs, 5000), 2, 1)
side = np.c_[0.5 * tone(2000, 1, 2, fs, 1), -0.5 * tone(2000, 1, 2, fs, 1)] + np.repeat(tone(500, 0.01, 2, fs, 1), 2, 1)
rs = [Run("msS", fs, sets={"Opto On": "Off"}), Run("msM", fs, sets={"Opto On": "Off", "Channel Mode": "Mid/Side"}),
      Run("msL", fs, sets={"Opto On": "Off", "Channel Mode": "Mid/Side", "Stereo Link": "0", "Color": "0"})]
out = run_all(rs, {"msS": mono, "msM": mono, "msL": side}, jobname="ms")
d = np.abs(out["msS"] - out["msM"]).max()
report("Mid/Side on identical channels = stereo mode", d < 1e-5, f"max |difference| {d:.3g}")
yl = out["msL"][-fs:]
mid, sid = 0.5 * (yl[:, 0] + yl[:, 1]), 0.5 * (yl[:, 0] - yl[:, 1])
def amp_at(y, f0):
    t = np.arange(len(y)) / fs
    return np.hypot(2 * np.mean(y * np.sin(2 * np.pi * f0 * t)), 2 * np.mean(y * np.cos(2 * np.pi * f0 * t)))
gm, gs = db(amp_at(mid, 500)) - db(0.01) - 6, db(amp_at(sid, 2000)) - db(0.5) - 6
report("Mid/Side, link 0: loud side compressed, quiet mid untouched", abs(gm) < 0.05 and gs < -3,
       f"mid gain {gm:+.2f} dB, side gain {gs:+.2f} dB (re +6 dB FET input)")

# ---------------------------------------------------------------- 9. live latency changes in a real-time host
fs = 48000
seq = [("Lookahead", "3"), ("Limiter On", "On"), ("Lookahead", "0"), ("Oversampling", "1x"), ("Limiter On", "Off"),
       ("Oversampling", "4x"), ("Lookahead", "7.5")]
# band-limited noise: the 2x/4x oversampling filters change content above 0.45 fs, which is not misalignment
x = lowpassed(noise(int(fs * 0.5 * (len(seq) + 1)), 1, 14), fs, 0.4 * fs)
r = Run("pdc", fs, ch=1, sets={**OFF, "Limiter Ceiling": "0"}, ats=[((i + 1) * (fs // 2), k, v) for i, (k, v) in enumerate(seq)],
        nonrt=False, pdc=2, preroll=fs // 4)
y = run_all([r], {"pdc": x}, jobname="pdc")["pdc"][:, 0]
ev = [(int(a), int(b)) for a, b in re.findall(r"pdc (\d+) (-?\d+)", r.log)]
Lh = np.zeros(len(y), int)
for pos, lat in ev:
    Lh[pos:] = lat
idx = np.arange(len(y)) - Lh
xd = np.where(idx >= 0, x[np.clip(idx, 0, None), 0], 0.0)
W = 16; nw = len(y) // W
Y, X = y[:nw * W].reshape(nw, W), xd[:nw * W].reshape(nw, W)
g = (Y * X).sum(1) / np.maximum((X * X).sum(1), 1e-12)
res = np.abs(Y - g[:, None] * X).max(1)
bad = (res > 0.02) | (g < -0.01) | (g > 1.01)
audible = (np.abs(Y).max(1) > 1e-3).mean()
lats = sorted(set(l for _, l in ev))
report("live latency changes (lookahead, limiter, oversampling): output always aligned with the host's compensation",
       bad.sum() == 0 and audible > 0.6 and len(lats) >= 4,
       f"{len(ev)} host latency updates {lats}, misaligned 16-sample windows {int(bad.sum())}, audible {100 * audible:.0f} %")

# ---------------------------------------------------------------- 10. host block-size invariance with every 2.0 feature on
fs = 44100
prog = lowpassed(noise(fs * 3, 2, 21, 0.7), fs, 9000) * np.repeat((0.3 + 0.7 * np.abs(np.sin(np.arange(fs * 3) / fs * 9)))[:, None], 2, 1)
sets = {"FET Detector": "Clean", "FET Hold": "20", "Opto Range": "8", "Sidechain LPF": "8000", "SC EQ Gain": "4",
        "Lookahead": "2", "Limiter On": "On", "Limiter Ceiling": "-3", "Channel Mode": "Mid/Side"}
rs = [Run("b512", fs, sets=sets, blocks="512"), Run("brand", fs, sets=sets, blocks="random 1 4096 7")]
out = run_all(rs, {r.name: prog for r in rs}, jobname="blk")
d = np.abs(out["b512"] - out["brand"]).max()
report("every 2.0 feature on: fixed vs random host block sizes (1..4096)", d < 1e-6, f"max |difference| {d:.3g}")

fails = [r for r in results if not r[1]]
print(f"\nAUDIT: {len(results) - len(fails)}/{len(results)} pass")
sys.exit(len(fails))
