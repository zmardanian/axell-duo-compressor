#pragma once
// Factory presets (framework independent so the test suite can verify them).
//
// The first six are the ones named in ENGINEERING_SPEC.md section 4. The rest recreate widely used,
// well-documented vocal and mix-bus compression techniques (serial FET -> opto, opto-first,
// New York parallel, all-buttons smash, mix-bus glue, mastering leveling/peak control). Names
// describe the technique; no third-party preset data or branding is used.
//
// Gain staging (tuned with the engine, verified by DuoDspTests at every sample rate):
//   vocal presets: vocal-like test signal at -18 dBFS RMS (peaks -4 dBFS)
//   mix/master presets: mix-like signal (kick, snare, pad, vocal) at -14 dBFS RMS
// INPUT sets the FET peak GR, FET OUTPUT recovers it (as on a 1176), PEAK REDUCTION sets the
// average opto GR and Opto GAIN (or OUTPUT for parallel presets) makes it up. Output loudness
// is within +-1.5 dB of the input, so switching presets compares sound, not level.
// The intended GR is listed in each description (FET = peak GR, opto = average GR).
#include "Engine.h"

#include <array>
#include <string_view>

namespace duo
{
enum class PresetCategory { vocal, mixMaster, utility };

struct FactoryPreset
{
    const char* name;
    PresetCategory category;
    const char* description;
    Params params;
};

namespace preset_detail
{
struct Row
{
    const char* name;
    PresetCategory cat;
    const char* desc;
    bool fetOn; float fetIn, fetOut, atkMs, relMs; int ratio;
    bool optoOn; float pr, optoGain; int mode;
    int order; float hpf, color, mix, out, link, emph; int os;
};

using C = PresetCategory;
// clang-format off
// fields: fetOn in fetOut atk(ms) rel(ms) ratio | optoOn PR optoGain mode | order hpf color mix out link emph os
// PR values are for the Measured opto law (2.0; tools/retune_presets.cpp kept each preset's 1.x opto gain reduction)
constexpr Row kRows[] = {
    { "Rap Lead", C::vocal, "Upfront lead rap vocal: FET grabs consonant peaks (~7 dB), opto adds ~3 dB of steady leveling.",
      true, 1.0f, 4.0f, 0.25f, 120.f, 0,  true, 57.8f, 3.5f, 0,  0, 100.f, 35.f, 100.f, 0.0f, 100.f, 20.f, 2 },
    { "Boom Bap Double", C::vocal, "Doubles/stacks: harder 8:1 FET (~10 dB) and ~4.5 dB opto so the double sits flat behind the lead.",
      true, 3.0f, 5.0f, 0.4f, 200.f, 1,  true, 62.7f, 4.5f, 0,  0, 140.f, 45.f, 100.f, 0.0f, 100.f, 35.f, 2 },
    { "Aggressive Parallel (All)", C::vocal, "All-buttons smash (~18 dB) blended 45 % under the dry vocal: loud and dense, transients kept.",
      true, 9.0f, -2.0f, 0.15f, 80.f, 4,  true, 68.1f, 1.0f, 0,  0, 120.f, 60.f, 45.f, 2.5f, 100.f, 0.f, 2 },
    { "Gentle Leveling", C::vocal, "The opto does the work (~4 dB), the FET only touches the loudest peaks (~3 dB).",
      true, -4.0f, 6.0f, 0.6f, 300.f, 0,  true, 61.2f, 4.5f, 0,  0, 80.f, 20.f, 100.f, 0.0f, 100.f, 0.f, 2 },
    { "Spoken Word", C::vocal, "Podcast/interview: even and natural (FET ~5 dB, opto ~4 dB), low colour, no pumping between words.",
      true, -1.5f, 5.5f, 0.4f, 250.f, 0,  true, 60.3f, 4.0f, 0,  0, 90.f, 15.f, 100.f, 0.0f, 100.f, 10.f, 2 },
    { "Init", C::utility, "All controls at their defaults.",
      true, 6.0f, 0.0f, 0.2f, 150.f, 0,  true, 30.0f, 0.0f, 0,  0, 80.f, 30.f, 100.f, 0.0f, 100.f, 0.f, 2 },
    // vocal techniques
    { "Vocal: Classic Serial (FET > Opto)", C::vocal, "The classic chain: fast FET catches peaks (~5 dB), opto behind it levels the phrase (~3 dB).",
      true, -1.5f, 4.5f, 0.3f, 90.f, 0, true, 58.6f, 3.5f, 0, 0, 90.f, 30.f, 100.f, 0.0f, 100.f, 0.f, 2 },
    { "Vocal: Opto First, FET Catch", C::vocal, "Opto levels the performance first (~4 dB), then an 8:1 FET only catches what still pokes out (~4 dB).",
      true, -3.5f, 6.0f, 0.1f, 80.f, 1, true, 60.8f, 4.0f, 0, 1, 90.f, 25.f, 100.f, 0.0f, 100.f, 0.f, 2 },
    { "Vocal: New York Parallel", C::vocal, "Heavy 12:1 FET (~14 dB) mixed 40 % under the dry vocal: thickness without losing the attack.",
      true, 7.0f, 0.0f, 0.1f, 60.f, 2, true, 61.9f, 1.0f, 0, 0, 110.f, 50.f, 40.f, 1.5f, 100.f, 0.f, 2 },
    { "Vocal: Upfront Hip-Hop", C::vocal, "Punchy, in-your-face rap: fast attack and release (~8 dB FET) with some grit, opto ~3 dB.",
      true, 2.0f, 2.5f, 0.15f, 60.f, 0, true, 60.2f, 4.0f, 0, 0, 110.f, 45.f, 100.f, 0.0f, 100.f, 25.f, 2 },
    { "Vocal: Ad-libs & Hype", C::vocal, "Shouted ad-libs and hype tracks: 8:1 FET (~11 dB), thin low end in the detector, extra grit.",
      true, 4.5f, 2.0f, 0.1f, 70.f, 1, true, 66.3f, 5.5f, 0, 0, 160.f, 55.f, 100.f, 0.0f, 100.f, 40.f, 2 },
    { "Vocal: Sibilance-Aware Leveling", C::vocal, "Opto with strong HF emphasis (~4 dB): reacts more to harsh S/T sounds, gentle FET (~3 dB).",
      true, -4.5f, 6.0f, 0.5f, 200.f, 0, true, 58.0f, 4.8f, 0, 0, 90.f, 15.f, 100.f, 0.0f, 100.f, 80.f, 2 },
    { "Vocal: Transparent Peak Control", C::vocal, "Clean 4:1 FET only (~6 dB), slow attack keeps consonants natural, no colour, opto off.",
      true, -0.5f, 5.0f, 0.8f, 200.f, 0, false, 0.0f, 0.0f, 0, 0, 90.f, 0.f, 100.f, 0.0f, 100.f, 0.f, 2 },
    { "Vocal: Sung Hook", C::vocal, "Melodic hooks/choruses: smooth opto (~5 dB) does most of it, light FET (~4 dB), slower release.",
      true, -3.0f, 5.5f, 0.5f, 300.f, 0, true, 65.4f, 6.0f, 0, 0, 90.f, 25.f, 100.f, 0.0f, 100.f, 10.f, 2 },
    { "Vocal: Lo-fi Grit (All)", C::vocal, "Dirty all-buttons crunch (~15 dB) with full colour, 70 % wet, detector ignores lows.",
      true, 7.5f, -2.0f, 0.2f, 100.f, 4, true, 71.4f, 1.0f, 0, 0, 200.f, 100.f, 70.f, 4.0f, 100.f, 0.f, 2 },
    { "Vocal: Broadcast Voice", C::vocal, "Radio/voice-over density: 8:1 FET (~6 dB) into opto Limit mode (~5 dB), very consistent level.",
      true, -1.5f, 5.0f, 0.3f, 150.f, 1, true, 66.6f, 6.0f, 1, 0, 100.f, 10.f, 100.f, 0.0f, 100.f, 10.f, 2 },
    // mix bus / master
    { "Mix Bus: Glue", C::mixMaster, "Gentle bus glue: slow-attack 4:1 FET (~2 dB) plus light opto (~1.5 dB); SC HPF stops kick pumping.",
      true, -7.5f, 8.5f, 0.8f, 300.f, 0, true, 45.1f, 1.5f, 0, 0, 120.f, 10.f, 100.f, 0.0f, 100.f, 0.f, 2 },
    { "Mix Bus: Punch", C::mixMaster, "Slow attack, quick release FET (~4 dB): lets drum transients through and brings up the body.",
      true, -5.5f, 7.0f, 0.8f, 100.f, 0, false, 0.0f, 0.0f, 0, 0, 90.f, 20.f, 100.f, 0.0f, 100.f, 0.f, 2 },
    { "Master: Transparent Leveling", C::mixMaster, "Mastering: opto only (~2 dB average), no colour, 4x oversampling.",
      false, 0.0f, 0.0f, 0.2f, 150.f, 0, true, 43.8f, 2.5f, 0, 0, 60.f, 0.f, 100.f, 0.0f, 100.f, 0.f, 2 },
    { "Master: Peak Catch", C::mixMaster, "Mastering: fast 20:1 FET only shaves the highest peaks (~2 dB), no colour, 4x oversampling.",
      true, -9.5f, 10.0f, 0.05f, 80.f, 3, false, 0.0f, 0.0f, 0, 0, 60.f, 0.f, 100.f, 0.0f, 100.f, 0.f, 2 },
    { "Master: Parallel Density", C::mixMaster, "Mastering parallel: 8:1 FET (~8 dB) blended 25 % for density without squashing transients.",
      true, -2.5f, 0.0f, 0.2f, 120.f, 1, false, 0.0f, 0.0f, 0, 0, 80.f, 20.f, 25.f, 1.0f, 100.f, 0.f, 2 },
    // 2.0: limiter, Clean detector, lookahead, Mid/Side, external side-chain (their 2.0 settings: kExtras below)
    { "Master: True-Peak Safety (-1 dBTP)", C::mixMaster, "Light opto leveling (~2 dB) into the true-peak limiter: nothing passes -1 dBTP, ready for streaming.",
      false, 0.0f, 0.0f, 0.2f, 150.f, 0, true, 44.7f, 2.0f, 0, 0, 60.f, 0.f, 100.f, 0.0f, 100.f, 0.f, 2 },
    { "Bass & 808: Clean Control", C::mixMaster, "Clean-detector FET (~8 dB peaks) with 1 ms lookahead: tight low end, no added bass distortion.",
      true, -2.0f, 6.0f, 0.1f, 120.f, 0, false, 0.0f, 0.0f, 0, 0, 20.f, 0.f, 100.f, 0.0f, 100.f, 0.f, 2 },
    { "Mix Bus: Mid/Side Glue", C::mixMaster, "Mid/Side: the centre (kick, bass, vocal) and the sides are leveled separately (~2-3 dB), image stays steady.",
      true, -6.0f, 7.5f, 0.8f, 250.f, 0, true, 45.6f, 1.5f, 0, 0, 100.f, 10.f, 100.f, 0.0f, 0.f, 0.f, 2 },
    { "Vocal: Lookahead Peak Control", C::vocal, "Clean 8:1 FET with 2 ms lookahead catches every consonant (~8 dB, capped at 9 dB), opto levels (~3 dB).",
      true, 0.5f, 4.5f, 0.3f, 120.f, 1, true, 58.2f, 3.5f, 0, 0, 100.f, 15.f, 100.f, 0.0f, 100.f, 0.f, 2 },
    { "Utility: Duck to Side-Chain", C::utility, "Route a beat or vocal to the side-chain input: the FET ducks this track under it (8:1, 30 ms hold).",
      true, 6.0f, 0.0f, 0.1f, 150.f, 1, false, 0.0f, 0.0f, 0, 0, 60.f, 0.f, 100.f, 0.0f, 100.f, 0.f, 2 },
    { "Drum Bus: Boom Bap Smash", C::mixMaster, "Beat/drum bus: all-buttons smash (~15 dB) at 40 %, attack lets the kick/snare crack through.",
      true, 3.5f, -2.0f, 0.3f, 60.f, 4, false, 0.0f, 0.0f, 0, 0, 60.f, 60.f, 40.f, 2.0f, 100.f, 0.f, 2 },
};
// clang-format on

// 2.0 settings of the presets that use them (every other 2.0 parameter stays at its default = 1.x sound)
struct Extra
{
    const char* name;
    void (*apply) (Params&);
};
constexpr Extra kExtras[] = {
    { "Master: True-Peak Safety (-1 dBTP)", [] (Params& p) { p.limOn = true; p.limCeilDb = -1.0f; p.limRelMs = 80.0f; p.limLookMs = 1.5f; } },
    { "Bass & 808: Clean Control", [] (Params& p) { p.fetClean = true; p.lookaheadMs = 1.0f; p.fetKneeDb = 6.0f; } },
    { "Mix Bus: Mid/Side Glue", [] (Params& p) { p.midSide = true; } },
    { "Vocal: Lookahead Peak Control", [] (Params& p) { p.fetClean = true; p.lookaheadMs = 2.0f; p.fetRangeDb = 9.0f; } },
    { "Utility: Duck to Side-Chain", [] (Params& p) { p.fetExt = true; p.fetHoldMs = 30.0f; p.fetThrDb = -24.0f; p.scHpfHz = 40.0f; } },
};
} // namespace preset_detail

constexpr int kNumFactoryPresets = (int) (sizeof (preset_detail::kRows) / sizeof (preset_detail::kRows[0]));

inline std::array<FactoryPreset, kNumFactoryPresets> factoryPresets()
{
    std::array<FactoryPreset, kNumFactoryPresets> list {};
    for (int i = 0; i < kNumFactoryPresets; ++i)
    {
        const auto& r = preset_detail::kRows[i];
        Params p;
        p.fetOn = r.fetOn;   p.fetInDb = r.fetIn;   p.fetOutDb = r.fetOut;
        p.fetAtkMs = r.atkMs; p.fetRelMs = r.relMs; p.fetRatio = r.ratio;
        p.optoOn = r.optoOn; p.optoPR = r.pr;       p.optoGainDb = r.optoGain; p.optoMode = r.mode;
        p.order = r.order;   p.scHpfHz = r.hpf;     p.colorPct = r.color;
        p.mixPct = r.mix;    p.outDb = r.out;       p.linkPct = r.link;
        p.emphasisPct = r.emph; p.osLog2 = r.os;
        p.optoMeasured = true; // every factory preset uses the measured opto law (PR retuned for the same opto GR)
        // 3.0 VOICE HOLD: on for the vocal presets and Init (the new-instance default), off for beats, buses, masters
        // and ducking - the voice network takes drum hits for a voice and would hold their release
        p.voiceHold = r.cat == PresetCategory::vocal || std::string_view (r.name) == "Init";
        // 4.0 ESS HOLD: the same rule - the sibilance network fires on hi-hats now and then (0.2-4 % of a beat's time)
        p.essHold = p.voiceHold;
        for (const auto& x : preset_detail::kExtras)
            if (std::string_view (x.name) == r.name)
                x.apply (p);
        list[(size_t) i] = { r.name, r.cat, r.desc, p };
    }
    return list;
}
} // namespace duo
