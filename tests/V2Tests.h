#pragma once
// Axell Duo Compressor 2.0 - tests of the 2.0 features (section V of the DSP suite).
// Included by DspTests.cpp after its helpers (makeEngine, runProbe, steadyGr, ...).

#include <atomic>
#include <new>

namespace v2
{
using namespace dt;

// allocation counter: only counts on threads that switched it on (the suite runs configurations in parallel)
inline thread_local bool countAllocs = false;
inline thread_local long allocCount = 0;

// THD+N re the fundamental: residual after a least-squares fit of the fundamental (sin, cos) and DC
inline double thdnDb (const float* x, int n, double f0, double fs)
{
    double ss = 0, cc = 0, sc = 0, s1 = 0, c1 = 0, xs = 0, xc = 0, x1 = 0;
    for (int i = 0; i < n; ++i)
    {
        const double w = 2.0 * kPi * f0 * i / fs, s = std::sin (w), c = std::cos (w);
        ss += s * s; cc += c * c; sc += s * c; s1 += s; c1 += c;
        xs += x[i] * s; xc += x[i] * c; x1 += x[i];
    }
    // 3x3 normal equations [ss sc s1; sc cc c1; s1 c1 n] [a b d] = [xs xc x1]
    const double A[3][3] = { { ss, sc, s1 }, { sc, cc, c1 }, { s1, c1, (double) n } };
    const double B[3] = { xs, xc, x1 };
    auto det3 = [] (const double m[3][3])
    {
        return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
             + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
    };
    const double D = det3 (A);
    double sol[3];
    for (int k = 0; k < 3; ++k)
    {
        double M[3][3];
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                M[i][j] = j == k ? B[i] : A[i][j];
        sol[k] = det3 (M) / D;
    }
    double res = 0;
    for (int i = 0; i < n; ++i)
    {
        const double w = 2.0 * kPi * f0 * i / fs;
        const double e = x[i] - (sol[0] * std::sin (w) + sol[1] * std::cos (w) + sol[2]);
        res += e * e;
    }
    const double fundRms = std::hypot (sol[0], sol[1]) / std::sqrt (2.0);
    return db (std::sqrt (res / n) / std::max (fundRms, 1.0e-30));
}

// independent true-peak reference: Hann-windowed sinc (+-512 taps), 16 points per sample, evaluated
// around every sample within 3 dB of the largest one
inline double truePeakRef (const std::vector<float>& x, size_t from, size_t to)
{
    const long H = 512;
    double amax = 0;
    for (size_t i = from; i < to; ++i)
        amax = std::max (amax, (double) std::abs (x[i]));
    double best = amax;
    for (size_t i = from; i < to; ++i)
    {
        if (std::abs (x[i]) < amax * 0.7)
            continue;
        for (int p = 1; p < 16; ++p)
        {
            const double t = (double) i + p / 16.0;
            double s = 0;
            for (long k = (long) i - H; k <= (long) i + H + 1; ++k)
            {
                if (k < 0 || k >= (long) x.size())
                    continue;
                const double d = t - (double) k;
                const double w = 0.5 + 0.5 * std::cos (kPi * d / (H + 2));
                s += x[(size_t) k] * std::sin (kPi * d) / (kPi * d) * w;
            }
            best = std::max (best, std::abs (s));
        }
    }
    return best;
}

inline duo::Params fetOnly()
{
    duo::Params p;
    p.optoOn = false;
    p.colorPct = 0.0f;
    p.osLog2 = 2;
    return p;
}

inline duo::Params optoOnly()
{
    duo::Params p;
    p.fetOn = false;
    p.colorPct = 0.0f;
    p.osLog2 = 2;
    return p;
}

// run with an external key buffer (same length as in)
inline Buf runKeyed (duo::Engine& e, const Buf& in, const Buf& key, int block = 512, bool count = false)
{
    Buf out = in;
    const int nCh = (int) out.size(), kc = (int) key.size();
    const int n = (int) out[0].size();
    for (int pos = 0; pos < n; pos += block)
    {
        const int len = std::min (block, n - pos);
        float* ptr[2] {};
        const float* kp[2] {};
        for (int c = 0; c < nCh; ++c)
            ptr[c] = out[(size_t) c].data() + pos;
        for (int c = 0; c < kc; ++c)
            kp[c] = key[(size_t) c].data() + pos;
        countAllocs = count;
        e.process (ptr, nCh, len, kp, kc);
        countAllocs = false;
    }
    return out;
}

// level (dBFS) of a tone in the last `secs` of channel c
inline double tailTone (const Buf& b, int c, double f, double fs, double secs = 0.2)
{
    const int m = (int) (secs * fs);
    const int n = (int) b[(size_t) c].size();
    return db (toneAmp (b[(size_t) c].data() + n - m, m, f, fs));
}

// band-limited test material (multisine 100 Hz .. min(18 kHz, 0.4 fs), slow envelope): the oversampling
// filters are transparent for it, so a null against the input measures the processing, not the band edge
inline Buf multitone (int nCh, int n, double fs, double amp = 0.04)
{
    Buf b = zeros (nCh, n);
    for (double f = 100.0; f < 0.4 * fs && f < 18000.0; f *= 1.37)
        addSine (b, fs, f, amp, 0, -1, f * 0.001);
    for (auto& ch : b)
        for (size_t i = 0; i < ch.size(); ++i)
            ch[i] *= (float) (0.6 + 0.4 * std::sin (2.0 * kPi * 1.3 * (double) i / fs));
    return b;
}

// linear-phase low-pass (Blackman-windowed sinc, +-400 taps) at fc * fs: material inside the limiter's
// guaranteed band (content up to 0.48 fs, DECISIONS D21)
inline Buf bandlimit (const Buf& in, double fc)
{
    const int H = 400;
    std::vector<double> h ((size_t) (2 * H + 1));
    for (int k = -H; k <= H; ++k)
    {
        const double w = 0.42 + 0.5 * std::cos (kPi * k / H) + 0.08 * std::cos (2.0 * kPi * k / H);
        h[(size_t) (k + H)] = (k == 0 ? 2.0 * fc : std::sin (2.0 * kPi * fc * k) / (kPi * k)) * w;
    }
    Buf out = in;
    for (size_t c = 0; c < in.size(); ++c)
    {
        const auto& x = in[c];
        const long n = (long) x.size();
        for (long i = 0; i < n; ++i)
        {
            double a = 0.0;
            for (long k = std::max (-(long) H, i - n + 1); k <= std::min ((long) H, i); ++k)
                a += x[(size_t) (i - k)] * h[(size_t) (k + H)];
            out[c][(size_t) i] = (float) a;
        }
    }
    return out;
}

// max |out - in delayed by L| over the settled part
inline double nullDb (const Buf& in, const Buf& out, int L, double fs)
{
    double e = 0;
    const int skip = (int) (0.05 * fs);
    for (size_t c = 0; c < in.size(); ++c)
        for (size_t i = (size_t) (L + skip); i < in[c].size(); ++i)
            e = std::max (e, (double) std::abs (out[c][i] - in[c][i - (size_t) L]));
    return db (e);
}

inline void testV2 (Recorder& r)
{
    const Ctx c = r.ctx;
    const double fs = c.fs;
    r.section = "V 2.0 features";

    //== V1 Clean detector: no low-frequency gain ripple (bass distortion) ====================
    for (double f0 : { 30.0, 100.0 })
    {
        for (int clean = 0; clean < 2; ++clean)
        {
            auto p = fetOnly();
            p.fetAtkMs = 0.02f;
            p.fetRelMs = 50.0f;
            p.fetInDb = 12.0f;
            p.fetClean = clean == 1;
            auto e = makeEngine (c, p);
            Buf in = zeros (c.nCh, (int) (2.5 * fs));
            addSine (in, fs, f0, 0.5);
            auto out = run (*e, in);
            const int m = (int) (1.0 * fs);
            const double t = thdnDb (out[0].data() + out[0].size() - (size_t) m, m, f0, fs);
            const std::string what = "FET 4:1, 20 us / 50 ms, Color 0, " + fmtNum (f0) + " Hz -6 dBFS (+12 dB in): THD+N";
            if (clean)
                r.less ("Clean detector: " + what, t, -90.0, "dB");
            else
                r.info ("Vintage detector (1.x): " + what, t, "dB");
        }
    }
    {
        // Clean keeps the static curve and the instant attack
        auto pv = fetOnly(), pc = fetOnly();
        pc.fetClean = true;
        r.range ("Clean vs Vintage: steady GR at 1 kHz -6 dBFS (difference)",
                 steadyGr (c, pc, -6.0, true) - steadyGr (c, pv, -6.0, true), -0.3, 0.3, "dB");
        for (auto* pp : { &pv, &pc })
        {
            pp->fetAtkMs = 0.02f;
            pp->fetRatio = 3;
            pp->fetInDb = 24.0f;
        }
        auto onset = [&] (const duo::Params& p)
        {
            auto e = makeEngine (c, p);
            Buf in = zeros (c.nCh, (int) (0.3 * fs));
            addSine (in, fs, 1000.0, 0.5, (int) (0.1 * fs));
            auto out = run (*e, in);
            const int L = e->getLatencySamples();
            const size_t a = (size_t) (0.1 * fs) + (size_t) L;
            return db (maxAbs (out[0], (int) a, (int) a + (int) (0.0005 * fs)));
        };
        r.range ("Clean vs Vintage: onset peak (first 0.5 ms, 20:1, 20 us) difference", onset (pc) - onset (pv), -0.5, 0.5, "dB");
    }

    //== V2 4x oversampling (the 2.0 default): aliasing at heavy drive ===========================
    {
        auto p = fetOnly();
        p.fetRatio = 3;
        p.fetInDb = 24.0f;
        p.colorPct = 100.0f;
        auto e = makeEngine (c, p);
        Buf in = zeros (c.nCh, (int) (1.5 * fs));
        addSine (in, fs, 15000.0, fromDb (-3.0));
        auto out = run (*e, in);
        const int n = nextPow2 ((int) (0.5 * fs)) / 2;
        auto spec = spectrum (out[0].data() + out[0].size() - (size_t) n, n);
        double worst = 0;
        for (size_t k = 1; k < spec.size(); ++k)
        {
            const double f = (double) k * fs / n;
            bool harmonic = false;
            for (int h = 1; h <= 6; ++h)
                harmonic = harmonic || std::abs (f - h * 15000.0) < 80.0;
            if (! harmonic && f > 20.0 && f < std::min (20000.0, 0.5 * fs))
                worst = std::max (worst, spec[k]);
        }
        r.less ("4x: worst non-harmonic component, 15 kHz -3 dBFS, 20:1 +24 dB, Color 100", db (worst), -120.0, "dBFS");
    }

    //== V3 threshold, knee, range, hold ========================================================
    {
        auto p = fetOnly();
        p.fetInDb = 0.0f;
        auto q = p;
        q.fetThrDb = -24.0f;
        r.range ("FET threshold -24 dB: GR at -18 dBFS = GR of the -12 dB threshold at -6 dBFS",
                 steadyGr (c, q, -18.0, true) - steadyGr (c, p, -6.0, true), -0.1, 0.1, "dB");
        auto h = p;
        h.fetKneeDb = 0.0f;
        r.range ("FET hard knee: 4:1, 6 dB over threshold -> GR 4.5 dB", steadyGr (c, h, -6.0, true), 4.35, 4.65, "dB");
        r.less ("FET hard knee: 1 dB under threshold -> GR", steadyGr (c, h, -13.0, true), 0.05, "dB");
        auto k = p;
        k.fetKneeDb = 12.0f;
        r.range ("FET 12 dB knee: GR at threshold = slope * knee / 8 (1.125 dB)", steadyGr (c, k, -12.0, true), 1.0, 1.25, "dB");
        auto rg = p;
        rg.fetRatio = 3;
        rg.fetInDb = 20.0f;
        rg.fetRangeDb = 6.0f;
        r.range ("FET range 6 dB: GR with 20 dB of demand", steadyGr (c, rg, -6.0, true), 5.9, 6.01, "dB");
        auto o = optoOnly();
        o.optoRangeDb = 3.0f;
        o.optoPR = 80.0f;
        r.range ("Opto range 3 dB: GR with heavy demand", steadyGr (c, o, -6.0, false, 1000.0, 2.0), 2.9, 3.01, "dB");
        auto ot = optoOnly(), ot2 = optoOnly();
        ot2.optoThrDb = -20.0f;
        r.range ("Opto threshold -20 dB: GR at -16 dBFS = GR of the -10 dB threshold at -6 dBFS",
                 steadyGr (c, ot2, -16.0, false, 1000.0, 2.0) - steadyGr (c, ot, -6.0, false, 1000.0, 2.0), -0.15, 0.15, "dB");

        // hold: the GR stays for `hold` after the signal drops, then releases
        for (int stage = 0; stage < 2; ++stage)
        {
            auto ph = stage == 0 ? fetOnly() : optoOnly();
            if (stage == 0)
            {
                ph.fetInDb = 12.0f;
                ph.fetRelMs = 50.0f;
                ph.fetHoldMs = 100.0f;
            }
            else
            {
                ph.optoPR = 60.0f;
                ph.optoHoldMs = 100.0f;
            }
            auto e = makeEngine (c, ph);
            Buf in = zeros (c.nCh, (int) (1.6 * fs));
            addSine (in, fs, 1000.0, 0.5, 0, (int) (1.2 * fs));
            addSine (in, fs, 1000.0, 0.005, (int) (1.2 * fs)); // drops 40 dB (below threshold)
            GrTrace tr;
            runProbe (*e, in, tr);
            const auto& g = stage == 0 ? tr.fet : tr.opto;
            const double osr = tr.rate;
            const size_t drop = (size_t) (1.2 * osr) + (size_t) (0.002 * osr);
            const double g0 = g[drop - (size_t) (0.004 * osr)];
            const double g80 = g[drop + (size_t) (0.08 * osr)];
            const double g200 = g[drop + (size_t) (0.2 * osr)];
            const std::string nm = stage == 0 ? "FET" : "Opto";
            r.less (nm + " hold 100 ms: GR change 80 ms after the drop", std::abs (g0 - g80), 0.1, "dB");
            r.less (nm + " hold 100 ms: released 200 ms after the drop (GR fraction left)", g200 / std::max (g0, 1e-9), 0.8, "x");
        }
    }

    //== V4 side-chain EQ ======================================================================
    {
        auto p = fetOnly();
        p.fetInDb = 6.0f;
        auto b = p;
        b.scBellHz = 3000.0f;
        b.scBellDb = 12.0f;
        r.greater ("SC bell +12 dB at 3 kHz: GR(3 kHz) - GR(300 Hz), equal levels",
                   steadyGr (c, b, -12.0, true, 3000.0) - steadyGr (c, b, -12.0, true, 300.0), 6.0, "dB");
        auto l = p;
        l.scLpfHz = 1000.0f;
        r.less ("SC LPF 1 kHz: GR on an 8 kHz tone at -6 dBFS", steadyGr (c, l, -6.0, true, 8000.0), 1.5, "dB");
        r.greater ("SC LPF 1 kHz: GR on a 300 Hz tone at -6 dBFS", steadyGr (c, l, -6.0, true, 300.0), 3.0, "dB");
    }

    //== V5 external side-chain =================================================================
    for (int stage = 0; stage < 2; ++stage)
    {
        auto p = stage == 0 ? fetOnly() : optoOnly();
        (stage == 0 ? p.fetExt : p.optoExt) = true;
        if (stage == 1)
            p.optoPR = 50.0f;
        const std::string nm = stage == 0 ? "FET" : "Opto";
        const int n = (int) (2.0 * fs);
        Buf in = zeros (c.nCh, n);
        addSine (in, fs, 1000.0, fromDb (-30.0));
        Buf loud = zeros (c.nCh, n), quiet = zeros (c.nCh, n);
        addSine (loud, fs, 120.0, fromDb (-3.0));
        addSine (quiet, fs, 120.0, fromDb (-50.0));
        auto e1 = makeEngine (c, p);
        const double ducked = tailTone (runKeyed (*e1, in, loud), 0, 1000.0, fs);
        auto e2 = makeEngine (c, p);
        const double free = tailTone (runKeyed (*e2, in, quiet), 0, 1000.0, fs);
        auto e3 = makeEngine (c, p);
        const double none = tailTone (run (*e3, in), 0, 1000.0, fs);
        const double gainNoGr = stage == 0 ? p.fetInDb + p.fetOutDb : p.optoGainDb;
        r.greater (nm + " external key 120 Hz -3 dBFS ducks a -30 dBFS 1 kHz track by", free - ducked, 6.0, "dB");
        r.range (nm + " external key quiet: track passes at its gain", free - (-30.0 + gainNoGr), -0.05, 0.05, "dB");
        r.range (nm + " external key, no side-chain input: track passes at its gain", none - (-30.0 + gainNoGr), -0.05, 0.05, "dB");
    }

    //== V6 lookahead ===========================================================================
    {
        auto p = fetOnly();
        p.fetOn = false;
        p.lookaheadMs = 5.0f;
        auto e = makeEngine (c, p);
        const int la = (int) std::lround (0.005 * fs);
        r.equal ("lookahead 5 ms: latency = 4x + 5 ms", e->getLatencySamples(), e->latencyForOversampling (2) + la, "samples");
        Buf in = multitone (c.nCh, (int) (0.5 * fs), fs);
        auto out = run (*e, in);
        r.less ("lookahead 5 ms, stages off: output = input delayed by the latency (<18 kHz)", nullDb (in, out, e->getLatencySamples(), fs), -80.0, "dBFS");
        auto p0 = p;
        p0.lookaheadMs = 0.0f;
        auto e0 = makeEngine (c, p0);
        Buf full = program (c.nCh, (int) (0.5 * fs), fs, 3);
        r.less ("lookahead 5 ms, stages off: = the engine without lookahead, delayed 5 ms", nullDb (run (*e0, full), run (*e, full), la, fs), -140.0, "dBFS");

        auto burstPeak = [&] (float laMs)
        {
            auto q = fetOnly();
            q.fetRatio = 3;
            q.fetAtkMs = 0.8f;
            q.fetInDb = 24.0f;
            q.lookaheadMs = laMs;
            auto en = makeEngine (c, q);
            Buf b = zeros (c.nCh, (int) (0.6 * fs));
            addSine (b, fs, 1000.0, 0.5, (int) (0.1 * fs));
            auto o = run (*en, b);
            const double steady = db (maxAbs (o[0], (int) (0.5 * fs), (int) (0.6 * fs)));
            return db (maxAbs (o[0])) - steady;
        };
        const double o0 = burstPeak (0.0f), o2 = burstPeak (2.0f);
        r.info ("FET 20:1, 800 us attack: onset overshoot without lookahead", o0, "dB");
        r.less ("FET 20:1, 800 us attack: onset overshoot with 2 ms lookahead", o2, o0 - 3.0, "dB");
    }

    //== V7 true-peak limiter ===================================================================
    {
        auto p = fetOnly();
        p.fetOn = false;
        p.limOn = true;
        p.limCeilDb = -1.0f;
        p.outDb = 12.0f;
        auto e = makeEngine (c, p);
        const int L = e->getLatencySamples();
        const int la = (int) std::lround (0.0015 * fs);
        r.equal ("limiter on (1.5 ms): latency = 4x + 64 (true-peak detector) + lookahead", L, e->latencyForOversampling (2) + 64 + la, "samples");

        // drum-bus program and hard-edged square bursts, +12 dB into a -1 dBTP ceiling. Guaranteed: content
        // up to 0.48 fs; full band (energy up to Nyquist): within the documented bound (DECISIONS D21)
        Buf sq = zeros (c.nCh, (int) (0.6 * fs));
        for (auto& ch : sq)
            for (size_t i = 0; i < ch.size(); ++i)
                ch[i] = (float) (0.25 * ((i / 7) % 2 ? 1.0 : -1.0) * (i % 9000 < 4500 ? 1.0 : 0.4));
        const Buf mixFull = mixProgram (c.nCh, (int) (1.0 * fs), fs, 11);
        struct Case { const char* name; Buf sig; bool full; };
        const Case cases[] = { { "drum-bus program, content <= 0.48 fs", bandlimit (mixFull, 0.48), false },
                               { "square bursts, content <= 0.48 fs", bandlimit (sq, 0.48), false },
                               { "drum-bus program, full band", mixFull, true },
                               { "square bursts, full band", sq, true } };
        for (const auto& cs : cases)
        {
            auto el = makeEngine (c, p);
            auto out = run (*el, cs.sig);
            double sp = 0, tp = 0;
            const size_t from = (size_t) (0.2 * fs), to = out[0].size() - 2000;
            for (size_t ch = 0; ch < out.size(); ++ch)
            {
                sp = std::max (sp, (double) maxAbs (out[ch], (int) from, (int) to));
                tp = std::max (tp, truePeakRef (out[ch], from, to));
            }
            r.less (std::string ("limiter -1 dBTP, +12 dB, ") + cs.name + ": sample peak", db (sp), -1.0, "dBFS");
            if (cs.full)
                r.less (std::string ("limiter -1 dBTP, +12 dB, ") + cs.name + ": true peak (reference), documented bound +0.4 dB",
                        db (tp), -0.6, "dBTP");
            else
                r.less (std::string ("limiter -1 dBTP, +12 dB, ") + cs.name + ": true peak (independent long-sinc reference)",
                        db (tp), -1.0, "dBTP");
            if (! cs.full)
                r.greater (std::string ("limiter, ") + cs.name + ": GR meter", el->getMeters().limGrDb, 3.0, "dB");
        }

        // below the ceiling the limiter is transparent
        auto pq = p;
        pq.outDb = -12.0f;
        auto e3 = makeEngine (c, pq);
        auto pOff = pq;
        pOff.limOn = false;
        auto e4 = makeEngine (c, pOff);
        Buf q = program (c.nCh, (int) (0.6 * fs), fs, 5);
        r.less ("limiter below the ceiling: = the engine without limiter, delayed by its latency (null)",
                nullDb (run (*e4, q), run (*e3, q), e3->getLatencySamples() - e4->getLatencySamples(), fs), -140.0, "dBFS");
    }

    //== V8 Mid/Side ============================================================================
    if (c.nCh == 2)
    {
        auto p = fetOnly();
        p.fetOn = false;
        p.midSide = true;
        auto e = makeEngine (c, p);
        auto pst = p;
        pst.midSide = false;
        auto est = makeEngine (c, pst);
        Buf in = program (2, (int) (0.5 * fs), fs, 9);
        r.less ("Mid/Side, stages off: = stereo mode (M/S encode + decode is exact)", nullDb (run (*est, in), run (*e, in), 0, fs), -130.0, "dBFS");

        // identical channels: M/S (S = 0) sounds like stereo mode
        auto ps = fetOnly(), pm = fetOnly();
        pm.midSide = true;
        Buf mono = program (2, (int) (1.0 * fs), fs, 4, true);
        auto es = makeEngine (c, ps);
        auto em = makeEngine (c, pm);
        auto os = run (*es, mono);
        auto om = run (*em, mono);
        r.less ("Mid/Side on identical channels = stereo mode (difference)", nullDb (os, om, 0, fs), -100.0, "dBFS");

        // a loud side component is compressed, the quiet mid is not (link 0)
        auto pl = fetOnly();
        pl.midSide = true;
        pl.linkPct = 0.0f;
        pl.fetInDb = 6.0f;
        auto el = makeEngine (c, pl);
        Buf st = zeros (2, (int) (1.0 * fs));
        for (size_t i = 0; i < st[0].size(); ++i)
        {
            const double t = (double) i / fs;
            const double mid = fromDb (-40.0) * std::sin (2.0 * kPi * 500.0 * t);
            const double side = 0.5 * std::sin (2.0 * kPi * 2000.0 * t);
            st[0][i] = (float) (mid + side);
            st[1][i] = (float) (mid - side);
        }
        auto ol = run (*el, st);
        Buf mOut = zeros (1, (int) st[0].size()), sOut = zeros (1, (int) st[0].size());
        for (size_t i = 0; i < st[0].size(); ++i)
        {
            mOut[0][i] = 0.5f * (ol[0][i] + ol[1][i]);
            sOut[0][i] = 0.5f * (ol[0][i] - ol[1][i]);
        }
        r.range ("Mid/Side, link 0: quiet mid passes at its gain (6 dB input)", tailTone (mOut, 0, 500.0, fs) - (-40.0 + 6.0), -0.05, 0.05, "dB");
        r.greater ("Mid/Side, link 0: loud side compressed by", (-6.02 + 6.0) - tailTone (sOut, 0, 2000.0, fs), 3.0, "dB");
    }

    //== V9 latency handshake ==================================================================
    {
        duo::Params p = fetOnly();
        p.fetOn = false;
        duo::Engine e;
        e.prepare (fs, c.nCh, 512);
        e.setParams (p);
        e.reset();
        e.setHostLatency (e.getLatencySamples()); // the wrapper reported it: tracking on
        const int L0 = e.getLatencySamples();
        p.lookaheadMs = 3.0f;
        e.setParams (p);
        Buf in = zeros (c.nCh, (int) (0.3 * fs));
        addSine (in, fs, 1000.0, 0.5);
        auto out = run (e, in);
        r.equal ("handshake: unacknowledged latency change keeps the old latency", e.getLatencySamples(), L0, "samples");
        r.less ("handshake: muted while waiting for the host", db (maxAbs (out[0], (int) (0.05 * fs))), -120.0, "dBFS");
        int lat = 0;
        const bool posted = e.takeLatencyRequest (lat);
        r.truth ("handshake: a request with the new latency is posted", posted && lat == L0 + (int) std::lround (0.003 * fs));
        e.ackLatencyRequest();
        Buf in2 = zeros (c.nCh, (int) (0.3 * fs));
        in2[0][(size_t) (0.1 * fs)] = 0.5f;
        if (c.nCh == 2)
            in2[1][(size_t) (0.1 * fs)] = 0.5f;
        auto o2 = run (e, in2);
        size_t pk = 0;
        for (size_t i = 0; i < o2[0].size(); ++i)
            if (std::abs (o2[0][i]) > std::abs (o2[0][pk]))
                pk = i;
        r.equal ("handshake: after the acknowledgement the new latency plays (impulse position - input)",
                 (double) pk - 0.1 * fs, e.getLatencySamples(), "samples");
        r.equal ("handshake: engine latency = the acknowledged one", e.getLatencySamples(), lat, "samples");

        // the host never answers: after 0.5 s the old configuration plays again (unmuted)
        p.lookaheadMs = 6.0f;
        e.setParams (p);
        Buf in3 = zeros (c.nCh, (int) (1.0 * fs));
        addSine (in3, fs, 1000.0, 0.5);
        auto o3 = run (e, in3);
        r.equal ("handshake timeout: latency unchanged", e.getLatencySamples(), lat, "samples");
        r.greater ("handshake timeout: the old configuration plays again after 0.5 s", db (maxAbs (o3[0], (int) (0.8 * fs))), -7.0, "dBFS");
    }

    //== V11 level-matched compare and display history (2.1) =====================================
    {
        auto p = fetOnly();
        p.fetInDb = 18.0f; // heavy compression: the processed signal is much louder than the input after OUTPUT
        p.fetOutDb = 6.0f;
        Buf in = program (c.nCh, (int) (3.0 * fs), fs, 31);
        auto e0 = makeEngine (c, p);
        auto proc = run (*e0, in);
        auto q = p;
        q.compare = true;
        auto e1 = makeEngine (c, q);
        auto cmp = run (*e1, in);
        // loudness (plain RMS of the last 1.5 s; K-weighting changes it by the same amount on both for this material)
        auto rmsDb = [&] (const Buf& b)
        {
            double s2 = 0;
            const size_t from = b[0].size() - (size_t) (1.5 * fs);
            for (size_t ch = 0; ch < b.size(); ++ch)
                for (size_t i = from; i < b[ch].size(); ++i)
                    s2 += (double) b[ch][i] * b[ch][i];
            return 10.0 * std::log10 (s2 / (1.5 * fs * (double) b.size()));
        };
        double inLevel = rmsDb (in);
        r.info ("compare: processed signal louder than the input by", rmsDb (proc) - inLevel, "dB");
        r.range ("compare: unprocessed signal played at the processed signal's loudness (difference)", rmsDb (cmp) - rmsDb (proc), -1.0, 1.0, "dB");
        // and it is the input itself (latency-aligned), only scaled: residual after the best gain
        const int L = e1->getLatencySamples();
        double sxy = 0, sxx = 0;
        const size_t from = (size_t) (2.0 * fs);
        for (size_t i = from; i < cmp[0].size(); ++i)
        {
            sxy += (double) cmp[0][i] * in[0][i - (size_t) L];
            sxx += (double) in[0][i - (size_t) L] * in[0][i - (size_t) L];
        }
        const double g = sxy / sxx;
        double res = 0, ref = 0;
        for (size_t i = from; i < cmp[0].size(); ++i)
        {
            const double d = cmp[0][i] - g * in[0][i - (size_t) L];
            res += d * d;
            ref += (double) cmp[0][i] * cmp[0][i];
        }
        r.less ("compare: output is the unprocessed input (residual after one gain)", 10.0 * std::log10 (res / ref + 1e-30), -40.0, "dB");

        // history: one frame per ~5 ms, gain reduction as on the meters
        auto e2 = makeEngine (c, p);
        Buf in2 = zeros (c.nCh, (int) fs);
        addSine (in2, fs, 1000.0, 0.5);
        run (*e2, in2);
        duo::HistoryFrame f;
        int frames = 0;
        float lastGr = 0;
        while (e2->popHistory (f))
        {
            ++frames;
            lastGr = f.fetGr;
        }
        r.range ("history: frames per second of audio", frames, 195, 205, "");
        r.range ("history: FET gain reduction in the last frame vs the stage's steady GR",
                 lastGr - steadyGr (c, p, -6.02, true), -0.3, 0.3, "dB");
    }

    //== V10 block-size invariance, allocations, with every 2.0 feature on ======================
    {
        auto p = duo::Params();
        p.osLog2 = 2;
        p.fetClean = true;
        p.fetHoldMs = 20.0f;
        p.optoRangeDb = 8.0f;
        p.scLpfHz = 8000.0f;
        p.scBellDb = 4.0f;
        p.lookaheadMs = 2.0f;
        p.limOn = true;
        p.limCeilDb = -3.0f;
        p.midSide = c.nCh == 2;
        p.optoExt = true;
        Buf in = program (c.nCh, (int) (1.0 * fs), fs, 21);
        Buf key = program (c.nCh, (int) (1.0 * fs), fs, 22);
        auto e1 = makeEngine (c, p);
        auto e2 = makeEngine (c, p);
        auto a = runKeyed (*e1, in, key, 512);
        allocCount = 0;
        auto b = runKeyed (*e2, in, key, 37, true);
        double d = 0;
        for (size_t ch = 0; ch < a.size(); ++ch)
            for (size_t i = 0; i < a[ch].size(); ++i)
                d = std::max (d, (double) std::abs (a[ch][i] - b[ch][i]));
        r.less ("all 2.0 features: block 512 vs 37 (difference)", db (d), -140.0, "dBFS");
        r.truth ("all 2.0 features: output finite", allFinite (b));
        r.equal ("all 2.0 features: heap allocations inside process()", (double) allocCount, 0.0, "");
    }
}
} // namespace v2
