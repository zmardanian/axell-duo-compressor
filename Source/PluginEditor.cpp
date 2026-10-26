#include "PluginEditor.h"

#include <axell/gui/BrandTitle.h>
#include "BinaryData.h"

#include <cstring>
#include <tuple>

using namespace ui;

namespace
{
juce::Typeface::Ptr typeface (int weight)
{
    static juce::Typeface::Ptr regular = juce::Typeface::createSystemTypefaceFor (BinaryData::InterRegular_otf,
                                                                                    BinaryData::InterRegular_otfSize);
    static juce::Typeface::Ptr semi = juce::Typeface::createSystemTypefaceFor (BinaryData::InterSemiBold_otf,
                                                                                 BinaryData::InterSemiBold_otfSize);
    static juce::Typeface::Ptr bold = juce::Typeface::createSystemTypefaceFor (BinaryData::InterBold_otf,
                                                                                 BinaryData::InterBold_otfSize);
    return weight >= 2 ? bold : (weight == 1 ? semi : regular);
}

juce::Font caps (float h, int weight = 1)
{
    auto f = ui::font (h, weight);
    f.setExtraKerningFactor (0.08f);
    return f;
}

const juce::Identifier kUiWidth ("uiWidth");

// Tooltips are written for a rap vocalist recording at home.
namespace tip
{
const char* fetOn = "Turns the FET peak compressor on/off. Switching is click-free.";
const char* ratio = "How hard the FET clamps peaks. 4:1 = natural lead vocal. 8:1 / 12:1 = tighter doubles and ad-libs. "
                    "20:1 = peak limiting. ALL = 'all buttons in': aggressive, crunchy, pumping - great for parallel smash.";
const char* fetIn = "Drives your vocal into the FET's fixed threshold: more INPUT = more compression. "
                    "Aim for 3-7 dB on the FET meter on your loudest syllables.";
const char* fetOut = "Makes up the level after the FET. Set it so switching the FET off doesn't change the loudness.";
const char* fetAtk = "How fast the FET grabs a peak (20-800 us). Fast tames hard consonants and plosive spikes; "
                     "slower (400 us+) lets the bite of K, T, P through for more punch.";
const char* fetRel = "How fast the FET lets go (50-1100 ms). Fast (50-120 ms) = energetic, louder rap delivery. "
                     "Slow (300 ms+) = smoother, less pumping between words. Long heavy compression slows it a bit, like the hardware.";
const char* optoOn = "Turns the opto leveler on/off. Switching is click-free.";
const char* optoMode = "COMPRESS (~3:1, soft) evens out the whole performance. LIMIT (~10:1) holds the level tighter "
                       "for shouted or very dynamic takes.";
const char* optoPR = "How much the opto levels your performance. Turn it up until the opto meter shows 2-5 dB on the louder lines. "
                     "It grabs in ~10 ms and lets go slowly: half in ~60 ms, the rest over 0.5-5 s (longer after long, hard compression).";
const char* optoGain = "Makes up the level after the opto.";
const char* optoEmph = "Makes the opto react more to bright, harsh sounds (S, T, SH) - tames sibilance and harsh lines "
                       "without a de-esser. 0 % = flat. Only changes what the opto hears, not your EQ.";
const char* order = "FET > OPTO: catch the peaks first, then level (the classic chain). "
                    "OPTO > FET: level first, the FET only catches what is left - smoother.";
const char* os = "Oversampling. 4x (default) keeps drive and colour free of aliasing (below -140 dBFS). 2x saves CPU, "
                 "1x gives the lowest CPU and latency while tracking (some aliasing when driven hard).";
const char* fetDet = "FET detector. VINTAGE behaves like the hardware: fast settings add a little low-end distortion on bass "
                     "and 808s (part of the FET sound). CLEAN listens with a ripple-free detector: same attack and curve, "
                     "no added bass distortion - use it on bass, 808s, full mixes.";
const char* fetKey = "What the FET listens to. INT = the track itself. EXT = the plugin's side-chain input (route a beat, "
                     "kick or another vocal there): ducking and keyed compression.";
const char* fetThr = "Where the FET starts working (default -12 dB = the classic fixed threshold; INPUT still drives into it). "
                     "Lower = compresses quieter parts too.";
const char* fetKnee = "How gently the FET goes into compression. AUTO = the classic ratio-dependent knee (softer at 4:1, harder at 20:1). "
                      "0 dB = hard knee, 12+ dB = very smooth.";
const char* fetRange = "The most gain reduction the FET may apply. OFF = unlimited. 3-6 dB keeps a heavy setting from flattening the vocal.";
const char* fetHold = "Holds the FET's gain reduction this long before the release starts. Smooths pumping on sustained notes and "
                      "between hits. 0 = classic behaviour.";
const char* optoModel = "MEASURED = the opto law calibrated on recordings of real leveling-amplifier hardware (how hard PEAK "
                        "REDUCTION drives it, its ratio, knee and light-cell timing). CLASSIC = the Duo Vocal Comp 1.x law "
                        "(projects from 1.x load with it).";
const char* optoKey = "What the opto listens to. INT = the track (after the FET in FET > OPTO). EXT = the side-chain input.";
const char* optoThr = "Where the opto starts leveling (default -10 dB). PEAK REDUCTION still sets how hard it drives into it.";
const char* optoKnee = "How gently the opto goes into compression. AUTO = the classic soft knee (softer in COMPRESS than in LIMIT).";
const char* optoRange = "The most gain reduction the opto may apply. OFF = unlimited.";
const char* optoHold = "Holds the opto's gain reduction before it starts to release. 0 = classic behaviour.";
const char* limOn = "True-peak limiter at the very end: nothing passes the CEILING - not even the peaks between samples "
                    "that appear after conversion or MP3/streaming encoding. Adds its lookahead to the latency.";
const char* limCeil = "The highest level the output may reach, in dB true peak. -1 dBTP is the usual streaming target.";
const char* limRel = "How fast the limiter recovers. Short = louder but can distort bass; long = cleaner, less loud.";
const char* limLook = "How far ahead the limiter looks (0.5-5 ms). Longer = smoother, cleaner limiting of fast transients "
                      "(more latency).";
const char* scLpf = "Side-chain low-pass: stops cymbals, hi-hats and air from triggering the compressors. OFF = full range.";
const char* scBellF = "Side-chain EQ frequency: boost to make the compressors react more to that range (e.g. 5-8 kHz for "
                      "sibilance), cut to make them ignore it.";
const char* scBellG = "Side-chain EQ gain. Only changes what the compressors hear, never your sound.";
const char* scBellQ = "Side-chain EQ width. Low = broad, high = narrow.";
const char* chMode = "STEREO = left/right. MID-SIDE = the compressors work on the centre (vocal, kick, bass) and the sides "
                     "(wide synths, reverb) separately - set LINK to 0 % for fully independent M and S.";
const char* look = "Compressor lookahead (0-10 ms): both stages see transients before they arrive and catch them cleanly "
                   "(no overshoot). 0 = classic behaviour. Adds latency (compensated by the DAW).";
const char* listen = "Hear what the compressors listen to (the side-chain after its filters and EMPHASIS; the external input "
                     "when a stage uses EXT). Use it to set the filters, then turn it off!";
const char* voiceHold = "VOICE HOLD: in the pauses of the voice (the suite's shared voice detector, Axell Gate's) the gain "
                        "reduction holds at the level of your voice instead of releasing - so the compressor no longer lifts "
                        "the room, the breaths and the headphone bleed between phrases (measured: +23 dB of lift down to "
                        "+5 dB). Your voice is compressed exactly as before.";
const char* autoGain = "Slowly (2 s) adds back the average gain reduction so your vocal keeps its loudness while you tweak. "
                       "Off = set the makeup yourself.";
const char* scHpf = "Stops low end (plosives, 808 bleed, mic bumps) from triggering both compressors. "
                    "80-150 Hz is typical for vocals. It does not EQ your sound.";
const char* color = "Analog-style harmonics. The FET adds 2nd + 3rd harmonic grit that grows with compression; "
                    "the opto adds warm 2nd harmonic. 0 % = completely clean.";
const char* link = "Stereo link. 100 % = both sides compress together (keeps the image of stereo doubles). "
                   "0 % = each side on its own.";
const char* mix = "Parallel compression: blend the compressed vocal with the dry one. Keeps natural transients while adding "
                  "density. Perfectly time-aligned, no phasing.";
const char* out = "Final output level after everything.";
const char* inMeter = "Input level (peak dBFS). The number is the highest peak - click to reset.";
const char* outMeter = "Output level (peak dBFS). The number is the highest peak - click to reset.";
const char* fetMeter = "FET gain reduction in dB. The line marks the recent peak - click to reset.";
const char* compare = "COMPARE plays your unprocessed track at the same loudness as the processed one (measured like a "
                      "loudness meter). Louder always sounds better - this removes that trick, so you hear only what the "
                      "compression does to tone and punch. Turn it off to hear the processing again.";
const char* graph = "The last 5 seconds: input level (grey area), output level (white line) and the gain reduction of the "
                    "FET (orange), opto (teal) and limiter (pink), drawn from the top.";
const char* transfer = "Input level (across) against output level (up) for a steady tone with the current settings: where "
                       "the curve bends is where compression starts; flat at the top is the limiter's ceiling. The dot is "
                       "your signal right now.";
const char* limMeter = "Limiter gain reduction in dB. The line marks the recent peak - click to reset.";
const char* optoMeter = "Opto gain reduction in dB. The line marks the recent peak - click to reset.";
const char* presets = "Presets: rap/vocal and mix-bus/master starting points, all level-matched. "
                      "Save your own; they live in ~/.local/share/AxellDuoCompressor/Presets.";
const char* ab = "A/B compare: click A or B to switch between two complete settings.";
const char* copy = "Copy the current slot's settings into the other slot.";
} // namespace tip
} // namespace

juce::Font ui::font (float height, int weight)
{
    return juce::Font (juce::FontOptions (typeface (weight)).withHeight (height));
}

//==============================================================================
DuoLookAndFeel::DuoLookAndFeel()
{
    using namespace juce;
    setColour (ResizableWindow::backgroundColourId, col::bg);
    setColour (PopupMenu::backgroundColourId, Colour (0xff1e2128));
    setColour (PopupMenu::textColourId, col::text);
    setColour (PopupMenu::headerTextColourId, col::dim);
    setColour (PopupMenu::highlightedBackgroundColourId, Colour (0xff343944));
    setColour (PopupMenu::highlightedTextColourId, col::text);
    setColour (Slider::textBoxTextColourId, col::text);
    setColour (Slider::textBoxOutlineColourId, Colours::transparentBlack);
    setColour (Slider::textBoxBackgroundColourId, col::well);
    setColour (Slider::textBoxHighlightColourId, col::fet.withAlpha (0.4f));
    setColour (Label::textColourId, col::text);
    setColour (TextEditor::backgroundColourId, col::well);
    setColour (TextEditor::textColourId, col::text);
    setColour (TextEditor::highlightColourId, col::fet.withAlpha (0.4f));
    setColour (TextEditor::outlineColourId, col::track);
    setColour (TextEditor::focusedOutlineColourId, col::fet);
    setColour (CaretComponent::caretColourId, col::text);
    setColour (TooltipWindow::backgroundColourId, Colour (0xf0262a32));
    setColour (TooltipWindow::textColourId, col::text);
    setColour (TooltipWindow::outlineColourId, col::track);
    setColour (AlertWindow::backgroundColourId, Colour (0xff1e2128));
    setColour (AlertWindow::textColourId, col::text);
    setColour (AlertWindow::outlineColourId, col::track);
    setColour (TextButton::buttonColourId, col::track);
    setColour (TextButton::textColourOffId, col::text);
}

juce::Typeface::Ptr DuoLookAndFeel::getTypefaceForFont (const juce::Font& f)
{
    const auto style = f.getTypefaceStyle();
    if (f.isBold() || style.containsIgnoreCase ("bold"))
        return typeface (2);
    if (style.containsIgnoreCase ("semi") || style.containsIgnoreCase ("medium"))
        return typeface (1);
    return typeface (0);
}

void DuoLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float startAngle,
                                       float endAngle, juce::Slider& s)
{
    using namespace juce;
    const auto bounds = Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (2.0f);
    const float size = jmin (bounds.getWidth(), bounds.getHeight());
    const auto centre = bounds.getCentre();
    const float r = size * 0.5f;
    const float track = jmax (3.0f, r * 0.085f);
    const float angle = startAngle + pos * (endAngle - startAngle);
    const auto accent = s.findColour (Slider::rotarySliderFillColourId);
    const bool enabled = s.isEnabled();
    const PathStrokeType stroke (track, PathStrokeType::curved, PathStrokeType::rounded);
    const float arcR = r - track * 0.5f;

    Path bg;
    bg.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    g.setColour (col::track);
    g.strokePath (bg, stroke);

    // bipolar parameters (min < 0 < max) draw their arc from the centre
    const auto range = s.getRange();
    float from = startAngle;
    if (range.getStart() < 0.0 && range.getEnd() > 0.0)
        from = startAngle + (float) s.valueToProportionOfLength (0.0) * (endAngle - startAngle);
    Path val;
    val.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, jmin (from, angle), jmax (from, angle), true);
    g.setColour (enabled ? accent : accent.withSaturation (0.1f).withAlpha (0.5f));
    g.strokePath (val, stroke);

    // knob body
    const auto body = Rectangle<float> (size, size).withCentre (centre).reduced (track * 2.6f);
    g.setColour (Colours::black.withAlpha (0.45f));
    g.fillEllipse (body.translated (0.0f, size * 0.025f).expanded (1.0f));
    g.setGradientFill (ColourGradient (Colour (0xff3d424c), body.getX(), body.getY(),
                                       Colour (0xff1d2026), body.getRight(), body.getBottom(), false));
    g.fillEllipse (body);
    g.setColour (Colour (0xff4a505b));
    g.drawEllipse (body.reduced (0.5f), 1.0f);

    // pointer
    const float pr = body.getWidth() * 0.5f;
    Path pointer;
    const float pw = jmax (2.5f, pr * 0.09f);
    pointer.addRoundedRectangle (-pw * 0.5f, -pr + pr * 0.14f, pw, pr * 0.42f, pw * 0.5f);
    g.setColour (enabled ? accent.brighter (0.25f) : col::dim);
    g.fillPath (pointer, AffineTransform::rotation (angle).translated (centre.x, centre.y));
}

void DuoLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down)
{
    using namespace juce;
    const auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    const auto accent = b.findColour (TextButton::buttonOnColourId);
    const bool on = b.getToggleState();
    const float corner = 5.0f;
    if (on)
    {
        g.setColour (accent.withAlpha (b.isEnabled() ? 0.95f : 0.4f));
        g.fillRoundedRectangle (r, corner);
    }
    else
    {
        g.setColour (col::well.brighter (over ? 0.25f : 0.08f).withMultipliedAlpha (down ? 0.8f : 1.0f));
        g.fillRoundedRectangle (r, corner);
        g.setColour (col::track.brighter (over ? 0.3f : 0.0f));
        g.drawRoundedRectangle (r, corner, 1.0f);
    }
}

juce::Font DuoLookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return caps (juce::jmin (13.0f, buttonHeight * 0.48f), 1);
}

void DuoLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool)
{
    g.setFont (getTextButtonFont (b, b.getHeight()));
    const bool on = b.getToggleState();
    g.setColour (on ? juce::Colour (0xff121418) : (b.isEnabled() ? col::text : col::faint));
    g.drawFittedText (b.getButtonText(), b.getLocalBounds().reduced (4, 0), juce::Justification::centred, 1, 0.9f);
}

void DuoLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool over, bool)
{
    using namespace juce;
    const auto accent = b.findColour (ToggleButton::tickColourId);
    const bool on = b.getToggleState();
    const auto style = b.getProperties()["style"].toString();
    auto r = b.getLocalBounds().toFloat();

    if (style == "pill")
    {
        r = r.reduced (0.5f);
        g.setColour (on ? accent.withAlpha (0.18f) : col::well.brighter (over ? 0.2f : 0.06f));
        g.fillRoundedRectangle (r, r.getHeight() * 0.5f);
        g.setColour (on ? accent : col::track.brighter (over ? 0.3f : 0.0f));
        g.drawRoundedRectangle (r, r.getHeight() * 0.5f, 1.2f);
        const float d = r.getHeight() * 0.34f;
        auto led = Rectangle<float> (d, d).withCentre ({ r.getX() + r.getHeight() * 0.55f, r.getCentreY() });
        g.setColour (on ? accent : col::faint);
        g.fillEllipse (led);
        g.setColour (on ? col::text : col::dim);
        g.setFont (caps (12.0f, 1));
        g.drawText (b.getButtonText(), r.withTrimmedLeft (r.getHeight()).withTrimmedRight (r.getHeight() * 0.35f),
                    Justification::centred);
        return;
    }

    // power: LED + caption
    const float d = r.getHeight() * 0.5f;
    auto led = Rectangle<float> (d, d).withCentre ({ r.getX() + d * 0.8f, r.getCentreY() });
    if (on)
    {
        g.setColour (accent.withAlpha (0.25f));
        g.fillEllipse (led.expanded (d * 0.35f));
    }
    g.setColour (on ? accent : col::faint);
    g.fillEllipse (led);
    g.setColour (Colours::black.withAlpha (0.4f));
    g.drawEllipse (led, 1.0f);
    g.setColour (on ? col::text : col::dim);
    g.setFont (caps (r.getHeight() * 0.62f, 2));
    g.drawText (b.getButtonText(), r.withTrimmedLeft (d * 1.8f), Justification::centredLeft);
}

juce::Font DuoLookAndFeel::getLabelFont (juce::Label& l)
{
    return ui::font (juce::jmin (14.0f, (float) l.getHeight() * 0.72f), 0);
}

void DuoLookAndFeel::drawLabel (juce::Graphics& g, juce::Label& l)
{
    using namespace juce;
    const bool inSlider = dynamic_cast<Slider*> (l.getParentComponent()) != nullptr;
    if (inSlider)
    {
        g.setColour (col::well);
        g.fillRoundedRectangle (l.getLocalBounds().toFloat(), 4.0f);
    }
    else
    {
        g.fillAll (l.findColour (Label::backgroundColourId));
    }
    if (! l.isBeingEdited())
    {
        g.setColour (l.findColour (Label::textColourId).withMultipliedAlpha (l.isEnabled() ? 1.0f : 0.5f));
        g.setFont (inSlider ? ui::font (13.0f, 1) : l.getFont());
        g.drawFittedText (l.getText(), l.getLocalBounds().reduced (3, 0), l.getJustificationType(), 1, 0.9f);
    }
}

juce::Font DuoLookAndFeel::getPopupMenuFont() { return ui::font (15.0f, 0); }
juce::Font DuoLookAndFeel::getAlertWindowMessageFont() { return ui::font (15.0f, 0); }
juce::Font DuoLookAndFeel::getAlertWindowTitleFont() { return ui::font (17.0f, 2); }

void DuoLookAndFeel::drawTooltip (juce::Graphics& g, const juce::String& text, int w, int h)
{
    using namespace juce;
    const auto r = Rectangle<float> ((float) w, (float) h);
    g.setColour (findColour (TooltipWindow::backgroundColourId));
    g.fillRoundedRectangle (r, 6.0f);
    g.setColour (findColour (TooltipWindow::outlineColourId));
    g.drawRoundedRectangle (r.reduced (0.5f), 6.0f, 1.0f);
    AttributedString s;
    s.setJustification (Justification::topLeft);
    s.append (text, ui::font (14.0f), findColour (TooltipWindow::textColourId));
    TextLayout tl;
    tl.createLayoutWithBalancedLineLengths (s, (float) w - 20.0f);
    tl.draw (g, r.reduced (10.0f, 8.0f));
}

juce::Rectangle<int> DuoLookAndFeel::getTooltipBounds (const juce::String& tipText, juce::Point<int> screenPos,
                                                       juce::Rectangle<int> parentArea)
{
    using namespace juce;
    AttributedString s;
    s.append (tipText, ui::font (14.0f));
    TextLayout tl;
    tl.createLayoutWithBalancedLineLengths (s, 320.0f);
    const int w = (int) std::ceil (tl.getWidth()) + 22, h = (int) std::ceil (tl.getHeight()) + 18;
    return Rectangle<int> (screenPos.x > parentArea.getCentreX() ? screenPos.x - (w + 12) : screenPos.x + 24,
                           screenPos.y > parentArea.getCentreY() ? screenPos.y - (h + 6) : screenPos.y + 6, w, h)
        .constrainedWithin (parentArea);
}

//==============================================================================
DuoKnob::DuoKnob (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, const juce::String& name,
                  juce::Colour accent, const juce::String& tooltip, bool isLarge)
    : DuoKnob (apvts, paramId, name, accent, tooltip, isLarge ? Size::large : Size::normal)
{
}

DuoKnob::DuoKnob (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, const juce::String& name,
                  juce::Colour accent, const juce::String& tooltip, Size sz)
    : size (sz), maxDiameter (sz == Size::large ? 132 : sz == Size::normal ? 72 : 44)
{
    const bool large = size == Size::large, compact = size == Size::compact;
    using namespace juce;
    slider.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    slider.setRotaryParameters (MathConstants<float>::pi * 1.22f, MathConstants<float>::pi * 2.78f, true);
    slider.setTextBoxStyle (Slider::TextBoxBelow, false, large ? 92 : compact ? 74 : 80, compact ? 18 : 20);
    slider.setColour (Slider::textBoxOutlineColourId, Colours::transparentBlack);
    slider.setColour (Slider::rotarySliderFillColourId, accent);
    slider.setMouseDragSensitivity (220);
    // Ctrl or Shift + drag = fine adjust (velocity mode, low sensitivity, no jump)
    slider.setVelocityModeParameters (0.35, 1, 0.0, true,
                                      (ModifierKeys::Flags) (ModifierKeys::ctrlModifier | ModifierKeys::shiftModifier));
    slider.setTooltip (tooltip);
    addAndMakeVisible (slider);

    label.setText (name, dontSendNotification);
    label.setJustificationType (Justification::centred);
    label.setFont (caps (large ? 13.0f : compact ? 10.5f : 12.0f, 1));
    label.setColour (Label::textColourId, col::dim);
    label.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (label);

    attachment = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (apvts, paramId, slider);
    if (auto* p = apvts.getParameter (paramId))
        slider.setDoubleClickReturnValue (true, p->convertFrom0to1 (p->getDefaultValue()));
}

void DuoKnob::resized()
{
    // label, knob (capped diameter, identical for every normal knob), value box; centred vertically
    const bool compact = size == Size::compact;
    const int labelH = compact ? 15 : 18, boxH = compact ? 21 : 24;
    auto b = getLocalBounds();
    const int d = juce::jmin (maxDiameter, b.getWidth() - 8, b.getHeight() - labelH - boxH - 4);
    const int total = labelH + 2 + d + boxH;
    b = b.withSizeKeepingCentre (b.getWidth(), total);
    label.setBounds (b.removeFromTop (labelH));
    b.removeFromTop (2);
    slider.setBounds (b.withSizeKeepingCentre (juce::jmax (d, size == Size::large ? 96 : compact ? 76 : 84), d + boxH));
}

//==============================================================================
Segmented::Segmented (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId,
                      const juce::StringArray& labels, juce::Colour accent, const juce::String& tooltip)
{
    using namespace juce;
    auto* param = apvts.getParameter (paramId);
    jassert (param != nullptr);
    for (int i = 0; i < labels.size(); ++i)
    {
        auto* b = buttons.add (new TextButton (labels[i]));
        b->setClickingTogglesState (false);
        b->setColour (TextButton::buttonOnColourId, accent);
        b->setTooltip (tooltip);
        b->onClick = [this, i] { if (attachment) attachment->setValueAsCompleteGesture ((float) i); };
        addAndMakeVisible (b);
    }
    attachment = std::make_unique<ParameterAttachment> (*param, [this] (float v)
    {
        const int idx = roundToInt (v);
        for (int i = 0; i < buttons.size(); ++i)
            buttons[i]->setToggleState (i == idx, dontSendNotification);
    });
    attachment->sendInitialUpdate();
}

void Segmented::resized()
{
    auto b = getLocalBounds();
    const int gap = 4;
    const int n = buttons.size();
    const int w = (b.getWidth() - gap * (n - 1)) / n;
    for (int i = 0; i < n; ++i)
    {
        buttons[i]->setBounds (i == n - 1 ? b : b.removeFromLeft (w));
        b.removeFromLeft (gap);
    }
}

//==============================================================================
DuoToggle::DuoToggle (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, const juce::String& caption,
                      juce::Colour accent, const juce::String& style, const juce::String& tooltip)
    : juce::ToggleButton (caption)
{
    setColour (juce::ToggleButton::tickColourId, accent);
    getProperties().set ("style", style);
    setTooltip (tooltip);
    attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, paramId, *this);
}

//==============================================================================
void MenuButton::paintButton (juce::Graphics& g, bool over, bool down)
{
    auto& lf = getLookAndFeel();
    lf.drawButtonBackground (g, *this, findColour (juce::TextButton::buttonColourId), over, down);
    g.setColour (col::text);
    g.setFont (ui::font (14.0f, 1));
    auto r = getLocalBounds().reduced (12, 0);
    auto arrowArea = r.removeFromRight (12).toFloat();
    g.drawFittedText (getButtonText(), r, juce::Justification::centred, 1, 0.85f);
    juce::Path p;
    const auto c = arrowArea.getCentre();
    p.addTriangle (c.x - 4.0f, c.y - 2.0f, c.x + 4.0f, c.y - 2.0f, c.x, c.y + 3.0f);
    g.setColour (col::dim);
    g.fillPath (p);
}

//==============================================================================
LevelMeter::LevelMeter (juce::String c, bool scaleOnLeft) : caption (std::move (c)), scaleLeft (scaleOnLeft) {}

void LevelMeter::push (float l, float r, int numChannels)
{
    channels = juce::jlimit (1, 2, numChannels);
    const float in[2] { juce::Decibels::gainToDecibels (l, -100.0f), juce::Decibels::gainToDecibels (r, -100.0f) };
    for (int c = 0; c < 2; ++c)
    {
        // instant rise, 24 dB/s fall at 30 fps; peak hold 1.5 s
        level[c] = in[c] > level[c] ? in[c] : juce::jmax (in[c], level[c] - 0.8f);
        if (in[c] >= hold[c])
        {
            hold[c] = in[c];
            holdFrames[c] = 45;
        }
        else if (--holdFrames[c] < 0)
            hold[c] = juce::jmax (level[c], hold[c] - 0.8f);
        maxHold = juce::jmax (maxHold, in[c]);
    }
    repaint();
}

void LevelMeter::mouseDown (const juce::MouseEvent&)
{
    maxHold = -100.0f;
    repaint();
}

void LevelMeter::paint (juce::Graphics& g)
{
    using namespace juce;
    auto b = getLocalBounds().toFloat();
    g.setColour (col::dim);
    g.setFont (caps (11.0f, 1));
    g.drawText (caption, b.removeFromTop (16.0f), Justification::centred);
    auto readout = b.removeFromBottom (20.0f);
    b.removeFromBottom (4.0f);

    auto gutter = scaleLeft ? b.removeFromLeft (16.0f) : b.removeFromRight (16.0f);
    g.setColour (col::well);
    g.fillRoundedRectangle (b, 4.0f);

    constexpr float lo = -48.0f, hi = 6.0f;
    auto yFor = [&] (float db) { return b.getBottom() - b.getHeight() * (jlimit (lo, hi, db) - lo) / (hi - lo); };

    const auto bars = b.reduced (3.0f);
    const float gap = 2.0f;
    const float bw = (bars.getWidth() - gap * (float) (channels - 1)) / (float) channels;
    for (int c = 0; c < channels; ++c)
    {
        const auto bar = Rectangle<float> (bars.getX() + (float) c * (bw + gap), bars.getY(), bw, bars.getHeight());
        const float top = yFor (level[c]);
        if (top < bar.getBottom())
        {
            ColourGradient grad (col::clip, 0.0f, yFor (0.0f), col::ok, 0.0f, yFor (-18.0f), false);
            grad.addColour (0.35, col::warn);
            g.setGradientFill (grad);
            g.fillRect (Rectangle<float>::leftTopRightBottom (bar.getX(), top, bar.getRight(), bar.getBottom()));
        }
        if (hold[c] > lo)
        {
            g.setColour (hold[c] > 0.0f ? col::clip : col::text.withAlpha (0.85f));
            g.fillRect (bar.getX(), yFor (hold[c]) - 1.0f, bar.getWidth(), 2.0f);
        }
    }

    // scale ticks + labels in the gutter
    g.setFont (ui::font (9.5f));
    for (float db : { 0.0f, -6.0f, -12.0f, -18.0f, -24.0f, -36.0f, -48.0f })
    {
        const float yy = yFor (db);
        g.setColour (db > -0.5f ? col::clip.withAlpha (0.6f) : Colour (0x30ffffff));
        if (db > -47.0f)
            g.drawHorizontalLine ((int) yy, b.getX(), b.getRight());
        g.setColour (col::faint);
        g.drawText (String ((int) std::abs (db)), gutter.withY (yy - 6.0f).withHeight (12.0f),
                    scaleLeft ? Justification::centredRight : Justification::centredLeft);
    }

    g.setColour (maxHold > 0.0f ? col::clip : col::text);
    g.setFont (ui::font (12.0f, 1));
    g.drawText (maxHold <= -99.0f ? String ("-inf") : String (maxHold, 1), readout, Justification::centred);
}

//==============================================================================
GrMeter::GrMeter (juce::Colour c) : accent (c) {}

void GrMeter::push (float grDb, bool isActive)
{
    active = isActive;
    const float v = juce::jmax (0.0f, grDb);
    value = v > value ? v : value + (v - value) * 0.3f;
    if (v >= peak)
    {
        peak = v;
        peakFrames = 45;
    }
    else if (--peakFrames < 0)
        peak = juce::jmax (value, peak - 0.25f);
    repaint();
}

void GrMeter::paint (juce::Graphics& g)
{
    using namespace juce;
    auto b = getLocalBounds().toFloat();
    g.setColour (col::dim);
    g.setFont (caps (11.0f, 1));
    g.drawText ("GR", b.removeFromTop (16.0f), Justification::centred);
    auto readout = b.removeFromBottom (20.0f);
    b.removeFromBottom (4.0f);

    g.setColour (col::well);
    g.fillRoundedRectangle (b, 4.0f);
    const auto bar = b.reduced (3.0f);
    constexpr float range = 20.0f;
    auto yFor = [&] (float db) { return bar.getY() + bar.getHeight() * jlimit (0.0f, 1.0f, db / range); };

    const auto c = active ? accent : col::faint;
    if (value > 0.01f)
    {
        g.setGradientFill (ColourGradient (c.withAlpha (0.95f), 0.0f, bar.getY(), c.withAlpha (0.55f), 0.0f, bar.getBottom(), false));
        g.fillRoundedRectangle (bar.withHeight (yFor (value) - bar.getY()), 2.0f);
    }
    if (peak > 0.05f)
    {
        g.setColour (c.brighter (0.4f));
        g.fillRect (bar.getX(), yFor (peak) - 1.0f, bar.getWidth(), 2.0f);
    }

    // tick marks + labels drawn over the bar so they stay readable
    g.setFont (ui::font (9.5f, 1));
    for (float db : { 3.0f, 6.0f, 10.0f, 15.0f })
    {
        const float yy = yFor (db);
        const bool covered = yy < yFor (value);
        g.setColour (covered ? Colour (0x40000000) : Colour (0x30ffffff));
        g.drawHorizontalLine ((int) yy, bar.getX(), bar.getRight());
        g.setColour (covered ? Colour (0xb0121418) : col::faint);
        g.drawText (String ((int) db), Rectangle<float> (bar.getX(), yy + 1.0f, bar.getWidth(), 12.0f), Justification::centred);
    }

    g.setColour (active ? col::text : col::faint);
    g.setFont (ui::font (12.0f, 1));
    g.drawText (value < 0.05f ? String ("0.0") : "-" + String (value, 1), readout, Justification::centred);
}

//==============================================================================
DuoCompEditor::DuoCompEditor (DuoCompProcessor& p)
    : AudioProcessorEditor (&p),
      proc (p),
      fetOn (p.apvts, "fetOn", "FET", col::fet, "power", tip::fetOn),
      fetRatio (p.apvts, "fetRatio", { "4", "8", "12", "20", "ALL" }, col::fet, tip::ratio),
      fetDet (p.apvts, "fetDet", { "VINTAGE", "CLEAN" }, col::fet, tip::fetDet),
      fetKey (p.apvts, "fetKey", { "INT", "EXT" }, col::sc, tip::fetKey),
      fetIn (p.apvts, "fetIn", "INPUT", col::fet, tip::fetIn),
      fetOut (p.apvts, "fetOut", "OUTPUT", col::fet, tip::fetOut),
      fetAtk (p.apvts, "fetAtk", "ATTACK", col::fet, tip::fetAtk),
      fetRel (p.apvts, "fetRel", "RELEASE", col::fet, tip::fetRel),
      fetThr (p.apvts, "fetThr", "THRESH", col::fet, tip::fetThr, DuoKnob::Size::compact),
      fetKnee (p.apvts, "fetKnee", "KNEE", col::fet, tip::fetKnee, DuoKnob::Size::compact),
      fetRange (p.apvts, "fetRange", "RANGE", col::fet, tip::fetRange, DuoKnob::Size::compact),
      fetHold (p.apvts, "fetHold", "HOLD", col::fet, tip::fetHold, DuoKnob::Size::compact),
      optoOn (p.apvts, "optoOn", "OPTO", col::opto, "power", tip::optoOn),
      optoMode (p.apvts, "optoMode", { "COMPRESS", "LIMIT" }, col::opto, tip::optoMode),
      optoModel (p.apvts, "optoModel", { "CLASSIC", "MEASURED" }, col::opto, tip::optoModel),
      optoKey (p.apvts, "optoKey", { "INT", "EXT" }, col::sc, tip::optoKey),
      optoPR (p.apvts, "optoPR", "PEAK REDUCTION", col::opto, tip::optoPR, true),
      optoGain (p.apvts, "optoGain", "GAIN", col::opto, tip::optoGain),
      optoEmph (p.apvts, "optoEmph", "EMPHASIS", col::opto, tip::optoEmph),
      optoThr (p.apvts, "optoThr", "THRESH", col::opto, tip::optoThr, DuoKnob::Size::compact),
      optoKnee (p.apvts, "optoKnee", "KNEE", col::opto, tip::optoKnee, DuoKnob::Size::compact),
      optoRange (p.apvts, "optoRange", "RANGE", col::opto, tip::optoRange, DuoKnob::Size::compact),
      optoHold (p.apvts, "optoHold", "HOLD", col::opto, tip::optoHold, DuoKnob::Size::compact),
      limOn (p.apvts, "limOn", "LIMITER", col::lim, "power", tip::limOn),
      limCeil (p.apvts, "limCeil", "CEILING", col::lim, tip::limCeil, true),
      limRel (p.apvts, "limRel", "RELEASE", col::lim, tip::limRel, DuoKnob::Size::compact),
      limLook (p.apvts, "limLook", "LOOKAHEAD", col::lim, tip::limLook, DuoKnob::Size::compact),
      scListen (p.apvts, "scListen", "SC LISTEN", col::warn, "pill", tip::listen),
      scHpf (p.apvts, "scHpf", "HPF", col::sc, tip::scHpf),
      scLpf (p.apvts, "scLpf", "LPF", col::sc, tip::scLpf),
      scBellF (p.apvts, "scBellF", "EQ FREQ", col::sc, tip::scBellF),
      scBellG (p.apvts, "scBellG", "EQ GAIN", col::sc, tip::scBellG),
      scBellQ (p.apvts, "scBellQ", "EQ Q", col::sc, tip::scBellQ),
      order (p.apvts, "order", { "FET > OPTO", "OPTO > FET" }, col::global, tip::order),
      oversampling (p.apvts, "os", { "1x", "2x", "4x" }, col::global, tip::os),
      chMode (p.apvts, "chMode", { "STEREO", "MID-SIDE" }, col::global, tip::chMode),
      autoGain (p.apvts, "autoGain", "AUTO MAKEUP", col::global, "pill", tip::autoGain),
      voiceHold (p.apvts, "voiceHold", "VOICE HOLD", col::global, "pill", tip::voiceHold),
      look (p.apvts, "look", "LOOKAHEAD", col::global, tip::look),
      color (p.apvts, "color", "COLOR", col::global, tip::color),
      link (p.apvts, "link", "LINK", col::global, tip::link),
      mix (p.apvts, "mix", "MIX", col::global, tip::mix),
      out (p.apvts, "out", "OUTPUT", col::global, tip::out),
      compare (p.apvts, "compare", "COMPARE", col::warn, "pill", tip::compare)
{
    // read before setResizeLimits(), which already triggers resized()
    const int savedWidth = (int) proc.apvts.state.getProperty (kUiWidth, kWidth);
    setLookAndFeel (&lnf);
    addAndMakeVisible (content);

    for (auto* c : std::initializer_list<juce::Component*> {
             &presetPrev, &presetNext, &presetName, &presetSave, &slotA, &slotB, &slotCopy,
             &fetOn, &fetRatio, &fetDet, &fetKey, &fetIn, &fetOut, &fetAtk, &fetRel, &fetThr, &fetKnee, &fetRange, &fetHold, &fetMeter,
             &optoOn, &optoMode, &optoModel, &optoKey, &optoPR, &optoGain, &optoEmph, &optoThr, &optoKnee, &optoRange, &optoHold, &optoMeter,
             &limOn, &limCeil, &limRel, &limLook, &limMeter,
             &scListen, &scHpf, &scLpf, &scBellF, &scBellG, &scBellQ,
             &order, &oversampling, &chMode, &autoGain, &voiceHold, &look, &color, &link, &mix, &out, &inMeter, &outMeter,
             &graph, &transfer, &compare })
        content.addAndMakeVisible (c);

    for (auto* b : { &slotA, &slotB })
        b->setColour (juce::TextButton::buttonOnColourId, col::global);
    presetName.setColour (juce::TextButton::buttonOnColourId, col::global);
    presetName.setTooltip (tip::presets);
    presetPrev.setTooltip ("Previous factory preset");
    presetNext.setTooltip ("Next factory preset");
    presetSave.setTooltip ("Save the current settings as a user preset");
    slotA.setTooltip (tip::ab);
    slotB.setTooltip (tip::ab);
    slotCopy.setTooltip (tip::copy);
    inMeter.setTooltip (tip::inMeter);
    outMeter.setTooltip (tip::outMeter);
    fetMeter.setTooltip (tip::fetMeter);
    optoMeter.setTooltip (tip::optoMeter);
    limMeter.setTooltip (tip::limMeter);
    graph.setTooltip (tip::graph);
    transfer.setTooltip (tip::transfer);

    presetPrev.onClick = [this] { stepPreset (-1); };
    presetNext.onClick = [this] { stepPreset (1); };
    presetName.onClick = [this] { showPresetMenu(); };
    presetSave.onClick = [this] { promptSavePreset(); };
    slotA.onClick = [this] { proc.selectSlot (0); timerCallback(); };
    slotB.onClick = [this] { proc.selectSlot (1); timerCallback(); };
    slotCopy.onClick = [this] { proc.copyActiveToOther(); };

    setResizable (true, true);
    setResizeLimits (kWidth, kHeight, kWidth * 2, kHeight * 2);
    if (auto* c = getConstrainer())
        c->setFixedAspectRatio ((double) kWidth / (double) kHeight);
    const int w = juce::jlimit (kWidth, kWidth * 2, savedWidth);
    setSize (w, juce::roundToInt (w * (double) kHeight / kWidth));
    constructed = true;

    timerCallback();
    startTimerHz (30);
}

DuoCompEditor::~DuoCompEditor()
{
    stopTimer();
    saveDialog.reset();
    setLookAndFeel (nullptr);
}

void DuoCompEditor::paint (juce::Graphics& g)
{
    g.fillAll (col::bg);
}

void DuoCompEditor::resized()
{
    const float scale = (float) getWidth() / (float) kWidth;
    content.setBounds (0, 0, kWidth, kHeight);
    content.setTransform (juce::AffineTransform::scale (scale));
    if (constructed)
        proc.apvts.state.setProperty (kUiWidth, getWidth(), nullptr);
}

void DuoCompEditor::setUiScale (float scale)
{
    const int w = juce::roundToInt (kWidth * juce::jlimit (1.0f, 2.0f, scale));
    setSize (w, juce::roundToInt (w * (double) kHeight / kWidth));
}

void DuoCompEditor::Content::resized() { owner.layout(); }

void DuoCompEditor::layout()
{
    using juce::Rectangle;
    const int m = 16;

    // header
    headerArea = { 0, 0, kWidth, 58 };
    {
        auto pb = Rectangle<int> (410, 14, 400, 30);
        presetPrev.setBounds (pb.removeFromLeft (30));
        pb.removeFromLeft (4);
        presetSave.setBounds (pb.removeFromRight (62));
        pb.removeFromRight (4);
        presetNext.setBounds (pb.removeFromRight (30));
        pb.removeFromRight (4);
        presetName.setBounds (pb);

        compare.setBounds (836, 16, 150, 26);

        auto ab = Rectangle<int> (kWidth - m - 160, 14, 160, 30);
        slotA.setBounds (ab.removeFromLeft (34));
        ab.removeFromLeft (4);
        slotB.setBounds (ab.removeFromLeft (34));
        ab.removeFromLeft (8);
        slotCopy.setBounds (ab);
    }

    // display strip: history graph + transfer curve
    displayArea = { m, 68, kWidth - 2 * m, 134 };
    {
        auto d = displayArea;
        transfer.setBounds (d.removeFromRight (180));
        d.removeFromRight (10);
        graph.setBounds (d);
    }

    // stage row: [IN] [FET] [OPTO] [LIMITER] [OUT]
    const int top = 212, rowH = 372;
    inMeter.setBounds (m, top, 44, rowH);
    outMeter.setBounds (kWidth - m - 44, top, 44, rowH);
    const int x0 = m + 44 + 10;
    fetArea = { x0, top, 404, rowH };
    optoArea = { fetArea.getRight() + 10, top, 404, rowH };
    limArea = Rectangle<int>::leftTopRightBottom (optoArea.getRight() + 10, top, kWidth - m - 44 - 10, top + rowH);

    auto stagePanel = [] (Rectangle<int> a, juce::Component& power, GrMeter& meter)
    {
        auto inner = a.reduced (14, 0);
        power.setBounds (inner.getX(), a.getY() + 10, 110, 26);
        meter.setBounds (inner.getRight() - 40, a.getY() + 50, 40, a.getHeight() - 62);
        return Rectangle<int>::leftTopRightBottom (inner.getX(), a.getY() + 50, inner.getRight() - 40 - 12, a.getBottom() - 8);
    };
    auto smallRow = [] (Rectangle<int> r, std::initializer_list<juce::Component*> cs)
    {
        const int w = r.getWidth() / (int) cs.size();
        for (auto* c : cs)
            c->setBounds (r.removeFromLeft (w));
    };

    {
        auto k = stagePanel (fetArea, fetOn, fetMeter);
        fetRatio.setBounds (k.removeFromTop (28));
        k.removeFromTop (8);
        auto sel = k.removeFromTop (40);
        auto detCol = sel.removeFromLeft (sel.getWidth() * 58 / 100);
        fetDetLabel = detCol.removeFromTop (14);
        fetDet.setBounds (detCol.removeFromTop (24).withTrimmedRight (10));
        fetKeyLabel = sel.removeFromTop (14);
        fetKey.setBounds (sel.removeFromTop (24));
        k.removeFromTop (4);
        auto mainRow = k.removeFromTop (k.getHeight() * 54 / 100);
        smallRow (mainRow, { &fetIn, &fetOut, &fetAtk, &fetRel });
        smallRow (k, { &fetThr, &fetKnee, &fetRange, &fetHold });
    }
    {
        auto k = stagePanel (optoArea, optoOn, optoMeter);
        optoMode.setBounds (k.removeFromTop (28));
        k.removeFromTop (8);
        auto sel = k.removeFromTop (40);
        auto modelCol = sel.removeFromLeft (sel.getWidth() * 58 / 100);
        optoModelLabel = modelCol.removeFromTop (14);
        optoModel.setBounds (modelCol.removeFromTop (24).withTrimmedRight (10));
        optoKeyLabel = sel.removeFromTop (14);
        optoKey.setBounds (sel.removeFromTop (24));
        k.removeFromTop (4);
        auto mainRow = k.removeFromTop (k.getHeight() * 60 / 100);
        auto big = mainRow.removeFromLeft (mainRow.getWidth() * 55 / 100);
        optoPR.setBounds (big);
        optoGain.setBounds (mainRow.removeFromTop (mainRow.getHeight() / 2));
        optoEmph.setBounds (mainRow);
        smallRow (k, { &optoThr, &optoKnee, &optoRange, &optoHold });
    }
    {
        auto k = stagePanel (limArea, limOn, limMeter);
        limCeil.setBounds (k.removeFromTop (k.getHeight() * 52 / 100));
        limInfoArea = k.removeFromTop (34);
        k.removeFromTop (4);
        smallRow (k, { &limRel, &limLook });
    }

    // bottom row: [SIDE-CHAIN] [GLOBAL]
    const int top2 = top + rowH + 12;
    scArea = { m, top2, 470, kHeight - top2 - m };
    globalArea = Rectangle<int>::leftTopRightBottom (scArea.getRight() + 10, top2, kWidth - m, kHeight - m);
    {
        auto a = scArea.reduced (14, 0);
        scListen.setBounds (a.getRight() - 118, a.getY() + 10, 118, 26);
        scStatusArea = { a.getX(), a.getY() + 38, a.getWidth(), 16 };
        auto k = Rectangle<int>::leftTopRightBottom (a.getX(), a.getY() + 58, a.getRight(), a.getBottom() - 8);
        smallRow (k, { &scHpf, &scLpf, &scBellF, &scBellG, &scBellQ });
    }
    {
        auto g = globalArea.reduced (16, 0);
        autoGain.setBounds (g.getRight() - 140, g.getY() + 10, 140, 26);
        makeupArea = { g.getRight() - 300, g.getY() + 10, 152, 26 };
        voiceHold.setBounds (g.getRight() - 300 - 8 - 130, g.getY() + 10, 130, 26);
        g = g.withTrimmedTop (48).withTrimmedBottom (10);
        auto left = g.removeFromLeft (232);
        left = left.withSizeKeepingCentre (left.getWidth(), 3 * (15 + 28) + 2 * 10);
        orderLabel = left.removeFromTop (15);
        order.setBounds (left.removeFromTop (28));
        left.removeFromTop (10);
        osLabel = left.removeFromTop (15);
        oversampling.setBounds (left.removeFromTop (28));
        left.removeFromTop (10);
        chLabel = left.removeFromTop (15);
        chMode.setBounds (left.removeFromTop (28));

        g.removeFromLeft (14);
        smallRow (g, { &look, &color, &link, &mix, &out });
    }
}

void DuoCompEditor::Content::paint (juce::Graphics& g)
{
    using namespace juce;
    auto& e = owner;
    g.fillAll (col::bg);

    // header
    g.setColour (col::header);
    g.fillRect (e.headerArea);
    axell::brand::drawHeaderRule (g, (float) e.headerArea.getBottom() - 1.0f, 0.0f, (float) kWidth, col::grid);
    // Axell's Dojo: the family emblem, its wave and dot in this plugin's colour
    axell::brand::drawEmblem (g, { 12.0f, 6.0f, 46.0f, 46.0f }, juce::Colour (0xfff08a3c));
    axell::brand::drawTitle (g, 68.0f, 28.0f, "Duo Compressor", juce::String (juce::CharPointer_UTF8 ("\xe5\x8f\x8c"))); // 双
    {
        const auto f = caps (10.0f, 1);
        g.setFont (f);
        float x = 67.0f;
        auto word = [&] (const String& t, Colour c, float gapAfter)
        {
            g.setColour (c);
            const float w = GlyphArrangement::getStringWidth (f, t);
            g.drawText (t, Rectangle<float> (x, 37.0f, w + 2.0f, 14.0f), Justification::centredLeft);
            x += w + gapAfter;
        };
        word ("FET", col::fet, 6.0f);
        word ("+", col::dim, 6.0f);
        word ("OPTO", col::opto, 6.0f);
        word ("+", col::dim, 6.0f);
        word ("TRUE-PEAK LIMITER", col::lim, 0.0f);
    }

    auto panel = [&g] (Rectangle<int> a, Colour accent, const String& subtitle, bool active)
    {
        const auto r = a.toFloat();
        g.setGradientFill (ColourGradient (col::panelTop, 0.0f, r.getY(), col::panel, 0.0f, r.getBottom(), false));
        g.fillRoundedRectangle (r, 10.0f);
        g.setColour (accent.withAlpha (active ? 0.55f : 0.18f));
        g.drawRoundedRectangle (r.reduced (0.5f), 10.0f, 1.2f);
        if (subtitle.isNotEmpty())
        {
            // accent strip under the title row
            g.setColour (accent.withAlpha (active ? 0.9f : 0.3f));
            g.fillRoundedRectangle (r.getX() + 14.0f, r.getY() + 41.0f, 26.0f, 2.0f, 1.0f);
            g.setColour (col::dim);
            g.setFont (caps (10.5f, 1));
            g.drawText (subtitle, a.withTrimmedLeft (120).withTrimmedRight (16).withHeight (46), Justification::centredRight);
        }
    };
    panel (e.fetArea, col::fet, "FET PEAK COMPRESSOR", e.fetOn.getToggleState());
    panel (e.optoArea, col::opto, "OPTO LEVELING AMP", e.optoOn.getToggleState());
    panel (e.limArea, col::lim, "TRUE PEAK", e.limOn.getToggleState());
    panel (e.scArea, col::sc, {}, true);
    panel (e.globalArea, col::global, {}, true);

    for (auto [area, title, accent] : { std::tuple { e.scArea, "SIDE-CHAIN", col::sc }, std::tuple { e.globalArea, "GLOBAL", col::global } })
    {
        g.setColour (col::text);
        g.setFont (caps (13.0f, 2));
        g.drawText (title, area.getX() + 16, area.getY() + 10, 200, 26, Justification::centredLeft);
        g.setColour (accent.withAlpha (0.9f));
        g.fillRoundedRectangle ((float) area.getX() + 14.0f, (float) area.getY() + 37.0f, 26.0f, 2.0f, 1.0f);
    }
    g.setFont (ui::font (11.0f));
    g.setColour (col::faint);
    g.drawText (e.scStatusText, e.scStatusArea.withTrimmedLeft (34), Justification::centredLeft);

    g.setColour (col::dim);
    g.setFont (caps (10.5f, 1));
    g.drawText ("ORDER", e.orderLabel, Justification::centredLeft);
    g.drawText ("OVERSAMPLING", e.osLabel, Justification::centredLeft);
    g.drawText ("CHANNELS", e.chLabel, Justification::centredLeft);
    g.drawText ("DETECTOR", e.fetDetLabel, Justification::centredLeft);
    g.drawText ("KEY", e.fetKeyLabel, Justification::centredLeft);
    g.drawText ("MODEL", e.optoModelLabel, Justification::centredLeft);
    g.drawText ("KEY", e.optoKeyLabel, Justification::centredLeft);
    if (! e.limInfoArea.isEmpty())
    {
        g.setFont (ui::font (11.0f));
        g.setColour (col::faint);
        g.drawFittedText ("8x true-peak detection\nno overs above the ceiling", e.limInfoArea, Justification::centred, 2);
    }
    g.setFont (ui::font (11.0f));
    g.setColour (col::faint);
    g.drawText (e.latencyText, e.osLabel, Justification::centredRight);
    if (e.makeupText.isNotEmpty())
    {
        g.setColour (col::dim);
        g.drawText (e.makeupText, e.makeupArea, Justification::centredRight);
    }
}

void DuoCompEditor::updatePanelStates()
{
    const bool f = fetOn.getToggleState(), o = optoOn.getToggleState(), l = limOn.getToggleState();
    for (auto* c : std::initializer_list<juce::Component*> { &fetRatio, &fetDet, &fetKey, &fetIn, &fetOut, &fetAtk, &fetRel,
                                                            &fetThr, &fetKnee, &fetRange, &fetHold })
        c->setAlpha (f ? 1.0f : 0.4f);
    for (auto* c : std::initializer_list<juce::Component*> { &optoMode, &optoModel, &optoKey, &optoPR, &optoGain, &optoEmph, &optoThr,
                                                            &optoKnee, &optoRange, &optoHold })
        c->setAlpha (o ? 1.0f : 0.4f);
    for (auto* c : std::initializer_list<juce::Component*> { &limCeil, &limRel, &limLook })
        c->setAlpha (l ? 1.0f : 0.4f);
}

void DuoCompEditor::timerCallback()
{
    auto& mv = proc.meters;
    const int nCh = juce::jmax (1, proc.getTotalNumOutputChannels());
    inMeter.push (mv.inPeak[0].exchange (0.0f), mv.inPeak[1].exchange (0.0f), nCh);
    outMeter.push (mv.outPeak[0].exchange (0.0f), mv.outPeak[1].exchange (0.0f), nCh);
    fetMeter.push (mv.fetGr.exchange (0.0f), fetOn.getToggleState());
    optoMeter.push (mv.optoGr.exchange (0.0f), optoOn.getToggleState());
    limMeter.push (mv.limGr.exchange (0.0f), limOn.getToggleState());
    duo::HistoryFrame hf;
    float lastIn = -120.0f, lastOut = -120.0f;
    bool any = false;
    while (proc.engine().popHistory (hf))
    {
        graph.push (hf);
        lastIn = hf.inDb;
        lastOut = hf.outDb;
        any = true;
    }
    if (any)
        graph.repaint();
    transfer.setParams (proc.readParams());
    if (any)
        transfer.setLevel (lastIn, lastOut);

    presetName.setButtonText (proc.getPresetName() + (proc.isPresetModified() ? " *" : ""));
    slotA.setToggleState (proc.getActiveSlot() == 0, juce::dontSendNotification);
    slotB.setToggleState (proc.getActiveSlot() == 1, juce::dontSendNotification);
    slotCopy.setButtonText (proc.getActiveSlot() == 0 ? "COPY A>B" : "COPY B>A");
    updatePanelStates();

    const int lat = proc.getLatencySamples();
    const auto lt = "latency " + juce::String (lat) + " smp";
    const float mk = mv.makeup.load();
    const auto mt = autoGain.getToggleState() ? "makeup +" + juce::String (mk, 1) + " dB" : juce::String();
    const bool scConnected = proc.getBusCount (true) > 1 && proc.getBus (true, 1) != nullptr && proc.getBus (true, 1)->isEnabled();
    const auto* pf = proc.apvts.getRawParameterValue ("fetKey");
    const auto* po = proc.apvts.getRawParameterValue ("optoKey");
    const bool usesExt = pf->load() > 0.5f || po->load() > 0.5f;
    const juce::String st = usesExt ? (scConnected ? "external input in use" : "EXT selected - route a signal to the side-chain input")
                                    : (scConnected ? "external input available (set KEY to EXT)" : "internal (no external input routed)");
    if (lt != latencyText || mt != makeupText || st != scStatusText)
    {
        latencyText = lt;
        makeupText = mt;
        scStatusText = st;
        content.repaint (globalArea);
        content.repaint (scArea);
    }
    content.repaint (fetArea.withHeight (48));
    content.repaint (optoArea.withHeight (48));
    content.repaint (limArea.withHeight (48));
}

//==============================================================================
void DuoCompEditor::stepPreset (int delta)
{
    const int n = proc.getNumPrograms();
    proc.setCurrentProgram ((proc.getCurrentProgram() + delta + n) % n);
    timerCallback();
}

void DuoCompEditor::showPresetMenu()
{
    using namespace juce;
    PopupMenu menu, vocal, mixMaster, user, size;
    const auto list = duo::factoryPresets();
    for (int i = 0; i < (int) list.size(); ++i)
    {
        PopupMenu::Item item (list[(size_t) i].name);
        item.itemID = 1 + i;
        item.isTicked = proc.getPresetName() == list[(size_t) i].name;
        if (list[(size_t) i].category == duo::PresetCategory::vocal)
            vocal.addItem (item);
        else if (list[(size_t) i].category == duo::PresetCategory::mixMaster)
            mixMaster.addItem (item);
    }
    menu.addSectionHeader ("FACTORY");
    menu.addSubMenu ("Vocal", vocal);
    menu.addSubMenu ("Mix bus / Master", mixMaster);
    for (int i = 0; i < (int) list.size(); ++i)
        if (list[(size_t) i].category == duo::PresetCategory::utility)
            menu.addItem (1 + i, list[(size_t) i].name);

    auto files = DuoCompProcessor::userPresetFolder().findChildFiles (File::findFiles, false, "*.duopreset");
    // presets saved by Duo Vocal Comp 1.x are listed too (unless one of the same name exists)
    for (auto& f : DuoCompProcessor::legacyUserPresetFolder().findChildFiles (File::findFiles, false, "*.duopreset"))
        if (! DuoCompProcessor::userPresetFolder().getChildFile (f.getFileName()).existsAsFile())
            files.add (f);
    files.sort();
    for (int i = 0; i < files.size(); ++i)
        user.addItem (1000 + i, files[i].getFileNameWithoutExtension());
    menu.addSeparator();
    menu.addSectionHeader ("USER");
    if (files.isEmpty())
        menu.addItem (-1, "(no user presets yet)", false);
    else
        menu.addSubMenu ("User presets", user);
    menu.addItem (900, "Save current as...");
    menu.addItem (901, "Show preset folder");

    menu.addSeparator();
    for (int pct : { 100, 125, 150, 175, 200 })
        size.addItem (950 + pct / 25, String (pct) + " %", true,
                      std::abs (getWidth() - kWidth * pct / 100) < 4);
    menu.addSubMenu ("Interface size", size);

    Component::SafePointer<DuoCompEditor> safe (this);
    menu.showMenuAsync (PopupMenu::Options().withTargetComponent (&presetName), [safe, files] (int id)
    {
        if (safe == nullptr || id == 0)
            return;
        auto& self = *safe;
        if (id >= 1 && id < 900)
            self.proc.setCurrentProgram (id - 1);
        else if (id == 900)
            self.promptSavePreset();
        else if (id == 901)
        {
            auto folder = DuoCompProcessor::userPresetFolder();
            folder.createDirectory();
            folder.startAsProcess();
        }
        else if (id >= 950 && id < 1000)
            self.setUiScale ((float) (id - 950) * 0.25f);
        else if (id >= 1000 && id - 1000 < files.size())
            self.proc.loadUserPreset (files[id - 1000]);
        self.timerCallback();
    });
}

void DuoCompEditor::promptSavePreset()
{
    using namespace juce;
    saveDialog = std::make_unique<AlertWindow> ("Save preset", "Name for this preset:", MessageBoxIconType::NoIcon, this);
    auto* w = saveDialog.get();
    w->setLookAndFeel (&lnf);
    w->addTextEditor ("name", proc.getPresetName(), {});
    w->addButton ("Save", 1, KeyPress (KeyPress::returnKey));
    w->addButton ("Cancel", 0, KeyPress (KeyPress::escapeKey));
    Component::SafePointer<DuoCompEditor> safe (this);
    w->enterModalState (true, ModalCallbackFunction::create ([safe] (int result)
    {
        if (safe == nullptr || safe->saveDialog == nullptr)
            return;
        const auto name = safe->saveDialog->getTextEditorContents ("name").trim();
        if (result == 1 && name.isNotEmpty() && ! safe->proc.saveUserPreset (name))
            AlertWindow::showMessageBoxAsync (MessageBoxIconType::WarningIcon, "Save preset",
                                              "Could not write to " + DuoCompProcessor::userPresetFolder().getFullPathName());
        safe->timerCallback();
        // delete after the modal callback has returned
        MessageManager::callAsync ([safe] { if (safe != nullptr) safe->saveDialog.reset(); });
    }), false);
}

//==============================================================================
GrGraph::GrGraph() : ring (1400) {}

void GrGraph::push (const duo::HistoryFrame& f)
{
    ring[(size_t) pos] = f;
    pos = (pos + 1) % (int) ring.size();
}

void GrGraph::paint (juce::Graphics& g)
{
    using namespace juce;
    const auto r = getLocalBounds().toFloat();
    g.setColour (col::well);
    g.fillRoundedRectangle (r, 8.0f);
    const auto plot = r.reduced (8.0f, 6.0f);
    const float lo = -60.0f;
    auto yLevel = [&] (float dbv) { return plot.getBottom() - (jlimit (lo, 0.0f, dbv) - lo) / -lo * plot.getHeight(); };
    auto yGr = [&] (float gr) { return plot.getY() + jlimit (0.0f, 24.0f, gr) / 24.0f * plot.getHeight(); };
    // grid: level every 12 dB, GR every 6 dB (labels left / right)
    g.setFont (ui::font (9.5f));
    for (float d = -12.0f; d > lo; d -= 12.0f)
    {
        g.setColour (col::track.withAlpha (0.6f));
        g.drawHorizontalLine ((int) yLevel (d), plot.getX(), plot.getRight());
        g.setColour (col::faint);
        g.drawText (String ((int) d), Rectangle<float> (plot.getX() + 2.0f, yLevel (d) - 11.0f, 30.0f, 10.0f), Justification::left);
    }
    for (float gr = 6.0f; gr < 24.0f; gr += 6.0f)
        g.drawText ("-" + String ((int) gr), Rectangle<float> (plot.getRight() - 32.0f, yGr (gr) - 11.0f, 30.0f, 10.0f), Justification::right);

    const int n = (int) plot.getWidth();
    const int N = (int) ring.size();
    auto at = [&] (int i) -> const duo::HistoryFrame& { return ring[(size_t) ((pos - n + i + N * 4) % N)]; };
    auto xAt = [&] (int i) { return plot.getX() + (float) i; };
    Path in;
    in.startNewSubPath (xAt (0), plot.getBottom());
    for (int i = 0; i < n; ++i)
        in.lineTo (xAt (i), yLevel (at (i).inDb));
    in.lineTo (xAt (n - 1), plot.getBottom());
    in.closeSubPath();
    g.setColour (col::dim.withAlpha (0.28f));
    g.fillPath (in);
    auto line = [&] (auto value, Colour c, float w, bool fromTop)
    {
        Path p;
        bool started = false;
        for (int i = 0; i < n; ++i)
        {
            const float v = value (at (i));
            const float y = fromTop ? yGr (v) : yLevel (v);
            if (! started) { p.startNewSubPath (xAt (i), y); started = true; }
            else p.lineTo (xAt (i), y);
        }
        g.setColour (c);
        g.strokePath (p, PathStrokeType (w));
    };
    line ([] (const duo::HistoryFrame& f) { return f.outDb; }, col::text.withAlpha (0.85f), 1.2f, false);
    // gain reduction hangs from the top as filled areas; a stage that never reduced in the window is not drawn
    auto grArea = [&] (auto value, Colour c)
    {
        float mx = 0.0f;
        for (int i = 0; i < n; ++i)
            mx = std::max (mx, value (at (i)));
        if (mx < 0.05f)
            return;
        Path p;
        p.startNewSubPath (xAt (0), plot.getY());
        for (int i = 0; i < n; ++i)
            p.lineTo (xAt (i), yGr (value (at (i))));
        p.lineTo (xAt (n - 1), plot.getY());
        p.closeSubPath();
        g.setColour (c.withAlpha (0.25f));
        g.fillPath (p);
        line (value, c, 1.5f, true);
    };
    grArea ([] (const duo::HistoryFrame& f) { return f.fetGr; }, col::fet);
    grArea ([] (const duo::HistoryFrame& f) { return f.optoGr; }, col::opto);
    grArea ([] (const duo::HistoryFrame& f) { return f.limGr; }, col::lim);
    g.setColour (col::faint);
    g.drawText ("LEVEL / GAIN REDUCTION", plot.withTrimmedLeft (36).withHeight (12.0f), Justification::topLeft);
}

//==============================================================================
TransferCurve::TransferCurve() {}

void TransferCurve::setParams (const duo::Params& p)
{
    const bool changed = std::memcmp (&p, &params, sizeof p) != 0;
    params = p;
    fetCalc.setParams (duo::fetSettings (p));
    optoCalc.setParams (duo::optoSettings (p));
    if (changed)
        repaint();
}

void TransferCurve::setLevel (float inDb, float outDb)
{
    levelIn = inDb;
    levelOut = outDb;
    repaint();
}

float TransferCurve::outputFor (float inDb) const
{
    const auto& p = params;
    auto fetF = [&] (float v) { if (! p.fetOn) return v; const float lv = v + p.fetInDb; return lv - fetCalc.staticGainReductionDb (lv) + p.fetOutDb; };
    auto optoF = [&] (float v) { return p.optoOn ? v - optoCalc.staticGainReductionDb (v) + p.optoGainDb : v; };
    const float w = p.order == 0 ? optoF (fetF (inDb)) : fetF (optoF (inDb));
    const float mixv = std::clamp (p.mixPct, 0.0f, 100.0f) * 0.01f;
    const float a = (1.0f - mixv) * duo::dbToGain (inDb) + mixv * duo::dbToGain (w);
    float out = duo::gainToDb (std::max (a, 1.0e-9f)) + p.outDb;
    if (p.limOn)
        out = std::min (out, p.limCeilDb);
    return out;
}

void TransferCurve::paint (juce::Graphics& g)
{
    using namespace juce;
    const auto r = getLocalBounds().toFloat();
    g.setColour (col::well);
    g.fillRoundedRectangle (r, 8.0f);
    const float side = jmin (r.getWidth(), r.getHeight()) - 16.0f;
    const auto plot = Rectangle<float> (r.getCentreX() - side * 0.5f + 6.0f, r.getY() + 8.0f, side, side);
    const float lo = -60.0f, hi = 6.0f;
    auto X = [&] (float d) { return plot.getX() + (jlimit (lo, hi, d) - lo) / (hi - lo) * plot.getWidth(); };
    auto Y = [&] (float d) { return plot.getBottom() - (jlimit (lo, hi, d) - lo) / (hi - lo) * plot.getHeight(); };
    g.setColour (col::track.withAlpha (0.6f));
    for (float d = -48.0f; d <= 0.0f; d += 12.0f)
    {
        g.drawVerticalLine ((int) X (d), plot.getY(), plot.getBottom());
        g.drawHorizontalLine ((int) Y (d), plot.getX(), plot.getRight());
    }
    g.setColour (col::faint);
    g.drawLine (X (lo), Y (lo), X (hi), Y (hi), 1.0f); // 1:1
    Path c;
    for (int i = 0; i <= 132; ++i)
    {
        const float d = lo + (hi - lo) * (float) i / 132.0f;
        const float o = outputFor (d);
        if (i == 0) c.startNewSubPath (X (d), Y (o)); else c.lineTo (X (d), Y (o));
    }
    g.setColour (col::text);
    g.strokePath (c, PathStrokeType (1.8f));
    if (levelIn > lo)
    {
        g.setColour (col::warn);
        g.fillEllipse (X (levelIn) - 3.5f, Y (outputFor (levelIn)) - 3.5f, 7.0f, 7.0f);
    }
    g.setColour (col::faint);
    g.setFont (ui::font (9.5f));
    g.drawText ("IN", Rectangle<float> (plot.getRight() - 20.0f, plot.getBottom() - 12.0f, 20.0f, 10.0f), Justification::right);
    g.drawText ("OUT", Rectangle<float> (plot.getX() + 2.0f, plot.getY(), 24.0f, 10.0f), Justification::left);
}
