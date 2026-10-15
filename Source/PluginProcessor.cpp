#include "PluginProcessor.h"
#include "PluginEditor.h"

using APVTS = juce::AudioProcessorValueTreeState;

namespace
{
juce::AudioParameterFloatAttributes fmt (std::function<juce::String (float)> toText,
                                         std::function<float (const juce::String&)> fromText = nullptr)
{
    auto a = juce::AudioParameterFloatAttributes()
                 .withStringFromValueFunction ([toText] (float v, int) { return toText (v); });
    if (fromText == nullptr)
        fromText = [] (const juce::String& t) { return t.getFloatValue(); };
    return a.withValueFromStringFunction (fromText);
}

juce::String dbText (float v)  { return juce::String (v, 1) + " dB"; }
juce::String msText (float v)  { return v < 10.0f ? juce::String (v, 1) + " ms" : juce::String (juce::roundToInt (v)) + " ms"; }
juce::String hzText (float v)  { return v >= 1000.0f ? juce::String (v / 1000.0f, 2) + " kHz" : juce::String (juce::roundToInt (v)) + " Hz"; }
float hzFromText (const juce::String& t)
{
    const auto v = t.getFloatValue();
    return t.containsIgnoreCase ("k") ? v * 1000.0f : v;
}
juce::String pctText (float v) { return juce::String (juce::roundToInt (v)) + " %"; }

const juce::Identifier kPresetNameId ("presetName");
} // namespace

APVTS::ParameterLayout DuoCompProcessor::createLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;

    //== v1 parameters: IDs, ranges and ORDER are frozen (saved projects / automation) ==
    // FET
    p.push_back (std::make_unique<AudioParameterBool> (ParameterID { "fetOn", 1 }, "FET On", true));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "fetIn", 1 }, "FET Input",
        NormalisableRange<float> (-12.0f, 30.0f, 0.1f), 6.0f, fmt (dbText)));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "fetOut", 1 }, "FET Output",
        NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f, fmt (dbText)));

    NormalisableRange<float> atk (0.02f, 0.8f, 0.001f);
    atk.setSkewForCentre (0.2f);
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "fetAtk", 1 }, "FET Attack", atk, 0.2f,
        fmt ([] (float v) { return String (roundToInt (v * 1000.0f)) + String::fromUTF8 (" \xc2\xb5s"); },
             [] (const String& t) { return t.getFloatValue() * 0.001f; })));

    NormalisableRange<float> rel (50.0f, 1100.0f, 1.0f);
    rel.setSkewForCentre (250.0f);
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "fetRel", 1 }, "FET Release", rel, 150.0f,
        fmt ([] (float v) { return String (roundToInt (v)) + " ms"; })));

    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "fetRatio", 1 }, "FET Ratio",
        StringArray { "4:1", "8:1", "12:1", "20:1", "All" }, 0));

    // Opto
    p.push_back (std::make_unique<AudioParameterBool> (ParameterID { "optoOn", 1 }, "Opto On", true));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "optoPR", 1 }, "Opto Peak Reduction",
        NormalisableRange<float> (0.0f, 100.0f, 0.1f), 30.0f, // frozen 1.x default: a restored 1.x value of 30 stays exact
        fmt ([] (float v) { return String (v, 1); })));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "optoGain", 1 }, "Opto Gain",
        NormalisableRange<float> (-12.0f, 24.0f, 0.1f), 0.0f, fmt (dbText)));
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "optoMode", 1 }, "Opto Mode",
        StringArray { "Compress", "Limit" }, 0));

    // Global
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "order", 1 }, "Order",
        StringArray { "FET > Opto", "Opto > FET" }, 0));

    NormalisableRange<float> hpf (20.0f, 300.0f, 1.0f);
    hpf.setSkewForCentre (80.0f);
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "scHpf", 1 }, "Sidechain HPF", hpf, 80.0f,
        fmt ([] (float v) { return String (roundToInt (v)) + " Hz"; })));

    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "color", 1 }, "Color",
        NormalisableRange<float> (0.0f, 100.0f, 1.0f), 30.0f, fmt (pctText)));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "mix", 1 }, "Mix",
        NormalisableRange<float> (0.0f, 100.0f, 1.0f), 100.0f, fmt (pctText)));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "out", 1 }, "Output",
        NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f, fmt (dbText)));

    //== v2 parameters (appended so existing parameter indices never move) ====
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "link", 2 }, "Stereo Link",
        NormalisableRange<float> (0.0f, 100.0f, 1.0f), 100.0f, fmt (pctText)));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "optoEmph", 2 }, "Opto HF Emphasis",
        NormalisableRange<float> (0.0f, 100.0f, 1.0f), 0.0f, fmt (pctText)));
    // default 4x since 2.0 (aliasing <= -150 dBFS at heavy drive); saved sessions keep their own setting
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "os", 2 }, "Oversampling",
        StringArray { "1x", "2x", "4x" }, 2));
    p.push_back (std::make_unique<AudioParameterBool> (ParameterID { "scListen", 2 }, "SC Listen", false));
    p.push_back (std::make_unique<AudioParameterBool> (ParameterID { "autoGain", 2 }, "Auto Makeup", false));

    //== v3 parameters (Axell Duo Compressor 2.0), appended; defaults = the 1.x sound ==============
    auto knee = [] (const char* id, const char* name)
    {
        return std::make_unique<AudioParameterFloat> (ParameterID { id, 3 }, name, NormalisableRange<float> (-1.0f, 24.0f, 0.1f), -1.0f,
            fmt ([] (float v) { return v < 0.0f ? String ("Auto") : String (v, 1) + " dB"; },
                 [] (const String& t) { return t.trim().startsWithIgnoreCase ("a") ? -1.0f : t.getFloatValue(); }));
    };
    auto range = [] (const char* id, const char* name)
    {
        return std::make_unique<AudioParameterFloat> (ParameterID { id, 3 }, name, NormalisableRange<float> (0.0f, 60.0f, 0.1f), 60.0f,
            fmt ([] (float v) { return v >= 60.0f ? String ("Off") : String (v, 1) + " dB"; },
                 [] (const String& t) { return t.trim().startsWithIgnoreCase ("o") ? 60.0f : t.getFloatValue(); }));
    };
    auto hold = [] (const char* id, const char* name)
    {
        NormalisableRange<float> r (0.0f, 500.0f, 0.1f);
        r.setSkewForCentre (50.0f);
        return std::make_unique<AudioParameterFloat> (ParameterID { id, 3 }, name, r, 0.0f, fmt (msText));
    };
    auto keySrc = [] (const char* id, const char* name)
    {
        return std::make_unique<AudioParameterChoice> (ParameterID { id, 3 }, name, StringArray { "Internal", "External" }, 0);
    };
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "fetThr", 3 }, "FET Threshold",
        NormalisableRange<float> (-40.0f, 0.0f, 0.1f), -12.0f, fmt (dbText)));
    p.push_back (knee ("fetKnee", "FET Knee"));
    p.push_back (range ("fetRange", "FET Range"));
    p.push_back (hold ("fetHold", "FET Hold"));
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "fetDet", 3 }, "FET Detector",
        StringArray { "Vintage", "Clean" }, 0));
    p.push_back (keySrc ("fetKey", "FET Key"));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "optoThr", 3 }, "Opto Threshold",
        NormalisableRange<float> (-40.0f, 0.0f, 0.1f), -10.0f, fmt (dbText)));
    p.push_back (knee ("optoKnee", "Opto Knee"));
    p.push_back (range ("optoRange", "Opto Range"));
    p.push_back (hold ("optoHold", "Opto Hold"));
    p.push_back (keySrc ("optoKey", "Opto Key"));

    NormalisableRange<float> lpf (200.0f, 20000.0f, 1.0f);
    lpf.setSkewForCentre (3000.0f);
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "scLpf", 3 }, "Sidechain LPF", lpf, 20000.0f,
        fmt ([] (float v) { return v >= 19999.0f ? String ("Off") : hzText (v); },
             [] (const String& t) { return t.trim().startsWithIgnoreCase ("o") ? 20000.0f : hzFromText (t); })));
    NormalisableRange<float> bellF (20.0f, 20000.0f, 1.0f);
    bellF.setSkewForCentre (1000.0f);
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "scBellF", 3 }, "SC EQ Freq", bellF, 3000.0f,
        fmt (hzText, hzFromText)));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "scBellG", 3 }, "SC EQ Gain",
        NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f, fmt (dbText)));
    NormalisableRange<float> bellQ (0.1f, 10.0f, 0.01f);
    bellQ.setSkewForCentre (1.0f);
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "scBellQ", 3 }, "SC EQ Q", bellQ, 1.0f,
        fmt ([] (float v) { return String (v, 2); })));

    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "look", 3 }, "Lookahead",
        NormalisableRange<float> (0.0f, 10.0f, 0.01f), 0.0f, fmt (msText)));

    p.push_back (std::make_unique<AudioParameterBool> (ParameterID { "limOn", 3 }, "Limiter On", false));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "limCeil", 3 }, "Limiter Ceiling",
        NormalisableRange<float> (-24.0f, 0.0f, 0.1f), -1.0f,
        fmt ([] (float v) { return String (v, 1) + " dBTP"; })));
    NormalisableRange<float> limRel (1.0f, 1000.0f, 0.1f);
    limRel.setSkewForCentre (50.0f);
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "limRel", 3 }, "Limiter Release", limRel, 50.0f, fmt (msText)));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { "limLook", 3 }, "Limiter Lookahead",
        NormalisableRange<float> (0.5f, 5.0f, 0.01f), 1.5f, fmt (msText)));

    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "chMode", 3 }, "Channel Mode",
        StringArray { "Stereo", "Mid/Side" }, 0));
    // the opto law: Classic (1.x) or Measured (calibrated on hardware recordings). New instances: Measured;
    // states saved before 2.0 load as Classic (upgradeLegacyState), so they sound as they did
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { "optoModel", 3 }, "Opto Model",
        StringArray { "Classic", "Measured" }, 1));
    // monitoring: hear the unprocessed signal at the processed signal's loudness (like SC Listen, not part of presets)
    p.push_back (std::make_unique<AudioParameterBool> (ParameterID { "compare", 3 }, "Level-Matched Compare", false));

    return { p.begin(), p.end() };
}

DuoCompProcessor::DuoCompProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                          .withInput ("Sidechain", juce::AudioChannelSet::stereo(), false)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
    pFetOn    = apvts.getRawParameterValue ("fetOn");
    pFetIn    = apvts.getRawParameterValue ("fetIn");
    pFetOut   = apvts.getRawParameterValue ("fetOut");
    pFetAtk   = apvts.getRawParameterValue ("fetAtk");
    pFetRel   = apvts.getRawParameterValue ("fetRel");
    pFetRatio = apvts.getRawParameterValue ("fetRatio");
    pOptoOn   = apvts.getRawParameterValue ("optoOn");
    pOptoPR   = apvts.getRawParameterValue ("optoPR");
    pOptoGain = apvts.getRawParameterValue ("optoGain");
    pOptoMode = apvts.getRawParameterValue ("optoMode");
    pOrder    = apvts.getRawParameterValue ("order");
    pScHpf    = apvts.getRawParameterValue ("scHpf");
    pColor    = apvts.getRawParameterValue ("color");
    pMix      = apvts.getRawParameterValue ("mix");
    pOut      = apvts.getRawParameterValue ("out");
    pLink     = apvts.getRawParameterValue ("link");
    pEmph     = apvts.getRawParameterValue ("optoEmph");
    pOs       = apvts.getRawParameterValue ("os");
    pListen   = apvts.getRawParameterValue ("scListen");
    pAuto     = apvts.getRawParameterValue ("autoGain");
    for (size_t i = 0; i < v3Ids.size(); ++i)
        pV3[i] = apvts.getRawParameterValue (v3Ids[i]);

    // the oversampling factor changes on the audio thread (after a 3 ms fade); the new
    // latency is reported to the host from the message thread
    startTimerHz (20);
}

DuoCompProcessor::~DuoCompProcessor()
{
    stopTimer();
}

bool DuoCompProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;
    if (out != layouts.getMainInputChannelSet())
        return false;
    // optional external side-chain: off, mono or stereo
    if (layouts.inputBuses.size() > 1)
    {
        const auto sc = layouts.getChannelSet (true, 1);
        if (! sc.isDisabled() && sc != juce::AudioChannelSet::mono() && sc != juce::AudioChannelSet::stereo())
            return false;
    }
    return true;
}

duo::Params DuoCompProcessor::readParams() const
{
    duo::Params p;
    p.fetOn      = pFetOn->load() > 0.5f;
    p.fetInDb    = pFetIn->load();
    p.fetOutDb   = pFetOut->load();
    p.fetAtkMs   = pFetAtk->load();
    p.fetRelMs   = pFetRel->load();
    p.fetRatio   = juce::roundToInt (pFetRatio->load());
    p.optoOn     = pOptoOn->load() > 0.5f;
    p.optoPR     = pOptoPR->load();
    p.optoGainDb = pOptoGain->load();
    p.optoMode   = juce::roundToInt (pOptoMode->load());
    p.order      = juce::roundToInt (pOrder->load());
    p.scHpfHz    = pScHpf->load();
    p.colorPct   = pColor->load();
    p.mixPct     = pMix->load();
    p.outDb      = pOut->load();
    p.linkPct    = pLink->load();
    p.emphasisPct = pEmph->load();
    p.osLog2     = juce::roundToInt (pOs->load());
    p.scListen   = pListen->load() > 0.5f;
    p.autoMakeup = pAuto->load() > 0.5f;
    auto v = [this] (V3 i) { return pV3[(size_t) i]->load(); };
    p.fetThrDb    = v (V3::fetThr);
    p.fetKneeDb   = v (V3::fetKnee);
    p.fetRangeDb  = v (V3::fetRange);
    p.fetHoldMs   = v (V3::fetHold);
    p.fetClean    = juce::roundToInt (v (V3::fetDet)) == 1;
    p.fetExt      = juce::roundToInt (v (V3::fetKey)) == 1;
    p.optoThrDb   = v (V3::optoThr);
    p.optoKneeDb  = v (V3::optoKnee);
    p.optoRangeDb = v (V3::optoRange);
    p.optoHoldMs  = v (V3::optoHold);
    p.optoExt     = juce::roundToInt (v (V3::optoKey)) == 1;
    p.scLpfHz     = v (V3::scLpf);
    p.scBellHz    = v (V3::scBellF);
    p.scBellDb    = v (V3::scBellG);
    p.scBellQ     = v (V3::scBellQ);
    p.lookaheadMs = v (V3::look);
    p.limOn       = v (V3::limOn) > 0.5f;
    p.limCeilDb   = v (V3::limCeil);
    p.limRelMs    = v (V3::limRel);
    p.limLookMs   = v (V3::limLook);
    p.midSide     = juce::roundToInt (v (V3::chMode)) == 1;
    p.optoMeasured = juce::roundToInt (v (V3::optoModel)) == 1;
    p.compare     = v (V3::compare) > 0.5f;
    return p;
}

void DuoCompProcessor::syncEngineParams (bool bypass)
{
    auto p = readParams();
    p.bypass = bypass;
    dsp.setParams (p);
}

void DuoCompProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    preparedChannels = juce::jlimit (1, duo::kMaxCh, getMainBusNumOutputChannels());
    syncEngineParams (false);
    dsp.prepare (sampleRate, preparedChannels, samplesPerBlock);
    dsp.setNonRealtime (isNonRealtime());
    dsp.reset();
    setLatencySamples (dsp.getLatencySamples());
    dsp.setHostLatency (getLatencySamples());
}

void DuoCompProcessor::setNonRealtime (bool isNonRealtime) noexcept
{
    AudioProcessor::setNonRealtime (isNonRealtime);
    dsp.setNonRealtime (isNonRealtime);
}

void DuoCompProcessor::reset()
{
    if (preparedChannels > 0)
        dsp.reset();
}

juce::String DuoCompProcessor::getPresetName() const
{
    const juce::ScopedLock sl (nameLock);
    return presetName;
}

void DuoCompProcessor::setPresetName (const juce::String& n)
{
    const juce::ScopedLock sl (nameLock);
    presetName = n;
}

bool DuoCompProcessor::isPresetModified() const
{
    const auto a = readParams();
    const auto& b = presetReference;
    auto diff = [] (float x, float y) { return std::abs (x - y) > 1.0e-3f; };
    return a.fetOn != b.fetOn || diff (a.fetInDb, b.fetInDb) || diff (a.fetOutDb, b.fetOutDb)
        || diff (a.fetAtkMs, b.fetAtkMs) || diff (a.fetRelMs, b.fetRelMs) || a.fetRatio != b.fetRatio
        || a.optoOn != b.optoOn || diff (a.optoPR, b.optoPR) || diff (a.optoGainDb, b.optoGainDb)
        || a.optoMode != b.optoMode || a.order != b.order || diff (a.scHpfHz, b.scHpfHz)
        || diff (a.colorPct, b.colorPct) || diff (a.mixPct, b.mixPct) || diff (a.outDb, b.outDb)
        || diff (a.linkPct, b.linkPct) || diff (a.emphasisPct, b.emphasisPct) || a.osLog2 != b.osLog2
        || a.scListen != b.scListen || a.autoMakeup != b.autoMakeup
        || diff (a.fetThrDb, b.fetThrDb) || diff (a.fetKneeDb, b.fetKneeDb) || diff (a.fetRangeDb, b.fetRangeDb)
        || diff (a.fetHoldMs, b.fetHoldMs) || a.fetClean != b.fetClean || a.fetExt != b.fetExt
        || diff (a.optoThrDb, b.optoThrDb) || diff (a.optoKneeDb, b.optoKneeDb) || diff (a.optoRangeDb, b.optoRangeDb)
        || diff (a.optoHoldMs, b.optoHoldMs) || a.optoExt != b.optoExt
        || diff (a.scLpfHz, b.scLpfHz) || diff (a.scBellHz, b.scBellHz) || diff (a.scBellDb, b.scBellDb)
        || diff (a.scBellQ, b.scBellQ) || diff (a.lookaheadMs, b.lookaheadMs) || a.limOn != b.limOn
        || diff (a.limCeilDb, b.limCeilDb) || diff (a.limRelMs, b.limRelMs) || diff (a.limLookMs, b.limLookMs)
        || a.midSide != b.midSide || a.optoMeasured != b.optoMeasured;
}

void DuoCompProcessor::markPresetReference()
{
    presetReference = readParams();
}

void DuoCompProcessor::timerCallback()
{
    if (programNamePending.exchange (false))
    {
        setPresetName (getProgramName (currentProgram.load()));
        presetReference = duo::factoryPresets()[(size_t) currentProgram.load()].params;
    }
    // latency handshake: a new latency plays only after it was reported to the host (Engine::setHostLatency)
    dsp.heartbeat();
    int lat = 0;
    if (preparedChannels > 0 && dsp.takeLatencyRequest (lat))
    {
        if (lat != getLatencySamples())
            setLatencySamples (lat);
        dsp.ackLatencyRequest();
    }
}

void DuoCompProcessor::storeMax (std::atomic<float>& a, float v) noexcept
{
    float cur = a.load (std::memory_order_relaxed);
    while (v > cur && ! a.compare_exchange_weak (cur, v, std::memory_order_relaxed))
    {
    }
}

void DuoCompProcessor::publishMeters()
{
    const auto& m = dsp.getMeters();
    for (int c = 0; c < 2; ++c)
    {
        storeMax (meters.inPeak[c], m.inPeak[c]);
        storeMax (meters.outPeak[c], m.outPeak[c]);
    }
    storeMax (meters.fetGr, m.fetGrDb);
    storeMax (meters.optoGr, m.optoGrDb);
    storeMax (meters.limGr, m.limGrDb);
    meters.makeup.store (m.makeupDb, std::memory_order_relaxed);
    dsp.clearMeters();
}

void DuoCompProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numIn = getTotalNumInputChannels();
    const int numOut = getTotalNumOutputChannels();
    const int n = buffer.getNumSamples();
    for (int c = numIn; c < numOut; ++c)
        buffer.clear (c, 0, n);
    if (preparedChannels == 0 || n == 0)
        return;

    syncEngineParams (false);
    runEngine (buffer);
}

void DuoCompProcessor::runEngine (juce::AudioBuffer<float>& buffer)
{
    auto main = getBusBuffer (buffer, true, 0);
    const float* key[duo::kMaxCh] {};
    int keyCh = 0;
    if (getBusCount (true) > 1)
        if (auto* bus = getBus (true, 1); bus != nullptr && bus->isEnabled())
        {
            auto sc = getBusBuffer (buffer, true, 1);
            keyCh = juce::jmin (sc.getNumChannels(), duo::kMaxCh);
            for (int c = 0; c < keyCh; ++c)
                key[c] = sc.getReadPointer (c);
        }
    dsp.process (main.getArrayOfWritePointers(), juce::jmin (main.getNumChannels(), preparedChannels), main.getNumSamples(),
                 keyCh > 0 ? key : nullptr, keyCh);
    publishMeters();
}

// host bypass: keep running the engine so the output stays latency-aligned and the switch
// is a 10 ms crossfade instead of a click
void DuoCompProcessor::processBlockBypassed (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    for (int c = getTotalNumInputChannels(); c < getTotalNumOutputChannels(); ++c)
        buffer.clear (c, 0, n);
    if (preparedChannels == 0 || n == 0)
        return;

    syncEngineParams (true);
    runEngine (buffer);
}

//==============================================================================
void DuoCompProcessor::applyParams (const duo::Params& p)
{
    auto set = [this] (const char* id, float value)
    {
        if (auto* param = apvts.getParameter (id))
            param->setValueNotifyingHost (param->convertTo0to1 (value));
    };
    set ("fetOn", p.fetOn ? 1.0f : 0.0f);
    set ("fetIn", p.fetInDb);
    set ("fetOut", p.fetOutDb);
    set ("fetAtk", p.fetAtkMs);
    set ("fetRel", p.fetRelMs);
    set ("fetRatio", (float) p.fetRatio);
    set ("optoOn", p.optoOn ? 1.0f : 0.0f);
    set ("optoPR", p.optoPR);
    set ("optoGain", p.optoGainDb);
    set ("optoMode", (float) p.optoMode);
    set ("order", (float) p.order);
    set ("scHpf", p.scHpfHz);
    set ("color", p.colorPct);
    set ("mix", p.mixPct);
    set ("out", p.outDb);
    set ("link", p.linkPct);
    set ("optoEmph", p.emphasisPct);
    set ("os", (float) p.osLog2);
    set ("scListen", p.scListen ? 1.0f : 0.0f);
    set ("autoGain", p.autoMakeup ? 1.0f : 0.0f);
    set ("fetThr", p.fetThrDb);
    set ("fetKnee", p.fetKneeDb);
    set ("fetRange", p.fetRangeDb);
    set ("fetHold", p.fetHoldMs);
    set ("fetDet", p.fetClean ? 1.0f : 0.0f);
    set ("fetKey", p.fetExt ? 1.0f : 0.0f);
    set ("optoThr", p.optoThrDb);
    set ("optoKnee", p.optoKneeDb);
    set ("optoRange", p.optoRangeDb);
    set ("optoHold", p.optoHoldMs);
    set ("optoKey", p.optoExt ? 1.0f : 0.0f);
    set ("scLpf", p.scLpfHz);
    set ("scBellF", p.scBellHz);
    set ("scBellG", p.scBellDb);
    set ("scBellQ", p.scBellQ);
    set ("look", p.lookaheadMs);
    set ("limOn", p.limOn ? 1.0f : 0.0f);
    set ("limCeil", p.limCeilDb);
    set ("limRel", p.limRelMs);
    set ("limLook", p.limLookMs);
    set ("chMode", p.midSide ? 1.0f : 0.0f);
    set ("optoModel", p.optoMeasured ? 1.0f : 0.0f);
}

int DuoCompProcessor::getNumPrograms()
{
    return (int) duo::factoryPresets().size();
}

void DuoCompProcessor::setCurrentProgram (int index)
{
    const auto list = duo::factoryPresets();
    if (index < 0 || index >= (int) list.size())
        return;
    currentProgram.store (index);
    applyParams (list[(size_t) index].params);
    if (juce::MessageManager::existsAndIsCurrentThread())
    {
        setPresetName (list[(size_t) index].name);
        presetReference = list[(size_t) index].params;
    }
    else
        programNamePending.store (true); // no String allocation on the audio thread
}

const juce::String DuoCompProcessor::getProgramName (int index)
{
    const auto list = duo::factoryPresets();
    if (index < 0 || index >= (int) list.size())
        return {};
    return list[(size_t) index].name;
}

juce::File DuoCompProcessor::userPresetFolder()
{
    auto base = juce::SystemStats::getEnvironmentVariable ("XDG_DATA_HOME", {});
    juce::File root = base.isNotEmpty() ? juce::File (base)
                                        : juce::File::getSpecialLocation (juce::File::userHomeDirectory)
                                              .getChildFile (".local/share");
    return root.getChildFile ("AxellDuoCompressor").getChildFile ("Presets");
}

juce::File DuoCompProcessor::legacyUserPresetFolder()
{
    return userPresetFolder().getParentDirectory().getParentDirectory().getChildFile ("DuoVocalComp").getChildFile ("Presets");
}

bool DuoCompProcessor::saveUserPreset (const juce::String& name)
{
    const auto clean = juce::File::createLegalFileName (name.trim());
    if (clean.isEmpty())
        return false;
    auto folder = userPresetFolder();
    if (! folder.createDirectory())
        return false;
    auto state = apvts.copyState();
    state.setProperty (kPresetNameId, name.trim(), nullptr);
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    if (xml == nullptr || ! xml->writeTo (folder.getChildFile (clean + ".duopreset")))
        return false;
    setPresetName (name.trim());
    markPresetReference();
    return true;
}

bool DuoCompProcessor::loadUserPreset (const juce::File& file)
{
    auto xml = juce::XmlDocument::parse (file);
    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType()))
        return false;
    auto tree = juce::ValueTree::fromXml (*xml);
    upgradeLegacyState (tree);
    restoreTree (tree);
    setPresetName (tree.getProperty (kPresetNameId, file.getFileNameWithoutExtension()).toString());
    markPresetReference();
    return true;
}

void DuoCompProcessor::selectSlot (int slot)
{
    slot = juce::jlimit (0, 1, slot);
    if (slot == activeSlot)
        return;
    slots[activeSlot] = apvts.copyState();
    if (slots[slot].isValid())
        restoreTree (slots[slot].createCopy());
    else
        slots[slot] = apvts.copyState();
    activeSlot = slot;
}

void DuoCompProcessor::copyActiveToOther()
{
    slots[1 - activeSlot] = apvts.copyState();
}

// A state saved before a parameter existed gets the value that version played with, not today's default:
// the oversampling default became 4x in 2.0 (a state without "os" played at 2x), and the opto default became the
// Measured law (a state without "optoModel" played with the Classic one). Every other 2.0 parameter's default is
// the 1.x behaviour.
void DuoCompProcessor::upgradeLegacyState (juce::ValueTree& tree)
{
    auto add = [&tree] (const char* id, float value)
    {
        if (tree.getChildWithProperty ("id", id).isValid())
            return;
        juce::ValueTree p ("PARAM");
        p.setProperty ("id", id, nullptr);
        p.setProperty ("value", value, nullptr);
        tree.appendChild (p, nullptr);
    };
    add ("os", 1.0f);
    add ("optoModel", 0.0f);
}

// APVTS::replaceState skips a parameter whose *snapped* value already matches the tree, so a
// raw value written by a host (e.g. 0.64 on a bool) could survive a restore (found by pluginval's
// state-restoration test). Re-apply every stored value explicitly after replacing the state.
void DuoCompProcessor::restoreTree (const juce::ValueTree& tree)
{
    apvts.replaceState (tree);
    for (auto* p : getParameters())
        if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p))
        {
            const auto child = apvts.state.getChildWithProperty ("id", rp->paramID);
            if (child.isValid() && child.hasProperty ("value"))
            {
                const float norm = rp->convertTo0to1 ((float) child.getProperty ("value"));
                if (std::abs (rp->getValue() - norm) > 1.0e-6f)
                    rp->setValueNotifyingHost (norm);
            }
        }
}

//==============================================================================
void DuoCompProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty (kPresetNameId, getPresetName(), nullptr);
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void DuoCompProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
    {
        auto tree = juce::ValueTree::fromXml (*xml);
        setPresetName (tree.getProperty (kPresetNameId, getPresetName()).toString());
        upgradeLegacyState (tree);
        restoreTree (tree);
        markPresetReference(); // a restored session counts as 'unmodified' until you change something
    }
}

juce::AudioProcessorEditor* DuoCompProcessor::createEditor()
{
    return new DuoCompEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DuoCompProcessor();
}
