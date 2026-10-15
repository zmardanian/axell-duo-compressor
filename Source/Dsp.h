#pragma once
// Axell Duo Compressor - DSP core (framework independent, no JUCE: unit-testable with plain g++)
// Stage 1: FET (1176-style) peak compressor
// Stage 2: Opto (LA-2A-style) leveling amplifier with two-stage, program-dependent release
//
// Both stages run at the oversampled rate. Each stage can be prepared for 1x/2x/4x and
// switched between them without allocating (coefficients for every factor are precomputed).

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

namespace duo
{
constexpr int kMaxCh = 2;
constexpr double kPi = 3.141592653589793;
constexpr int kNumOsFactors = 3; // 1x, 2x, 4x

//==============================================================================
// Fast, deterministic dB conversions (called per oversampled sample).
// fastLog2: |error| < 1e-6 (=> < 1e-5 dB). fastExp2: relative error < 2e-7, exp2(0) == 1 exactly.
inline float fastLog2 (float x) noexcept
{
    uint32_t bits;
    std::memcpy (&bits, &x, sizeof bits);
    int e = (int) ((bits >> 23) & 0xffu) - 127;
    bits = (bits & 0x007fffffu) | 0x3f800000u;
    float m;
    std::memcpy (&m, &bits, sizeof m);
    if (m > 1.41421356f)
    {
        m *= 0.5f;
        ++e;
    }
    const float t = (m - 1.0f) / (m + 1.0f);
    const float t2 = t * t;
    const float s = t * (1.0f + t2 * (0.333333333f + t2 * (0.2f + t2 * 0.142857143f)));
    return (float) e + 2.88539008f * s;
}

inline float fastExp2 (float x) noexcept
{
    x = std::clamp (x, -126.0f, 126.0f);
    const float r = std::floor (x + 0.5f);
    const float f = (x - r) * 0.693147181f; // |f| <= 0.3466
    const float p = 1.0f + f * (1.0f + f * (0.5f + f * (0.166666667f + f * (0.0416666667f + f * (0.00833333333f + f * 0.00138888889f)))));
    const uint32_t bits = (uint32_t) ((int) r + 127) << 23;
    float scale;
    std::memcpy (&scale, &bits, sizeof scale);
    return p * scale;
}

inline float dbToGain (float db) noexcept { return fastExp2 (db * 0.166096405f); }
inline float gainToDb (float g) noexcept { return 6.02059991f * fastLog2 (std::max (g, 1.0e-9f)); }
inline float powerToDb (float p) noexcept { return 3.01029996f * fastLog2 (std::max (p, 1.0e-18f)); }

// one-pole smoothing coefficient for a time constant in seconds (computed at block rate)
inline float onePole (float seconds, double fs) noexcept
{
    return seconds <= 0.0f ? 0.0f : (float) std::exp (-1.0 / ((double) seconds * fs));
}

// 1 - exp(-x) for the small per-sample rates of slow envelopes (x < 0.01): avoids a
// transcendental per sample and lets release times change smoothly every sample.
inline float oneMinusExpSmall (float x) noexcept { return x * (1.0f - x * (0.5f - x * 0.166666667f)); }

// static soft-knee curve, returns gain reduction in dB (positive number)
inline float gainReductionDb (float levelDb, float thresholdDb, float ratio, float kneeDb) noexcept
{
    const float over  = levelDb - thresholdDb;
    const float slope = 1.0f - 1.0f / ratio;

    if (kneeDb > 0.0f)
    {
        if (2.0f * over < -kneeDb)
            return 0.0f;
        if (2.0f * std::abs (over) <= kneeDb)
        {
            const float t = over + kneeDb * 0.5f;
            return slope * t * t / (2.0f * kneeDb);
        }
    }
    else if (over <= 0.0f)
    {
        return 0.0f;
    }
    return slope * over;
}

//==============================================================================
// RBJ biquad (high-pass / high-shelf), double precision state, up to kMaxCh channels.
class Biquad
{
public:
    void setHighPass (double fs, double freq, double q = 0.7071067811865476)
    {
        if (std::abs (freq - lastFreq) < 1.0e-9 && std::abs (fs - lastFs) < 1.0e-9 && lastType == 1)
            return;
        lastFreq = freq;
        lastFs = fs;
        lastType = 1;
        const double w0 = 2.0 * kPi * std::min (freq, fs * 0.45) / fs;
        const double cosw = std::cos (w0), alpha = std::sin (w0) / (2.0 * q), a0 = 1.0 + alpha;
        b0 = (1.0 + cosw) * 0.5 / a0;
        b1 = -(1.0 + cosw) / a0;
        b2 = b0;
        a1 = -2.0 * cosw / a0;
        a2 = (1.0 - alpha) / a0;
    }

    void setHighShelf (double fs, double freq, double gainDb)
    {
        if (std::abs (freq - lastFreq) < 1.0e-9 && std::abs (fs - lastFs) < 1.0e-9 && lastType == 2
            && std::abs (gainDb - lastGain) < 1.0e-9)
            return;
        lastFreq = freq;
        lastFs = fs;
        lastType = 2;
        lastGain = gainDb;
        const double A = std::pow (10.0, gainDb / 40.0);
        const double w0 = 2.0 * kPi * std::min (freq, fs * 0.45) / fs;
        const double cosw = std::cos (w0), sinw = std::sin (w0);
        const double alpha = sinw / 2.0 * std::sqrt (2.0); // shelf slope S = 1
        const double sq = 2.0 * std::sqrt (A) * alpha;
        const double a0 = (A + 1.0) - (A - 1.0) * cosw + sq;
        b0 = A * ((A + 1.0) + (A - 1.0) * cosw + sq) / a0;
        b1 = -2.0 * A * ((A - 1.0) + (A + 1.0) * cosw) / a0;
        b2 = A * ((A + 1.0) + (A - 1.0) * cosw - sq) / a0;
        a1 = 2.0 * ((A - 1.0) - (A + 1.0) * cosw) / a0;
        a2 = ((A + 1.0) - (A - 1.0) * cosw - sq) / a0;
    }

    void setLowPass (double fs, double freq, double q = 0.7071067811865476)
    {
        if (std::abs (freq - lastFreq) < 1.0e-9 && std::abs (fs - lastFs) < 1.0e-9 && lastType == 3)
            return;
        lastFreq = freq;
        lastFs = fs;
        lastType = 3;
        const double w0 = 2.0 * kPi * std::min (freq, fs * 0.45) / fs;
        const double cosw = std::cos (w0), alpha = std::sin (w0) / (2.0 * q), a0 = 1.0 + alpha;
        b0 = (1.0 - cosw) * 0.5 / a0;
        b1 = (1.0 - cosw) / a0;
        b2 = b0;
        a1 = -2.0 * cosw / a0;
        a2 = (1.0 - alpha) / a0;
    }

    // RBJ peaking bell
    void setPeak (double fs, double freq, double q, double gainDb)
    {
        if (std::abs (freq - lastFreq) < 1.0e-9 && std::abs (fs - lastFs) < 1.0e-9 && lastType == 4
            && std::abs (gainDb - lastGain) < 1.0e-9 && std::abs (q - lastQ) < 1.0e-9)
            return;
        lastFreq = freq;
        lastFs = fs;
        lastType = 4;
        lastGain = gainDb;
        lastQ = q;
        const double A = std::pow (10.0, gainDb / 40.0);
        const double w0 = 2.0 * kPi * std::min (freq, fs * 0.45) / fs;
        const double cosw = std::cos (w0), alpha = std::sin (w0) / (2.0 * std::max (0.05, q));
        const double a0 = 1.0 + alpha / A;
        b0 = (1.0 + alpha * A) / a0;
        b1 = -2.0 * cosw / a0;
        b2 = (1.0 - alpha * A) / a0;
        a1 = b1;
        a2 = (1.0 - alpha / A) / a0;
    }

    double process (double x, int channel) noexcept
    {
        const auto ch = (size_t) channel;
        const double y = b0 * x + z1[ch];
        z1[ch] = b1 * x - a1 * y + z2[ch];
        z2[ch] = b2 * x - a2 * y;
        return y;
    }

    void resetState() noexcept
    {
        z1.fill (0.0);
        z2.fill (0.0);
    }

    void invalidate() noexcept { lastType = 0; }

    void setRaw (double nb0, double nb1, double nb2, double na1, double na2) noexcept
    {
        b0 = nb0; b1 = nb1; b2 = nb2; a1 = na1; a2 = na2;
        lastType = 9;
    }

private:
    double b0 = 1.0, b1 = 0.0, b2 = 0.0, a1 = 0.0, a2 = 0.0;
    std::array<double, kMaxCh> z1 {}, z2 {};
    double lastFreq = -1.0, lastFs = -1.0, lastGain = 0.0, lastQ = 0.0;
    int lastType = 0;
};

//==============================================================================
// Running maximum over the last `window` samples (monotonic deque, amortised O(1) per sample).
// For any steady periodic |x| whose period fits in the window the output is exactly constant: a
// ripple-free peak detector with an instant attack (the Clean FET detector, DECISIONS D19).
// prepare() allocates; setWindow() and process() never do.
class SlidingMax
{
public:
    void prepare (int maxWindow)
    {
        cap = 1;
        while (cap < maxWindow + 2)
            cap <<= 1;
        for (auto& b : buf)
            b.assign ((size_t) cap, Entry {});
        setWindow (std::min (maxWindow, 1));
        resetState();
    }
    void setWindow (int w) noexcept { window = std::clamp (w, 1, cap - 2); }
    void resetState() noexcept
    {
        head.fill (0);
        tail.fill (0);
        n.fill (0);
    }
    float process (float a, int c) noexcept
    {
        auto& b = buf[(size_t) c];
        int& h = head[(size_t) c];
        int& t = tail[(size_t) c];
        const int64_t now = n[(size_t) c]++;
        const int mask = cap - 1;
        while (t != h && b[(size_t) ((t - 1) & mask)].v <= a)
            t = (t - 1) & mask;
        b[(size_t) t] = { a, now };
        t = (t + 1) & mask;
        while (b[(size_t) h].at <= now - window)
            h = (h + 1) & mask;
        return b[(size_t) h].v;
    }

private:
    struct Entry { float v = 0.0f; int64_t at = 0; };
    std::array<std::vector<Entry>, kMaxCh> buf;
    std::array<int, kMaxCh> head {}, tail {};
    std::array<int64_t, kMaxCh> n {};
    int cap = 2, window = 1;
};

//==============================================================================
// Side-chain EQ shared by both stages' detectors (2.0): the stage's own high-pass (1.x) plus an optional
// low-pass and bell. Off by default (LPF at 20 kHz, bell at 0 dB): then the detector is exactly 1.x's.
struct SideChainEq
{
    float lpfHz = 20000.0f, bellHz = 3000.0f, bellDb = 0.0f, bellQ = 1.0f;
    bool lpfOn() const noexcept { return lpfHz < 19999.0f; }
    bool bellOn() const noexcept { return std::abs (bellDb) > 0.01f; }
};

class SideChainFilter
{
public:
    void configure (double fs, const SideChainEq& e) noexcept
    {
        const bool l = e.lpfOn(), b = e.bellOn();
        if (l && ! useLpf)
            lpf.resetState();
        if (b && ! useBell)
            bell.resetState();
        useLpf = l;
        useBell = b;
        if (l)
            lpf.setLowPass (fs, std::clamp ((double) e.lpfHz, 200.0, fs * 0.45));
        if (b)
            bell.setPeak (fs, std::clamp ((double) e.bellHz, 20.0, fs * 0.45), std::clamp ((double) e.bellQ, 0.1, 10.0),
                          std::clamp ((double) e.bellDb, -24.0, 24.0));
    }
    double process (double x, int c) noexcept
    {
        if (useLpf)
            x = lpf.process (x, c);
        if (useBell)
            x = bell.process (x, c);
        return x;
    }
    void resetState() noexcept
    {
        lpf.resetState();
        bell.resetState();
    }
    void invalidate() noexcept
    {
        lpf.invalidate();
        bell.invalidate();
    }

private:
    Biquad lpf, bell;
    bool useLpf = false, useBell = false;
};

//==============================================================================
// 90-degree phase-difference allpass pair ("IIR Hilbert transformer").
// |a + j b| is the instantaneous amplitude of the analytic signal: for a sine it is constant
// and independent of where the samples fall on the waveform. A plain sample-peak detector
// reads a different peak every half cycle, which creates non-harmonic GR modulation at high
// frequencies (that is what the 15 kHz aliasing test catches).
// Coefficients: elliptic halfband polyphase design (closed form, exact elliptic nome),
// frequency-shifted by fs/4. Sections (c - z^-2) / (1 - c z^-2); even coefficients in chain A,
// odd in chain B, chain B delayed by one sample.
class HilbertPair
{
public:
    static constexpr int kMaxCoefs = 20;

    // accurate band [fLow, fs/2 - fLow]. Error with 20 coefficients and fLow = 15 Hz:
    // 0.0012 deg at 96 kHz, 0.0136 deg at 768 kHz (192 kHz x 4).
    static void design (double fs, double fLow, int numCoefs, double* coefs)
    {
        const double t = std::clamp (2.0 * fLow / fs, 1.0e-7, 0.45);
        double k = std::tan ((1.0 - 2.0 * t) * kPi / 4.0);
        k *= k;
        auto ellipK = [] (double m)
        {
            double a = 1.0, b = std::sqrt (std::max (1.0e-300, 1.0 - m));
            for (int i = 0; i < 64 && std::abs (a - b) > 1.0e-15 * a; ++i)
            {
                const double an = 0.5 * (a + b);
                b = std::sqrt (a * b);
                a = an;
            }
            return kPi / (2.0 * a);
        };
        const double q = std::exp (-kPi * ellipK (1.0 - k * k) / ellipK (k * k));
        const int order = 2 * numCoefs + 1;
        for (int idx = 0; idx < numCoefs; ++idx)
        {
            const int c = idx + 1;
            double num = 0.0, den = 0.0;
            for (int i = 0, j = 1; i < 200; ++i, j = -j)
            {
                const double term = std::pow (q, (double) (i * (i + 1))) * std::sin ((i * 2 + 1) * c * kPi / order) * j;
                num += term;
                if (std::abs (term) < 1.0e-30)
                    break;
            }
            for (int i = 1, j = -1; i < 200; ++i, j = -j)
            {
                const double term = std::pow (q, (double) (i * i)) * std::cos (i * 2 * c * kPi / order) * j;
                den += term;
                if (std::abs (term) < 1.0e-30)
                    break;
            }
            const double ww = num * std::pow (q, 0.25) / (den + 0.5);
            const double w2 = ww * ww;
            const double x = std::sqrt ((1.0 - w2 * k) * (1.0 - w2 / k)) / (1.0 + w2);
            coefs[idx] = (1.0 - x) / (1.0 + x);
        }
    }

    void setCoefficients (const double* c, int n) noexcept
    {
        numCoefs = std::clamp (n, 2, kMaxCoefs);
        nA = (numCoefs + 1) / 2;
        nB = numCoefs / 2;
        for (int i = 0; i < numCoefs; ++i)
            (i % 2 == 0 ? cA[i / 2] : cB[i / 2]) = c[i];
        resetState();
    }

    void resetState() noexcept
    {
        for (auto& s : st)
            s = Chan();
    }

    // returns |analytic signal|^2
    double power (double x, int ch) noexcept
    {
        Chan& s = st[(size_t) ch];
        double a = x, b = x;
        for (int i = 0; i < nA; ++i)
        {
            const double y = cA[i] * (a + s.ay2[i]) - s.ax2[i];
            s.ax2[i] = s.ax1[i];
            s.ax1[i] = a;
            s.ay2[i] = s.ay1[i];
            s.ay1[i] = y;
            a = y;
        }
        for (int i = 0; i < nB; ++i)
        {
            const double y = cB[i] * (b + s.by2[i]) - s.bx2[i];
            s.bx2[i] = s.bx1[i];
            s.bx1[i] = b;
            s.by2[i] = s.by1[i];
            s.by1[i] = y;
            b = y;
        }
        const double bd = s.bDelay;
        s.bDelay = b;
        return a * a + bd * bd;
    }

private:
    struct Chan
    {
        double ax1[kMaxCoefs / 2] {}, ax2[kMaxCoefs / 2] {}, ay1[kMaxCoefs / 2] {}, ay2[kMaxCoefs / 2] {};
        double bx1[kMaxCoefs / 2] {}, bx2[kMaxCoefs / 2] {}, by1[kMaxCoefs / 2] {}, by2[kMaxCoefs / 2] {};
        double bDelay = 0.0;
    };
    double cA[kMaxCoefs / 2] {}, cB[kMaxCoefs / 2] {};
    int numCoefs = 2, nA = 1, nB = 1;
    std::array<Chan, kMaxCh> st {};
};

// 5 Hz one-pole DC blocker applied only to generated even-order distortion, so the
// main signal path itself stays perfectly linear-phase (dry/wet alignment is untouched).
class DcBlocker
{
public:
    void prepare (double fs) noexcept
    {
        r = 1.0 - 2.0 * kPi * 5.0 / fs;
        resetState();
    }
    void resetState() noexcept
    {
        x1.fill (0.0);
        y1.fill (0.0);
    }
    double process (double x, int channel) noexcept
    {
        const auto ch = (size_t) channel;
        const double y = x - x1[ch] + r * y1[ch];
        x1[ch] = x;
        y1[ch] = y;
        return y;
    }

private:
    double r = 0.999;
    std::array<double, kMaxCh> x1 {}, y1 {};
};

//==============================================================================
// ITU-R BS.1770 K-weighting (pre-filter high shelf + RLB high-pass), coefficients derived for any rate by the
// standard's bilinear forms. Used for the level-matched compare (loudness, not peaks).
class KWeighting
{
public:
    void prepare (double fs) noexcept
    {
        {
            const double f0 = 1681.974450955533, G = 3.999843853973347, Q = 0.7071752369554196;
            const double K = std::tan (kPi * f0 / fs), Vh = std::pow (10.0, G / 20.0), Vb = std::pow (Vh, 0.4996667741545416);
            const double a0 = 1.0 + K / Q + K * K;
            shelf.setRaw ((Vh + Vb * K / Q + K * K) / a0, 2.0 * (K * K - Vh) / a0, (Vh - Vb * K / Q + K * K) / a0,
                          2.0 * (K * K - 1.0) / a0, (1.0 - K / Q + K * K) / a0);
        }
        {
            const double f0 = 38.13547087602444, Q = 0.5003270373238773;
            const double K = std::tan (kPi * f0 / fs), a0 = 1.0 + K / Q + K * K;
            hp.setRaw (1.0, -2.0, 1.0, 2.0 * (K * K - 1.0) / a0, (1.0 - K / Q + K * K) / a0);
        }
        resetState();
    }
    void resetState() noexcept
    {
        shelf.resetState();
        hp.resetState();
    }
    double process (double x, int c) noexcept { return hp.process (shelf.process (x, c), c); }

private:
    Biquad shelf, hp;
};

//==============================================================================
// FET stage: INPUT drives into the threshold (like the hardware; -12 dBFS by default, adjustable since 2.0),
// attack 20-800 us, release 50-1100 ms, 4/8/12/20/All ratios.
//
// 2.0: threshold, knee (Auto = the 1.x ratio-dependent knee), range (largest GR), hold, side-chain EQ, a
// Clean detector, and detection separated from the audio (detect() on a key, apply() on the audio): the
// engine uses that for lookahead and external side-chain keying. frame() = detect() + apply() on the same
// signal - with every 2.0 control at its default it computes exactly what 1.x computed.
class FetStage
{
public:
    struct Settings
    {
        float inDb = 6.0f, outDb = 0.0f, attackMs = 0.2f, releaseMs = 150.0f;
        int ratio = 0;            // 0..4 = 4, 8, 12, 20, All
        float scHz = 80.0f, color = 0.3f, link = 1.0f;
        // 2.0
        float thresholdDb = -12.0f, kneeDb = -1.0f /* < 0: Auto */, rangeDb = 60.0f /* >= 60: off */, holdMs = 0.0f;
        bool clean = false;       // Clean detector: ripple-free full-band envelope
        SideChainEq eq;
        // the ALL ("all buttons") law. Defaults = the 1.x law (educated guesses, DECISIONS D9);
        // measuredAll() = calibrated on hardware 1176LN recordings (DECISIONS D27)
        struct AllModel
        {
            float ratio = 20.0f, knee = 2.0f, thrOffsetDb = 0.0f, attackMul = 0.6f, releaseMul = 0.7f;
            float bumpOffsetDb = 10.0f, bumpSlope = 0.15f, bumpMaxDb = 6.0f;
        } all;
    };
    static Settings::AllModel classicAll() noexcept { return {}; }
    static Settings::AllModel measuredAll() noexcept
    {
        Settings::AllModel m;
        // not calibrated yet (DECISIONS D27): the Classic law
        return m;
    }

    static constexpr float kThresholdDb = -12.0f; // 1.x fixed threshold = the default

    void prepare (double baseRate)
    {
        baseFs = baseRate;
        // the HF analytic path runs at the base rate (it is an envelope; interpolated per sample)
        double coefs[HilbertPair::kMaxCoefs] {};
        HilbertPair::design (baseFs, kHilbertLowHz, kHilbertCoefs, coefs);
        hil.setCoefficients (coefs, kHilbertCoefs);
        cleanMax.prepare ((int) std::ceil (kCleanWindowSec * baseFs * (1 << (kNumOsFactors - 1))) + 1);
        hf1.setHighPass (baseFs, 1000.0, 0.5411961); // 4th-order Butterworth high-pass at 1 kHz
        hf2.setHighPass (baseFs, 1000.0, 1.3065630);
        setOversampling (1);
        resetState();
    }

    // RT-safe: keeps the envelope state (GR, memory); no allocation
    void setOversampling (int log2Factor) noexcept
    {
        osIndex = std::clamp (log2Factor, 0, kNumOsFactors - 1);
        fs = baseFs * (1 << osIndex);
        decim = 1 << osIndex;
        ctlCount = 0;
        sc.invalidate();
        sc.resetState();
        scEq.invalidate();
        scEq.resetState();
        holdLen = std::max (1, (int) std::lround (0.002 * fs));
        cleanMax.setWindow ((int) std::lround (kCleanWindowSec * fs));
        cleanMax.resetState();
        dc.prepare (fs);
        smoothK = 1.0f - onePole (0.02f, fs);
        memRiseK = 1.0f - onePole (2.0f, fs);
        memFallK = 1.0f - onePole (4.0f, fs);
        setParams (settings);
    }

    void setParams (const Settings& s) noexcept
    {
        settings = s;
        static constexpr float ratios[] = { 4.0f, 8.0f, 12.0f, 20.0f, 20.0f };
        // knee gets harder as the ratio rises (spec 2.1) - the Auto knee
        static constexpr float knees[]  = { 6.0f, 4.0f, 3.0f, 2.0f, 2.0f };

        const int idx = std::clamp (s.ratio, 0, 4);
        allButtons = (idx == 4);
        allm = s.all;
        ratio = allButtons ? std::max (1.0f, allm.ratio) : ratios[idx];
        knee  = s.kneeDb < 0.0f ? (allButtons ? std::clamp (allm.knee, 0.0f, 24.0f) : knees[idx]) : std::clamp (s.kneeDb, 0.0f, 24.0f);
        threshold = std::clamp (s.thresholdDb + (allButtons ? allm.thrOffsetDb : 0.0f), -60.0f, 0.0f);
        range = s.rangeDb >= 60.0f ? -1.0f : std::clamp (s.rangeDb, 0.0f, 60.0f);
        // the curve is exactly 0 below threshold - knee/2; 1 dB of margin (1.x: -16 dBFS)
        quietLevel = dbToGain (threshold - 0.5f * std::max (knee, 6.0f) - 1.0f);
        holdSamples = (int) std::lround (std::clamp (s.holdMs, 0.0f, 500.0f) * 0.001 * fs);
        clean = s.clean;
        inTarget  = dbToGain (s.inDb);
        outTarget = dbToGain (s.outDb);
        // All-buttons: attack 0.6x and release 0.7x of the knob (faster, pumping; documented in DECISIONS.md)
        const float atkSec = std::clamp (s.attackMs, 0.02f, 0.8f) * 0.001f * (allButtons ? std::max (0.05f, allm.attackMul) : 1.0f);
        const float relSec = std::clamp (s.releaseMs, 50.0f, 1100.0f) * 0.001f * (allButtons ? std::max (0.05f, allm.releaseMul) : 1.0f);
        atkK = 1.0f - onePole (atkSec, fs);
        relRate = (float) (1.0 / (relSec * fs));
        sc.setHighPass (fs, std::clamp (s.scHz, 20.0f, 300.0f));
        scEq.configure (fs, s.eq);
        colorTarget = std::clamp (s.color, 0.0f, 1.0f);
        link = std::clamp (s.link, 0.0f, 1.0f);
    }

    void snap() noexcept
    {
        inGain = inTarget;
        outGain = outTarget;
        color = colorTarget;
    }

    void resetState() noexcept
    {
        hil.resetState();
        cleanMax.resetState();
        sc.resetState();
        scEq.resetState();
        hf1.resetState();
        hf2.resetState();
        dc.resetState();
        gr.fill (0.0f);
        memory.fill (0.0f);
        held.fill (0.0f);
        holdCount.fill (0);
        grHold.fill (0);
        hCur.fill (0.0f);
        hStep.fill (0.0f);
        ctlCount = 0;
    }

    // processes one multichannel frame in place, returns the largest channel GR in dB
    float processFrame (float* x, int numCh) noexcept
    {
        return numCh >= 2 ? frame<2> (x) : frame<1> (x);
    }

    // split use (lookahead / external key): detect on `key` (pre-INPUT), then apply to the audio frame
    float detectFrame (const float* key, int numCh) noexcept
    {
        return numCh >= 2 ? detect<2> (key) : detect<1> (key);
    }
    // the stage's current linear gain (input, gain reduction, output) - what it does to a signal
    float stageGain (int c) const noexcept { return inGain * gLin[(size_t) c] * outGain; }
    void applyFrame (float* x, int numCh) noexcept
    {
        if (numCh >= 2)
            apply<2> (x);
        else
            apply<1> (x);
    }

    float grDb (int ch) const noexcept { return gr[(size_t) ch]; }
    double sampleRate() const noexcept { return fs; }
    // static gain reduction for a steady signal whose detector level (after INPUT) is levelDb (displays)
    float staticGainReductionDb (float levelDb) const noexcept
    {
        float g = curve (levelDb);
        return range >= 0.0f ? std::min (g, range) : g;
    }

private:
    template <int numCh>
    float frame (float* x) noexcept
    {
        const float g = detect<numCh> (x);
        apply<numCh> (x);
        return g;
    }

    // channel count as a compile-time constant: loops unroll and the two channels'
    // dependency chains interleave (measured ~10 % less CPU for the stereo stage)
    template <int numCh>
    float detect (const float* key) noexcept
    {
        inGain  += (inTarget - inGain) * smoothK;
        outGain += (outTarget - outGain) * smoothK;
        color   += (colorTarget - color) * smoothK;
        if (std::abs (color - colorTarget) < 1.0e-6f)
            color = colorTarget;

        // Vintage detector = max(2 ms peak-hold of |x|, |analytic signal of the >1 kHz band|).
        // - |x| makes transients instant (20 us attack is real).
        // - the 2 ms hold bridges the gaps between waveform peaks, so the level is steady from
        //   the first peak on: attack/release behave like the knob says on tones >= 250 Hz.
        // - with only a few samples per cycle the sampled peak depends on where the samples
        //   fall (up to -1.3 dB at 15 kHz / 88.2 kHz) and the GR would "beat" -> non-harmonic
        //   sidebands. The analytic magnitude is sample-phase independent; it is only needed
        //   (and only used) above ~1 kHz, where the Hilbert pair can be short and fast.
        // Below ~250 Hz the 2 ms hold is shorter than half a cycle: the level ripples and fast settings
        // modulate the gain (hardware-like low-frequency distortion). The Clean detector (2.0) replaces the
        // hold by a true 25 ms running maximum: still instant on transients, but exactly constant for any
        // steady tone down to 20 Hz, so the gain does not ripple (the release starts up to 25 ms later).
        float env[kMaxCh] {};
        double sv[kMaxCh] {};
        float envMax = 0.0f;
        for (int c = 0; c < numCh; ++c)
        {
            double v = sc.process (key[c] * inGain, c);
            v = scEq.process (v, c);
            sv[c] = v;
        }
        if (--ctlCount <= 0)
        {
            ctlCount = decim;
            for (int c = 0; c < numCh; ++c)
            {
                const float h = (float) std::sqrt (hil.power (hf2.process (hf1.process (sv[c], c), c), c));
                hStep[(size_t) c] = (h - hCur[(size_t) c]) / (float) decim;
            }
        }
        for (int c = 0; c < numCh; ++c)
        {
            hCur[(size_t) c] += hStep[(size_t) c];
            if (clean)
                env[c] = std::max (cleanMax.process ((float) std::abs (sv[c]), c), hCur[(size_t) c]);
            else
            {
                const double s = sv[c];
                const float a = (float) std::abs (s);
                float& hv = held[(size_t) c];
                int& hc = holdCount[(size_t) c];
                if (a >= hv || hc <= 0)
                {
                    hv = a;
                    hc = holdLen;
                }
                else
                {
                    --hc;
                }
                env[c] = std::max (hv, hCur[(size_t) c]);
            }
            envMax = std::max (envMax, env[c]);
        }

        float grMax = 0.0f;
        for (int c = 0; c < numCh; ++c)
        {
            const float e = envMax + (env[c] - envMax) * (1.0f - link);
            // far below the knee nothing happens: skip the log/curve (quiet passages, pauses)
            float target = e < quietLevel ? 0.0f : curve (gainToDb (e));
            if (range >= 0.0f)
                target = std::min (target, range);

            // program-dependent release: memory builds up over ~2 s of heavy (>2 dB, full at
            // 10 dB) GR and slows the release by up to +40 % (spec 2.1); decays over ~4 s.
            float& g = gr[(size_t) c];
            float& m = memory[(size_t) c];
            // hold (2.0): while the demand stays at the current GR (within 0.05 dB) the hold restarts; the
            // release begins `hold` after the demand fell. Hold 0 = 1.x.
            bool holding = false;
            if (holdSamples > 0)
            {
                int& hc = grHold[(size_t) c];
                if (target >= g - kHoldTolDb)
                {
                    hc = holdSamples;
                    holding = true;
                }
                else if (hc > 0)
                {
                    --hc;
                    holding = true;
                }
            }
            if (target > g)
                g += (target - g) * atkK;
            else if (! holding)
                g += (target - g) * oneMinusExpSmall (relRate / (1.0f + 0.4f * m));
            // memory follows the compression demand (target), so it stops charging the moment
            // the signal drops - otherwise a long release would keep slowing itself down
            const float w = std::clamp ((target - 2.0f) * 0.125f, 0.0f, 1.0f);
            m += (w - m) * (w > m ? memRiseK : memFallK);
            gLin[(size_t) c] = g > 0.0f ? dbToGain (-g) : 1.0f;
            grMax = std::max (grMax, g);
        }
        return grMax;
    }

    template <int numCh>
    void apply (float* x) noexcept
    {
        for (int c = 0; c < numCh; ++c)
        {
            const float g = gr[(size_t) c];
            const float xi = x[c] * inGain;
            float y = g > 0.0f ? xi * gLin[(size_t) c] : xi;

            if (color > 0.0f)
                y += distortion (y, g, c);
            else
                (void) dc.process (0.0, c); // keep the blocker state decaying

            x[c] = y * outGain;
        }
    }

    float curve (float levelDb) const noexcept
    {
        float g = gainReductionDb (levelDb, threshold, ratio, knee);
        if (allButtons)
        {
            // over-compression: above threshold + 10 dB the output level falls by 0.1 dB per dB
            // (soft 6 dB knee, capped at 6 dB extra) - the "nuke" bump of all-buttons-in.
            g += std::min (allm.bumpMaxDb, gainReductionDb (levelDb, threshold + allm.bumpOffsetDb,
                                                           1.0f / (1.0f - std::clamp (allm.bumpSlope, 0.0f, 0.9f)), 6.0f));
        }
        return g;
    }

    // FET gain-element distortion: 2nd + 3rd harmonic only (a cubic polynomial cannot create
    // the higher harmonics that would fold back as aliasing). Amount rises with GR like the FET's
    // channel non-linearity, scaled by Color; All-buttons 2.5x. The polynomial input is bounded
    // (s / sqrt(1 + s^2/16)) and the amount capped so the transfer stays monotonic.
    float distortion (float y, float grDb, int c) noexcept
    {
        constexpr float T = 0.251188643f;          // reference amplitude (-12 dBFS)
        constexpr float k2 = 0.0129f, k3 = 0.0208f; // calibrated: ~0.45 % H2 + ~0.5 % H3 at 10 dB GR, Color 50
        const float amount = std::min (4.0f, color * (0.08f + std::min (grDb, 20.0f) * 0.1f) * (allButtons ? 2.5f : 1.0f));
        const float s = y * (1.0f / T);
        const float sb = s / std::sqrt (1.0f + s * s * (1.0f / 16.0f));
        const float even = (float) dc.process ((double) (amount * k2 * sb * sb), c);
        const float odd = -amount * k3 * sb * sb * sb;
        return T * (even + odd);
    }

    // 500 Hz lower edge, 12 coefficients: <= 0.0001 deg at 44.1 kHz, <= 0.003 deg at 192 kHz
    static constexpr int kHilbertCoefs = 12;
    static constexpr double kHilbertLowHz = 500.0;
    static constexpr float kHoldTolDb = 0.05f;
    // Clean detector: running maximum over half a period of 20 Hz (DECISIONS D19)
    static constexpr double kCleanWindowSec = 0.025;
    Settings settings;
    double baseFs = 44100.0, fs = 88200.0;
    int osIndex = 1, decim = 2, ctlCount = 0;
    std::array<float, kMaxCh> hCur {}, hStep {}, gLin { 1.0f, 1.0f };
    Biquad sc, hf1, hf2;
    SideChainFilter scEq;
    HilbertPair hil;
    SlidingMax cleanMax;
    DcBlocker dc;
    int holdLen = 176, holdSamples = 0;
    std::array<float, kMaxCh> held {};
    std::array<int, kMaxCh> holdCount {}, grHold {};
    float ratio = 4.0f, knee = 6.0f, threshold = kThresholdDb, range = -1.0f, quietLevel = 0.158489319f;
    float atkK = 1.0f, relRate = 0.0f, link = 1.0f;
    float memRiseK = 0.0f, memFallK = 0.0f;
    float inGain = 1.0f, inTarget = 1.0f, outGain = 1.0f, outTarget = 1.0f;
    float color = 0.0f, colorTarget = 0.0f, smoothK = 0.001f;
    std::array<float, kMaxCh> gr {}, memory {};
    bool allButtons = false, clean = false;
    Settings::AllModel allm;
};

//==============================================================================
// Opto stage (LA-2A-style): power detector (analytic signal, ~2 ms), ~10 ms attack,
// two-stage release: 60 % of the GR follows a fast 40 ms release, 40 % a slow "light cell"
// release of 0.5-5 s whose length grows with how long and how hard it has been compressing.
//
// The detector and ballistics run at the BASE rate (every 1/2/4 oversampled frames) and the
// gain reduction is interpolated linearly in dB per oversampled sample. Nothing in an opto
// moves faster than ~8 ms, so this halves the stage's CPU at 2x and makes the opto behave
// identically at 1x, 2x and 4x. The make-up/tube stage runs at the oversampled rate.
class OptoStage
{
public:
    struct Settings
    {
        float peakReduction = 30.0f, gainDb = 0.0f;
        bool limit = false;
        float scHz = 80.0f, color = 0.3f, link = 1.0f, emphasis = 0.0f;
        // 2.0 (defaults = 1.x)
        float thresholdDb = -10.0f, kneeDb = -1.0f /* < 0: Auto */, rangeDb = 60.0f /* >= 60: off */, holdMs = 0.0f;
        SideChainEq eq;
        // the opto law: knob scale, static curve per mode and the light-cell timing. The defaults are the 1.x
        // ("Classic") model; hardwareModel() is the one calibrated on LA-2A recordings (DECISIONS D24).
        struct Model
        {
            float prDbPerUnit = 0.4f, prOffsetDb = 0.0f;   // Peak Reduction -> dB into the cell
            float ratioComp = 3.0f, ratioLim = 10.0f, kneeComp = 12.0f, kneeLim = 6.0f;
            float attackSec = 0.0085f, fastReleaseSec = 0.040f, fastShare = 0.6f;
            float slowMinSec = 0.5f, slowMaxSec = 5.0f, memRiseSec = 4.0f, memFallSec = 8.0f;
            // onset flare (2.1): extra GR = onsetGain * max(0, fast(target) - slow(target)) - the cell over-reacts to
            // a rising drive, then settles (measured on hardware onsets, DECISIONS D28). 0 = none (1.x, 2.0)
            float onsetGain = 0.0f, onsetFastSec = 0.008f, onsetSlowSec = 0.040f;
        } model;
    };

    static Settings::Model classicModel() noexcept { return {}; }
    // calibrated on 45 min of hardware LA-2A recordings (SignalTrain test split), tools/validation/la2a_joint_fit.py
    static Settings::Model measuredModel() noexcept
    {
        Settings::Model m;
        m.prDbPerUnit = 0.4433f;     // Peak Reduction 0..100 -> -20.2 .. +24.1 dB into the cell
        m.prOffsetDb = -20.19f;
        m.ratioComp = 3.66f;         // the hardware's two modes differ in knee, hardly in ratio
        m.ratioLim = 3.60f;
        m.kneeComp = 3.04f;
        m.kneeLim = 0.0f;
        m.attackSec = 0.00133f;
        m.fastReleaseSec = 0.0385f;
        m.fastShare = 0.557f;
        m.slowMinSec = 0.998f;
        m.slowMaxSec = 1.557f;
        return m;
    }

    static constexpr float kThresholdDb = -10.0f; // 1.x fixed threshold = the default

    void prepare (double baseRate)
    {
        baseFs = baseRate;
        double coefs[HilbertPair::kMaxCoefs] {};
        HilbertPair::design (baseFs, 15.0, kHilbertCoefs, coefs);
        hil.setCoefficients (coefs, kHilbertCoefs);
        detK     = 1.0f - onePole (kDetectorSec, baseFs);
        invFs    = (float) (1.0 / baseFs);
        modelApplied.prDbPerUnit = -1.0f; // coefficients depend on the rate: recompute
        setOversampling (1);
        resetState();
    }

    void setOversampling (int log2Factor) noexcept
    {
        osIndex = std::clamp (log2Factor, 0, kNumOsFactors - 1);
        fs = baseFs * (1 << osIndex);
        decim = 1 << osIndex;
        ctlCount = 0;
        dc.prepare (fs);
        smoothK = 1.0f - onePole (0.02f, fs);
        setParams (settings);
    }

    void setParams (const Settings& s) noexcept
    {
        settings = s;
        const auto& md = s.model;
        prGainDb     = std::clamp (s.peakReduction, 0.0f, 100.0f) * md.prDbPerUnit + md.prOffsetDb; // dB into the cell
        ratio        = std::max (1.0f, s.limit ? md.ratioLim : md.ratioComp);
        knee         = s.kneeDb < 0.0f ? (s.limit ? md.kneeLim : md.kneeComp) : std::clamp (s.kneeDb, 0.0f, 24.0f);
        if (std::memcmp (&md, &modelApplied, sizeof md) != 0)
        {
            modelApplied = md;
            atkK       = 1.0f - onePole (std::max (0.0005f, md.attackSec), baseFs);
            relFastK   = 1.0f - onePole (std::max (0.002f, md.fastReleaseSec), baseFs);
            memRiseK   = 1.0f - onePole (std::max (0.05f, md.memRiseSec), baseFs);
            memFallK   = 1.0f - onePole (std::max (0.05f, md.memFallSec), baseFs);
            fastShare  = std::clamp (md.fastShare, 0.0f, 1.0f);
            slowMin    = std::max (0.01f, md.slowMinSec);
            slowSpan   = std::max (0.0f, md.slowMaxSec - slowMin);
            onsetGain  = std::max (0.0f, md.onsetGain);
            onsetFastK = 1.0f - onePole (std::max (0.0005f, md.onsetFastSec), baseFs);
            onsetSlowK = 1.0f - onePole (std::max (0.001f, md.onsetSlowSec), baseFs);
        }
        threshold    = std::clamp (s.thresholdDb, -60.0f, 0.0f);
        range        = s.rangeDb >= 60.0f ? -1.0f : std::clamp (s.rangeDb, 0.0f, 60.0f);
        holdSteps    = (int) std::lround (std::clamp (s.holdMs, 0.0f, 500.0f) * 0.001 * baseFs);
        scEq.configure (baseFs, s.eq);
        makeupTarget = dbToGain (s.gainDb);
        sc.setHighPass (baseFs, std::clamp (s.scHz, 20.0f, 300.0f));
        // R37-style HF emphasis: up to +12 dB high shelf at 3 kHz in the detector only
        const double emphDb = 12.0 * std::clamp (s.emphasis, 0.0f, 1.0f);
        if (emphDb > 0.0 && ! useShelf)
            shelf.resetState();
        useShelf = emphDb > 0.0;
        shelf.setHighShelf (baseFs, 3000.0, emphDb);
        colorTarget = std::clamp (s.color, 0.0f, 1.0f);
        link = std::clamp (s.link, 0.0f, 1.0f);
    }

    void snap() noexcept
    {
        makeup = makeupTarget;
        color = colorTarget;
    }

    void resetState() noexcept
    {
        hil.resetState();
        sc.resetState();
        shelf.resetState();
        scEq.resetState();
        dc.resetState();
        holdCount.fill (0);
        onsetFast.fill (0.0f);
        onsetSlow.fill (0.0f);
        ms.fill (0.0f);
        grFast.fill (0.0f);
        grSlow.fill (0.0f);
        memory.fill (0.0f);
        grCur.fill (0.0f);
        grStep.fill (0.0f);
        gainCur.fill (1.0f);
        gainStep.fill (0.0f);
        ctlCount = 0;
    }

    float processFrame (float* x, int numCh) noexcept
    {
        const float g = detectFrame (x, numCh);
        applyFrame (x, numCh);
        return g;
    }

    // split use (lookahead / external key), see FetStage
    float detectFrame (const float* key, int numCh) noexcept
    {
        makeup += (makeupTarget - makeup) * smoothK;
        color  += (colorTarget - color) * smoothK;
        if (std::abs (color - colorTarget) < 1.0e-6f)
            color = colorTarget;

        if (--ctlCount <= 0)
        {
            ctlCount = decim;
            control (key, numCh);
        }

        // GR (dB, for meters/tests) and the linear gain are both interpolated linearly
        // between base-rate control points; no transcendental per oversampled sample
        float grMax = 0.0f;
        for (int c = 0; c < numCh; ++c)
        {
            grCur[(size_t) c] += grStep[(size_t) c];
            gainCur[(size_t) c] += gainStep[(size_t) c];
            grMax = std::max (grMax, grCur[(size_t) c]);
        }
        return grMax;
    }

    void applyFrame (float* x, int numCh) noexcept
    {
        for (int c = 0; c < numCh; ++c)
        {
            float y = x[c] * gainCur[(size_t) c] * makeup;
            if (color > 0.0f)
                y += tube (y, c);
            else
                (void) dc.process (0.0, c);
            x[c] = y;
        }
    }

    float stageGain (int c) const noexcept { return gainCur[(size_t) c] * makeup; }
    double sampleRate() const noexcept { return fs; }
    // static gain reduction for a steady sine of peak level peakDb at the stage input (displays)
    float staticGainReductionDb (float peakDb) const noexcept
    {
        const float g = gainReductionDb (peakDb + prGainDb, threshold, ratio, knee);
        return range >= 0.0f ? std::min (g, range) : g;
    }

private:
    // one base-rate step of detector + ballistics; sets the per-frame GR interpolation
    void control (const float* x, int numCh) noexcept
    {
        float msMax = 0.0f;
        for (int c = 0; c < numCh; ++c)
        {
            double v = sc.process (x[c], c);
            v = scEq.process (v, c);
            if (useShelf)
                v = shelf.process (v, c);
            const float p = (float) hil.power (v, c);
            ms[(size_t) c] += (p - ms[(size_t) c]) * detK;
            msMax = std::max (msMax, ms[(size_t) c]);
        }

        for (int c = 0; c < numCh; ++c)
        {
            const float p = msMax + (ms[(size_t) c] - msMax) * (1.0f - link);
            float target = gainReductionDb (powerToDb (p) + prGainDb, threshold, ratio, knee);
            if (range >= 0.0f)
                target = std::min (target, range);

            float& f = grFast[(size_t) c];
            float& sl = grSlow[(size_t) c];
            float& m = memory[(size_t) c];
            // slow tail 0.5 s .. 5 s, set per step from the light-cell memory (no stepping)
            const float tauSlow = slowMin + slowSpan * m;
            // hold (2.0): as in the FET stage; hold 0 = 1.x
            bool holding = false;
            if (holdSteps > 0)
            {
                int& hc = holdCount[(size_t) c];
                if (target >= fastShare * f + (1.0f - fastShare) * sl - 0.05f)
                {
                    hc = holdSteps;
                    holding = true;
                }
                else if (hc > 0)
                {
                    --hc;
                    holding = true;
                }
            }
            if (target > f)
                f += (target - f) * atkK;
            else if (! holding)
                f += (target - f) * relFastK;
            if (target > sl)
                sl += (target - sl) * atkK;
            else if (! holding)
                sl += (target - sl) * oneMinusExpSmall (invFs / tauSlow);
            const float g = fastShare * f + (1.0f - fastShare) * sl;
            // memory follows the compression demand: full build-up at 10 dB
            const float w = std::clamp (target * 0.1f, 0.0f, 1.0f);
            m += (w - m) * (w > m ? memRiseK : memFallK);
            float gOut = g;
            if (onsetGain > 0.0f)
            {
                float& t1 = onsetFast[(size_t) c];
                float& t2 = onsetSlow[(size_t) c];
                t1 += (target - t1) * onsetFastK;
                t2 += (target - t2) * onsetSlowK;
                gOut += onsetGain * std::max (0.0f, t1 - t2);
                if (range >= 0.0f)
                    gOut = std::min (gOut, range);
            }
            const float inv = 1.0f / (float) decim;
            grStep[(size_t) c] = (gOut - grCur[(size_t) c]) * inv;
            gainStep[(size_t) c] = (dbToGain (-gOut) - gainCur[(size_t) c]) * inv;
        }
    }

    // Tube make-up colour: mostly 2nd harmonic (plus a little 3rd), level dependent, DC-free.
    // Referenced to -6 dBFS; Color 50 gives ~0.3 % H2 at -12 dBFS out, ~1 % near 0 dBFS.
    float tube (float y, int c) noexcept
    {
        constexpr float R = 0.5f;
        const float s = y * (1.0f / R);
        const float sb = s / std::sqrt (1.0f + s * s * (1.0f / 9.0f));
        const float a = color * 0.048f;
        const float even = (float) dc.process ((double) (a * sb * sb), c);
        const float odd = -a * 0.15f * sb * sb * sb;
        return R * (even + odd);
    }

    static constexpr float kDetectorSec = 0.002f;
    // power is averaged (2 ms + 8.5 ms attack): 8 coefficients (<= ~1 deg) is ample here
    static constexpr int kHilbertCoefs = 8;

    Settings settings;
    double baseFs = 44100.0, fs = 88200.0;
    int osIndex = 1, decim = 2, ctlCount = 0;
    bool useShelf = false;
    Biquad sc, shelf;
    SideChainFilter scEq;
    HilbertPair hil;
    DcBlocker dc;
    float prGainDb = 12.0f, ratio = 3.0f, knee = 12.0f, link = 1.0f, threshold = kThresholdDb, range = -1.0f;
    int holdSteps = 0;
    std::array<int, kMaxCh> holdCount {};
    float makeup = 1.0f, makeupTarget = 1.0f, color = 0.0f, colorTarget = 0.0f, smoothK = 0.001f;
    float detK = 0.0f, atkK = 0.0f, relFastK = 0.0f, memRiseK = 0.0f, memFallK = 0.0f, invFs = 0.0f;
    float fastShare = 0.6f, slowMin = 0.5f, slowSpan = 4.5f, onsetGain = 0.0f, onsetFastK = 0.0f, onsetSlowK = 0.0f;
    std::array<float, kMaxCh> onsetFast {}, onsetSlow {};
    Settings::Model modelApplied { -1.0f };
    std::array<float, kMaxCh> ms {}, grFast {}, grSlow {}, memory {}, grCur {}, grStep {};
    std::array<float, kMaxCh> gainCur { 1.0f, 1.0f }, gainStep {};
};
//==============================================================================
// True-peak brickwall limiter (2.0), base rate, stereo-linked. DECISIONS D21.
//
// 1. True-peak detector: 8x polyphase interpolation (Kaiser-windowed sinc, beta 8, 8 phases x 128 taps)
//    plus parabolic refinement of every local maximum. Interpolation error <= 0.003 dB for content up to
//    0.48 fs (21.2 kHz at 44.1 kHz, 23.1 kHz at 48 kHz) against an exact band-limited reference; closer
//    to Nyquist no low-latency interpolator is exact (DECISIONS D21). peak[b] = the largest true peak in
//    (b - 1, b + 1); the ceiling carries a fixed design margin (kMarginDb).
// 2. Required gain r[b] = min(1, ceiling / peak[b]) -> running minimum over the lookahead L -> release
//    (rises with the release time constant, falls instantly) -> kSmooth cascaded moving averages whose
//    combined support is L. Every value that enters the smoothing for sample b is <= r[b]: the gain never
//    exceeds what sample b and its neighbourhood need. One average would ramp linearly, and the corners of
//    that ramp modulate high frequencies into new inter-sample peaks (measured up to +0.02 dB); the
//    cascade's smooth S-curve does not (DECISIONS D21).
// Latency: kTaps / 2 + L samples. prepare() allocates; everything else is RT-safe.
class TruePeakLimiter
{
public:
    static constexpr int kPhases = 8, kTaps = 128;
    static constexpr int kSmooth = 3;
    static constexpr int latencyFor (int lookaheadSamples) noexcept { return kTaps / 2 + std::max (kSmooth, lookaheadSamples); }

    void prepare (double sampleRate, int maxLookahead)
    {
        fs = sampleRate;
        maxLook = std::max (1, maxLookahead);
        designInterpolator();
        int cap = 1;
        while (cap < maxLook + kTaps + 4)
            cap <<= 1;
        ringMask = cap - 1;
        for (auto& d : delay)
            d.assign ((size_t) cap, 0.0f);
        for (auto& r : avgRing)
            r.assign ((size_t) cap, 1.0);
        attMax.prepare (maxLook);
        setLookahead (std::min (maxLook, (int) std::lround (0.0015 * fs)));
        setParams (-1.0f, 50.0f);
        resetState();
    }

    // structural (changes the latency): call while the output is muted, then resetState()
    void setLookahead (int samples) noexcept
    {
        look = std::clamp (samples, kSmooth, maxLook);
        attMax.setWindow (look);
        // lengths with sum(len) - (kSmooth - 1) == look
        int rest = look + kSmooth - 1;
        for (int k = 0; k < kSmooth; ++k)
        {
            avgLen[(size_t) k] = rest / (kSmooth - k);
            rest -= avgLen[(size_t) k];
        }
    }
    int lookahead() const noexcept { return look; }
    int latency() const noexcept { return latencyFor (look); }

    void setParams (float ceilingDb, float releaseMs) noexcept
    {
        ceiling = std::pow (10.0f, (std::clamp (ceilingDb, -24.0f, 0.0f) - kMarginDb) / 20.0f);
        relK = 1.0f - onePole (std::clamp (releaseMs, 1.0f, 1000.0f) * 0.001f, fs);
    }

    void resetState() noexcept
    {
        for (auto& h : hist)
            h.fill (0.0f);
        histPos = 0;
        for (auto& d : delay)
            std::fill (d.begin(), d.end(), 0.0f);
        for (int k = 0; k < kSmooth; ++k)
        {
            std::fill (avgRing[(size_t) k].begin(), avgRing[(size_t) k].end(), 1.0);
            avgSum[(size_t) k] = (double) avgLen[(size_t) k];
        }
        pos = 0;
        prevU.fill (0.0f);
        prevPrevU.fill (0.0f);
        openPeak = completedPrev = 0.0f;
        attMax.resetState();
        env = 1.0f;
        gainNow = 1.0f;
    }

    // in place, n base-rate frames; returns the deepest gain reduction in dB in this call
    float process (float* const* io, int numCh, int n) noexcept
    {
        float minGain = 1.0f;
        const int nc = std::clamp (numCh, 1, kMaxCh);
        constexpr int centre = kTaps / 2 - 1;
        for (int i = 0; i < n; ++i)
        {
            // --- detector. w[k] = x[now - 63 + k]; this step yields the points c + p/8 (p = 0..7) of the
            //     interval [c, c + 1), c = now - 32. A local maximum is refined when its successor arrives;
            //     the successor of the p = 7 point is this step's p = 0 point, so interval c - 1 completes now.
            float thisPeak = 0.0f, lateForPrev = 0.0f;
            for (int c = 0; c < nc; ++c)
            {
                auto& h = hist[(size_t) c];
                const float v = io[c][i];
                h[(size_t) histPos] = v;
                h[(size_t) (histPos + kTaps)] = v;
                const float* w = h.data() + histPos + 1;
                float a = prevPrevU[(size_t) c], b = prevU[(size_t) c];
                // all 8 phases at once (vectorises); phase 0 is a delta: the centre sample itself
                float acc[kPhases] {};
                for (int k = 0; k < kTaps; ++k)
                {
                    const float wk = w[k];
                    const float* t = tapsT[(size_t) k].data();
                    for (int p = 0; p < kPhases; ++p)
                        acc[p] += t[p] * wk;
                }
                acc[0] = w[centre];
                for (int p = 0; p < kPhases; ++p)
                {
                    const float u = std::abs (acc[p]);
                    if (b >= a && b >= u)
                    {
                        const float den = a - 2.0f * b + u;
                        const float pk = den < 0.0f ? b + 0.125f * (a - u) * (a - u) / -den : b;
                        (p == 0 ? lateForPrev : thisPeak) = std::max (p == 0 ? lateForPrev : thisPeak, pk);
                    }
                    thisPeak = std::max (thisPeak, u);
                    a = b;
                    b = u;
                }
                prevPrevU[(size_t) c] = a;
                prevU[(size_t) c] = b;
            }
            histPos = (histPos + 1) % kTaps;
            const float completed = std::max (openPeak, lateForPrev);   // interval [now - 33, now - 32)
            const float peak = std::max (completedPrev, completed);     // (b - 1, b + 1), b = now - 33
            completedPrev = completed;
            openPeak = thisPeak;

            // --- gain computer (for sample b)
            const float att = peak > ceiling ? peak / ceiling : 1.0f;
            const float need = 1.0f / attMax.process (att, 0);         // running min of the gain over L
            env = need < env ? need : std::min (need, env + (need - env) * relK);
            const int ap = pos & ringMask;
            double g = env;
            for (int k = 0; k < kSmooth; ++k)
            {
                auto& r = avgRing[(size_t) k];
                const int len = avgLen[(size_t) k];
                avgSum[(size_t) k] += g - r[(size_t) ((pos - len) & ringMask)];
                r[(size_t) ap] = g;
                g = avgSum[(size_t) k] / (double) len;
            }
            gainNow = std::min (1.0f, (float) g);
            minGain = std::min (minGain, gainNow);

            // --- audio: the average just computed belongs to sample b - (L - 1) = now - (32 + L)
            for (int c = 0; c < nc; ++c)
            {
                auto& d = delay[(size_t) c];
                d[(size_t) ap] = io[c][i];
                io[c][i] = d[(size_t) ((pos - latency()) & ringMask)] * gainNow;
            }
            ++pos;
        }
        return minGain < 1.0f ? -20.0f * std::log10 (minGain) : 0.0f;
    }

    float currentGain() const noexcept { return gainNow; }

private:
    void designInterpolator()
    {
        constexpr int N = kPhases * kTaps + 1, M = N / 2; // h[n] = sinc((n - M) / 8) * kaiser
        auto bessel0 = [] (double x)
        {
            double sum = 1.0, term = 1.0;
            for (int k = 1; k < 60; ++k)
            {
                term *= (x / (2.0 * k)) * (x / (2.0 * k));
                sum += term;
            }
            return sum;
        };
        constexpr double beta = 8.0;
        const double i0b = bessel0 (beta);
        std::vector<double> h ((size_t) N, 0.0);
        for (int n = 0; n < N; ++n)
        {
            const double t = (double) (n - M) / kPhases;
            const double sinc = n == M ? 1.0 : std::sin (kPi * t) / (kPi * t);
            const double r = (double) (n - M) / M;
            h[(size_t) n] = sinc * bessel0 (beta * std::sqrt (std::max (0.0, 1.0 - r * r))) / i0b;
        }
        // point at centre + p/8 = sum_k w[k] h[M + 8 (centre - k) + p]
        constexpr int centre = kTaps / 2 - 1;
        for (int p = 1; p < kPhases; ++p)
        {
            double sum = 0.0;
            for (int k = 0; k < kTaps; ++k)
            {
                const int idx = M + kPhases * (centre - k) + p;
                const double v = (idx >= 0 && idx < N) ? h[(size_t) idx] : 0.0;
                tapsT[(size_t) k][(size_t) p] = (float) v;
                sum += v;
            }
            for (int k = 0; k < kTaps; ++k) // unity DC gain per phase
                tapsT[(size_t) k][(size_t) p] = (float) (tapsT[(size_t) k][(size_t) p] / sum);
        }
    }

    static constexpr float kMarginDb = 0.05f;
    double fs = 44100.0;
    int maxLook = 66, look = 66, ringMask = 0, pos = 0, histPos = 0;
    std::array<std::array<float, kPhases>, kTaps> tapsT {}; // [tap][phase]
    std::array<std::array<float, 2 * kTaps>, kMaxCh> hist {};
    std::array<std::vector<float>, kMaxCh> delay;
    std::array<std::vector<double>, kSmooth> avgRing;
    std::array<double, kSmooth> avgSum {};
    std::array<int, kSmooth> avgLen {};
    std::array<float, kMaxCh> prevU {}, prevPrevU {};
    float openPeak = 0.0f, completedPrev = 0.0f;
    SlidingMax attMax;
    float ceiling = 0.891f, relK = 0.001f, env = 1.0f, gainNow = 1.0f;
};
} // namespace duo
