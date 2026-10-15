// Plugin-level tests (JUCE): parameters, state save/restore, latency reporting, presets, A/B, editor.
// Exit code = number of failures. Registered with CTest as "plugin_suite".
#include "../Source/PluginEditor.h"
#include "../Source/PluginProcessor.h"
#include "Measure.h"

#include <cstdio>
#include <thread>

namespace
{
int failures = 0, checks = 0;

void check (bool ok, const juce::String& what)
{
    ++checks;
    if (! ok)
        ++failures;
    std::printf ("%s  %s\n", ok ? "PASS" : "FAIL", what.toRawUTF8());
}

void setParam (DuoCompProcessor& p, const char* id, float value)
{
    auto* prm = p.apvts.getParameter (id);
    prm->setValueNotifyingHost (prm->convertTo0to1 (value));
}

void prepare (DuoCompProcessor& p, double fs = 48000.0, int block = 256)
{
    p.setPlayConfigDetails (2, 2, fs, block);
    p.prepareToPlay (fs, block);
}

std::vector<std::vector<float>> render (DuoCompProcessor& p, const dt::Buf& in, int block = 256)
{
    std::vector<std::vector<float>> out (2, std::vector<float> (in[0].size()));
    juce::AudioBuffer<float> buf (2, block);
    juce::MidiBuffer midi;
    for (size_t pos = 0; pos + (size_t) block <= in[0].size(); pos += (size_t) block)
    {
        for (int c = 0; c < 2; ++c)
            buf.copyFrom (c, 0, in[(size_t) c].data() + pos, block);
        p.processBlock (buf, midi);
        for (int c = 0; c < 2; ++c)
            std::copy (buf.getReadPointer (c), buf.getReadPointer (c) + block, out[(size_t) c].begin() + (long) pos);
    }
    return out;
}

void pumpMessages (int ms)
{
    juce::MessageManager::getInstance()->runDispatchLoopUntil (ms);
}
} // namespace

int main()
{
    juce::ScopedJuceInitialiser_GUI init;

    // user presets go to a temporary XDG data dir, never the real one
    auto tmp = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("duo-plugin-tests");
    tmp.deleteRecursively();
    tmp.createDirectory();
    setenv ("XDG_DATA_HOME", tmp.getFullPathName().toRawUTF8(), 1);

    //== 1. frozen parameter IDs, order (REAPER automation uses the index), versions
    {
        DuoCompProcessor p;
        const juce::StringArray frozen { "fetOn", "fetIn", "fetOut", "fetAtk", "fetRel", "fetRatio", "optoOn", "optoPR",
                                         "optoGain", "optoMode", "order", "scHpf", "color", "mix", "out" };
        const juce::StringArray added { "link", "optoEmph", "os", "scListen", "autoGain" };
        // Axell Duo Compressor 2.0
        const juce::StringArray added3 { "fetThr", "fetKnee", "fetRange", "fetHold", "fetDet", "fetKey", "optoThr", "optoKnee",
                                         "optoRange", "optoHold", "optoKey", "scLpf", "scBellF", "scBellG", "scBellQ", "look",
                                         "limOn", "limCeil", "limRel", "limLook", "chMode", "optoModel" };
        auto& params = p.getParameters();
        const int total = frozen.size() + added.size() + added3.size();
        check (params.size() == total, "parameter count = 42 (15 frozen + 5 v2 + 22 v3)");
        for (int i = 0; i < params.size() && i < total; ++i)
        {
            auto* wp = dynamic_cast<juce::AudioProcessorParameterWithID*> (params[i]);
            const int nf = frozen.size(), n2 = nf + added.size();
            const auto expected = i < nf ? frozen[i] : i < n2 ? added[i - nf] : added3[i - n2];
            const int hint = i < nf ? 1 : i < n2 ? 2 : 3;
            check (wp != nullptr && wp->getParameterID() == expected,
                   "parameter index " + juce::String (i) + " is '" + expected + "'");
            if (wp != nullptr)
                check (wp->getVersionHint() == hint, "parameter '" + expected + "' version hint " + juce::String (hint));
        }
    }

    //== 2. state save/restore: non-default values -> identical parameters + identical audio
    {
        DuoCompProcessor a;
        prepare (a);
        const std::pair<const char*, float> values[] = {
            { "fetOn", 1.0f }, { "fetIn", 11.3f }, { "fetOut", -3.7f }, { "fetAtk", 0.41f }, { "fetRel", 333.0f },
            { "fetRatio", 2.0f }, { "optoOn", 1.0f }, { "optoPR", 47.5f }, { "optoGain", 5.5f }, { "optoMode", 1.0f },
            { "order", 1.0f }, { "scHpf", 133.0f }, { "color", 62.0f }, { "mix", 73.0f }, { "out", -1.5f },
            { "link", 40.0f }, { "optoEmph", 55.0f }, { "os", 1.0f }, { "scListen", 0.0f }, { "autoGain", 1.0f },
            { "fetThr", -20.5f }, { "fetKnee", 7.5f }, { "fetRange", 12.0f }, { "fetHold", 35.0f }, { "fetDet", 1.0f },
            { "fetKey", 1.0f }, { "optoThr", -14.0f }, { "optoKnee", 3.0f }, { "optoRange", 9.5f }, { "optoHold", 120.0f },
            { "optoKey", 1.0f }, { "scLpf", 6000.0f }, { "scBellF", 2500.0f }, { "scBellG", 6.0f }, { "scBellQ", 2.0f },
            { "look", 2.5f }, { "limOn", 1.0f }, { "limCeil", -0.3f }, { "limRel", 120.0f }, { "limLook", 2.0f },
            { "chMode", 1.0f }, { "optoModel", 0.0f } };
        for (auto& v : values)
            setParam (a, v.first, v.second);
        a.setPresetName ("My Session Vocal");
        juce::MemoryBlock state;
        a.getStateInformation (state);

        DuoCompProcessor b;
        b.setStateInformation (state.getData(), (int) state.getSize());
        bool same = true;
        for (auto& v : values)
        {
            const float va = a.apvts.getRawParameterValue (v.first)->load();
            const float vb = b.apvts.getRawParameterValue (v.first)->load();
            if (std::abs (va - vb) > 1.0e-6f)
            {
                same = false;
                std::printf ("   mismatch %s: %f vs %f\n", v.first, va, vb);
            }
        }
        check (same, "state restore: all 20 parameter values identical");
        check (b.getPresetName() == "My Session Vocal", "state restore: preset name restored");

        prepare (a); // both instances start from a clean, snapped state
        prepare (b);
        const auto sig = dt::program (2, 48000 * 2, 48000.0, 5);
        const auto oa = render (a, sig);
        const auto ob = render (b, sig);
        double diff = 0.0;
        for (int c = 0; c < 2; ++c)
            for (size_t i = 0; i < oa[0].size(); ++i)
                diff = std::max (diff, (double) std::abs (oa[(size_t) c][i] - ob[(size_t) c][i]));
        check (diff <= 0.0, "state restore: restored instance renders bit-identical audio (max diff "
                                + juce::String (diff) + ")");
        // 2x + 2.5 ms lookahead + limiter (64-sample true-peak detector + 2 ms lookahead) at 48 kHz
        const int expectLat = b.engine().latencyForOversampling (1) + 120 + 64 + 96;
        check (a.getLatencySamples() == b.getLatencySamples() && b.getLatencySamples() == expectLat,
               "state restore: latency = 2x + lookahead + limiter (" + juce::String (b.getLatencySamples()) + " = "
                   + juce::String (expectLat) + " samples)");
    }

    //== 2b. pluginval-style: raw (unsnapped) normalised values on every parameter, save, scramble, restore
    {
        juce::Random rng (1234);
        for (int round = 0; round < 50; ++round)
        {
            DuoCompProcessor p;
            auto& params = p.getParameters();
            for (auto* prm : params)
                prm->setValueNotifyingHost (rng.nextFloat());
            std::vector<float> saved;
            for (auto* prm : params)
                saved.push_back (prm->getValue());
            juce::MemoryBlock mb;
            p.getStateInformation (mb);
            for (auto* prm : params)
                prm->setValueNotifyingHost (rng.nextFloat());
            p.setStateInformation (mb.getData(), (int) mb.getSize());
            bool ok = true;
            for (int i = 0; i < params.size(); ++i)
            {
                // expected: the value stored in the state (snapped by the parameter's range)
                auto* rp = dynamic_cast<juce::RangedAudioParameter*> (params[i]);
                const float expected = rp->convertTo0to1 (rp->convertFrom0to1 (saved[(size_t) i]));
                if (std::abs (params[i]->getValue() - expected) > 0.01f)
                {
                    ok = false;
                    std::printf ("   round %d %s: %f expected %f\n", round, rp->paramID.toRawUTF8(), params[i]->getValue(), expected);
                }
            }
            if (! ok || round == 49)
            {
                check (ok, "raw-value state restore (pluginval scenario), 50 random rounds");
                break;
            }
        }
    }

    //== 3. a project saved by the original draft (only the 15 v1 parameters) still loads
    {
        const char* oldXml = R"(<PARAMS><PARAM id="fetOn" value="1.0"/><PARAM id="fetIn" value="9.0"/>
            <PARAM id="fetOut" value="-2.0"/><PARAM id="fetAtk" value="0.3"/><PARAM id="fetRel" value="200.0"/>
            <PARAM id="fetRatio" value="1.0"/><PARAM id="optoOn" value="0.0"/><PARAM id="optoPR" value="40.0"/>
            <PARAM id="optoGain" value="2.0"/><PARAM id="optoMode" value="1.0"/><PARAM id="order" value="1.0"/>
            <PARAM id="scHpf" value="120.0"/><PARAM id="color" value="10.0"/><PARAM id="mix" value="80.0"/>
            <PARAM id="out" value="1.0"/></PARAMS>)";
        auto xml = juce::XmlDocument::parse (oldXml);
        juce::MemoryBlock mb;
        juce::AudioProcessor::copyXmlToBinary (*xml, mb);
        DuoCompProcessor p;
        p.setStateInformation (mb.getData(), (int) mb.getSize());
        auto v = [&p] (const char* id) { return p.apvts.getRawParameterValue (id)->load(); };
        check (std::abs (v ("fetIn") - 9.0f) < 1e-5f && std::abs (v ("mix") - 80.0f) < 1e-5f && v ("optoOn") < 0.5f
                   && std::abs (v ("fetRatio") - 1.0f) < 1e-5f,
               "draft-format project: v1 values restored");
        check (std::abs (v ("link") - 100.0f) < 1e-5f && std::abs (v ("os") - 1.0f) < 1e-5f && v ("autoGain") < 0.5f,
               "draft-format project: new parameters at their 1.x values (link 100 %, 2x, auto makeup off)");
        check (v ("limOn") < 0.5f && v ("look") < 1.0e-6f && v ("fetDet") < 0.5f && v ("chMode") < 0.5f
                   && std::abs (v ("fetThr") + 12.0f) < 1e-5f && v ("fetKnee") < 0.0f && v ("fetRange") >= 60.0f,
               "draft-format project: 2.0 parameters at their 1.x-equivalent defaults");
        check (v ("optoModel") < 0.5f, "draft-format project: opto model Classic (the law it was made with)");
    }

    //== 3b. a project saved by Duo Vocal Comp 1.0.0 (20 parameters, the os value saved) keeps its oversampling
    {
        DuoCompProcessor old;
        setParam (old, "os", 0.0f);
        juce::MemoryBlock st;
        old.getStateInformation (st);
        auto xml = juce::AudioProcessor::getXmlFromBinary (st.getData(), (int) st.getSize());
        for (auto* e = xml->getFirstChildElement(); e != nullptr;)
        {
            auto* next = e->getNextElement();
            const auto id = e->getStringAttribute ("id");
            if (id.startsWith ("fet") && (id.endsWith ("Thr") || id.endsWith ("Knee") || id.endsWith ("Range")
                                          || id.endsWith ("Hold") || id.endsWith ("Det") || id.endsWith ("Key")))
                xml->removeChildElement (e, true);
            else if (juce::StringArray { "optoThr", "optoKnee", "optoRange", "optoHold", "optoKey", "scLpf", "scBellF", "scBellG",
                                         "scBellQ", "look", "limOn", "limCeil", "limRel", "limLook", "chMode", "optoModel" }.contains (id))
                xml->removeChildElement (e, true);
            e = next;
        }
        juce::MemoryBlock mb;
        juce::AudioProcessor::copyXmlToBinary (*xml, mb);
        DuoCompProcessor p;
        p.setStateInformation (mb.getData(), (int) mb.getSize());
        check (p.apvts.getRawParameterValue ("os")->load() < 0.5f && p.apvts.getRawParameterValue ("limOn")->load() < 0.5f,
               "1.0.0-format project: saved 1x kept, 2.0 parameters at defaults");
        check (p.apvts.getRawParameterValue ("optoModel")->load() < 0.5f, "1.0.0-format project: opto model Classic");
        DuoCompProcessor fresh;
        check (fresh.apvts.getRawParameterValue ("optoModel")->load() > 0.5f, "new instance: opto model Measured");
    }

    //== 4. latency reporting follows the oversampling parameter (fade + message-thread timer)
    {
        DuoCompProcessor p;
        prepare (p);
        check (p.getLatencySamples() == p.engine().latencyForOversampling (2), "prepare: latency = 4x latency (the 2.0 default)");
        const auto sig = dt::program (2, 4800, 48000.0, 1);
        for (int f : { 2, 0, 1 })
        {
            setParam (p, "os", (float) f);
            render (p, sig);     // audio thread performs the faded switch
            pumpMessages (150);  // 20 Hz timer reports the new latency
            check (p.getLatencySamples() == p.engine().latencyForOversampling (f),
                   "after switching to " + juce::String (1 << f) + "x: reported latency " + juce::String (p.getLatencySamples()));
        }
    }

    //== 5. factory programs
    {
        DuoCompProcessor p;
        const auto list = duo::factoryPresets();
        check (p.getNumPrograms() == (int) list.size(), "programs: " + juce::String (p.getNumPrograms()) + " factory presets");
        const juce::StringArray required { "Rap Lead", "Boom Bap Double", "Aggressive Parallel (All)", "Gentle Leveling",
                                           "Spoken Word", "Init" };
        for (int i = 0; i < required.size(); ++i)
            check (p.getProgramName (i) == required[i], "program " + juce::String (i) + " is '" + required[i] + "'");
        bool allOk = true;
        for (int i = 0; i < p.getNumPrograms(); ++i)
        {
            p.setCurrentProgram (i);
            const auto r = p.readParams();
            const auto& e = list[(size_t) i].params;
            if (std::abs (r.fetInDb - e.fetInDb) > 0.051f || std::abs (r.optoPR - e.optoPR) > 0.051f
                || r.fetRatio != e.fetRatio || std::abs (r.mixPct - e.mixPct) > 0.51f || r.osLog2 != e.osLog2
                || r.order != e.order || r.optoOn != e.optoOn || std::abs (r.fetAtkMs - e.fetAtkMs) > 0.0011f)
            {
                allOk = false;
                std::printf ("   program %d (%s) did not apply\n", i, list[(size_t) i].name);
            }
        }
        check (allOk, "every factory program applies its values");
        p.setCurrentProgram (5);
        bool defaults = true;
        for (auto* prm : p.getParameters())
            if (std::abs (prm->getValue() - prm->getDefaultValue()) > 1.0e-4f)
                defaults = false;
        check (defaults, "'Init' program = all parameter defaults");

        // program change from a non-message thread (what a VST3 host's audio thread may do):
        // values apply immediately, the display name is resolved later on the message thread
        std::thread t ([&p] { p.setCurrentProgram (1); });
        t.join();
        check (p.getCurrentProgram() == 1 && std::abs (p.readParams().fetInDb - list[1].params.fetInDb) < 0.051f,
               "program change from another thread applies the values");
        pumpMessages (150);
        check (p.getPresetName() == list[1].name, "program change from another thread: name updated by the timer");
        check (! p.isPresetModified(), "preset just loaded: not marked modified");
        setParam (p, "color", 99.0f);
        check (p.isPresetModified(), "preset tweaked: marked modified (*)");
    }

    //== 6. user presets save/load
    {
        DuoCompProcessor p;
        setParam (p, "fetIn", 4.2f);
        setParam (p, "color", 77.0f);
        check (p.saveUserPreset ("Test / Preset: 1"), "user preset saved (name with illegal file characters)");
        auto files = DuoCompProcessor::userPresetFolder().findChildFiles (juce::File::findFiles, false, "*.duopreset");
        check (files.size() == 1 && DuoCompProcessor::userPresetFolder().isAChildOf (tmp),
               "user preset written to $XDG_DATA_HOME/AxellDuoCompressor/Presets");
        DuoCompProcessor q;
        check (files.size() == 1 && q.loadUserPreset (files[0]), "user preset loads");
        check (std::abs (q.apvts.getRawParameterValue ("fetIn")->load() - 4.2f) < 1e-4f
                   && std::abs (q.apvts.getRawParameterValue ("color")->load() - 77.0f) < 1e-4f
                   && q.getPresetName() == "Test / Preset: 1",
               "user preset restores values and name");
    }

    //== 7. A/B compare
    {
        DuoCompProcessor p;
        setParam (p, "fetIn", 3.0f);
        p.selectSlot (1);                         // B starts as a copy of A
        setParam (p, "fetIn", 12.0f);
        p.selectSlot (0);
        const float a = p.apvts.getRawParameterValue ("fetIn")->load();
        p.selectSlot (1);
        const float b = p.apvts.getRawParameterValue ("fetIn")->load();
        check (std::abs (a - 3.0f) < 1e-4f && std::abs (b - 12.0f) < 1e-4f, "A/B: each slot keeps its own settings");
        p.copyActiveToOther();
        p.selectSlot (0);
        check (std::abs (p.apvts.getRawParameterValue ("fetIn")->load() - 12.0f) < 1e-4f, "A/B: copy B > A");
    }

    //== 8. editor: open/close repeatedly while processing, size limits and aspect ratio
    {
        DuoCompProcessor p;
        prepare (p);
        const auto sig = dt::program (2, 2048, 48000.0, 2);
        bool ok = true;
        for (int i = 0; i < 5; ++i)
        {
            std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
            render (p, sig);
            pumpMessages (40);
            ok = ok && ed->getWidth() >= ui::kWidth;
        }
        check (ok, "editor: opens/closes 5x while processing");
        std::unique_ptr<juce::AudioProcessorEditor> ed (p.createEditor());
        auto* c = ed->getConstrainer();
        check (c != nullptr && c->getMinimumWidth() == ui::kWidth && c->getMaximumWidth() == 2 * ui::kWidth
                   && std::abs (c->getFixedAspectRatio() - (double) ui::kWidth / ui::kHeight) < 1e-9,
               "editor: resizable 100-200 %, aspect locked");
        ed->setSize (ui::kWidth * 3 / 2, ui::kHeight * 3 / 2);
        ed.reset();
        std::unique_ptr<juce::AudioProcessorEditor> ed2 (p.createEditor());
        check (ed2->getWidth() == ui::kWidth * 3 / 2, "editor: re-opens at the last used size (150 %)");
    }

    tmp.deleteRecursively();
    std::printf ("%s: %d of %d plugin checks passed\n", failures == 0 ? "ALL PASS" : "FAILURES", checks - failures, checks);
    return failures;
}
