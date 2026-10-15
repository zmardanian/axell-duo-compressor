#pragma once
// Measurement helpers for the DSP test suite (no JUCE).
#include "../Source/Engine.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdint>
#include <numeric>
#include <vector>

namespace dt
{
using Buf = std::vector<std::vector<float>>;
constexpr double kPi = 3.14159265358979323846;

inline double db (double a) { return 20.0 * std::log10 (std::max (a, 1.0e-30)); }
inline double fromDb (double d) { return std::pow (10.0, d / 20.0); }

inline int nextPow2 (int n)
{
    int p = 1;
    while (p < n)
        p <<= 1;
    return p;
}

//------------------------------------------------------------------------------
// 7-term Blackman-Harris window: ~-180 dB sidelobes, main lobe +-7 bins.
inline std::vector<double> bh7 (int n)
{
    static const double a[7] = { 0.27105140069342, 0.43329793923448, 0.21812299954311, 0.06592544638803,
                                 0.01081174209837, 0.00077658482522, 0.00001388721735 };
    std::vector<double> w ((size_t) n);
    for (int i = 0; i < n; ++i)
    {
        const double t = 2.0 * kPi * i / n;
        double v = 0.0;
        for (int k = 0; k < 7; ++k)
            v += (k % 2 == 0 ? 1.0 : -1.0) * a[k] * std::cos (k * t);
        w[(size_t) i] = v;
    }
    return w;
}

// amplitude (peak) of the sinusoid at `freq` within x[0..n)
inline double toneAmp (const float* x, int n, double freq, double fs)
{
    const auto w = bh7 (n);
    double re = 0.0, im = 0.0, ws = 0.0;
    const double dw = 2.0 * kPi * freq / fs;
    for (int i = 0; i < n; ++i)
    {
        re += w[(size_t) i] * x[i] * std::cos (dw * i);
        im -= w[(size_t) i] * x[i] * std::sin (dw * i);
        ws += w[(size_t) i];
    }
    return 2.0 * std::sqrt (re * re + im * im) / ws;
}

inline void fft (std::vector<std::complex<double>>& a)
{
    const size_t n = a.size();
    for (size_t i = 1, j = 0; i < n; ++i)
    {
        size_t bit = n >> 1;
        for (; j & bit; bit >>= 1)
            j ^= bit;
        j ^= bit;
        if (i < j)
            std::swap (a[i], a[j]);
    }
    for (size_t len = 2; len <= n; len <<= 1)
    {
        const double ang = -2.0 * kPi / (double) len;
        const std::complex<double> wl (std::cos (ang), std::sin (ang));
        for (size_t i = 0; i < n; i += len)
        {
            std::complex<double> w (1.0, 0.0);
            for (size_t j = 0; j < len / 2; ++j)
            {
                const auto u = a[i + j], v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w *= wl;
            }
        }
    }
}

// sine-calibrated amplitude spectrum (bins 0..n/2), n must be a power of two
inline std::vector<double> spectrum (const float* x, int n)
{
    const auto w = bh7 (n);
    std::vector<std::complex<double>> a ((size_t) n);
    double ws = 0.0;
    for (int i = 0; i < n; ++i)
    {
        a[(size_t) i] = w[(size_t) i] * x[i];
        ws += w[(size_t) i];
    }
    fft (a);
    std::vector<double> m ((size_t) (n / 2 + 1));
    for (int i = 0; i <= n / 2; ++i)
        m[(size_t) i] = 2.0 * std::abs (a[(size_t) i]) / ws;
    return m;
}

struct Thd
{
    double h1 = 0.0, thd = 0.0, h2 = 0.0, h3 = 0.0, sumSq = 0.0;
    double share23() const { return sumSq > 0.0 ? (h2 * h2 + h3 * h3) / sumSq : 1.0; }
};

inline Thd thd (const float* x, int n, double f0, double fs)
{
    Thd r;
    r.h1 = toneAmp (x, n, f0, fs);
    for (int k = 2; k <= 15 && k * f0 < 0.45 * fs; ++k)
    {
        const double h = toneAmp (x, n, k * f0, fs);
        r.sumSq += h * h;
        if (k == 2) r.h2 = h;
        if (k == 3) r.h3 = h;
    }
    r.thd = std::sqrt (r.sumSq) / std::max (r.h1, 1.0e-30);
    return r;
}

//------------------------------------------------------------------------------
// signals
inline Buf zeros (int nCh, int n) { return Buf ((size_t) nCh, std::vector<float> ((size_t) n, 0.0f)); }

inline void addSine (Buf& b, double fs, double f, double amp, int from = 0, int to = -1, double phase = 0.0)
{
    for (auto& ch : b)
    {
        const int end = to < 0 ? (int) ch.size() : to;
        for (int i = from; i < end; ++i)
            ch[(size_t) i] += (float) (amp * std::sin (2.0 * kPi * f * i / fs + phase));
    }
}

struct Rng
{
    uint64_t s;
    explicit Rng (uint64_t seed) : s (seed * 0x9E3779B97F4A7C15ull + 1) {}
    uint64_t next() { s ^= s << 13; s ^= s >> 7; s ^= s << 17; return s; }
    double uni() { return (double) (next() >> 11) * (1.0 / 9007199254740992.0); }      // [0,1)
    double bip() { return 2.0 * uni() - 1.0; }
    double gauss() { double a = 0.0; for (int i = 0; i < 4; ++i) a += bip(); return a * 0.866; }
};

inline void addNoise (Buf& b, double amp, uint64_t seed, bool sameOnAllChannels = false)
{
    for (size_t c = 0; c < b.size(); ++c)
    {
        Rng r (seed + (sameOnAllChannels ? 0 : 7919 * c));
        for (auto& v : b[c])
            v += (float) (amp * r.gauss());
    }
}

// vocal-like program material: noise + harmonic tone, syllable-rate envelope
inline Buf program (int nCh, int n, double fs, uint64_t seed, bool identicalChannels = false)
{
    Buf b = zeros (nCh, n);
    for (size_t c = 0; c < b.size(); ++c)
    {
        Rng r (seed + (identicalChannels ? 0 : 31 * c));
        double lp = 0.0;
        for (int i = 0; i < n; ++i)
        {
            const double t = i / fs;
            const double env = 0.15 + 0.85 * std::pow (std::abs (std::sin (2.0 * kPi * 2.7 * t)), 3.0);
            lp += 0.2 * (r.gauss() - lp);
            const double tone = std::sin (2.0 * kPi * 180.0 * t) + 0.5 * std::sin (2.0 * kPi * 360.0 * t + 0.3)
                              + 0.25 * std::sin (2.0 * kPi * 2400.0 * t);
            b[c][(size_t) i] = (float) (0.35 * env * (0.6 * tone + 0.8 * lp));
        }
    }
    return b;
}

// mix-bus-like program material: kick (55 Hz decaying, every 0.5 s), snare (noise burst on
// the off-beat), sustained chord pad and a vocal-like layer. Used to check mix/master presets.
inline Buf mixProgram (int nCh, int n, double fs, uint64_t seed)
{
    Buf b = zeros (nCh, n);
    const Buf vox = program (nCh, n, fs, seed + 99);
    for (size_t c = 0; c < b.size(); ++c)
    {
        Rng r (seed + 17 * c);
        for (int i = 0; i < n; ++i)
        {
            const double t = i / fs;
            const double beat = std::fmod (t, 0.5);
            const double kick = std::exp (-beat * 18.0) * std::sin (2.0 * kPi * (55.0 + 90.0 * std::exp (-beat * 40.0)) * beat);
            const double sb = std::fmod (t + 0.25, 0.5);
            const double snare = std::exp (-sb * 30.0) * r.gauss() * 0.5;
            const double pad = 0.12 * (std::sin (2.0 * kPi * 220.0 * t) + std::sin (2.0 * kPi * 277.2 * t + 1.0)
                                       + std::sin (2.0 * kPi * 329.6 * t + 2.0));
            b[c][(size_t) i] = (float) (0.9 * kick + 0.35 * snare + pad + 0.5 * vox[c][(size_t) i]);
        }
    }
    return b;
}

//------------------------------------------------------------------------------
// run an engine over a buffer in host-sized blocks (in place on a copy)
template <typename PerBlock>
inline Buf run (duo::Engine& e, const Buf& in, int block, PerBlock&& perBlock)
{
    Buf out = in;
    const int nCh = (int) out.size();
    const int n = (int) out[0].size();
    for (int pos = 0; pos < n; pos += block)
    {
        const int len = std::min (block, n - pos);
        perBlock (pos, len);
        float* ptr[2] {};
        for (int c = 0; c < nCh; ++c)
            ptr[c] = out[(size_t) c].data() + pos;
        e.process (ptr, nCh, len);
    }
    return out;
}

inline Buf run (duo::Engine& e, const Buf& in, int block = 512)
{
    return run (e, in, block, [] (int, int) {});
}

inline double maxAbs (const std::vector<float>& x, int from = 0, int to = -1)
{
    const int end = to < 0 ? (int) x.size() : to;
    double m = 0.0;
    for (int i = from; i < end; ++i)
        m = std::max (m, (double) std::abs (x[(size_t) i]));
    return m;
}

inline bool allFinite (const Buf& b)
{
    for (auto& ch : b)
        for (float v : ch)
            if (! std::isfinite (v))
                return false;
    return true;
}
} // namespace dt
