#pragma once
// The only place in the test suite that knows the stage-level API of Source/Dsp.h.
// Stage tests (ballistics) drive a stage directly at the oversampled rate with an ideal,
// sample-exact step: a step that first passes through the 44.1 kHz input band limit cannot
// rise faster than ~23 us, which would make a 20 us attack physically unmeasurable.
#include "../Source/Engine.h"

namespace dt
{
struct FetUT
{
    duo::FetStage st;
    int nCh = 1;
    // fs = the stage's (oversampled) rate; stages are prepared for base rate fs/2 at 2x
    void prepare (double fs, int ch)
    {
        nCh = ch;
        st.prepare (fs / 2.0);
        st.setOversampling (1);
    }
    void set (const duo::Params& p)
    {
        st.setParams (duo::fetSettings (p));
        st.snap();
    }
    float frame (float* x) { return st.processFrame (x, nCh); }
};

struct OptoUT
{
    duo::OptoStage st;
    int nCh = 1;
    void prepare (double fs, int ch)
    {
        nCh = ch;
        st.prepare (fs / 2.0);
        st.setOversampling (1);
    }
    void set (const duo::Params& p)
    {
        st.setParams (duo::optoSettings (p));
        st.snap();
    }
    float frame (float* x) { return st.processFrame (x, nCh); }
};
} // namespace dt
