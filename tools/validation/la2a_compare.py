"""Axell Duo Compressor - opto stage vs a hardware Teletronix LA-2A (SignalTrain dataset, Hawley et al.,
Zenodo 3824876, CC BY 4.0: the 3 input/target pairs of its Test split, 15 min each, 44.1 kHz).

    python3 la2a_compare.py [--pr-map identity|fit] [--json out.json]

Environment: DUO_RENDER (path of build/duo_render), LA2A_DIR (the dataset's Test folder), WORK (scratch dir).

Metrics, as published for this dataset (Steinmetz & Reiss 2021, arXiv 2102.06200, Table 1):
  MAE  - mean absolute error of the time-domain signal
  STFT - multi-resolution STFT error (spectral convergence + log-magnitude L1; FFT 1024/2048/512, hop
         120/240/50, window 600/1200/240 - the auraloss defaults used there)
  LUFS - |integrated loudness (BS.1770) of model - of hardware|
computed on consecutive 10 s segments and averaged. Plus a compression-specific measure:
  GR   - gain-reduction envelope error: frame (10 ms) level of output minus input, model vs hardware (dB RMS
         over frames with input above -45 dBFS), after one static gain offset per file (the hardware's Gain
         knob setting is not part of the data).
"""
import json, os, subprocess, sys
import numpy as np
from scipy.io import wavfile
from scipy import signal as sg

RENDER = os.environ.get("DUO_RENDER", os.path.expanduser("~/axell-duo-compressor/build/duo_render"))
LA2A = os.environ.get("LA2A_DIR", os.path.expanduser("~/axell-duo-data/la2a/SignalTrain_LA2A_Dataset_1.1/Test"))
WORK = os.environ.get("WORK", "/tmp/duo-la2a")
PAIRS = [(235, "target_235_LA2A_2c__0__65"), (256, "target_256_LA2A_2c__1__65"), (259, "target_259_LA2A_2c__1__80")]
FS = 44100


def load(idx, name):
    _, x = wavfile.read(f"{LA2A}/input_{idx}_.wav", mmap=True)
    _, y = wavfile.read(f"{LA2A}/{name}.wav", mmap=True)
    n = min(len(x), len(y))
    return np.asarray(x[:n], np.float32), np.asarray(y[:n], np.float32)


def settings(name):
    # SignalTrain file names: LA2A_2c__<limit switch>__<peak reduction>
    parts = name.split("__")
    return int(parts[1]), float(parts[2])


def render(x, params, tag):
    os.makedirs(WORK, exist_ok=True)
    fi, fo = f"{WORK}/{tag}_in.f32", f"{WORK}/{tag}_out.f32"
    if not os.path.exists(fi):
        x.astype(np.float32).tofile(fi)
    args = [RENDER, fi, fo, str(FS), "1"] + [f"{k}={v}" for k, v in params.items()]
    subprocess.run(args, check=True, stderr=subprocess.DEVNULL)
    return np.fromfile(fo, np.float32)


def frames_db(x, F):
    m = len(x) // F
    return 10 * np.log10(np.mean(x[: m * F].astype(np.float64).reshape(m, F) ** 2, 1) + 1e-20)


# --- BS.1770 loudness (mono), gated -------------------------------------------------------------
def k_weight(x):
    # pre-filter (high shelf) + RLB high-pass, coefficients for 48 kHz re-derived for fs (bilinear, BS.1770-4)
    f0, G, Q = 1681.974450955533, 3.999843853973347, 0.7071752369554196
    K = np.tan(np.pi * f0 / FS); Vh = 10 ** (G / 20); Vb = Vh ** 0.4996667741545416
    a0 = 1 + K / Q + K * K
    b1 = [(Vh + Vb * K / Q + K * K) / a0, 2 * (K * K - Vh) / a0, (Vh - Vb * K / Q + K * K) / a0]
    a1 = [1, 2 * (K * K - 1) / a0, (1 - K / Q + K * K) / a0]
    f0, Q = 38.13547087602444, 0.5003270373238773
    K = np.tan(np.pi * f0 / FS)
    a2 = [1, 2 * (K * K - 1) / (1 + K / Q + K * K), (1 - K / Q + K * K) / (1 + K / Q + K * K)]
    b2 = [1, -2, 1]
    return sg.lfilter(b2, a2, sg.lfilter(b1, a1, x))


def lufs(x):
    y = k_weight(x.astype(np.float64))
    B, H = int(0.4 * FS), int(0.1 * FS)
    if len(y) < B:
        return -70.0
    z = np.array([np.mean(y[i:i + B] ** 2) for i in range(0, len(y) - B + 1, H)])
    L = -0.691 + 10 * np.log10(z + 1e-20)
    z = z[L > -70]
    if len(z) == 0:
        return -70.0
    rel = -0.691 + 10 * np.log10(np.mean(z)) - 10
    z = z[(-0.691 + 10 * np.log10(z + 1e-20)) > rel]
    return -0.691 + 10 * np.log10(np.mean(z) + 1e-20)


# --- multi-resolution STFT error (auraloss defaults) --------------------------------------------
def mrstft(y_hat, y):
    total = 0.0
    for n_fft, hop, win in ((1024, 120, 600), (2048, 240, 1200), (512, 50, 240)):
        w = np.hanning(win + 1)[:-1]
        def mag(s):
            _, _, Z = sg.stft(s, nperseg=win, noverlap=win - hop, nfft=n_fft, window=w, boundary=None, padded=False)
            return np.sqrt(np.maximum(np.abs(Z) ** 2, 1e-8))
        A, B = mag(y), mag(y_hat)
        sc = np.linalg.norm(A - B) / np.linalg.norm(A)
        lm = np.mean(np.abs(np.log(A) - np.log(B)))
        total += sc + lm
    return total / 3.0


def compare(x, y, z, seg_s=10.0):
    """z = model output (already gain-aligned), y = hardware output"""
    S = int(seg_s * FS)
    mae, st, lu = [], [], []
    for i in range(0, len(x) - S + 1, S):
        a, b = z[i:i + S], y[i:i + S]
        if np.sqrt(np.mean(b.astype(np.float64) ** 2)) < 1e-4:
            continue  # silent segment
        mae.append(np.mean(np.abs(a - b)))
        st.append(mrstft(a, b))
        lu.append(abs(lufs(a) - lufs(b)))
    return float(np.mean(mae)), float(np.mean(st)), float(np.mean(lu)), len(mae)


def gr_error(x, y, z):
    F = int(0.01 * FS)
    lx, ly, lz = frames_db(x, F), frames_db(y, F), frames_db(z, F)
    act = lx > -45
    hw, md = (ly - lx)[act], (lz - lx)[act]
    return float(np.sqrt(np.mean((md - hw) ** 2))), float(np.median(md - hw)), hw, md, lx[act]


def run(pr_map=lambda limit, pr: pr, extra=None, verbose=True, minutes=None):
    out = []
    for idx, name in PAIRS:
        limit, pr = settings(name)
        x, y = load(idx, name)
        if minutes:
            x, y = x[: int(minutes * 60 * FS)], y[: int(minutes * 60 * FS)]
        params = {"fetOn": 0, "optoOn": 1, "optoMode": limit, "optoPR": pr_map(limit, pr), "colorPct": 0,
                  "scHpfHz": 20, "osLog2": 1}
        if extra:
            params.update(extra)
        z = render(x, params, f"{idx}_{len(x)}")
        # one static gain per file (the hardware Gain knob): least squares on the waveform
        g = float(np.dot(z.astype(np.float64), y) / np.dot(z.astype(np.float64), z))
        zg = (z * g).astype(np.float32)
        e_rms, e_med, hw, md, lx = gr_error(x, y, zg)
        mae, st, lu, nseg = compare(x, y, zg)
        r = {"pair": name, "limit": limit, "pr_hw": pr, "pr_model": pr_map(limit, pr), "gain_db": 20 * np.log10(g),
             "gr_env_rms_db": e_rms, "gr_env_median_db": e_med, "mae": mae, "stft": st, "lufs": lu, "segments": nseg,
             "hw_gr_p50": float(np.median(-hw)), "model_gr_p50": float(np.median(-md))}
        out.append(r)
        if verbose:
            print(f"{name}: model PR {r['pr_model']:.1f}, gain {r['gain_db']:+.2f} dB | GR env err {e_rms:.2f} dB rms "
                  f"(median {e_med:+.2f}) | MAE {mae:.2e} STFT {st:.3f} LUFS {lu:.3f} ({nseg} seg)", flush=True)
    return out


if __name__ == "__main__":
    res = run()
    if "--json" in sys.argv:
        json.dump(res, open(sys.argv[sys.argv.index("--json") + 1], "w"), indent=1)
