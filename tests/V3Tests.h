#pragma once
// Axell Duo Compressor 3.0 checks: VOICE HOLD (the gain holds through the voice's pauses, axell-core VoicePresence).
#include <random>

namespace v3
{
using namespace dt;

// synthetic voice (Axell Gate's test voice): vowels through formants with sibilants, 1.6 s phrases, 0.8 s pauses,
// over a noise floor; active[i] = inside a phrase
struct Voice
{
    Buf x;
    std::vector<uint8_t> active;
};
inline Voice makeVoice (double fs, int nCh, double secs, double gainDb, double noiseDb, unsigned seed = 5)
{
    Voice v;
    const size_t n = (size_t) (secs * fs);
    v.x.assign ((size_t) nCh, std::vector<float> (n, 0.0f));
    v.active.assign (n, 0);
    std::mt19937 g (seed);
    std::normal_distribution<double> nd;
    const double gain = std::pow (10.0, gainDb / 20.0), ng = std::pow (10.0, noiseDb / 20.0);
    double ph = 0.0, b0 = 0, b1 = 0, s1 = 0, s2 = 0;
    const double w = 2 * kPi * 7000 / fs, r = std::exp (-kPi * 1500 / fs);
    for (size_t i = 0; i < n; ++i)
    {
        const double t = (double) i / fs;
        const double phrase = std::fmod (t, 2.4);
        const bool on = phrase > 0.2 && phrase < 1.8;
        const double syl = std::fmod (phrase, 0.32);
        double y = 0.0;
        if (on)
        {
            const double f0 = 140.0 + 25.0 * std::sin (2 * kPi * 0.6 * t);
            ph += 2 * kPi * f0 / fs;
            if (syl < 0.22)
            {
                const double env = std::min (1.0, std::min (syl, 0.22 - syl) / 0.015);
                double s = 0;
                for (int h = 1; h * f0 < std::min (0.45 * fs, 8000.0); ++h)
                {
                    const double f = h * f0;
                    s += (std::exp (-std::pow ((f - 650) / 250, 2)) + 0.6 * std::exp (-std::pow ((f - 1150) / 300, 2))
                          + 0.2 * std::exp (-std::pow ((f - 2600) / 500, 2))) * std::sin (h * ph) / std::sqrt ((double) h);
                }
                y = 0.35 * env * s;
            }
            else
            {
                const double a = (1 - r) * nd (g) + 2 * r * std::cos (w) * s1 - r * r * s2;
                s2 = s1;
                s1 = a;
                y = 0.8 * a * std::sin (kPi * (syl - 0.22) / 0.1);
            }
            v.active[i] = 1;
        }
        const double wn = nd (g);
        b0 = 0.997 * b0 + 0.03 * wn;
        b1 = 0.6 * b1 + 0.4 * wn;
        const double nz = ng * (b0 + 0.3 * b1);
        for (auto& ch : v.x)
            ch[i] = (float) (gain * y + nz);
    }
    return v;
}

// RMS (dB) of channel 0 over the given part of every 2.4 s cycle, from `from` seconds on
inline double cycleRmsDb (const Buf& b, double fs, double lo, double hi, double from = 2.4)
{
    double e = 0.0;
    size_t k = 0;
    for (size_t i = (size_t) (from * fs); i < b[0].size(); ++i)
    {
        const double ph = std::fmod ((double) i / fs, 2.4);
        if (ph >= lo && ph < hi)
        {
            e += (double) b[0][i] * b[0][i];
            ++k;
        }
    }
    return 10.0 * std::log10 (e / std::max<size_t> (k, 1) + 1.0e-30);
}

template <class UT>
double heldGr (const Ctx& c, bool pause)
{
    // 1 s of a loud tone (voice), then 0.6 s of a quiet one; the gain reduction 0.4 s into the quiet part
    const double fs = c.fs * 2;
    UT u;
    u.prepare (fs, c.nCh);
    duo::Params p;
    p.fetInDb = 17.0f;
    p.optoPR = 70.0f;
    p.colorPct = 0.0f;
    u.set (p);
    const int loud = (int) fs, n = loud + (int) (0.6 * fs);
    const double w = 2.0 * kPi * 1000.0 / fs;
    float x[2] {}, gr = 0.0f;
    for (int i = 0; i < n; ++i)
    {
        u.st.setPause (pause && i >= loud);
        x[0] = x[1] = (float) ((i < loud ? fromDb (-6.0) : fromDb (-40.0)) * std::sin (w * i));
        const float g = u.frame (x);
        if (i == loud + (int) (0.4 * fs))
            gr = g;
    }
    return gr;
}
template <class UT>
double voiceGr (const Ctx& c)
{
    const double fs = c.fs * 2;
    UT u;
    u.prepare (fs, c.nCh);
    duo::Params p;
    p.fetInDb = 17.0f;
    p.optoPR = 70.0f;
    p.colorPct = 0.0f;
    u.set (p);
    const double w = 2.0 * kPi * 1000.0 / fs;
    float x[2] {}, g = 0.0f;
    for (int i = 0; i < (int) fs; ++i)
    {
        x[0] = x[1] = (float) (fromDb (-6.0) * std::sin (w * i));
        g = u.frame (x);
    }
    return g;
}

inline void testV3 (Recorder& r)
{
    r.section = "V11 Voice hold (3.0)";
    const Ctx& c = r.ctx;
    const double fs = c.fs;

    {
        bool ok = true;
        for (const auto& fp : duo::factoryPresets())
            ok = ok && fp.params.voiceHold == (fp.category == duo::PresetCategory::vocal || std::string (fp.name) == "Init");
        r.truth ("factory presets: VOICE HOLD on exactly for the vocal presets and Init", ok);
    }
    r.truth ("engine default: VOICE HOLD off (the 2.x path; the plugin turns it on for new instances)", ! duo::Params().voiceHold);

    //== stage level: in a pause the release stops and the GR sits at the voice's recent average
    {
        const double fv = voiceGr<FetUT> (c), fOff = heldGr<FetUT> (c, false), fOn = heldGr<FetUT> (c, true);
        r.info ("FET: gain reduction on the loud tone", fv, "dB");
        r.less ("FET without pause: share of the GR left 0.4 s after the drop (released)", fOff / fv, 0.15, "");
        r.range ("FET in a pause: gain reduction re the voice's (held)", fOn - fv, -2.0, 0.5, "dB");
        const double ov = voiceGr<OptoUT> (c), oOff = heldGr<OptoUT> (c, false), oOn = heldGr<OptoUT> (c, true);
        r.info ("opto: gain reduction on the loud tone", ov, "dB");
        r.greater ("opto in a pause: holds more than it would release", oOn - oOff, 0.5 * ov, "dB");
        r.range ("opto in a pause: gain reduction re the voice's (held)", oOn - ov, -3.0, 0.5, "dB");
    }

    //== engine: a vocal with pauses - the pauses are no longer lifted, the voice is compressed as before
    {
        auto v = makeVoice (fs, c.nCh, 12.0, -10.0, -56.0);
        duo::Params p;
        p.fetInDb = 17.0f;
        p.optoPR = 70.0f;
        auto eOff = makeEngine (c, p);
        const Buf off = run (*eOff, v.x);
        p.voiceHold = true;
        auto eOn = makeEngine (c, p);
        const Buf on = run (*eOn, v.x);
        // pauses: 1.85-2.35 s of each cycle; vowels: the whole phrase, edges excluded
        const double pauseOff = cycleRmsDb (off, fs, 1.9, 2.35), pauseOn = cycleRmsDb (on, fs, 1.9, 2.35);
        const double voiceOff = cycleRmsDb (off, fs, 0.3, 1.7), voiceOn = cycleRmsDb (on, fs, 0.3, 1.7);
        const double pauseIn = cycleRmsDb (v.x, fs, 1.9, 2.35), voiceIn = cycleRmsDb (v.x, fs, 0.3, 1.7);
        r.info ("pauses re voice, off: lift", (pauseOff - voiceOff) - (pauseIn - voiceIn), "dB");
        r.info ("pauses re voice, on: lift", (pauseOn - voiceOn) - (pauseIn - voiceIn), "dB");
        r.greater ("VOICE HOLD lowers the pauses' lift by", pauseOff - pauseOn, 8.0, "dB");
        r.range ("VOICE HOLD: voice level unchanged", voiceOn - voiceOff, -0.75, 0.75, "dB");
        r.equal ("VOICE HOLD: latency unchanged", (double) (eOn->getLatencySamples() - eOff->getLatencySamples()), 0.0, "samples");
    }

    //== no voice (a drum loop): VOICE HOLD changes nothing
    {
        duo::Params p;
        p.fetInDb = 17.0f;
        p.optoPR = 70.0f;
        const int n = (int) (6.0 * fs);
        Buf in = zeros (c.nCh, n);
        std::mt19937 g (31);
        std::normal_distribution<double> nd;
        for (int k = 0; k < n; ++k)
        {
            const double t = k / fs, beat = std::fmod (t, 0.5), sb = std::fmod (t + 0.25, 0.5), hh = std::fmod (t, 0.125);
            const double v = 0.9 * std::exp (-beat * 18.0) * std::sin (2.0 * kPi * (55.0 + 90.0 * std::exp (-beat * 40.0)) * beat)
                           + 0.35 * std::exp (-sb * 30.0) * nd (g) + 0.08 * std::exp (-hh * 120.0) * nd (g);
            for (auto& ch : in)
                ch[(size_t) k] = (float) (0.5 * v);
        }
        auto eOff = makeEngine (c, p);
        const Buf off = run (*eOff, in);
        p.voiceHold = true;
        auto eOn = makeEngine (c, p);
        const Buf on = run (*eOn, in);
        double d = 0;
        for (size_t ch = 0; ch < on.size(); ++ch)
            for (size_t k = 0; k < on[ch].size(); ++k)
                d = std::max (d, (double) std::abs (on[ch][k] - off[ch][k]));
        // the voice network takes drum hits for a voice (it never saw drums): the presets for beats and buses turn VOICE
        // HOLD off. Documented here so a change of that behaviour shows up.
        r.info ("drum loop: VOICE HOLD on vs off (peak difference; beat/bus presets turn it off)", db (d), "dBFS");
    }

    //== real-time: block-size invariance, no allocation, finite output
    {
        auto v = makeVoice (fs, c.nCh, 3.0, -10.0, -56.0, 9);
        duo::Params p;
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
        r.less ("VOICE HOLD: block 512 vs 37 (difference)", db (d), -140.0, "dBFS");
        r.truth ("VOICE HOLD: output finite", allFinite (b));
        r.equal ("VOICE HOLD: heap allocations inside process()", (double) v2::allocCount, 0.0, "");
    }
}
} // namespace v3
