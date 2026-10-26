#pragma once
// Axell Duo Compressor 4.0 checks: ESS HOLD (the gain holds through the voice's esses at the word's own, axell-core
// SibilanceDetector, Axell Velvet 3.0's LIVE network).
#include "V3Tests.h"

namespace v4
{
using namespace dt;

// ESS HOLD must remove at least this share of the esses' lift on the synthetic voice (measured: 79-84 % at 44.1-96 kHz)
constexpr double kMinEssShare = 0.6;

template <class UT>
double essGr (const Ctx& c, bool ess, double wordDb, double& wordGr)
{
    // 0.3 s of a loud tone (the word's peak), 0.2 s of a softer one (the word's tail), then 0.08 s of a quiet one
    // (the ess, marked or not); returns the gain reduction at the end of the quiet part, wordGr = the GR at the ess's
    // start (the word's own: ESS HOLD holds it and glides up to the word's recent peak)
    const double fs = c.fs * 2;
    UT u;
    u.prepare (fs, c.nCh);
    duo::Params p;
    p.fetInDb = 17.0f;
    p.optoPR = 70.0f;
    p.colorPct = 0.0f;
    u.set (p);
    const int a = (int) (0.3 * fs), b = a + (int) (0.2 * fs), n = b + (int) (0.08 * fs);
    const double w = 2.0 * kPi * 1000.0 / fs;
    float x[2] {}, g = 0.0f;
    wordGr = 0.0;
    for (int i = 0; i < n; ++i)
    {
        u.st.setEss (ess && i >= b);
        const double lv = i < a ? -6.0 : (i < b ? wordDb : -40.0);
        x[0] = x[1] = (float) (fromDb (lv) * std::sin (w * i));
        g = u.frame (x);
        if (i == b - 1)
            wordGr = g;
    }
    return g;
}

inline void testV4 (Recorder& r)
{
    r.section = "V12 Ess hold (4.0)";
    const Ctx& c = r.ctx;
    const double fs = c.fs;

    {
        bool ok = true;
        for (const auto& fp : duo::factoryPresets())
            ok = ok && fp.params.essHold == fp.params.voiceHold;
        r.truth ("factory presets: ESS HOLD on exactly where VOICE HOLD is (the vocal presets and Init)", ok);
    }
    r.truth ("engine default: ESS HOLD off (the 3.x path; the plugin turns it on for new instances)", ! duo::Params().essHold);

    //== stage level: through an ess the release stops and the GR glides to the word's (its recent peak)
    {
        double wf = 0, wo = 0;
        const double fOff = essGr<FetUT> (c, false, -14.0, wf), fOn = essGr<FetUT> (c, true, -14.0, wf);
        r.info ("FET: gain reduction at the ess's start", wf, "dB");
        r.less ("FET without ESS HOLD: share of that GR left at the end of the 80 ms ess (releasing)", fOff / wf, 0.75, "");
        r.range ("FET with ESS HOLD: GR at the end of the ess re its start (held, up to the word's peak)", fOn - wf, -0.25, 6.0, "dB");
        const double oOff = essGr<OptoUT> (c, false, -14.0, wo), oOn = essGr<OptoUT> (c, true, -14.0, wo);
        r.info ("opto: gain reduction at the ess's start", wo, "dB");
        r.greater ("opto with ESS HOLD: holds more than it would release", oOn - oOff, 0.5, "dB");
        r.range ("opto with ESS HOLD: GR at the end of the ess re its start (held, up to the word's peak)", oOn - wo, -0.25, 6.0, "dB");
    }

    //== engine: a vocal with esses - the esses are no longer lifted above the vowels, the vowels are compressed as before
    {
        auto v = v3::makeVoice (fs, c.nCh, 12.0, -10.0, -70.0);
        duo::Params p;
        p.fetInDb = 17.0f;
        p.optoPR = 70.0f;
        auto eOff = makeEngine (c, p);
        const Buf off = run (*eOff, v.x);
        p.essHold = true;
        auto eOn = makeEngine (c, p);
        const Buf on = run (*eOn, v.x);
        // per 0.32 s syllable: vowel 0.02-0.20 s, ess 0.24-0.31 s (of each phrase, phrases 0.2-1.8 s of the 2.4 s cycle)
        auto partDb = [fs] (const Buf& b, double lo, double hi)
        {
            double e = 0;
            size_t k = 0;
            for (size_t i = (size_t) (2.4 * fs); i < b[0].size(); ++i)
            {
                const double ph = std::fmod ((double) i / fs, 2.4);
                if (ph < 0.2 || ph >= 1.8)
                    continue;
                const double syl = std::fmod (ph, 0.32);
                if (syl >= lo && syl < hi)
                {
                    e += (double) b[0][i] * b[0][i];
                    ++k;
                }
            }
            return 10.0 * std::log10 (e / (double) std::max<size_t> (k, 1) + 1.0e-30);
        };
        const double vIn = partDb (v.x, 0.02, 0.20), sIn = partDb (v.x, 0.24, 0.31);
        const double vOff = partDb (off, 0.02, 0.20), sOff = partDb (off, 0.24, 0.31);
        const double vOn = partDb (on, 0.02, 0.20), sOn = partDb (on, 0.24, 0.31);
        r.info ("esses re vowels, off: lift", (sOff - vOff) - (sIn - vIn), "dB");
        r.info ("esses re vowels, on: lift", (sOn - vOn) - (sIn - vIn), "dB");
        const double liftOff = (sOff - vOff) - (sIn - vIn), liftOn = (sOn - vOn) - (sIn - vIn);
        r.greater ("ESS HOLD removes of the esses' lift (dB, against 60 % of the lift without it)", liftOff - liftOn,
                   kMinEssShare * std::max (liftOff, 0.0), "dB");
        r.less ("ESS HOLD never lifts the esses more", liftOn - liftOff, 0.0, "dB");
        r.range ("ESS HOLD: vowel level unchanged", vOn - vOff, -0.5, 0.5, "dB");
        r.equal ("ESS HOLD: latency unchanged", (double) (eOn->getLatencySamples() - eOff->getLatencySamples()), 0.0, "samples");
    }

    //== off: the 3.x path. The flag only ever adds a hold, so with it off nothing may move
    {
        auto v = v3::makeVoice (fs, c.nCh, 4.0, -10.0, -56.0, 11);
        duo::Params p;
        p.fetInDb = 17.0f;
        p.optoPR = 70.0f;
        p.voiceHold = true;
        auto e1 = makeEngine (c, p);
        auto e2 = makeEngine (c, p);
        const Buf a = run (*e1, v.x), b = run (*e2, v.x);
        double d = 0;
        for (size_t ch = 0; ch < a.size(); ++ch)
            for (size_t i = 0; i < a[ch].size(); ++i)
                d = std::max (d, (double) std::abs (a[ch][i] - b[ch][i]));
        r.equal ("ESS HOLD off: deterministic (two engines, same input)", d, 0.0, "");
    }

    //== real-time: block-size invariance, no allocation, finite output
    {
        auto v = v3::makeVoice (fs, c.nCh, 3.0, -10.0, -56.0, 9);
        duo::Params p;
        p.essHold = true;
        p.voiceHold = true;
        p.osLog2 = 2;
        auto e1 = makeEngine (c, p);
        auto e2 = makeEngine (c, p);
        const Buf key;
        auto a = v2::runKeyed (*e1, v.x, key, 512);
        v2::allocCount = 0;
        auto b = v2::runKeyed (*e2, v.x, key, 37, true);
        double d = 0;
        for (size_t ch = 0; ch < a.size(); ++ch)
            for (size_t i = 0; i < a[ch].size(); ++i)
                d = std::max (d, (double) std::abs (a[ch][i] - b[ch][i]));
        r.less ("ESS HOLD: block 512 vs 37 (difference)", db (d), -140.0, "dBFS");
        r.truth ("ESS HOLD: output finite", allFinite (b));
        r.equal ("ESS HOLD: heap allocations inside process()", (double) v2::allocCount, 0.0, "");
    }
}
} // namespace v4
