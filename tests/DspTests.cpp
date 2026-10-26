// Axell Duo Compressor - automated DSP test suite (ENGINEERING_SPEC.md section 3 + section 2 features).
// JUCE-free: tests Source/Engine.h, i.e. exactly the DSP the plugin runs.
//
//   DuoDspTests [--rates 44100,48000,...] [--ch 1,2] [--report tests/report.md] [--no-bench] [--bench-only]
//
// Every check is evaluated per (sample rate, channel count). The process exit code is the
// number of failed checks (0 = all pass), so CTest reports failure correctly.

#include "Adapters.h"
#include "Harness.h"
#include "Measure.h"
#include "../Source/Presets.h"

#include <chrono>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <thread>

using namespace dt;

namespace
{
//==============================================================================
// helpers
std::unique_ptr<duo::Engine> makeEngine (const Ctx& c, const duo::Params& p)
{
    auto e = std::make_unique<duo::Engine>();
    e->prepare (c.fs, c.nCh);
    e->setParams (p);
    e->reset();
    return e;
}

struct GrTrace
{
    std::vector<float> fet, opto;
    double rate = 0.0;
    double meanTail (bool fetSide, double seconds) const
    {
        const auto& v = fetSide ? fet : opto;
        const int m = std::max (1, (int) (seconds * rate));
        double s = 0.0;
        for (size_t i = v.size() - (size_t) m; i < v.size(); ++i)
            s += v[i];
        return s / m;
    }
};

Buf runProbe (duo::Engine& e, const Buf& in, GrTrace& tr, int block = 512)
{
    const size_t cap = in[0].size() * 4 + 64;
    tr.fet.assign (cap, 0.0f);
    tr.opto.assign (cap, 0.0f);
    tr.rate = e.getOversampledRate();
    duo::Probe probe { tr.fet.data(), tr.opto.data(), (int) cap, 0 };
    e.setProbe (&probe);
    auto out = run (e, in, block);
    e.setProbe (nullptr);
    tr.fet.resize ((size_t) probe.count);
    tr.opto.resize ((size_t) probe.count);
    return out;
}

// steady output level (dBFS, fundamental) of an engine fed a sine
double steadyOutDb (const Ctx& c, const duo::Params& p, double levelDb, double f = 1000.0, double seconds = 0.5)
{
    auto e = makeEngine (c, p);
    const int n = (int) (seconds * c.fs);
    Buf in = zeros (c.nCh, n);
    addSine (in, c.fs, f, fromDb (levelDb));
    auto out = run (*e, in);
    const int m = (int) (0.2 * c.fs);
    return db (toneAmp (out[0].data() + n - m, m, f, c.fs));
}

double steadyGr (const Ctx& c, const duo::Params& p, double levelDb, bool fetSide, double f = 1000.0,
                 double seconds = 0.6)
{
    auto e = makeEngine (c, p);
    Buf in = zeros (c.nCh, (int) (seconds * c.fs));
    addSine (in, c.fs, f, fromDb (levelDb));
    GrTrace tr;
    runProbe (*e, in, tr);
    return tr.meanTail (fetSide, 0.2);
}

// bisection: input level (dBFS) that produces `target` dB of gain reduction
double levelForGr (const std::function<double (double)>& grAt, double target, double lo = -80.0, double hi = 6.0)
{
    for (int i = 0; i < 18; ++i)
    {
        const double mid = 0.5 * (lo + hi);
        (grAt (mid) < target ? lo : hi) = mid;
    }
    return 0.5 * (lo + hi);
}

double slope (const std::vector<double>& x, const std::vector<double>& y)
{
    const double n = (double) x.size();
    const double mx = std::accumulate (x.begin(), x.end(), 0.0) / n;
    const double my = std::accumulate (y.begin(), y.end(), 0.0) / n;
    double sxy = 0.0, sxx = 0.0;
    for (size_t i = 0; i < x.size(); ++i)
    {
        sxy += (x[i] - mx) * (y[i] - my);
        sxx += (x[i] - mx) * (x[i] - mx);
    }
    return sxy / sxx;
}

// Time conventions for ballistics (samples -> seconds): the stage output at frame j after a
// step represents the state at the END of that sample period (zero-order-hold discretisation
// of a continuous RC), i.e. time (j + 1) / fs. A pure one-pole with time constant tau then
// measures exactly tau. Linear interpolation between frames.
double crossUp (const std::vector<float>& g, int start, double level, double fs)
{
    for (size_t i = (size_t) start; i < g.size(); ++i)
        if (g[i] >= level)
        {
            const double prev = g[i - 1];
            const double frac = (level - prev) / std::max (1.0e-12, (double) g[i] - prev);
            return ((double) i - start + std::clamp (frac, 0.0, 1.0)) / fs;
        }
    return 1.0e9;
}

double crossDown (const std::vector<float>& g, int start, double level, double fs)
{
    for (size_t i = (size_t) start; i < g.size(); ++i)
        if (g[i] <= level)
        {
            const double prev = g[i - 1];
            const double frac = (prev - level) / std::max (1.0e-12, prev - (double) g[i]);
            return ((double) i - start + std::clamp (frac, 0.0, 1.0)) / fs;
        }
    return 1.0e9;
}

constexpr int kDefaultOsLog2 = 1; // ballistics are measured at the default 2x internal rate

//==============================================================================
// 1. Static curves
void testStaticCurves (Recorder& r)
{
    r.section = "1 Static curve";
    const Ctx& c = r.ctx;
    const char* names[] = { "FET 4:1", "FET 8:1", "FET 12:1", "FET 20:1", "FET All" };
    const double nominal[] = { 4, 8, 12, 20, 20 };

    auto sweep = [&] (const duo::Params& p, std::vector<double>& L, std::vector<double>& O)
    {
        for (double lvl = -60.0; lvl <= 0.001; lvl += 2.0)
        {
            L.push_back (lvl);
            O.push_back (steadyOutDb (c, p, lvl));
        }
    };
    auto fit = [] (const std::vector<double>& L, const std::vector<double>& O, double lo, double hi)
    {
        std::vector<double> x, y;
        for (size_t i = 0; i < L.size(); ++i)
            if (L[i] >= lo - 1.0e-9 && L[i] <= hi + 1.0e-9)
            {
                x.push_back (L[i]);
                y.push_back (O[i]);
            }
        return slope (x, y);
    };

    for (int ratio = 0; ratio < 5; ++ratio)
    {
        duo::Params p;
        p.optoOn = false;
        p.fetRatio = ratio;
        p.colorPct = 0.0f;
        p.fetRelMs = 1100.0f; // static law: slowest release removes ballistic ripple
        std::vector<double> L, O;
        sweep (p, L, O);
        r.range (std::string (names[ratio]) + ": slope below threshold (-60..-44 dBFS)", fit (L, O, -60, -44), 0.98, 1.02, "dB/dB");
        if (ratio < 4)
        {
            const double measured = 1.0 / fit (L, O, -12, 0);
            r.range (std::string (names[ratio]) + ": ratio above threshold (-12..0 dBFS in)", measured,
                     0.9 * nominal[ratio], 1.1 * nominal[ratio], ":1");
        }
        else
        {
            // threshold referred to the input: fixed -12 dBFS internal threshold minus INPUT (+6 dB)
            const double thrIn = -12.0 - p.fetInDb;
            size_t imax = 0;
            for (size_t i = 0; i < L.size(); ++i)
                if (O[i] > O[imax])
                    imax = i;
            r.greater ("FET All: over-compression, drop from curve peak to 0 dBFS in", O[imax] - O.back(), 0.3, "dB");
            r.range ("FET All: curve peak position above threshold", L[imax] - thrIn, 4.0, 16.0, "dB");
            r.less ("FET All: drop stays 'slight' (peak to 0 dBFS in)", O[imax] - O.back(), 4.0, "dB");
        }
    }

    const char* onames[] = { "Opto Compress", "Opto Limit" };
    const double onom[] = { 3.0, 10.0 };
    for (int mode = 0; mode < 2; ++mode)
    {
        duo::Params p;
        p.fetOn = false;
        p.optoMode = mode;
        p.optoPR = 50.0f;
        p.colorPct = 0.0f;
        std::vector<double> L, O;
        sweep (p, L, O);
        r.range (std::string (onames[mode]) + ": slope below threshold (-60..-44 dBFS)", fit (L, O, -60, -44), 0.98, 1.02, "dB/dB");
        r.range (std::string (onames[mode]) + ": ratio above threshold (-12..0 dBFS in)", 1.0 / fit (L, O, -12, 0),
                 0.9 * onom[mode], 1.1 * onom[mode], ":1");
    }
}

//==============================================================================
// 2. FET ballistics (stage driven directly at the oversampled rate)
struct FetSteps
{
    double attack = 0.0, release = 0.0, grHold = 0.0;
};

FetSteps fetSteps (const Ctx& c, float atkMs, float relMs, int ratio, double hiDb = -6.0, double holdSec = 0.3,
                   float inDb = 6.0f)
{
    const double fs = c.fs * (1 << kDefaultOsLog2);
    FetUT f;
    f.prepare (fs, c.nCh);
    duo::Params p;
    p.fetAtkMs = atkMs;
    p.fetRelMs = relMs;
    p.fetRatio = ratio;
    p.fetInDb = inDb;
    p.colorPct = 0.0f;
    f.set (p);

    const int up = (int) (0.3 * fs), down = up + (int) (holdSec * fs);
    const int n = down + (int) (std::max (5.0 * relMs * 0.001 * 1.5, 0.5) * fs);
    std::vector<float> gr ((size_t) n);
    const double a1 = fromDb (-30.0), a2 = fromDb (hiDb), w = 2.0 * kPi * 1000.0 / fs;
    float x[2] {};
    for (int i = 0; i < n; ++i)
    {
        const double amp = (i >= up && i < down) ? a2 : a1;
        const float v = (float) (amp * std::cos (w * (i - up))); // step lands on a positive peak
        x[0] = x[1] = v;
        gr[(size_t) i] = f.frame (x);
    }
    FetSteps s;
    double g = 0.0;
    for (int i = down - (int) (0.05 * fs); i < down; ++i)
        g = std::max (g, (double) gr[(size_t) i]);
    s.grHold = g;
    s.attack = crossUp (gr, up, 0.632 * g, fs);
    const double g0 = gr[(size_t) down - 1];
    s.release = crossDown (gr, down, (1.0 - 0.632) * g0, fs);
    return s;
}

void testFetBallistics (Recorder& r)
{
    r.section = "2 FET attack/release";
    const Ctx& c = r.ctx;
    for (float atk : { 0.02f, 0.8f })
    {
        const auto s = fetSteps (c, atk, 150.0f, 0);
        r.range ("attack " + fmtNum (atk * 1000.0) + " us: time to 63 % GR", s.attack * 1.0e6, 0.8 * atk * 1000.0,
                 1.2 * atk * 1000.0, "us");
    }
    for (float rel : { 50.0f, 1100.0f })
    {
        const auto s = fetSteps (c, 0.2f, rel, 0);
        r.range ("release " + fmtNum (rel) + " ms: time to 63 % recovery", s.release * 1000.0, 0.8 * rel, 1.2 * rel, "ms");
    }

    // program-dependent release: sustained heavy GR slows release, at most +40 %
    {
        const auto shortHold = fetSteps (c, 0.2f, 150.0f, 0, 0.0, 0.3, 10.0f);
        const auto longHold = fetSteps (c, 0.2f, 150.0f, 0, 0.0, 6.0, 10.0f);
        r.info ("program-dependent: GR during hold", longHold.grHold, "dB");
        r.range ("program-dependent: release after 6 s vs 0.3 s of >10 dB GR", longHold.release / shortHold.release,
                 1.15, 1.40, "x");
    }

    // All-buttons: faster attack than 20:1 at the same knob setting
    {
        const auto all = fetSteps (c, 0.8f, 150.0f, 4);
        const auto r20 = fetSteps (c, 0.8f, 150.0f, 3);
        r.less ("All mode: attack time relative to 20:1 (800 us knob)", all.attack / r20.attack, 0.8, "x");
    }
}

//==============================================================================
// 3. Opto ballistics
struct OptoRun
{
    double t50 = 0, t70 = 0, t90 = 0, tailTau = 0, unsmooth = 0, attack = 0;
};

double optoSteady (const Ctx& c, const duo::Params& p, double lvl)
{
    const double fs = c.fs * (1 << kDefaultOsLog2);
    OptoUT o;
    o.prepare (fs, c.nCh);
    o.set (p);
    const int n = (int) (0.8 * fs), m = (int) (0.2 * fs);
    const double a = fromDb (lvl), w = 2.0 * kPi * 1000.0 / fs;
    float x[2] {};
    double sum = 0.0;
    for (int i = 0; i < n; ++i)
    {
        x[0] = x[1] = (float) (a * std::sin (w * i));
        const float g = o.frame (x);
        if (i >= n - m)
            sum += g;
    }
    return sum / m;
}

OptoRun optoRun (const Ctx& c, const duo::Params& p, double lvl, double holdSec)
{
    const double fs = c.fs * (1 << kDefaultOsLog2);
    OptoUT o;
    o.prepare (fs, c.nCh);
    o.set (p);
    const int up = (int) (0.05 * fs), down = up + (int) (holdSec * fs), n = down + (int) (16.0 * fs);
    std::vector<float> gr ((size_t) n);
    const double a = fromDb (lvl), aq = fromDb (-90.0), w = 2.0 * kPi * 1000.0 / fs;
    float x[2] {};
    for (int i = 0; i < n; ++i)
    {
        const double amp = (i >= up && i < down) ? a : aq;
        x[0] = x[1] = (float) (amp * std::cos (w * (i - up)));
        gr[(size_t) i] = o.frame (x);
    }
    OptoRun res;
    // attack: GR settles well within 0.4 s; use the mean over 0.3-0.4 s after the step
    if (holdSec >= 0.5)
    {
        double g = 0.0;
        const int a0 = up + (int) (0.3 * fs), a1 = up + (int) (0.4 * fs);
        for (int i = a0; i < a1; ++i)
            g += gr[(size_t) i];
        g /= (a1 - a0);
        res.attack = crossUp (gr, up, 0.632 * g, fs);
    }
    const double g0 = gr[(size_t) down - 1];
    res.t50 = crossDown (gr, down, 0.5 * g0, fs);
    res.t70 = crossDown (gr, down, 0.3 * g0, fs);
    res.t90 = crossDown (gr, down, 0.1 * g0, fs);
    res.tailTau = (res.t90 - res.t70) / std::log (3.0);

    // smoothness: per-sample GR slope vs. its own 1 ms moving average. Coefficient stepping
    // (e.g. updating the release every 32 samples) shows up as a periodic slope residual.
    const int from = down + (int) (0.005 * fs);
    const int to = std::min (n - 1, down + (int) (std::min (res.t90, 10.0) * fs));
    const int half = std::max (2, (int) (0.0005 * fs));
    double maxSlope = 0.0, maxResid = 0.0;
    for (int i = from + half + 1; i < to - half; ++i)
    {
        const double d = (double) gr[(size_t) i] - gr[(size_t) i - 1];
        double avg = 0.0;
        for (int j = i - half; j <= i + half; ++j)
            avg += (double) gr[(size_t) j] - gr[(size_t) j - 1];
        avg /= (2 * half + 1);
        maxSlope = std::max (maxSlope, std::abs (d));
        maxResid = std::max (maxResid, std::abs (d - avg));
    }
    res.unsmooth = maxResid / std::max (1.0e-20, maxSlope);
    return res;
}

void testOptoBallistics (Recorder& r)
{
    r.section = "3 Opto attack/release";
    const Ctx& c = r.ctx;
    duo::Params p;
    p.fetOn = false;
    p.optoPR = 50.0f;
    p.colorPct = 0.0f;
    const double lvl = levelForGr ([&] (double l) { return optoSteady (c, p, l); }, 10.0);

    const auto h1 = optoRun (c, p, lvl, 1.0);
    const auto h2 = optoRun (c, p, lvl, 2.0);
    const auto h10 = optoRun (c, p, lvl, 10.0);
    r.range ("attack: time to 63 % of 10 dB GR", h2.attack * 1000.0, 8.0, 12.0, "ms");
    r.range ("release after 2 s of 10 dB GR: 50 % recovery", h2.t50 * 1000.0, 40.0, 80.0, "ms");
    r.range ("release tail time constant after 2 s (70->90 % recovery)", h2.tailTau, 0.5, 5.0, "s");
    r.info ("90 % recovery after 1 s of 10 dB GR", h1.t90, "s");
    r.info ("90 % recovery after 10 s of 10 dB GR", h10.t90, "s");
    r.greater ("light-cell memory: 90 % recovery 10 s hold / 1 s hold", h10.t90 / h1.t90, 1.5, "x");
    r.less ("release smoothness (slope residual vs slope, 10 s hold)", h10.unsmooth, 0.01, "ratio");
}

//==============================================================================
// 4. THD
Thd engineThd (const Ctx& c, const duo::Params& p, double lvl, double f0 = 1000.0)
{
    auto e = makeEngine (c, p);
    const int n = (int) (0.9 * c.fs), m = (int) (0.4 * c.fs);
    Buf in = zeros (c.nCh, n);
    addSine (in, c.fs, f0, fromDb (lvl));
    auto out = run (*e, in);
    return thd (out[0].data() + n - m, m, f0, c.fs);
}

void testThd (Recorder& r)
{
    r.section = "4 THD (1 kHz)";
    const Ctx& c = r.ctx;
    {
        duo::Params p;
        p.optoOn = false;
        p.colorPct = 0.0f;
        r.less ("FET 0 dB GR, Color 0", engineThd (c, p, -30.0).thd * 100.0, 0.1, "%");
    }
    double thd4 = 0.0;
    {
        duo::Params p;
        p.optoOn = false;
        p.colorPct = 50.0f;
        const double lvl = levelForGr ([&] (double l) { return steadyGr (c, p, l, true); }, 10.0);
        const auto t = engineThd (c, p, lvl);
        thd4 = t.thd;
        r.range ("FET 10 dB GR, Color 50 (4:1)", t.thd * 100.0, 0.3, 1.5, "%");
        r.greater ("FET 10 dB GR, Color 50: share of 2nd+3rd in distortion power", t.share23() * 100.0, 80.0, "%");
    }
    {
        duo::Params p;
        p.optoOn = false;
        p.colorPct = 50.0f;
        p.fetRatio = 4;
        const double lvl = levelForGr ([&] (double l) { return steadyGr (c, p, l, true); }, 10.0);
        const auto t = engineThd (c, p, lvl);
        r.info ("FET All 10 dB GR, Color 50", t.thd * 100.0, "%");
        r.greater ("FET All vs 4:1 THD at 10 dB GR, Color 50", t.thd / std::max (thd4, 1.0e-12), 1.25, "x");
    }
    {
        duo::Params p;
        p.fetOn = false;
        p.optoPR = 50.0f;
        p.colorPct = 0.0f;
        const double lvl = levelForGr ([&] (double l) { return steadyGr (c, p, l, false); }, 10.0);
        r.less ("Opto 10 dB GR, Color 0 (clean)", engineThd (c, p, lvl).thd * 100.0, 0.1, "%");
        p.colorPct = 50.0f;
        const auto hi = engineThd (c, p, lvl);
        const auto lo = engineThd (c, p, lvl - 20.0);
        r.info ("Opto 10 dB GR, Color 50", hi.thd * 100.0, "%");
        r.greater ("Opto Color 50: H2 / H3 (predominantly 2nd)", hi.h2 / std::max (hi.h3, 1.0e-12), 2.0, "x");
        r.greater ("Opto Color 50: THD rises with level (0 dB vs -20 dB rel.)", hi.thd / std::max (lo.thd, 1.0e-12), 1.3, "x");

        // DC-free tube colour
        p.colorPct = 100.0f;
        auto e = makeEngine (c, p);
        const int n = (int) (2.0 * c.fs), m = (int) (0.5 * c.fs);
        Buf in = zeros (c.nCh, n);
        addSine (in, c.fs, 1000.0, fromDb (lvl + 6.0));
        auto out = run (*e, in);
        double mean = 0.0;
        for (int i = n - m; i < n; ++i)
            mean += out[0][(size_t) i];
        r.less ("Opto Color 100: DC offset", db (std::abs (mean / m)), -100.0, "dBFS");
    }
}

//==============================================================================
// 5. Aliasing
double aliasDb (const Ctx& c, const duo::Params& p)
{
    auto e = makeEngine (c, p);
    const int settle = (int) (0.5 * c.fs), N = nextPow2 ((int) (0.75 * c.fs));
    Buf in = zeros (c.nCh, settle + N);
    addSine (in, c.fs, 15000.0, fromDb (-3.0));
    auto out = run (*e, in);
    const auto sp = spectrum (out[0].data() + settle, N);
    const double binHz = c.fs / N;
    double worst = 0.0;
    for (size_t k = (size_t) (20.0 / binHz) + 12; k < sp.size(); ++k)
    {
        const double f = (double) k * binHz;
        bool nearHarmonic = false;
        for (int h = 1; h * 15000.0 <= c.fs * 0.5 + 1.0; ++h)
            if (std::abs (f - h * 15000.0) <= 12.0 * binHz)
                nearHarmonic = true;
        if (! nearHarmonic)
            worst = std::max (worst, sp[k]);
    }
    return db (worst);
}

void testAliasing (Recorder& r)
{
    r.section = "5 Aliasing (15 kHz, -3 dBFS, FET 20:1, Color 100)";
    const Ctx& c = r.ctx;
    for (int chain = 0; chain < 2; ++chain)
    {
        duo::Params p;
        p.fetRatio = 3;
        p.colorPct = 100.0f;
        p.optoOn = chain == 1;
        const std::string name = chain == 0 ? "FET only" : "FET + Opto (default)";
        p.osLog2 = 0;
        r.info (name + ", 1x (no oversampling)", aliasDb (c, p), "dBFS");
        p.osLog2 = 1;
        r.less (name + ", 2x", aliasDb (c, p), -70.0, "dBFS");
        p.osLog2 = 2;
        r.less (name + ", 4x", aliasDb (c, p), -85.0, "dBFS");
    }
}

//==============================================================================
// 6. Null / latency
void testNull (Recorder& r)
{
    r.section = "6 Null / latency";
    const Ctx& c = r.ctx;
    const char* osName[] = { "1x", "2x", "4x" };
    for (int os = 0; os < 3; ++os)
    {
        // Mix 0 % must be the input delayed by exactly the reported latency
        duo::Params p;
        p.mixPct = 0.0f;
        p.osLog2 = os;
        auto e = makeEngine (c, p);
        const int L = e->getLatencySamples();
        const int n = (int) c.fs;
        Buf in = zeros (c.nCh, n);
        addNoise (in, 0.25, 11);
        auto out = run (*e, in);
        double res = 0.0;
        for (int ch = 0; ch < c.nCh; ++ch)
            for (int i = L; i < n; ++i)
                res = std::max (res, (double) std::abs (out[(size_t) ch][(size_t) i] - in[(size_t) ch][(size_t) (i - L)]));
        r.less (std::string ("Mix 0 %: residual vs input delayed by reported latency, ") + osName[os], db (res), -100.0, "dBFS");
        r.equal (std::string ("reported latency matches factor, ") + osName[os],
                 e->getLatencySamples() - e->latencyForOversampling (os), 0.0, "samples");

        // impulse through the full wet path (stages off) peaks exactly at the reported latency
        duo::Params t;
        t.fetOn = t.optoOn = false;
        t.colorPct = 0.0f;
        t.osLog2 = os;
        auto e2 = makeEngine (c, t);
        Buf imp = zeros (c.nCh, 4096);
        for (auto& ch : imp)
            ch[1000] = 0.5f;
        auto o2 = run (*e2, imp);
        size_t pk = 0;
        for (size_t i = 0; i < o2[0].size(); ++i)
            if (std::abs (o2[0][i]) > std::abs (o2[0][pk]))
                pk = i;
        r.equal (std::string ("wet impulse peak offset - reported latency, ") + osName[os],
                 (double) pk - 1000.0 - e2->getLatencySamples(), 0.0, "samples");

        // wet path (stages off) vs delayed dry at Mix 50 %: no comb filtering in parallel use
        t.mixPct = 50.0f;
        auto e3 = makeEngine (c, t);
        const int n3 = (int) (0.5 * c.fs);
        Buf ms = zeros (c.nCh, n3);
        for (double f = 100.0; f < 0.4 * c.fs && f < 18000.0; f *= 1.37)
            addSine (ms, c.fs, f, 0.04, 0, -1, f * 0.001);
        auto o3 = run (*e3, ms);
        const int L3 = e3->getLatencySamples();
        double r3 = 0.0;
        for (int i = L3 + 2000; i < n3; ++i)
            r3 = std::max (r3, (double) std::abs (o3[0][(size_t) i] - ms[0][(size_t) (i - L3)]));
        r.less (std::string ("Mix 50 %, stages off: residual vs delayed dry (<18 kHz), ") + osName[os], db (r3), -80.0, "dBFS");
    }
}

//==============================================================================
// 7. Silence / NaN / Inf
void testSilence (Recorder& r)
{
    r.section = "7 Silence / stability";
    const Ctx& c = r.ctx;
    {
        duo::Params p;
        auto e = makeEngine (c, p);
        const int hot = (int) (2.0 * c.fs), quiet = (int) (1.5 * c.fs);
        Buf in = zeros (c.nCh, hot + quiet);
        for (auto& ch : in)
            ch.resize ((size_t) hot);
        addNoise (in, fromDb (24.0) * 0.3, 5);
        for (auto& ch : in)
            ch.resize ((size_t) (hot + quiet), 0.0f);
        auto out = run (*e, in);
        r.less ("zeros after +24 dBFS noise: max |out| after 1 s of silence", maxAbs (out[0], hot + (int) c.fs), 1.0e-7, "");
    }
    {
        duo::Params p;
        auto e = makeEngine (c, p);
        Buf in = zeros (c.nCh, (int) c.fs);
        auto out = run (*e, in);
        r.less ("zeros from fresh instance: max |out|", maxAbs (out[0]), 1.0e-7, "");
    }
    {
        // +24 dBFS program with random automation of every parameter every block
        duo::Params p;
        auto e = makeEngine (c, p);
        const int n = (int) (6.0 * c.fs), q = (int) (2.0 * c.fs);
        Buf in = zeros (c.nCh, n + q);
        Buf prog = program (c.nCh, n, c.fs, 3);
        for (int ch = 0; ch < c.nCh; ++ch)
            for (int i = 0; i < n; ++i)
                in[(size_t) ch][(size_t) i] = prog[(size_t) ch][(size_t) i] * (float) fromDb (24.0) * 2.5f;
        Rng rng (77);
        auto out = run (*e, in, 256, [&] (int pos, int)
        {
            if (pos >= n)
                return;
            duo::Params a;
            a.fetOn = rng.uni() > 0.2;
            a.fetInDb = (float) (-12 + 42 * rng.uni());
            a.fetOutDb = (float) (-24 + 48 * rng.uni());
            a.fetAtkMs = (float) (0.02 + 0.78 * rng.uni());
            a.fetRelMs = (float) (50 + 1050 * rng.uni());
            a.fetRatio = (int) (rng.uni() * 5);
            a.optoOn = rng.uni() > 0.2;
            a.optoPR = (float) (100 * rng.uni());
            a.optoGainDb = (float) (-12 + 36 * rng.uni());
            a.optoMode = rng.uni() > 0.5;
            a.order = rng.uni() > 0.5;
            a.scHpfHz = (float) (20 + 280 * rng.uni());
            a.colorPct = (float) (100 * rng.uni());
            a.mixPct = (float) (100 * rng.uni());
            a.outDb = (float) (-24 + 48 * rng.uni());
            a.linkPct = (float) (100 * rng.uni());
            a.emphasisPct = (float) (100 * rng.uni());
            a.osLog2 = (int) (rng.uni() * 3);
            a.scListen = rng.uni() > 0.8;
            a.autoMakeup = rng.uni() > 0.5;
            e->setParams (a);
        });
        r.truth ("random automation every block, +24 dBFS: all samples finite", allFinite (out), "no NaN/Inf");
        r.less ("random automation: peak output (sanity bound)", db (maxAbs (out[0], 0, n)), 80.0, "dBFS");
        r.less ("random automation then silence: max |out| in last 0.5 s", maxAbs (out[0], n + q - (int) (0.5 * c.fs)), 1.0e-7, "");
    }
}

//==============================================================================
// 8. Block-size invariance, 9. stereo identity
void testBlocksAndStereo (Recorder& r)
{
    const Ctx& c = r.ctx;
    r.section = "8 Block-size invariance";
    const Buf in = program (c.nCh, (int) (2.0 * c.fs), c.fs, 9);
    const char* osName[] = { "1x", "2x", "4x" };
    for (int os = 0; os < 3; ++os)
    {
        duo::Params p;
        p.osLog2 = os;
        auto e = makeEngine (c, p);
        const auto ref = run (*e, in, 512);
        double worst = 0.0;
        for (int bs : { 1, 17, 64, 2048, 4096 })
        {
            e = makeEngine (c, p);
            const auto o = run (*e, in, bs);
            for (size_t ch = 0; ch < o.size(); ++ch)
                for (size_t i = 0; i < o[ch].size(); ++i)
                    worst = std::max (worst, (double) std::abs (o[ch][i] - ref[ch][i]));
        }
        r.less (std::string ("max |diff| blocks 1/17/64/2048/4096 vs 512, ") + osName[os], worst, 1.0e-5, "");
    }

    r.section = "9 Stereo identity";
    if (c.nCh == 2)
    {
        const Buf same = program (2, (int) (1.5 * c.fs), c.fs, 21, true);
        for (int variant = 0; variant < 2; ++variant)
        {
            duo::Params p;
            if (variant == 1)
            {
                p.osLog2 = 2;
                p.linkPct = 0.0f;
                p.autoMakeup = true;
                p.fetRatio = 4;
                p.colorPct = 100.0f;
                p.emphasisPct = 70.0f;
            }
            auto e = makeEngine (c, p);
            const auto o = run (*e, same);
            size_t diff = 0;
            for (size_t i = 0; i < o[0].size(); ++i)
                if (std::memcmp (&o[0][i], &o[1][i], sizeof (float)) != 0)
                    ++diff;
            r.equal (variant == 0 ? "identical L/R in: differing output samples (defaults)"
                                  : "identical L/R in: differing samples (4x, link 0, All, auto makeup)",
                     (double) diff, 0.0, "samples");
        }
        // mono instance produces the same as one side of a dual-mono stereo instance
        duo::Params p;
        auto es = makeEngine (c, p);
        const auto os2 = run (*es, same);
        Ctx mc = c;
        mc.nCh = 1;
        auto em = makeEngine (mc, p);
        const Buf mono (1, same[0]);
        const auto om = run (*em, mono);
        double d = 0.0;
        for (size_t i = 0; i < om[0].size(); ++i)
            d = std::max (d, (double) std::abs (om[0][i] - os2[0][i]));
        r.less ("mono instance vs dual-mono stereo: max |diff|", d, 1.0e-6, "");
    }
}

//==============================================================================
// Section 2 feature checks
void testFeatures (Recorder& r)
{
    r.section = "F Features (spec section 2)";
    const Ctx& c = r.ctx;

    // sidechain HPF keeps low end from driving the detectors
    for (int stage = 0; stage < 2; ++stage)
    {
        duo::Params p;
        p.fetOn = stage == 0;
        p.optoOn = stage == 1;
        p.optoPR = 60.0f;
        p.colorPct = 0.0f;
        p.scHpfHz = 20.0f;
        const double g20 = steadyGr (c, p, -6.0, stage == 0, 60.0, 1.0);
        p.scHpfHz = 300.0f;
        const double g300 = steadyGr (c, p, -6.0, stage == 0, 60.0, 1.0);
        r.greater (std::string (stage == 0 ? "FET" : "Opto") + " SC HPF: GR reduction on 60 Hz, HPF 20 -> 300 Hz",
                   g20 - g300, 6.0, "dB");
    }

    // SC listen outputs what the detector hears
    {
        duo::Params p;
        p.scListen = true;
        p.scHpfHz = 300.0f;
        auto e = makeEngine (c, p);
        const int n = (int) c.fs, m = (int) (0.4 * c.fs);
        Buf in = zeros (c.nCh, n);
        addSine (in, c.fs, 50.0, 0.25);
        addSine (in, c.fs, 3000.0, 0.25);
        auto out = run (*e, in);
        const double a50 = toneAmp (out[0].data() + n - m, m, 50.0, c.fs);
        const double a3k = toneAmp (out[0].data() + n - m, m, 3000.0, c.fs);
        r.less ("SC Listen (HPF 300 Hz): 50 Hz relative to 3 kHz in output", db (a50 / std::max (a3k, 1.0e-12)), -20.0, "dB");
    }

    // opto HF emphasis
    {
        duo::Params p;
        p.fetOn = false;
        p.optoPR = 50.0f;
        p.colorPct = 0.0f;
        for (double f : { 8000.0, 200.0 })
        {
            if (f >= 0.45 * c.fs)
                continue;
            p.emphasisPct = 0.0f;
            const double lvl = levelForGr ([&] (double l) { return steadyGr (c, p, l, false, f); }, 3.0);
            const double g0 = steadyGr (c, p, lvl, false, f);
            p.emphasisPct = 100.0f;
            const double g1 = steadyGr (c, p, lvl, false, f);
            if (f > 1000.0)
                r.greater ("Emphasis 100 %: extra opto GR on 8 kHz", g1 - g0, 4.0, "dB");
            else
                r.less ("Emphasis 100 %: GR change on 200 Hz (should be ~0)", std::abs (g1 - g0), 0.5, "dB");
        }
    }

    // stereo link
    if (c.nCh == 2)
    {
        for (float link : { 100.0f, 0.0f })
        {
            duo::Params p;
            p.optoOn = false;
            p.colorPct = 0.0f;
            p.linkPct = link;
            auto e = makeEngine (c, p);
            const int n = (int) c.fs, m = (int) (0.3 * c.fs);
            Buf in = zeros (2, n);
            for (int i = 0; i < n; ++i)
            {
                in[0][(size_t) i] = (float) (fromDb (-6.0) * std::sin (2.0 * kPi * 1000.0 * i / c.fs));
                in[1][(size_t) i] = (float) (fromDb (-40.0) * std::sin (2.0 * kPi * 1000.0 * i / c.fs));
            }
            auto out = run (*e, in);
            const double grL = (-6.0 + 6.0) - db (toneAmp (out[0].data() + n - m, m, 1000.0, c.fs));
            const double grR = (-40.0 + 6.0) - db (toneAmp (out[1].data() + n - m, m, 1000.0, c.fs));
            if (link > 50.0f)
                r.less ("Link 100 %: quiet side follows loud side's GR (|GR_L - GR_R|)", std::abs (grL - grR), 0.5, "dB");
            else
                r.less ("Link 0 %: quiet side GR (independent detection)", grR, 0.2, "dB");
        }
    }

    // auto makeup: slow (>= 1 s) compensation of the average GR
    {
        duo::Params p;
        p.autoMakeup = true;
        auto e = makeEngine (c, p);
        const int n = (int) (14.0 * c.fs);
        Buf in = zeros (c.nCh, n);
        addSine (in, c.fs, 1000.0, fromDb (-6.0));
        std::vector<float> traj;
        GrTrace tr;
        const size_t cap = (size_t) n * 4 + 64;
        tr.fet.assign (cap, 0.0f);
        tr.opto.assign (cap, 0.0f);
        tr.rate = e->getOversampledRate();
        duo::Probe probe { tr.fet.data(), tr.opto.data(), (int) cap, 0 };
        e->setProbe (&probe);
        const int block = 64;
        run (*e, in, block, [&] (int, int)
        {
            traj.push_back (e->getMeters().makeupDb);
        });
        e->setProbe (nullptr);
        tr.fet.resize ((size_t) probe.count);
        tr.opto.resize ((size_t) probe.count);
        const double avgGr = tr.meanTail (true, 2.0) + tr.meanTail (false, 2.0);
        const double final = traj.back();
        r.less ("Auto makeup: final makeup vs average GR (|diff|)", std::abs (final - avgGr), 1.5, "dB");
        double t63 = 1.0e9;
        for (size_t i = 0; i < traj.size(); ++i)
            if (traj[i] >= 0.632 * final && final > 0.5)
            {
                t63 = (double) i * block / c.fs;
                break;
            }
        r.range ("Auto makeup: time to 63 % of final makeup (>= 1 s)", t63, 1.0, 6.0, "s");

        // extreme settings (~60 dB of GR): auto makeup is capped at +24 dB
        duo::Params x;
        x.autoMakeup = true;
        x.fetInDb = 30.0f;
        x.fetRatio = 3;
        x.optoPR = 100.0f;
        auto ex = makeEngine (c, x);
        Buf hot = zeros (c.nCh, (int) (12.0 * c.fs));
        addSine (hot, c.fs, 1000.0, 1.0);
        run (*ex, hot);
        r.less ("Auto makeup: cap under extreme GR", ex->getMeters().makeupDb, 24.001, "dB");
    }

    // oversampling switching: no NaN, no runaway, latency follows the factor
    {
        duo::Params p;
        auto e = makeEngine (c, p);
        const int n = (int) (3.0 * c.fs);
        Buf in = program (c.nCh, n, c.fs, 13);
        Rng rng (5);
        int blockCount = 0;
        auto out = run (*e, in, 256, [&] (int, int)
        {
            if (++blockCount % 37 == 0)
            {
                p.osLog2 = (int) (rng.uni() * 3);
                e->setParams (p);
            }
        });
        r.truth ("OS switching every 37 blocks: all samples finite", allFinite (out), "no NaN/Inf");
        r.less ("OS switching: peak output vs peak input", db (maxAbs (out[0]) / maxAbs (in[0])), 12.0, "dB");

        // settle at 4x with stages off, then check the impulse lands on the reported latency
        p.osLog2 = 2;
        p.fetOn = p.optoOn = false;
        p.colorPct = 0.0f;
        e->setParams (p);
        Buf z = zeros (c.nCh, (int) (0.3 * c.fs));
        run (*e, z);
        Buf imp = zeros (c.nCh, 8192);
        for (auto& ch : imp)
            ch[100] = 0.5f;
        auto o = run (*e, imp);
        size_t pk = 0;
        for (size_t i = 0; i < o[0].size(); ++i)
            if (std::abs (o[0][i]) > std::abs (o[0][pk]))
                pk = i;
        r.equal ("OS switched to 4x: reported latency - latency of 4x", e->getLatencySamples() - e->latencyForOversampling (2), 0.0, "samples");
        r.equal ("OS switched to 4x: impulse peak - reported latency", (double) pk - 100.0 - e->getLatencySamples(), 0.0, "samples");
    }

    // stage-order switch while playing: faded, no click (max sample step stays near the tone's own)
    {
        duo::Params p;
        p.colorPct = 0.0f;
        auto e = makeEngine (c, p);
        const int n = (int) (1.0 * c.fs), sw = (int) (0.5 * c.fs);
        Buf in = zeros (c.nCh, n);
        addSine (in, c.fs, 200.0, fromDb (-6.0));
        auto out = run (*e, in, 64, [&] (int pos, int len)
        {
            if (pos <= sw && sw < pos + len)
            {
                p.order = 1;
                e->setParams (p);
            }
        });
        auto maxStep = [&] (int from, int to)
        {
            double m = 0.0;
            for (int i = from + 1; i < to; ++i)
                m = std::max (m, (double) std::abs (out[0][(size_t) i] - out[0][(size_t) i - 1]));
            return m;
        };
        const double steady = maxStep ((int) (0.3 * c.fs), sw);
        const double around = maxStep (sw - 64, sw + (int) (0.05 * c.fs));
        r.less ("Order switch while playing: max sample step vs steady tone", around / steady, 1.5, "x");
    }

    // parameter smoothing (no zipper): 10-90 % transition time of gain changes
    {
        auto riseMs = [&] (duo::Params a, duo::Params b)
        {
            auto e = makeEngine (c, a);
            const int n = (int) (0.6 * c.fs), sw = (int) (0.2 * c.fs);
            Buf in = zeros (c.nCh, n);
            addSine (in, c.fs, 1000.0, fromDb (-40.0));
            auto out = run (*e, in, 64, [&] (int pos, int len) { if (pos <= sw && sw < pos + len) e->setParams (b); });
            // 1 ms peak envelope
            const int w = (int) (0.001 * c.fs);
            std::vector<double> env;
            for (int i = 0; i + w <= n; i += w)
                env.push_back (maxAbs (out[0], i, i + w));
            const double v0 = env[(size_t) (sw / w) - 5], v1 = env.back();
            const double lo = v0 + 0.1 * (v1 - v0), hi = v0 + 0.9 * (v1 - v0);
            int i10 = -1, i90 = -1;
            for (size_t i = (size_t) (sw / w) - 2; i < env.size(); ++i)
            {
                const bool past10 = v1 > v0 ? env[i] >= lo : env[i] <= lo;
                const bool past90 = v1 > v0 ? env[i] >= hi : env[i] <= hi;
                if (i10 < 0 && past10) i10 = (int) i;
                if (i90 < 0 && past90) i90 = (int) i;
            }
            return (double) (i90 - i10);
        };
        duo::Params a;
        a.colorPct = 0.0f;
        a.optoPR = 0.0f;
        duo::Params b = a;
        b.outDb = 12.0f;
        r.greater ("Output +12 dB step: 10-90 % transition", riseMs (a, b), 5.0, "ms");
        b = a;
        b.fetOutDb = -20.0f;
        r.greater ("FET Output -20 dB step: 10-90 % transition", riseMs (a, b), 5.0, "ms");
        b = a;
        b.optoGainDb = 12.0f;
        r.greater ("Opto Gain +12 dB step: 10-90 % transition", riseMs (a, b), 5.0, "ms");
        b = a;
        b.fetInDb = -12.0f;
        r.greater ("FET Input -18 dB step: 10-90 % transition", riseMs (a, b), 5.0, "ms");
        duo::Params m1 = a;
        m1.fetOutDb = -20.0f;
        duo::Params m2 = m1;
        m2.mixPct = 0.0f;
        r.greater ("Mix 100 -> 0 % step (wet 20 dB lower): 10-90 % transition", riseMs (m1, m2), 5.0, "ms");
    }
}


//==============================================================================
// O. Oversampling filters: design verification + audio-band flatness of the wet path
double firMagDb (const std::vector<double>& h, double f) // f = fraction of the filter's rate
{
    double re = 0.0, im = 0.0;
    for (size_t i = 0; i < h.size(); ++i)
    {
        re += h[i] * std::cos (2.0 * kPi * f * (double) i);
        im -= h[i] * std::sin (2.0 * kPi * f * (double) i);
    }
    return db (std::sqrt (re * re + im * im));
}

void testOversamplingFilters (Recorder& r)
{
    r.section = "O Oversampling filters";
    const Ctx& c = r.ctx;
    duo::Engine e;
    e.prepare (c.fs, c.nCh);
    const auto& os = e.oversampler();
    struct F { const char* name; const std::vector<double>* h; double pass, stop; };
    const bool hr = os.isHighRateDesign();
    std::vector<F> fl;
    fl.push_back ({ "2x stage (up/down)", &os.firstStage().coefficients(), hr ? 0.15 : 0.4535 / 2.0, hr ? 0.35 : 0.5 - 0.4535 / 2.0 });
    if (hr)
        fl.push_back ({ "2x decimator (stopband from base Nyquist)", &os.highRateDecimator().coefficients(), 0.15, 0.25 });
    fl.push_back ({ "4x stage", &os.secondStage().coefficients(), 0.1366, 0.5 - 0.1366 });
    for (auto& f : fl)
    {
        double stopMax = -1000.0, passDev = 0.0;
        for (int i = 0; i <= 4000; ++i)
        {
            const double fp = f.pass * i / 4000.0;
            passDev = std::max (passDev, std::abs (firMagDb (*f.h, fp)));
            const double fsx = f.stop + (0.5 - f.stop) * i / 4000.0;
            stopMax = std::max (stopMax, firMagDb (*f.h, fsx));
        }
        r.less (std::string (f.name) + ": stopband peak", stopMax, -100.0, "dB");
        r.less (std::string (f.name) + ": passband ripple", passDev, 0.001, "dB");
    }

    // complete wet path with stages off: magnitude flat 20 Hz .. 20 kHz
    const char* osName[] = { "1x", "2x", "4x" };
    for (int f = 0; f < 3; ++f)
    {
        duo::Params p;
        p.fetOn = p.optoOn = false;
        p.colorPct = 0.0f;
        p.osLog2 = f;
        auto eng = makeEngine (c, p);
        r.info (std::string ("latency ") + osName[f], eng->getLatencySamples(), "samples");
        const int N = 65536;
        Buf imp = zeros (c.nCh, N);
        for (auto& ch : imp)
            ch[0] = 1.0f;
        const auto o = run (*eng, imp);
        std::vector<std::complex<double>> a ((size_t) N);
        for (int i = 0; i < N; ++i)
            a[(size_t) i] = o[0][(size_t) i];
        fft (a);
        double dev = 0.0;
        for (int k = 1; k < N / 2; ++k)
        {
            const double fr = k * c.fs / N;
            if (fr >= 20.0 && fr <= 20000.0)
                dev = std::max (dev, std::abs (db (std::abs (a[(size_t) k]))));
        }
        r.less (std::string ("wet path (stages off) flatness 20 Hz-20 kHz, ") + osName[f], dev, 0.01, "dB");
    }
}


//==============================================================================
// P. Factory presets: loudness-matched and doing a sensible amount of work
void testPresets (Recorder& r)
{
    r.section = "P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS)";
    const Ctx& c = r.ctx;
    const int n = (int) (8.0 * c.fs), from = (int) (2.0 * c.fs);
    auto normalise = [n] (Buf b, double rmsDb)
    {
        double s = 0.0;
        for (float v : b[0])
            s += (double) v * v;
        const double g = fromDb (rmsDb) / std::sqrt (s / n);
        for (auto& ch : b)
            for (auto& v : ch)
                v *= (float) g;
        return b;
    };
    const Buf vox = normalise (program (c.nCh, n, c.fs, 42), -18.0);
    const Buf mixSig = normalise (mixProgram (c.nCh, n, c.fs, 7), -14.0);
    for (auto& fp : duo::factoryPresets())
    {
        if (fp.category == duo::PresetCategory::utility)
            continue;
        const Buf& in = fp.category == duo::PresetCategory::mixMaster ? mixSig : vox;
        auto e = makeEngine (c, fp.params);
        double fetMax = 0.0, optoSum = 0.0;
        int nb = 0;
        const int block = (int) (0.01 * c.fs);
        auto out = run (*e, in, block, [&] (int pos, int)
        {
            if (pos > from)
            {
                fetMax = std::max (fetMax, (double) e->getMeters().fetGrDb);
                optoSum += e->getMeters().optoGrDb;
                ++nb;
            }
            e->clearMeters();
        });
        double so = 0.0, si = 0.0;
        for (int i = from; i < n; ++i)
        {
            so += (double) out[0][(size_t) i] * out[0][(size_t) i];
            si += (double) in[0][(size_t) i] * in[0][(size_t) i];
        }
        r.range (std::string (fp.name) + ": output vs input loudness", 10.0 * std::log10 (so / si), -1.5, 1.5, "dB");
        r.truth (std::string (fp.name) + ": output finite", allFinite (out), "no NaN/Inf");
        r.info (std::string (fp.name) + ": FET peak GR", fetMax, "dB");
        r.info (std::string (fp.name) + ": opto average GR", optoSum / std::max (1, nb), "dB");
    }
}

} // namespace
#include "V2Tests.h"
#include "V3Tests.h"
namespace
{
//==============================================================================
// 10. CPU benchmark (sequential, after the parallel tests)
void benchmark (Recorder& r)
{
    r.section = "10 CPU";
    auto time = [] (double fs, int nCh, int block, const duo::Params& p, double seconds, bool silenceAfter, double* silenceTime)
    {
        duo::Engine e;
        e.prepare (fs, nCh);
        e.setParams (p);
        e.reset();
        const int n = (int) (seconds * fs);
        Buf in = program (nCh, n, fs, 1);
        float* ptr[2] {};
        // warm-up
        for (int pos = 0; pos + block <= (int) (0.5 * fs); pos += block)
        {
            for (int ch = 0; ch < nCh; ++ch)
                ptr[ch] = in[(size_t) ch].data() + pos;
            e.process (ptr, nCh, block);
        }
        Buf work = in;
        const auto t0 = std::chrono::steady_clock::now();
        for (int pos = 0; pos + block <= n; pos += block)
        {
            for (int ch = 0; ch < nCh; ++ch)
                ptr[ch] = work[(size_t) ch].data() + pos;
            e.process (ptr, nCh, block);
        }
        const double t = std::chrono::duration<double> (std::chrono::steady_clock::now() - t0).count();
        if (silenceAfter)
        {
            Buf z = zeros (nCh, n);
            const auto s0 = std::chrono::steady_clock::now();
            for (int pos = 0; pos + block <= n; pos += block)
            {
                for (int ch = 0; ch < nCh; ++ch)
                    ptr[ch] = z[(size_t) ch].data() + pos;
                e.process (ptr, nCh, block);
            }
            *silenceTime = std::chrono::duration<double> (std::chrono::steady_clock::now() - s0).count();
        }
        return t;
    };

    duo::Params p;
    double silence = 0.0;
    const double t = time (48000.0, 2, 128, p, 60.0, true, &silence);
    r.less ("48 kHz / 128 / stereo / 2x, 60 s program: CPU of one core", t / 60.0 * 100.0, 2.0, "%");
    r.info ("48 kHz / 128 / stereo / 2x: real-time factor", 60.0 / t, "x");
    r.less ("silence after program (denormal safety): CPU silence / CPU program", silence / t, 1.5, "x");
    for (int os : { 0, 2 })
    {
        p.osLog2 = os;
        r.info (std::string ("48 kHz / 128 / stereo / ") + (os == 0 ? "1x" : "4x") + ": CPU of one core",
                time (48000.0, 2, 128, p, 20.0, false, nullptr) / 20.0 * 100.0, "%");
    }
    {
        // 2.0, every feature on: 4x, Clean detector, lookahead, Mid/Side, side-chain EQ, Measured opto, limiter
        auto q = p;
        q.osLog2 = 2;
        q.fetClean = true;
        q.lookaheadMs = 2.0f;
        q.midSide = true;
        q.scLpfHz = 8000.0f;
        q.scBellDb = 4.0f;
        q.optoMeasured = true;
        q.optoPR = 65.0f;
        q.limOn = true;
        r.less ("48 kHz / 128 / stereo / 4x, every 2.0 feature on (limiter, lookahead, M/S, Clean, SC EQ): CPU of one core",
                time (48000.0, 2, 128, q, 20.0, false, nullptr) / 20.0 * 100.0, 8.0, "%");
    }
    p.osLog2 = 1;
    for (double fs : { 44100.0, 96000.0, 192000.0 })
        r.info ("stereo / 128 / 2x at " + fmtNum (fs / 1000.0) + " kHz: CPU of one core",
                time (fs, 2, 128, p, 20.0, false, nullptr) / 20.0 * 100.0, "%");
}

std::vector<double> parseList (const char* s)
{
    std::vector<double> v;
    std::string cur;
    for (const char* p = s;; ++p)
    {
        if (*p == ',' || *p == 0)
        {
            if (! cur.empty())
                v.push_back (std::stod (cur));
            cur.clear();
            if (*p == 0)
                break;
        }
        else
            cur += *p;
    }
    return v;
}
} // namespace

int main (int argc, char** argv)
{
    std::vector<double> rates { 44100.0, 48000.0, 88200.0, 96000.0, 192000.0 };
    std::vector<double> chans { 1, 2 };
    std::string report = "tests/report.md";
    bool bench = true, testsOn = true;
    for (int i = 1; i < argc; ++i)
    {
        const std::string a = argv[i];
        if (a == "--rates" && i + 1 < argc) rates = parseList (argv[++i]);
        else if (a == "--ch" && i + 1 < argc) chans = parseList (argv[++i]);
        else if (a == "--report" && i + 1 < argc) report = argv[++i];
        else if (a == "--no-bench") bench = false;
        else if (a == "--bench-only") testsOn = false;
        else
        {
            std::cerr << "unknown argument " << a << "\n";
            return 2;
        }
    }

    const auto t0 = std::chrono::steady_clock::now();
    std::vector<Ctx> ctxs;
    for (double fs : rates)
        for (double ch : chans)
            ctxs.push_back ({ fs, (int) ch });

    std::vector<Recorder> recs;
    for (auto& c : ctxs)
        recs.emplace_back (c);

    if (testsOn)
    {
        std::vector<std::thread> threads;
        for (auto& rec : recs)
            threads.emplace_back ([&rec]
            {
                testStaticCurves (rec);
                testFetBallistics (rec);
                testOptoBallistics (rec);
                testThd (rec);
                testAliasing (rec);
                testNull (rec);
                testSilence (rec);
                testBlocksAndStereo (rec);
                testFeatures (rec);
                testOversamplingFilters (rec);
                testPresets (rec);
                v2::testV2 (rec);
                v3::testV3 (rec);
            });
        for (auto& t : threads)
            t.join();
    }

    std::vector<Result> all;
    std::vector<std::string> configs;
    for (auto& rec : recs)
    {
        configs.push_back (rec.ctx.tag());
        all.insert (all.end(), rec.results.begin(), rec.results.end());
    }
    if (bench)
    {
        Recorder br ({ 48000.0, 2 });
        benchmark (br);
        all.insert (all.end(), br.results.begin(), br.results.end());
        if (std::find (configs.begin(), configs.end(), br.ctx.tag()) == configs.end())
            configs.push_back (br.ctx.tag());
    }

    const double secs = std::chrono::duration<double> (std::chrono::steady_clock::now() - t0).count();
    const auto now = std::chrono::system_clock::to_time_t (std::chrono::system_clock::now());
    char when[64];
    std::strftime (when, sizeof when, "%Y-%m-%d %H:%M", std::localtime (&now));
    std::string header = std::string ("Generated ") + when + " by `DuoDspTests` in " + fmtNum (secs) + " s. "
                       + "Stage ballistics are measured on the stage at the default 2x internal rate with a sample-exact step; "
                       + "everything else runs through the complete engine (oversampling, mix, output).";
#ifdef DUO_TEST_BUILD_INFO
    header += std::string (" Build: ") + DUO_TEST_BUILD_INFO + ".";
#endif
    writeReport (all, configs, report, header);

    int failed = 0;
    for (auto& r : all)
        if (! r.info && ! r.pass)
        {
            ++failed;
            std::cout << "FAIL  [" << r.config << "] " << r.section << " / " << r.check << ": " << fmtNum (r.value) << " "
                      << r.unit << " (target " << r.target << ")\n";
        }
    std::cout << (failed == 0 ? "ALL CHECKS PASSED" : "CHECKS FAILED: " + std::to_string (failed)) << " (" << all.size()
              << " results, " << fmtNum (secs) << " s). Report: " << report << "\n";
    return std::min (failed, 250);
}

// heap allocation counter for the real-time checks (V10): counts only while v2::countAllocs is set on the thread
void* operator new (std::size_t n)
{
    if (v2::countAllocs)
        ++v2::allocCount;
    if (void* p = std::malloc (n == 0 ? 1 : n))
        return p;
    throw std::bad_alloc();
}
void* operator new[] (std::size_t n) { return operator new (n); }
void operator delete (void* p) noexcept { std::free (p); }
void operator delete[] (void* p) noexcept { std::free (p); }
void operator delete (void* p, std::size_t) noexcept { std::free (p); }
void operator delete[] (void* p, std::size_t) noexcept { std::free (p); }
