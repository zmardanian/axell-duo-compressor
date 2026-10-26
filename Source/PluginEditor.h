#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

namespace ui
{
// design canvas; the editor scales it 1.0x .. 2.0x with a vector transform (sharp at any scale)
constexpr int kWidth = 1200;
constexpr int kHeight = 860;

juce::Font font (float height, int weight = 0); // 0 regular, 1 semibold, 2 bold

namespace col
{
// 3.0: this plugin's own ground - the surfaces take its hue (15 deg); text, meters and accents stay the family's
inline juce::Colour themed (float sat, float val) { return juce::Colour::fromHSV (15.0f / 360.0f, sat, val, 1.0f); }
const juce::Colour bg       = themed (0.62f, 0.115f);
const juce::Colour header   = themed (0.58f, 0.135f);
const juce::Colour panel    = themed (0.46f, 0.165f);
const juce::Colour panelTop = themed (0.42f, 0.195f);
const juce::Colour well     = themed (0.66f, 0.072f);
const juce::Colour track    = themed (0.30f, 0.255f);
const juce::Colour grid     = themed (0.38f, 0.215f);
const juce::Colour text     (0xffe7e9ee);
const juce::Colour dim      (0xff8d939e);
const juce::Colour faint    (0xff5b616c);
const juce::Colour fet      (0xfff08a3c);
const juce::Colour opto     (0xff33c1b5);
const juce::Colour lim      (0xffd9607a);
const juce::Colour sc       (0xff8f8ae6);
const juce::Colour global   (0xffc3c8d2);
const juce::Colour ok       (0xff58c27d);
const juce::Colour warn     (0xffe6c24a);
const juce::Colour clip     (0xffe5533d);
} // namespace col
} // namespace ui

//==============================================================================
class DuoLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DuoLookAndFeel();
    juce::Typeface::Ptr getTypefaceForFont (const juce::Font&) override;
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos, float startAngle,
                           float endAngle, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool over, bool down) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool over, bool down) override;
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool over, bool down) override;
    juce::Font getLabelFont (juce::Label&) override;
    juce::Font getPopupMenuFont() override;
    juce::Font getAlertWindowMessageFont() override;
    juce::Font getAlertWindowTitleFont() override;
    void drawLabel (juce::Graphics&, juce::Label&) override;
    void drawTooltip (juce::Graphics&, const juce::String& text, int w, int h) override;
    juce::Rectangle<int> getTooltipBounds (const juce::String& tipText, juce::Point<int> screenPos,
                                           juce::Rectangle<int> parentArea) override;
};

//==============================================================================
class DuoKnob : public juce::Component
{
public:
    enum class Size { compact, normal, large };
    DuoKnob (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, const juce::String& name,
             juce::Colour accent, const juce::String& tooltip, bool large = false);
    DuoKnob (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, const juce::String& name,
             juce::Colour accent, const juce::String& tooltip, Size size);
    void resized() override;
    juce::Slider slider;

private:
    juce::Label label;
    Size size;
    int maxDiameter;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

// N mutually exclusive buttons bound to a choice/bool parameter
class Segmented : public juce::Component
{
public:
    Segmented (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId,
               const juce::StringArray& labels, juce::Colour accent, const juce::String& tooltip);
    void resized() override;

private:
    juce::OwnedArray<juce::TextButton> buttons;
    std::unique_ptr<juce::ParameterAttachment> attachment;
};

// toggle drawn as an LED + caption ("power") or as a pill ("pill")
class DuoToggle : public juce::ToggleButton
{
public:
    DuoToggle (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, const juce::String& caption,
               juce::Colour accent, const juce::String& style, const juce::String& tooltip);

private:
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;
};

// preset name button with a drop-down chevron
class MenuButton : public juce::TextButton
{
public:
    void paintButton (juce::Graphics&, bool over, bool down) override;
};

class LevelMeter : public juce::Component, public juce::SettableTooltipClient
{
public:
    LevelMeter (juce::String caption, bool scaleOnLeft);
    void push (float peakL, float peakR, int numChannels);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;

private:
    juce::String caption;
    bool scaleLeft;
    float level[2] { -100.0f, -100.0f }, hold[2] { -100.0f, -100.0f }, maxHold = -100.0f;
    int holdFrames[2] {};
    int channels = 2;
};

class GrMeter : public juce::Component, public juce::SettableTooltipClient
{
public:
    explicit GrMeter (juce::Colour accent);
    void push (float grDb, bool active);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override { peak = 0.0f; repaint(); }

private:
    juce::Colour accent;
    float value = 0.0f, peak = 0.0f;
    int peakFrames = 0;
    bool active = true;
};

// scrolling history (~5 s): input and output level (dBFS, bottom-up), gain reduction of each stage (top-down)
class GrGraph : public juce::Component, public juce::SettableTooltipClient
{
public:
    GrGraph();
    void push (const duo::HistoryFrame& f);
    void paint (juce::Graphics&) override;

private:
    std::vector<duo::HistoryFrame> ring;
    int pos = 0;
};

// static input -> output curve of the whole chain at the current settings (steady 1 kHz sine), with the live level
class TransferCurve : public juce::Component, public juce::SettableTooltipClient
{
public:
    TransferCurve();
    void setParams (const duo::Params& p);
    void setLevel (float inDb, float outDb);
    void paint (juce::Graphics&) override;
    float outputFor (float inDb) const; // dBFS peak out for a sine of inDb peak in

private:
    duo::Params params;
    duo::FetStage fetCalc;
    duo::OptoStage optoCalc;
    float levelIn = -120.0f, levelOut = -120.0f;
};

//==============================================================================
class DuoCompEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit DuoCompEditor (DuoCompProcessor&);
    ~DuoCompEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    // used by the headless snapshot tool
    void refreshNow() { timerCallback(); }

private:
    class Content : public juce::Component
    {
    public:
        explicit Content (DuoCompEditor& e) : owner (e) {}
        void paint (juce::Graphics&) override;
        void resized() override;
        DuoCompEditor& owner;
    };

    void timerCallback() override;
    void layout();
    void showPresetMenu();
    void stepPreset (int delta);
    void promptSavePreset();
    void setUiScale (float scale);
    void updatePanelStates();

    DuoCompProcessor& proc;
    DuoLookAndFeel lnf;
    Content content { *this };
    juce::TooltipWindow tooltips { this, 700 };

    // header
    juce::TextButton presetPrev { "<" }, presetNext { ">" }, presetSave { "SAVE" };
    MenuButton presetName;
    juce::TextButton slotA { "A" }, slotB { "B" }, slotCopy { "COPY" };

    // FET
    DuoToggle fetOn;
    Segmented fetRatio, fetDet, fetKey;
    DuoKnob fetIn, fetOut, fetAtk, fetRel, fetThr, fetKnee, fetRange, fetHold;
    GrMeter fetMeter { ui::col::fet };

    // Opto
    DuoToggle optoOn;
    Segmented optoMode, optoModel, optoKey;
    DuoKnob optoPR, optoGain, optoEmph, optoThr, optoKnee, optoRange, optoHold;
    GrMeter optoMeter { ui::col::opto };

    // Limiter
    DuoToggle limOn;
    DuoKnob limCeil, limRel, limLook;
    GrMeter limMeter { ui::col::lim };

    // side-chain
    DuoToggle scListen;
    DuoKnob scHpf, scLpf, scBellF, scBellG, scBellQ;

    // global
    Segmented order, oversampling, chMode;
    DuoToggle autoGain, voiceHold;
    DuoKnob look, color, link, mix, out;

    LevelMeter inMeter { "IN", true }, outMeter { "OUT", false };
    GrGraph graph;
    TransferCurve transfer;
    DuoToggle compare;

    juce::Rectangle<int> headerArea, fetArea, optoArea, limArea, scArea, globalArea, displayArea;
    juce::Rectangle<int> orderLabel, osLabel, chLabel, makeupArea, fetDetLabel, fetKeyLabel, optoModelLabel, optoKeyLabel, scStatusArea, limInfoArea;
    juce::String latencyText, makeupText, scStatusText;
    float lastMakeup = 0.0f;
    bool constructed = false;
    std::unique_ptr<juce::AlertWindow> saveDialog; // owned so it can never outlive the editor / its look-and-feel

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DuoCompEditor)
};
