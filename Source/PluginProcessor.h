#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "Engine.h"
#include "Presets.h"

// Thin JUCE wrapper around duo::Engine (all DSP lives in the JUCE-free headers).
class DuoCompProcessor : public juce::AudioProcessor, private juce::Timer
{
public:
    DuoCompProcessor();
    ~DuoCompProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void setNonRealtime (bool isNonRealtime) noexcept override;
    void releaseResources() override {}
    void reset() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlockBypassed (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;
    using AudioProcessor::processBlockBypassed;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    // factory presets as host programs (REAPER shows them in its preset menu too)
    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram.load(); }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    juce::AudioProcessorValueTreeState apvts;

    //== presets / A-B (message thread only) ==================================
    void applyParams (const duo::Params& p);            // sets every parameter, notifying the host
    duo::Params readParams() const;                     // current parameter values
    static juce::File userPresetFolder();
    static juce::File legacyUserPresetFolder(); // where Duo Vocal Comp 1.x saved user presets (read only)
    bool saveUserPreset (const juce::String& name);
    bool loadUserPreset (const juce::File& file);
    // message thread (and host state calls); never touched by the audio thread
    juce::String getPresetName() const;
    void setPresetName (const juce::String& n);
    // true when the current settings no longer match the last loaded preset (shown as "*")
    bool isPresetModified() const;
    void markPresetReference();                         // current settings become the reference

    int getActiveSlot() const { return activeSlot; }
    void selectSlot (int slot);                          // A/B compare: stores current, recalls other
    void copyActiveToOther();

    //== metering (audio thread writes max, GUI exchanges with 0) =============
    struct MeterValues
    {
        std::atomic<float> inPeak[2] {}, outPeak[2] {};
        std::atomic<float> fetGr { 0.0f }, optoGr { 0.0f }, makeup { 0.0f }, limGr { 0.0f };
    };
    MeterValues meters;

    duo::Engine& engine() { return dsp; }

private:
    void timerCallback() override;
    void restoreTree (const juce::ValueTree& tree);
    static void upgradeLegacyState (juce::ValueTree& tree);
    void syncEngineParams (bool bypass);
    void publishMeters();
    void runEngine (juce::AudioBuffer<float>& buffer);
    static void storeMax (std::atomic<float>& a, float v) noexcept;

    duo::Engine dsp;
    int preparedChannels = 0;
    // VST3 hosts may change the program from the audio thread: only atomics are touched there,
    // the display name is resolved on the message thread by the timer
    std::atomic<int> currentProgram { 0 };
    std::atomic<bool> programNamePending { false };
    juce::CriticalSection nameLock;
    juce::String presetName { "Init" };
    duo::Params presetReference;                        // message thread only
    int activeSlot = 0;
    juce::ValueTree slots[2];

    std::atomic<float>* pFetOn = nullptr;   std::atomic<float>* pFetIn = nullptr;
    std::atomic<float>* pFetOut = nullptr;  std::atomic<float>* pFetAtk = nullptr;
    std::atomic<float>* pFetRel = nullptr;  std::atomic<float>* pFetRatio = nullptr;
    std::atomic<float>* pOptoOn = nullptr;  std::atomic<float>* pOptoPR = nullptr;
    std::atomic<float>* pOptoGain = nullptr; std::atomic<float>* pOptoMode = nullptr;
    std::atomic<float>* pOrder = nullptr;   std::atomic<float>* pScHpf = nullptr;
    std::atomic<float>* pColor = nullptr;   std::atomic<float>* pMix = nullptr;
    std::atomic<float>* pOut = nullptr;
    std::atomic<float>* pLink = nullptr;    std::atomic<float>* pEmph = nullptr;
    std::atomic<float>* pOs = nullptr;      std::atomic<float>* pListen = nullptr;
    std::atomic<float>* pAuto = nullptr;
    std::atomic<float>* pVoiceHold = nullptr;
    // v3 parameters
    enum class V3 { fetThr, fetKnee, fetRange, fetHold, fetDet, fetKey, optoThr, optoKnee, optoRange, optoHold, optoKey,
                    scLpf, scBellF, scBellG, scBellQ, look, limOn, limCeil, limRel, limLook, chMode, optoModel, compare, count };
    static constexpr std::array<const char*, (size_t) V3::count> v3Ids {
        "fetThr", "fetKnee", "fetRange", "fetHold", "fetDet", "fetKey", "optoThr", "optoKnee", "optoRange", "optoHold", "optoKey",
        "scLpf", "scBellF", "scBellG", "scBellQ", "look", "limOn", "limCeil", "limRel", "limLook", "chMode", "optoModel", "compare" };
    std::array<std::atomic<float>*, (size_t) V3::count> pV3 {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DuoCompProcessor)
};
