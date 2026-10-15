// retune_presets - for each factory preset with the opto on: the Peak Reduction under the Measured opto law that
// gives the same average opto gain reduction as the preset's Classic setting, on the suite's test signals
// (vocal -18 dBFS RMS / mix -14 dBFS RMS, 48 kHz stereo). Prints the new values for Source/Presets.h.
#include "../tests/Measure.h"
#include "../Source/Presets.h"

#include <cstdio>

using namespace dt;

static Buf normalised (Buf b, double rmsDb)
{
    double s = 0.0;
    for (float v : b[0])
        s += (double) v * v;
    const double g = fromDb (rmsDb) / std::sqrt (s / (double) b[0].size());
    for (auto& ch : b)
        for (auto& v : ch)
            v *= (float) g;
    return b;
}

static double optoAvgGr (const duo::Params& p, const Buf& in)
{
    duo::Engine e;
    e.prepare (48000.0, 2);
    e.setParams (p);
    e.reset();
    double sum = 0.0;
    int nb = 0;
    const int block = 480, from = 2 * 48000;
    run (e, in, block, [&] (int pos, int)
    {
        if (pos > from)
        {
            sum += e.getMeters().optoGrDb;
            ++nb;
        }
        e.clearMeters();
    });
    return sum / std::max (1, nb);
}

int main()
{
    const int n = 8 * 48000;
    const Buf vox = normalised (program (2, n, 48000.0, 42), -18.0);
    const Buf mix = normalised (mixProgram (2, n, 48000.0, 7), -14.0);
    for (const auto& fp : duo::factoryPresets())
    {
        if (! fp.params.optoOn)
            continue;
        const Buf& in = fp.category == duo::PresetCategory::mixMaster ? mix : vox;
        const double target = optoAvgGr (fp.params, in);
        auto p = fp.params;
        p.optoMeasured = true;
        double lo = 0.0, hi = 100.0;
        for (int i = 0; i < 16; ++i)
        {
            p.optoPR = (float) (0.5 * (lo + hi));
            (optoAvgGr (p, in) < target ? lo : hi) = p.optoPR;
        }
        p.optoPR = (float) (0.5 * (lo + hi));
        std::printf ("%-40s classic PR %5.1f -> measured PR %5.1f   (opto GR %.2f dB, measured %.2f dB)\n", fp.name,
                     fp.params.optoPR, p.optoPR, target, optoAvgGr (p, in));
    }
}
