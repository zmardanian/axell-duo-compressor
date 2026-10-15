"""Head-to-head benchmark: Axell Duo Compressor against Universal Audio's certified UADx emulations of the same
hardware (1176, LA-2A) and the online hardware references, all driven as
black boxes through DuoAuditHost (JUCE VST3 hosting; Windows plugins through yabridge) and measured with the same
numpy/scipy analysis. Nothing measured here is used to tune Axell Duo (clean-room).

    python3 bench.py [out.json]
Environment: DUO_AUDIT_HOST, BENCH_WORK (scratch dir), UAD_WINE (custom Wine for the UADx prefix, prepended to PATH).

Every compressor is set to ratio 4:1 (or its own law) and otherwise its defaults, then driven by its own input
control to the same gain reduction on a 1 kHz tone at -12 dBFS (6 dB; 10 dB for the stress tests). Settings are sent
before playback and again at the first sample (some plugins apply only changes made while processing).
"""
import json, math, os, sys
import numpy as np
from scipy import signal as sg
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "..", "tests", "independent"))
os.environ.setdefault("DUO_AUDIT_WORK", os.environ.get("BENCH_WORK", "/tmp/duo-bench"))
BASE_PATH = os.environ["PATH"]
UAD_WINE = os.environ.get("UAD_WINE", os.path.expanduser("~/opt/wine-uad-dcomp11/bin"))
import lib  # noqa: E402

V3 = os.path.expanduser("~/.vst3")
FS = 48000
LEAD = int(0.25 * FS)   # silence before every signal: the settings sent at sample 0 settle


class Plugin:
    def __init__(self, name, bundle, sub=None, sets=None, setns=None, drive=None, family="FET", note="", silent_sc=False):
        self.name, self.bundle, self.sub, self.family, self.note = name, bundle, sub, family, note
        self.silent_sc = silent_sc  # keep the side-chain bus enabled and silent (yabridge crashes when it is disabled)
        self.sets, self.setns = dict(sets or {}), dict(setns or {})
        self.drive = drive          # (kind 'set'|'setn', parameter, [sweep values])

    def run(self, rid, fs, value=None, ch=2, blocks="512", extra_sets=None):
        sets, setns = dict(self.sets), dict(self.setns)
        sets.update(extra_sets or {})
        if value is not None:
            kind, p, _ = self.drive
            (sets if kind == "set" else setns)[p] = value
        r = lib.Run(rid, fs, ch=ch, sets=sets, blocks=blocks, preroll=int(0.1 * fs))
        r.setns = setns
        r.ats = [(0, k, v) for k, v in sets.items()] + [(0, "#n#" + k, v) for k, v in setns.items()]
        return r

    def render(self, runs, inputs, sidechains=None, job="b"):
        # the UADx plugins live in a Wine prefix run by its own Wine build
        os.environ["PATH"] = (UAD_WINE + ":" + BASE_PATH) if "uaudio_" in self.bundle else BASE_PATH
        if self.silent_sc and sidechains is None:
            sidechains = {k: np.zeros((len(v), 2)) for k, v in inputs.items()}
        return lib.run_all(runs, inputs, sidechains, jobname=job, plugin=self.bundle, subplugin=self.sub, timeout=3600)


AXELL = f"{V3}/Axell Duo Compressor.vst3"
YB = f"{V3}/yabridge"
FET_PLUGINS = [
    Plugin("Axell Duo FET, Clean detector, Color 0", AXELL,
           sets={"Opto On": "Off", "Limiter On": "Off", "FET Detector": "Clean", "Color": "0", "FET Ratio": "4:1"},
           drive=("set", "FET Input", [str(v) for v in range(-12, 31, 4)])),
    Plugin("Axell Duo FET, Vintage detector, Color 30 (default)", AXELL,
           sets={"Opto On": "Off", "Limiter On": "Off", "FET Ratio": "4:1"},
           drive=("set", "FET Input", [str(v) for v in range(-12, 31, 4)])),
    Plugin("UADx 1176 FET Compressor", f"{YB}/uaudio_ua_1176.vst3", setns={"Ratio": "0"},
           drive=("setn", "Input", [f"{v:.3f}" for v in np.linspace(0.0, 0.5, 11)]), note="Windows, via yabridge/Wine"),
]
OPTO_PLUGINS = [
    Plugin("Axell Duo Opto, Measured law (default)", AXELL, sets={"FET On": "Off", "Limiter On": "Off"},
           drive=("set", "Opto Peak Reduction", [str(v) for v in range(0, 101, 10)]), family="opto"),
    Plugin("UADx LA-2A Tube Compressor", f"{YB}/uaudio_teletronix_la-2a_tc.vst3",
           drive=("setn", "Peak Reduct", [f"{v:.2f}" for v in np.linspace(0, 1, 11)]), family="opto", note="Windows, via yabridge/Wine"),
]


# ---------------------------------------------------------------- signals and analysis
def lead(x):
    return np.concatenate([np.zeros((LEAD, x.shape[1])), x])


def tone(f, dbfs, secs, fs=FS, ch=2):
    t = np.arange(int(secs * fs)) / fs
    return np.repeat((10 ** (dbfs / 20) * np.sin(2 * np.pi * f * t))[:, None], ch, 1)


def amp(y, f, fs=FS):
    t = np.arange(len(y)) / fs
    return float(np.hypot(2 * np.mean(y * np.sin(2 * np.pi * f * t)), 2 * np.mean(y * np.cos(2 * np.pi * f * t))))


def thdn_db(y, fs, f0):
    t = np.arange(len(y)) / fs
    A = np.stack([np.sin(2 * np.pi * f0 * t), np.cos(2 * np.pi * f0 * t), np.ones_like(t)], 1)
    c, *_ = np.linalg.lstsq(A, y, rcond=None)
    r = y - A @ c
    return 20 * np.log10(np.sqrt(np.mean(r ** 2)) / (np.hypot(c[0], c[1]) / np.sqrt(2)) + 1e-30)


def db(v):
    return 20 * math.log10(max(v, 1e-30))


def program(secs, fs=FS, seed=1):
    rng = np.random.default_rng(seed)
    n = int(secs * fs); t = np.arange(n) / fs
    env = 0.15 + 0.85 * np.abs(np.sin(2 * np.pi * 2.7 * t)) ** 3
    noise = sg.lfilter([0.2], [1, -0.8], rng.standard_normal(n))
    tonal = np.sin(2 * np.pi * 180 * t) + 0.5 * np.sin(2 * np.pi * 360 * t + 0.3) + 0.25 * np.sin(2 * np.pi * 2400 * t)
    x = 0.35 * env * (0.6 * tonal + 0.8 * noise)
    x *= 10 ** (-18 / 20) / np.sqrt(np.mean(x ** 2))
    return np.c_[x, np.roll(x, 37) * 0.9]


GR_SIG = lead(np.concatenate([tone(1000, -50, 1.5), tone(1000, -12, 2.0)]))


def gr_of(y):
    q = slice(LEAD + int(0.5 * FS), LEAD + int(1.5 * FS))
    l = slice(LEAD + int(2.5 * FS), LEAD + int(3.5 * FS))
    gq = amp(y[q, 0], 1000) / 10 ** (-50 / 20)
    gl = amp(y[l, 0], 1000) / 10 ** (-12 / 20)
    return db(gq) - db(gl)


def calibrate(p, target):
    """drive value giving `target` dB of GR (interpolated on the sweep), and the sweep itself"""
    kind, prm, vals = p.drive
    runs = [p.run(f"cal{i}", FS, v) for i, v in enumerate(vals)]
    out = p.render(runs, {r.name: GR_SIG for r in runs}, job="cal")
    grs = [gr_of(out[r.name]) for r in runs]
    xs = [float(v) for v in vals]
    best = None
    for i in range(len(xs) - 1):
        a, b = grs[i], grs[i + 1]
        if (a - target) * (b - target) <= 0 and a != b:
            best = xs[i] + (target - a) / (b - a) * (xs[i + 1] - xs[i])
            break
    if best is None:
        best = xs[int(np.argmin([abs(g - target) for g in grs]))]
    return best, list(zip(xs, grs))


def fmt(p, v):
    return f"{v:.4f}" if p.drive[0] == "setn" else f"{v:.2f}"


def scorecard(p):
    res = {"plugin": p.name, "family": p.family, "note": p.note}
    d6, sweep = calibrate(p, 6.0)
    d10, _ = calibrate(p, 10.0)
    res["sweep"] = sweep
    v6, v10 = fmt(p, d6), fmt(p, d10)
    sig = {
        "gr6": GR_SIG,
        "thd1k": lead(tone(1000, -12, 3.0)),
        "thd100": lead(tone(100, -12, 3.0)),
        "silence": np.zeros((LEAD + 2 * FS, 2)),
        "lat": lead(lib_noise(-40)),
        "prog_a": lead(program(6.0)), "prog_b": lead(program(6.0)), "prog_c": lead(program(6.0)),
        "burst": lead(np.concatenate([np.zeros((FS // 2, 2)), tone(1000, -6, 1.0)])),
    }
    runs = [p.run("gr6", FS, v6), p.run("thd1k", FS, v6), p.run("thd100", FS, v6), p.run("silence", FS, v6),
            p.run("lat", FS, v6), p.run("prog_a", FS, v6), p.run("prog_b", FS, v6),
            p.run("prog_c", FS, v6, blocks="random 1 4096 7"), p.run("burst", FS, v10)]
    out = p.render(runs, {r.name: sig[r.name] for r in runs}, job="sc")
    R = {r.name: r for r in runs}
    res["gr_at_6"] = gr_of(out["gr6"])
    w = slice(LEAD + int(1.5 * FS), LEAD + int(2.9 * FS))
    res["thdn_1k_db"] = thdn_db(out["thd1k"][w, 0], FS, 1000)
    res["thdn_100_db"] = thdn_db(out["thd100"][w, 0], FS, 100)
    s = out["silence"][LEAD + FS // 2:]
    res["noise_rms_dbfs"] = db(float(np.sqrt(np.mean(s ** 2))))
    res["noise_peak_dbfs"] = db(float(np.abs(s).max()))
    # latency honesty: lag of the cross-correlation peak vs the reported latency
    x, y = sig["lat"][:, 0], out["lat"][:, 0]
    n = 1 << int(np.ceil(np.log2(2 * len(x))))
    c = np.fft.irfft(np.fft.rfft(y, n) * np.conj(np.fft.rfft(x, n)), n)
    lag = int(np.argmax(np.abs(c[: FS // 4])))
    res["latency_reported"], res["latency_actual"] = R["lat"].latency, lag
    a, b, cc = out["prog_a"], out["prog_b"], out["prog_c"]
    res["repeat_diff_dbfs"] = db(float(np.abs(a - b).max()))
    res["blocksize_diff_dbfs"] = db(float(np.abs(a - cc).max()))
    res["cpu_pct_one_core"] = 100 * R["prog_a"].cpu / (len(sig["prog_a"]) / FS) if R["prog_a"].cpu else None
    yb = out["burst"][:, 0]
    on = LEAD + FS // 2 + R["burst"].latency
    steady = float(np.abs(yb[on + int(0.7 * FS): on + int(0.95 * FS)]).max())
    res["overshoot_db_at_10"] = db(float(np.abs(yb[on: on + int(0.003 * FS)]).max())) - db(steady)
    # aliasing: 15 kHz -3 dBFS at 44.1 kHz, driven at the 10 dB setting
    fs2 = 44100
    ra = p.run("alias", fs2, v10, ch=2)  # stereo: some plugins refuse mono
    ya = p.render([ra], {"alias": np.concatenate([np.zeros((int(0.25 * fs2), 2)), tone(15000, -3, 3.0, fs2, 2)])}, job="al")["alias"]
    ya = ya[int(1.75 * fs2): int(3.15 * fs2), 0]
    win = np.kaiser(len(ya), 20); Y = np.abs(np.fft.rfft(ya * win)) / win.sum() * 2
    f = np.fft.rfftfreq(len(ya), 1 / fs2)
    mask = (f > 20) & np.all([np.abs(f - 15000 * k) > 60 for k in (1, 2, 3)], axis=0)
    res["alias_worst_dbfs"] = db(float(Y[mask].max()))
    res["drive_6db"], res["drive_10db"] = v6, v10
    return res


def lib_noise(dbfs, secs=2.0, seed=3):
    x = np.random.default_rng(seed).standard_normal((int(secs * FS), 2))
    x = sg.sosfiltfilt(sg.butter(8, 16000, fs=FS, output="sos"), x, axis=0)
    return x * 10 ** (dbfs / 20) / np.sqrt(np.mean(x ** 2))


if __name__ == "__main__":
    results = []
    for p in FET_PLUGINS + OPTO_PLUGINS:
        print(f"== {p.name}", flush=True)
        try:
            r = scorecard(p)
        except Exception as e:  # a plugin that fails to load or run is reported, not hidden
            r = {"plugin": p.name, "error": str(e)[-500:]}
        print(json.dumps({k: v for k, v in r.items() if k != "sweep"}, indent=None, default=str), flush=True)
        results.append(r)
    json.dump(results, open(sys.argv[1] if len(sys.argv) > 1 else "bench.json", "w"), indent=1, default=str)
