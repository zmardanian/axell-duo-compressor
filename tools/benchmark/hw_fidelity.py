"""Which emulation is closest to the real hardware? Axell Duo Compressor and the other installed emulations run as
black boxes (DuoAuditHost) on the same public hardware recordings, scored with the same metrics on the same held-out
audio as tools/validation (la2a_report.py, fet1176_compare.py).

    python3 hw_fidelity.py out.json [la2a|1176|all]

LA-2A (SignalTrain Test split): every plugin at the hardware's knob settings; one global output offset per plugin,
taken from frames where the hardware is not compressing (input < -35 dBFS), so level calibration is equal for all.
For plugins whose Peak Reduction knob mapping is unknown (normalised values only), a "best case" variant also fits
that knob per hardware setting on the calibration blocks - the same data Axell's law was calibrated on.
1176 (Martinez Ramirez et al.): ALL, slowest attack and release; the input level is fitted per plugin on calibration
notes (every 2nd note, a quarter of them), evaluated on the held-out notes.
"""
import json, os, sys
import numpy as np
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "validation"))
import bench as B                     # noqa: E402  (plugins, host access)
import la2a_compare as L              # noqa: E402
import fet1176_compare as M           # noqa: E402

FS = 44100
OUT = {}


def host_render(p, x, sets=None, setns=None, tag="h", sc=False):
    """mono render of x (float array) through plugin p at FS, latency removed"""
    r = B.lib.Run(tag, FS, ch=2, sets=sets or {}, preroll=int(0.5 * FS))  # stereo (dual mono): every plugin takes it
    r.setns = setns or {}
    r.ats = [(0, k, v) for k, v in (sets or {}).items()] + [(0, "#n#" + k, v) for k, v in (setns or {}).items()]
    lead = np.zeros(int(0.5 * FS))
    xin = np.repeat(np.concatenate([lead, x.astype(np.float64)])[:, None], 2, 1)
    q = B.Plugin(p.name, p.bundle, p.sub, silent_sc=p.silent_sc)
    y = q.render([r], {tag: xin}, job=tag)[tag][:, 0]
    L_ = r.latency
    z = y[len(lead) + L_: len(lead) + L_ + len(x)]
    if len(z) < len(x):  # the render is as long as its input: the last `latency` samples are missing
        z = np.concatenate([z, np.zeros(len(x) - len(z))])
    return z, (r.cpu or 0.0)


# ------------------------------------------------------------------ LA-2A
def la2a():
    F = int(0.05 * FS)
    data = []
    for idx, name in L.PAIRS:
        limit, pr = L.settings(name)
        x, y = L.load(idx, name)
        data.append((idx, name, limit, pr, x, y, L.frames_db(x, F), L.frames_db(y, F)))
    t = lambda n: np.arange(n) * F / FS
    def held(n):
        tt = t(n); blk = np.floor((tt - 180) / 20).astype(int) % 4
        return ~((tt >= 180) & (tt < 420) & (blk == 0))
    def score(label, outs, method="quiet"):
        # one global output offset per plugin (its gain knob is not part of the comparison), by one of two methods:
        #  quiet - median level difference where the hardware is not compressing (input -60..-35 dBFS); the hardware's
        #          hiss biases it in quiet passages
        #  cal   - median level difference over the active frames of the calibration blocks (data every model may use)
        offs = []
        for (idx, name, limit, pr, x, y, lx, ly), z in zip(data, outs):
            lz = L.frames_db(z, F); n = min(len(lz), len(lx))
            if method == "quiet":
                sel = (lx[:n] < -35) & (lx[:n] > -60)
            else:
                sel = ~held(n) & (lx[:n] > -45)
            offs.append((lz[:n] - ly[:n])[sel])
        off = float(np.median(np.concatenate(offs)))
        rows = {}
        for (idx, name, limit, pr, x, y, lx, ly), z in zip(data, outs):
            z = z * 10 ** (-off / 20)
            lz = L.frames_db(z, F); n = min(len(lz), len(lx))
            d = (lz[:n] - lx[:n]) - (ly[:n] - lx[:n])
            m = held(n) & (lx[:n] > -45)
            music = t(n) < 300
            S = 10 * FS; st, lu = [], []
            for i in range(0, len(x) - S + 1, S):
                if 180 <= (i + S / 2) / FS < 420:
                    continue
                a, b = z[i:i + S], y[i:i + S]
                if np.sqrt(np.mean(b.astype(np.float64) ** 2)) < 1e-4:
                    continue
                st.append(L.mrstft(a, b)); lu.append(abs(L.lufs(a) - L.lufs(b)))
            rows[name] = {"gr_env_music_db": float(np.sqrt(np.mean(d[m & music] ** 2))),
                          "gr_env_noise_db": float(np.sqrt(np.mean(d[m & ~music] ** 2))),
                          "stft": float(np.mean(st)), "lufs": float(np.mean(lu))}
        summ = {k: float(np.mean([r[k] for r in rows.values()])) for k in ("gr_env_music_db", "gr_env_noise_db", "stft", "lufs")}
        print(f"LA-2A | {label} [{method}]: offset {off:+.2f} dB | " + " ".join(f"{k} {v:.3f}" for k, v in summ.items()), flush=True)
        OUT.setdefault("la2a", {})[f"{label} [{method}]"] = {"offset_db": off, "per_file": rows, "mean": summ}

    axell, uad = B.OPTO_PLUGINS
    cache = os.path.join(B.lib.WORK, "la2a_renders")
    os.makedirs(cache, exist_ok=True)
    def cached(key, fn):
        f = os.path.join(cache, key + ".npy")
        if os.path.exists(f):
            return np.load(f)
        z = fn()
        np.save(f, z.astype(np.float32))
        return z
    _score = score
    def score(label, outs):
        for m in ("quiet", "cal"):
            _score(label, outs, m)
    outs = [cached(f"ax{idx}", lambda: host_render(axell, x, {"FET On": "Off", "Limiter On": "Off", "Opto Mode": "Limit" if limit else "Compress",
                                   "Opto Peak Reduction": f"{pr:g}"}, tag=f"ax{idx}")[0])
            for idx, name, limit, pr, x, y, lx, ly in data]
    score("Axell Duo Opto, Measured law, hardware knob values", outs)
    outs = [cached(f"axc{idx}", lambda: host_render(axell, x, {"FET On": "Off", "Limiter On": "Off", "Opto Mode": "Limit" if limit else "Compress",
                                   "Opto Peak Reduction": f"{pr:g}", "Opto Model": "Classic"}, tag=f"axc{idx}")[0])
            for idx, name, limit, pr, x, y, lx, ly in data]
    score("Axell Duo Opto, Classic (1.x) law, hardware knob values", outs)
    outs = [cached(f"ua{idx}", lambda: host_render(uad, x, setns={"Peak Reduct": f"{pr / 100:.4f}", "Comp/Limit": "1" if limit else "0"}, tag=f"ua{idx}")[0])
            for idx, name, limit, pr, x, y, lx, ly in data]
    score("UADx LA-2A, hardware knob values (knob assumed linear 0-100)", outs)
    # best case for UAD: Peak Reduction fitted per setting on the calibration blocks (minutes 3-7, blocks 0, 4, ...)
    best = []
    fitted = {235: 0.35, 256: 0.35, 259: 0.50}  # result of the fit below (run 2026-10-15); refit if the cache is empty
    for idx, name, limit, pr, x, y, lx, ly in data:
        if os.path.exists(os.path.join(cache, f"ub{idx}.npy")) or os.environ.get("UAD_FIT_CACHED"):
            v = fitted[idx]
            best.append(cached(f"ub{idx}", lambda: host_render(uad, x, setns={"Peak Reduct": f"{v:.4f}", "Comp/Limit": "1" if limit else "0"}, tag=f"ub{idx}")[0]))
            continue
        seg = slice(180 * FS, 420 * FS); xs, ys = x[seg], y[seg]
        lxs, lys = L.frames_db(xs, F), L.frames_db(ys, F)
        blk = (np.arange(len(lxs)) * F / FS // 20).astype(int) % 4 == 0
        errs = []
        for v in np.linspace(0.2, 0.9, 15):
            z, _ = host_render(uad, xs, setns={"Peak Reduct": f"{v:.4f}", "Comp/Limit": "1" if limit else "0"}, tag=f"uf{idx}")
            lz = L.frames_db(z, F); n = min(len(lz), len(lxs))
            d = (lz[:n] - lys[:n]); q = (lxs[:n] < -35) & (lxs[:n] > -60)
            off = np.median(d[q]) if q.any() else 0.0
            act = (lxs[:n] > -45) & blk[:n]
            errs.append((float(np.sqrt(np.mean((d[act] - off) ** 2))), v))
        e, v = min(errs)
        print(f"  UADx LA-2A best Peak Reduct for {name}: {v:.3f} (calibration error {e:.2f} dB)", flush=True)
        best.append(cached(f"ub{idx}", lambda: host_render(uad, x, setns={"Peak Reduct": f"{v:.4f}", "Comp/Limit": "1" if limit else "0"}, tag=f"ub{idx}")[0]))
    score("UADx LA-2A, Peak Reduction fitted to the hardware (best case)", best)


# ------------------------------------------------------------------ 1176
def fet():
    notes = M.notes()
    cal, ev = notes[0::2][::4], notes[1::2]
    def batch(p, items, sets, setns, tag):
        xs = [M.load(f) for _, f, _ in items]
        gap = np.zeros(4 * FS, np.float32)
        seq, pos, q = [], [], 0
        for x in xs:
            seq += [x, gap]; pos.append(q); q += len(x) + len(gap)
        z, cpu = host_render(p, np.concatenate(seq), sets, setns, tag=tag)
        return xs, [M.load(g) for _, _, g in items], [z[a:a + len(x)] for a, x in zip(pos, xs)]
    def err(xs, ys, zs):
        return np.array([M.traj_error(x, y, z)[0] for x, y, z in zip(xs, ys, zs)])
    cands = [
        (B.FET_PLUGINS[1], lambda d: ({"Opto On": "Off", "Limiter On": "Off", "FET Ratio": "All", "FET Attack": "800",
                                       "FET Release": "1100", "Sidechain HPF": "20", "FET Input": f"{d:.2f}"}, {}), np.arange(-6, 15, 1.5)),
        (B.FET_PLUGINS[2], lambda d: ({}, {"Ratio": "1", "Attack": "0", "Release": "0", "Input": f"{d:.4f}"}), np.linspace(0.05, 0.6, 12)),
    ]
    labels = ["Axell Duo FET (Vintage), ALL, 800 us, 1100 ms", "UADx 1176, ALL, attack 1, release 1 (slowest)"]
    for (p, mk, grid), label in zip(cands, labels):
        best = None
        for d in grid:
            s, sn = mk(d)
            xs, ys, zs = batch(p, cal, s, sn, "fc")
            e = float(err(xs, ys, zs).mean())
            if best is None or e < best[0]:
                best = (e, d)
        s, sn = mk(best[1])
        xs, ys, zs = batch(p, ev, s, sn, "fe")
        e = err(xs, ys, zs)
        print(f"1176 | {label}: input {best[1]:.3f} -> held-out trajectory error mean {e.mean():.3f} dB, "
              f"median {np.median(e):.3f}, p90 {np.percentile(e, 90):.3f} ({len(e)} notes)", flush=True)
        OUT.setdefault("1176", {})[label] = {"input": float(best[1]), "cal_err": best[0], "mean": float(e.mean()),
                                             "median": float(np.median(e)), "p90": float(np.percentile(e, 90))}


if __name__ == "__main__":
    what = sys.argv[2] if len(sys.argv) > 2 else "all"
    if what in ("la2a", "all"):
        la2a()
    if what in ("1176", "all"):
        fet()
    json.dump(OUT, open(sys.argv[1], "w"), indent=1)
