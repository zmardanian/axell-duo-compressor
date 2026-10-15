// Headless GUI snapshot tool: builds the real processor + editor, feeds a vocal-like signal so
// the meters move, and saves PNGs (100 % = minimum size, 200 % = maximum size, HiDPI render).
//   DuoGuiSnapshot [outputDir]
#include "../Source/PluginEditor.h"
#include "../Source/PluginProcessor.h"
#include "Measure.h"

#include <functional>

namespace
{
void shoot (const juce::File& dir, const juce::String& name, float uiScale, float renderScale,
            const std::function<void (DuoCompProcessor&)>& setup)
{
    DuoCompProcessor proc;
    proc.setPlayConfigDetails (2, 2, 48000.0, 512);
    proc.prepareToPlay (48000.0, 512);
    setup (proc);

    std::unique_ptr<juce::AudioProcessorEditor> ed (proc.createEditor());
    auto* editor = dynamic_cast<DuoCompEditor*> (ed.get());
    ed->setSize (juce::roundToInt (ui::kWidth * uiScale), juce::roundToInt (ui::kHeight * uiScale));

    // ~1.5 s of a loud vocal-like signal, meters refreshed like the 30 Hz GUI timer would
    auto sig = dt::program (2, 72000, 48000.0, 3);
    for (auto& ch : sig)
        for (auto& v : ch)
            v *= 0.9f;
    juce::AudioBuffer<float> buf (2, 512);
    juce::MidiBuffer midi;
    for (int pos = 0, blk = 0; pos + 512 <= 72000; pos += 512, ++blk)
    {
        for (int c = 0; c < 2; ++c)
            buf.copyFrom (c, 0, sig[(size_t) c].data() + pos, 512);
        proc.processBlock (buf, midi);
        if (blk % 3 == 0)
            editor->refreshNow();
        juce::MessageManager::getInstance()->runDispatchLoopUntil (1);
    }
    editor->refreshNow();

    const auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, renderScale);
    const auto file = dir.getChildFile (name + ".png");
    file.deleteFile();
    juce::FileOutputStream os (file);
    juce::PNGImageFormat png;
    if (os.openedOk() && png.writeImageToStream (img, os))
        std::printf ("wrote %s (%dx%d)\n", file.getFullPathName().toRawUTF8(), img.getWidth(), img.getHeight());
    else
        std::printf ("FAILED to write %s\n", file.getFullPathName().toRawUTF8());
}

void setParam (DuoCompProcessor& p, const char* id, float value)
{
    if (auto* prm = p.apvts.getParameter (id))
        prm->setValueNotifyingHost (prm->convertTo0to1 (value));
}
} // namespace

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    const juce::File dir (argc > 1 ? juce::File::getCurrentWorkingDirectory().getChildFile (argv[1])
                                   : juce::File::getCurrentWorkingDirectory().getChildFile ("docs/screenshots"));
    dir.createDirectory();

    auto lead = [] (DuoCompProcessor& p) { p.setCurrentProgram (0); };
    shoot (dir, "duo-100pct-min", 1.0f, 1.0f, lead);
    shoot (dir, "duo-200pct-max", 2.0f, 1.0f, lead);
    shoot (dir, "duo-100pct-hidpi2x", 1.0f, 2.0f, lead);
    shoot (dir, "duo-150pct", 1.5f, 1.0f, lead);
    shoot (dir, "duo-states", 1.0f, 1.0f, [] (DuoCompProcessor& p)
    {
        p.setCurrentProgram (8); // a parallel preset
        setParam (p, "fetRatio", 4.0f);
        setParam (p, "optoOn", 0.0f);
        setParam (p, "scListen", 1.0f);
        setParam (p, "autoGain", 1.0f);
        setParam (p, "os", 2.0f);
        setParam (p, "order", 1.0f);
        setParam (p, "limOn", 1.0f);
        setParam (p, "fetDet", 1.0f);
        setParam (p, "optoKey", 1.0f);
        setParam (p, "chMode", 1.0f);
        setParam (p, "look", 2.0f);
        setParam (p, "scBellG", 6.0f);
        p.selectSlot (1);
    });
    return 0;
}
