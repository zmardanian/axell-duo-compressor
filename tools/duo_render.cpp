// duo_render - runs the shipping DSP (Source/Engine.h) over a raw float32 file. Validation tool.
//   duo_render in.f32 out.f32 <sampleRate> <channels> [key=value ...] [--key keyfile.f32]
// keys: the duo::Params field names (fetOn, fetInDb, optoPR, optoMode, osLog2, ...). Interleaved float32.
#include "../Source/Engine.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <vector>

int main (int argc, char** argv)
{
    if (argc < 5)
    {
        std::fprintf (stderr, "usage: duo_render in.f32 out.f32 fs channels [key=value ...] [--key key.f32]\n");
        return 2;
    }
    const double fs = std::atof (argv[3]);
    const int nCh = std::atoi (argv[4]);
    duo::Params p;
    std::map<std::string, float*> f {
        { "fetInDb", &p.fetInDb }, { "fetOutDb", &p.fetOutDb }, { "fetAtkMs", &p.fetAtkMs }, { "fetRelMs", &p.fetRelMs },
        { "optoPR", &p.optoPR }, { "optoGainDb", &p.optoGainDb }, { "scHpfHz", &p.scHpfHz }, { "colorPct", &p.colorPct },
        { "mixPct", &p.mixPct }, { "outDb", &p.outDb }, { "linkPct", &p.linkPct }, { "emphasisPct", &p.emphasisPct },
        { "fetThrDb", &p.fetThrDb }, { "fetKneeDb", &p.fetKneeDb }, { "fetRangeDb", &p.fetRangeDb }, { "fetHoldMs", &p.fetHoldMs },
        { "optoThrDb", &p.optoThrDb }, { "optoKneeDb", &p.optoKneeDb }, { "optoRangeDb", &p.optoRangeDb }, { "optoHoldMs", &p.optoHoldMs },
        { "scLpfHz", &p.scLpfHz }, { "scBellHz", &p.scBellHz }, { "scBellDb", &p.scBellDb }, { "scBellQ", &p.scBellQ },
        { "lookaheadMs", &p.lookaheadMs }, { "limCeilDb", &p.limCeilDb }, { "limRelMs", &p.limRelMs }, { "limLookMs", &p.limLookMs },
        { "m.prDbPerUnit", &p.optoModel.prDbPerUnit }, { "m.prOffsetDb", &p.optoModel.prOffsetDb },
        { "m.ratioComp", &p.optoModel.ratioComp }, { "m.ratioLim", &p.optoModel.ratioLim }, { "m.kneeComp", &p.optoModel.kneeComp },
        { "m.kneeLim", &p.optoModel.kneeLim }, { "m.attackSec", &p.optoModel.attackSec }, { "m.fastReleaseSec", &p.optoModel.fastReleaseSec },
        { "m.fastShare", &p.optoModel.fastShare }, { "m.slowMinSec", &p.optoModel.slowMinSec }, { "m.slowMaxSec", &p.optoModel.slowMaxSec },
        { "m.memRiseSec", &p.optoModel.memRiseSec }, { "m.memFallSec", &p.optoModel.memFallSec },
        { "m.onsetGain", &p.optoModel.onsetGain }, { "m.onsetFastSec", &p.optoModel.onsetFastSec }, { "m.onsetSlowSec", &p.optoModel.onsetSlowSec },
        { "a.ratio", &p.fetAll.ratio }, { "a.knee", &p.fetAll.knee }, { "a.thrOffsetDb", &p.fetAll.thrOffsetDb },
        { "a.attackMul", &p.fetAll.attackMul }, { "a.releaseMul", &p.fetAll.releaseMul }, { "a.bumpOffsetDb", &p.fetAll.bumpOffsetDb },
        { "a.bumpSlope", &p.fetAll.bumpSlope }, { "a.bumpMaxDb", &p.fetAll.bumpMaxDb } };
    std::map<std::string, int*> i { { "fetRatio", &p.fetRatio }, { "optoMode", &p.optoMode }, { "order", &p.order }, { "osLog2", &p.osLog2 } };
    std::map<std::string, bool*> b { { "fetOn", &p.fetOn }, { "optoOn", &p.optoOn }, { "autoMakeup", &p.autoMakeup }, { "fetClean", &p.fetClean },
        { "fetExt", &p.fetExt }, { "optoExt", &p.optoExt }, { "limOn", &p.limOn }, { "midSide", &p.midSide },
        { "optoMeasured", &p.optoMeasured }, { "fetMeasured", &p.fetMeasured }, { "voiceHold", &p.voiceHold }, { "essHold", &p.essHold } };
    const char* keyFile = nullptr;
    for (int a = 5; a < argc; ++a)
    {
        if (std::strcmp (argv[a], "--key") == 0 && a + 1 < argc)
        {
            keyFile = argv[++a];
            continue;
        }
        std::string kv = argv[a];
        const auto eq = kv.find ('=');
        const std::string k = kv.substr (0, eq);
        const double v = std::atof (kv.c_str() + eq + 1);
        if (f.count (k)) *f[k] = (float) v;
        else if (i.count (k)) *i[k] = (int) v;
        else if (b.count (k)) *b[k] = v != 0.0;
        else { std::fprintf (stderr, "unknown parameter %s\n", k.c_str()); return 2; }
    }
    auto load = [] (const char* path)
    {
        std::vector<float> v;
        FILE* fp = std::fopen (path, "rb");
        if (fp == nullptr) return v;
        std::fseek (fp, 0, SEEK_END);
        const long sz = std::ftell (fp);
        std::fseek (fp, 0, SEEK_SET);
        v.resize ((size_t) sz / sizeof (float));
        if (std::fread (v.data(), sizeof (float), v.size(), fp) != v.size()) v.clear();
        std::fclose (fp);
        return v;
    };
    const auto in = load (argv[1]);
    const auto key = keyFile ? load (keyFile) : std::vector<float>();
    if (in.empty()) { std::fprintf (stderr, "cannot read %s\n", argv[1]); return 1; }
    const long n = (long) in.size() / nCh;
    duo::Engine e;
    e.prepare (fs, nCh, 512);
    e.setParams (p);
    e.reset();
    std::vector<float> out (in.size());
    std::vector<float> ch[2], kc[2];
    for (int c = 0; c < nCh; ++c) { ch[c].resize (512); kc[c].resize (512); }
    const int L = e.getLatencySamples();
    long written = 0;
    for (long pos = 0; pos < n + L; pos += 512)
    {
        const int len = 512;
        float* io[2] {};
        const float* kp[2] {};
        for (int c = 0; c < nCh; ++c)
        {
            for (int s = 0; s < len; ++s)
            {
                const long idx = pos + s;
                ch[c][(size_t) s] = idx < n ? in[(size_t) (idx * nCh + c)] : 0.0f;
                kc[c][(size_t) s] = (! key.empty() && idx < n) ? key[(size_t) (idx * nCh + c)] : 0.0f;
            }
            io[c] = ch[c].data();
            kp[c] = kc[c].data();
        }
        e.process (io, nCh, len, key.empty() ? nullptr : kp, key.empty() ? 0 : nCh);
        for (int s = 0; s < len; ++s)
        {
            const long o = pos + s - L; // latency-compensated
            if (o >= 0 && o < n)
            {
                for (int c = 0; c < nCh; ++c)
                    out[(size_t) (o * nCh + c)] = ch[c][(size_t) s];
                ++written;
            }
        }
    }
    FILE* fo = std::fopen (argv[2], "wb");
    std::fwrite (out.data(), sizeof (float), out.size(), fo);
    std::fclose (fo);
    std::fprintf (stderr, "rendered %ld frames, latency %d compensated\n", n, L);
    return 0;
}
