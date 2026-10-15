#pragma once
// Axell Duo Compressor - complete signal chain, framework independent (no JUCE).
// The plugin processor is a thin wrapper around this class, so the DSP test suite
// (tests/) measures exactly the code that ships.
//
// Signal flow (per channel):
//   in -> sanitize -> [dry delay line: exactly the reported latency] -------------------+
//      -> upsample 1x/2x/4x -> FET / Opto (order switchable, each crossfaded on/off)    |
//      -> downsample -> auto makeup -> mix with delayed dry (base rate) -> output gain <-+
//      -> SC-listen crossfade -> OS-switch fade -> bypass crossfade -> out
//
// Real-time safety: prepare() allocates everything; process() never allocates, locks or
// throws. Oversampling changes are RT-safe (all filters for every factor are precomputed)
// and are hidden behind a 3 ms fade-out / fade-in.

#include "Dsp.h"
#include "Oversampling.h"

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <vector>

#if defined(__SSE__) || defined(_M_X64) || defined(__x86_64__)
 #include <xmmintrin.h>
 #define DUO_HAS_SSE 1
#endif

namespace duo
{
struct Params
{
    // v1 parameters (IDs frozen in the plugin)
    bool  fetOn = true;
    float fetInDb = 6.0f, fetOutDb = 0.0f, fetAtkMs = 0.2f, fetRelMs = 150.0f;
    int   fetRatio = 0;                         // 0..4 = 4, 8, 12, 20, All
    bool  optoOn = true;
    float optoPR = 30.0f, optoGainDb = 0.0f;
    int   optoMode = 0;                         // 0 Compress, 1 Limit
    int   order = 0;                            // 0 FET > Opto, 1 Opto > FET
    float scHpfHz = 80.0f, colorPct = 30.0f, mixPct = 100.0f, outDb = 0.0f;
    // v2 parameters
    float linkPct = 100.0f, emphasisPct = 0.0f;
    int   osLog2 = 1;                           // 0 = 1x, 1 = 2x, 2 = 4x
    bool  scListen = false, autoMakeup = false;
    bool  bypass = false;                       // host bypass (latency-compensated)
    // v3 parameters (Axell Duo Compressor 2.0). Defaults = the 1.x behaviour.
    float fetThrDb = -12.0f, fetKneeDb = -1.0f /* < 0 Auto */, fetRangeDb = 60.0f /* >= 60 Off */, fetHoldMs = 0.0f;
    bool  fetClean = false, fetExt = false;     // Clean detector; key from the external side-chain input
    float optoThrDb = -10.0f, optoKneeDb = -1.0f, optoRangeDb = 60.0f, optoHoldMs = 0.0f;
    bool  optoExt = false;
    float scLpfHz = 20000.0f /* 20 kHz = Off */, scBellHz = 3000.0f, scBellDb = 0.0f, scBellQ = 1.0f;
    float lookaheadMs = 0.0f;                   // compressor lookahead 0..10 ms
    bool  limOn = false;
    float limCeilDb = -1.0f, limRelMs = 50.0f, limLookMs = 1.5f;
    bool  midSide = false;
    bool  compare = false;                      // level-matched compare: dry, loudness-matched to the processed signal
    bool  optoMeasured = false;                 // opto law calibrated on hardware recordings (plugin default for new instances)
    bool  fetMeasured = false;                  // FET ALL law calibrated on hardware recordings (plugin default for new instances)
    FetStage::Settings::AllModel fetAll {};     // the Classic ALL law; validation tools override its fields
    OptoStage::Settings::Model optoModel {};    // the Classic (1.x) law; validation tools override its fields
};

inline SideChainEq scEq (const Params& p) noexcept
{
    SideChainEq e;
    e.lpfHz = p.scLpfHz;
    e.bellHz = p.scBellHz;
    e.bellDb = p.scBellDb;
    e.bellQ = p.scBellQ;
    return e;
}

inline FetStage::Settings fetSettings (const Params& p) noexcept
{
    FetStage::Settings s;
    s.inDb = p.fetInDb;
    s.outDb = p.fetOutDb;
    s.attackMs = p.fetAtkMs;
    s.releaseMs = p.fetRelMs;
    s.ratio = p.fetRatio;
    s.scHz = p.scHpfHz;
    s.color = p.colorPct * 0.01f;
    s.link = p.linkPct * 0.01f;
    s.thresholdDb = p.fetThrDb;
    s.kneeDb = p.fetKneeDb;
    s.rangeDb = p.fetRangeDb;
    s.holdMs = p.fetHoldMs;
    s.clean = p.fetClean;
    s.all = p.fetMeasured ? FetStage::measuredAll() : p.fetAll;
    s.eq = scEq (p);
    return s;
}

inline OptoStage::Settings optoSettings (const Params& p) noexcept
{
    OptoStage::Settings s;
    s.peakReduction = p.optoPR;
    s.gainDb = p.optoGainDb;
    s.limit = p.optoMode == 1;
    s.scHz = p.scHpfHz;
    s.color = p.colorPct * 0.01f;
    s.link = p.linkPct * 0.01f;
    s.emphasis = p.emphasisPct * 0.01f;
    s.thresholdDb = p.optoThrDb;
    s.kneeDb = p.optoKneeDb;
    s.rangeDb = p.optoRangeDb;
    s.holdMs = p.optoHoldMs;
    s.eq = scEq (p);
    s.model = p.optoMeasured ? OptoStage::measuredModel() : p.optoModel;
    return s;
}

// one point of the display history (one per ~5 ms of audio): written by the audio thread, read by the GUI
struct HistoryFrame
{
    float inDb = -120.0f, outDb = -120.0f, fetGr = 0.0f, optoGr = 0.0f, limGr = 0.0f;
};

// single-producer / single-consumer ring (audio thread pushes, GUI pops); drops frames when the GUI is closed
class HistoryFifo
{
public:
    static constexpr std::uint32_t kSize = 2048;
    void push (const HistoryFrame& f) noexcept
    {
        const auto w = write.load (std::memory_order_relaxed);
        if (w - read.load (std::memory_order_acquire) >= kSize)
            return;
        buf[w % kSize] = f;
        write.store (w + 1, std::memory_order_release);
    }
    bool pop (HistoryFrame& f) noexcept
    {
        const auto r = read.load (std::memory_order_relaxed);
        if (r == write.load (std::memory_order_acquire))
            return false;
        f = buf[r % kSize];
        read.store (r + 1, std::memory_order_release);
        return true;
    }

private:
    std::array<HistoryFrame, kSize> buf {};
    std::atomic<std::uint32_t> write { 0 }, read { 0 };
};

struct Meters
{
    float inPeak[2] {}, outPeak[2] {};
    float fetGrDb = 0.0f, optoGrDb = 0.0f, makeupDb = 0.0f, limGrDb = 0.0f;
    void clear() noexcept { *this = Meters(); }
};

// test hook: records the per-oversampled-sample gain reduction (max over channels)
struct Probe
{
    float* fetGr = nullptr;
    float* optoGr = nullptr;
    int capacity = 0;
    int count = 0;
};

// flush-to-zero / denormals-are-zero for the duration of a process call (restores the host's mode)
struct ScopedFlushDenormals
{
#if DUO_HAS_SSE
    ScopedFlushDenormals() noexcept : saved (_mm_getcsr()) { _mm_setcsr (saved | 0x8040u); }
    ~ScopedFlushDenormals() { _mm_setcsr (saved); }
    unsigned int saved;
#endif
};

class Engine
{
public:
    static constexpr int kChunk = 256;
    static constexpr double kMaxLookaheadMs = 10.0, kMaxLimLookMs = 5.0, kMinLimLookMs = 0.5;

    // structural settings: they change the latency or the signal path, so they switch behind a fade
    // (and, when the latency changes, only after the host was told - see setHostLatency)
    struct Config
    {
        int osLog2 = 1, order = 0, lookahead = 0 /* base samples */, limLook = 66 /* base samples */;
        bool limOn = false, midSide = false;
        bool operator== (const Config& o) const noexcept
        {
            return osLog2 == o.osLog2 && order == o.order && lookahead == o.lookahead && limOn == o.limOn
                && (! limOn || limLook == o.limLook) && midSide == o.midSide;
        }
        bool operator!= (const Config& o) const noexcept { return ! (*this == o); }
    };

    void prepare (double sampleRate, int numChannels, int maxBlockSize = 0)
    {
        fs = sampleRate;
        numCh = std::clamp (numChannels, 1, kMaxCh);
        maxBlock = std::max (0, maxBlockSize);
        os.prepare (kChunk, fs);
        osKey.prepare (kChunk, fs);
        fet.prepare (fs);
        opto.prepare (fs);
        maxLa = (int) std::ceil (kMaxLookaheadMs * 0.001 * fs);
        maxLimLook = (int) std::ceil (kMaxLimLookMs * 0.001 * fs);
        lim.prepare (fs, maxLimLook);

        const int maxLat = std::max (os.latencyFor (1), os.latencyFor (2)) + maxLa + TruePeakLimiter::latencyFor (maxLimLook);
        int ring = 1;
        while (ring < maxLat + kChunk + 1)
            ring <<= 1;
        ringMask = ring - 1;
        int laRing = 1;
        while (laRing < maxLa * (1 << (kNumOsFactors - 1)) + 2)
            laRing <<= 1;
        laMask = laRing - 1;
        for (int c = 0; c < kMaxCh; ++c)
        {
            dryRing[c].assign ((size_t) ring, 0.0f);
            keyRing[c].assign ((size_t) ring, 0.0f);
            wet[c].assign ((size_t) kChunk, 0.0f);
            keyIn[c].assign ((size_t) kChunk, 0.0f);
            laBuf[c].assign ((size_t) laRing, 0.0f);
        }
        grBase.assign ((size_t) kChunk, 0.0f);
        outGainBuf.assign ((size_t) kChunk, 1.0f);

        smoothBase  = 1.0f - onePole (0.02f, fs);
        switchK     = 1.0f - onePole (0.01f, fs);
        gateRelease = onePole (0.1f, fs);
        avgK        = 1.0f - onePole (kAutoMakeupSec, fs);
        makeupK     = 1.0f - onePole (0.3f, fs);
        fadeStep    = (float) (1.0 / std::max (1.0, 0.003 * fs));
        kwDry.prepare (fs);
        kwWet.prepare (fs);
        loudK       = 1.0f - onePole (0.4f, fs);
        cmpK        = 1.0f - onePole (0.25f, fs);
        hopLen      = std::max (1, (int) std::lround (0.005 * fs));

        setParams (params);
        active = want;
        applyConfig (active, true);
        // a fresh start: the wrapper reports this latency right after prepare, before audio runs
        hostLat = latency;
        reqOpen = holding = false;
        latReq.store (pack (kReqIdle, 0, latency), std::memory_order_release);
        reset();
    }

    // clears all signal state and snaps every smoother to its target
    void reset() noexcept
    {
        // a configuration whose latency the host does not compensate yet is not applied here: the old
        // one stays, the output starts muted and the handshake runs (see hostReady)
        const bool apply = ! tracking() || latencyOf (want) == hostLat;
        if (apply && want != active)
        {
            active = want;
            applyConfig (active, true);
        }
        os.reset();
        osKey.reset();
        fet.resetState();
        opto.resetState();
        lim.resetState();
        for (auto& r : dryRing)
            std::fill (r.begin(), r.end(), 0.0f);
        for (auto& r : keyRing)
            std::fill (r.begin(), r.end(), 0.0f);
        for (auto& r : laBuf)
            std::fill (r.begin(), r.end(), 0.0f);
        writePos = laPos = 0;
        fadeGain = fadeTgt = (apply || want == active) ? 1.0f : 0.0f;
        warm = 0;
        avgGrDb = makeupDb = 0.0f;
        gateEnv = 0.0f;
        listenHpf.resetState();
        listenEq.resetState();
        listenShelf.resetState();
        kwDry.resetState();
        kwWet.resetState();
        msDry = msWet = 0.0;
        cmpGain = 1.0f;
        cmpAmt = cmpTarget;
        hopCount = 0;
        hopIn = hopOut = hopFet = hopOpto = hopLim = 0.0f;
        fet.snap();
        opto.snap();
        fetAmt = fetTarget;
        optoAmt = optoTarget;
        mixCur = mixTarget;
        outCur = outTarget;
        listenAmt = listenTarget;
        bypassAmt = bypassTarget;
        meters.clear();
    }

    void setParams (const Params& p) noexcept
    {
        params = p;
        fet.setParams (fetSettings (p));
        opto.setParams (optoSettings (p));
        fetTarget = p.fetOn ? 1.0f : 0.0f;
        optoTarget = p.optoOn ? 1.0f : 0.0f;
        mixTarget = std::clamp (p.mixPct, 0.0f, 100.0f) * 0.01f;
        outTarget = dbToGain (std::clamp (p.outDb, -24.0f, 24.0f));
        listenTarget = p.scListen ? 1.0f : 0.0f;
        bypassTarget = p.bypass ? 1.0f : 0.0f;
        cmpTarget = p.compare ? 1.0f : 0.0f;
        lim.setParams (p.limCeilDb, p.limRelMs);
        want.osLog2 = std::clamp (p.osLog2, 0, 2);
        want.order = std::clamp (p.order, 0, 1);
        want.lookahead = std::clamp ((int) std::lround (std::clamp (p.lookaheadMs, 0.0f, (float) kMaxLookaheadMs) * 0.001 * fs), 0, maxLa);
        want.limOn = p.limOn;
        want.limLook = std::clamp ((int) std::lround (std::clamp ((double) p.limLookMs, kMinLimLookMs, kMaxLimLookMs) * 0.001 * fs),
                                   TruePeakLimiter::kSmooth, maxLimLook);
        want.midSide = p.midSide && numCh >= 2;
        listenHpf.setHighPass (fs, std::clamp (p.scHpfHz, 20.0f, 300.0f));
        listenEq.configure (fs, scEq (p));
        listenShelf.setHighShelf (fs, 3000.0, 12.0 * std::clamp (p.emphasisPct, 0.0f, 100.0f) * 0.01);
    }

    // offline (non-realtime) rendering: configurations switch at once (no host to wait for)
    void setNonRealtime (bool nonRealtime) noexcept { offline.store (nonRealtime, std::memory_order_release); }

    // latency of the configuration that plays (thread-safe read)
    int getLatencySamples() const noexcept { return latencyPub.load (std::memory_order_acquire); }
    int latencyForOversampling (int log2) const noexcept { return os.latencyFor (log2); }
    int latencyOf (const Config& c) const noexcept
    {
        return os.latencyFor (c.osLog2) + c.lookahead + (c.limOn ? TruePeakLimiter::latencyFor (c.limLook) : 0);
    }
    int getOversamplingLog2() const noexcept { return curOs.load (std::memory_order_relaxed); }
    double getOversampledRate() const noexcept { return fs * (1 << curOs.load (std::memory_order_relaxed)); }
    const Oversampler& oversampler() const noexcept { return os; }
    const Config& activeConfig() const noexcept { return active; }

    const Meters& getMeters() const noexcept { return meters; }
    // display history (GUI thread): one frame per ~5 ms of audio
    bool popHistory (HistoryFrame& f) noexcept { return history.pop (f); }
    void clearMeters() noexcept { meters.clear(); }
    void setProbe (Probe* p) noexcept { probe = p; }

    //== latency handshake with the wrapper (message thread) ======================
    // The engine never plays a configuration whose latency the host does not compensate. To change the
    // latency it fades out, then posts a request; the wrapper reports the latency to the host and
    // acknowledges; only then the new configuration is applied (warm-up, fade-in). If no report comes
    // within 0.5 s the engine withdraws the request - atomically, so a report can never land afterwards -
    // and fades the old configuration back in (it is what the host still compensates); it tries again
    // once the message thread is alive (heartbeat). Same design as Axell Penta EQ 2.0.1. DECISIONS D23.
    // after prepare: enables the handshake (without it - tests, tools - configurations switch at once)
    void setHostLatency (int samples) noexcept { hostTracking.store (samples >= 0, std::memory_order_release); }
    void heartbeat() noexcept { beat.fetch_add (1, std::memory_order_acq_rel); }
    bool takeLatencyRequest (int& samples) noexcept
    {
        auto r = latReq.load (std::memory_order_acquire);
        if (stateOf (r) != kReqPosted)
            return false;
        if (! latReq.compare_exchange_strong (r, pack (kReqReporting, seqOf (r), latOf (r)), std::memory_order_acq_rel))
            return false; // withdrawn meanwhile
        samples = latOf (r);
        return true;
    }
    void ackLatencyRequest() noexcept
    {
        const auto r = latReq.load (std::memory_order_acquire);
        if (stateOf (r) == kReqReporting)
            latReq.store (pack (kReqAcked, seqOf (r), latOf (r)), std::memory_order_release);
    }

    // io: main bus in place. key: optional external side-chain (keyChannels 1 or 2), not modified.
    void process (float* const* io, int numChannels, int numSamples, const float* const* key = nullptr, int keyChannels = 0) noexcept
    {
        ScopedFlushDenormals ftz;
        const int nCh = std::clamp (numChannels, 0, numCh);
        if (nCh == 0)
            return;
        for (int offset = 0; offset < numSamples; offset += kChunk)
        {
            const int n = std::min (kChunk, numSamples - offset);
            float* ptr[kMaxCh] {};
            const float* kp[kMaxCh] {};
            for (int c = 0; c < nCh; ++c)
                ptr[c] = io[c] + offset;
            const int kc = key != nullptr ? std::clamp (keyChannels, 0, kMaxCh) : 0;
            for (int c = 0; c < kc; ++c)
                kp[c] = key[c] + offset;
            processChunk (ptr, nCh, n, kp, kc);
        }
    }

private:
    static constexpr float kAutoMakeupSec = 2.0f;   // spec: time constant >= 1 s
    static constexpr float kGateLevel = 0.00316f;   // -50 dBFS: hold the GR average in pauses
    static constexpr float kAutoMakeupMaxDb = 24.0f; // never add more than +24 dB automatically

    void applyOversampling (int log2) noexcept
    {
        curOs.store (log2, std::memory_order_relaxed);
        os.reset();
        osKey.reset();
        fet.setOversampling (log2);
        opto.setOversampling (log2);
        stageK = 1.0f - onePole (0.01f, fs * (1 << log2));
    }

    void applyConfig (const Config& c, bool force) noexcept
    {
        if (force || c.osLog2 != curOs.load (std::memory_order_relaxed))
            applyOversampling (c.osLog2);
        laLen = c.lookahead * (1 << c.osLog2);
        for (auto& r : laBuf)
            std::fill (r.begin(), r.end(), 0.0f);
        laPos = 0;
        lim.setLookahead (c.limLook);
        lim.resetState();
        latency = latencyOf (c);
        latencyPub.store (latency, std::memory_order_release);
    }

    bool tracking() const noexcept { return hostTracking.load (std::memory_order_acquire) && ! offline.load (std::memory_order_acquire); }

    // request word: state (3 bits) | sequence (29 bits) | latency (32 bits)
    enum : std::uint64_t { kReqIdle = 0, kReqPosted = 1, kReqReporting = 2, kReqAcked = 3, kReqWithdrawn = 4 };
    static std::uint64_t pack (std::uint64_t st, std::uint64_t seq, int lat) noexcept
    {
        return st | ((seq & 0x1fffffffu) << 3) | ((std::uint64_t) (std::uint32_t) lat << 32);
    }
    static std::uint64_t stateOf (std::uint64_t r) noexcept { return r & 7u; }
    static std::uint64_t seqOf (std::uint64_t r) noexcept { return (r >> 3) & 0x1fffffffu; }
    static int latOf (std::uint64_t r) noexcept { return (int) (std::uint32_t) (r >> 32); }

    // Does the host compensate `need`? Posts a request only while the output is silent (mayPost), takes
    // the acknowledgement, withdraws a request that is stale (the target changed) or unanswered for 0.5 s;
    // after such a timeout the engine holds the configuration it plays until the message thread shows a
    // heartbeat again.
    bool hostReady (int need, bool mayPost, int n) noexcept
    {
        if (reqOpen)
        {
            auto r = latReq.load (std::memory_order_acquire);
            if (seqOf (r) == reqSeq && stateOf (r) == kReqAcked)
            {
                hostLat = latOf (r);
                reqOpen = false;
            }
            else
            {
                reqWait += n;
                const bool stale = latOf (r) != need, late = reqWait >= (int) (0.5 * fs);
                if ((stale || late) && stateOf (r) == kReqPosted
                    && latReq.compare_exchange_strong (r, pack (kReqWithdrawn, reqSeq, latOf (r)), std::memory_order_acq_rel))
                {
                    reqOpen = false;
                    if (late && ! stale)
                    {
                        holding = true;
                        holdBeat = beat.load (std::memory_order_acquire);
                    }
                }
            }
        }
        if (! reqOpen && hostLat != need && mayPost && ! holding)
        {
            reqSeq = (reqSeq + 1) & 0x1fffffffu;
            latReq.store (pack (kReqPosted, reqSeq, need), std::memory_order_release);
            reqOpen = true;
            reqWait = 0;
        }
        return ! reqOpen && hostLat == need;
    }

    // per chunk: structural changes fade out (3 ms), switch at a chunk boundary once the host compensates
    // the new latency, run muted until every delay line holds valid audio, fade back in - no clicks
    void control (int n) noexcept
    {
        const bool track = tracking();
        if (holding && beat.load (std::memory_order_acquire) != holdBeat)
            holding = false; // the message thread is alive again: try once more
        if (! track)
            holding = reqOpen = false;
        const Config tgt = holding ? active : want;
        const bool ready = ! track || hostReady (latencyOf (tgt), fadeGain <= 0.0f, n);
        if (tgt != active)
        {
            fadeTgt = 0.0f;
            if (fadeGain <= 0.0f && ready)
            {
                const bool ms = active.midSide != tgt.midSide;
                active = tgt;
                applyConfig (active, false);
                if (ms)
                {
                    fet.resetState();
                    opto.resetState();
                }
                // the new delay lines / filters hold valid output after `latency` samples; one more host
                // block after the host was told, which it may have started under the old compensation
                warm = latency + (track ? maxBlock : 0);
            }
        }
        else
            fadeTgt = (! ready || warm > 0) ? 0.0f : 1.0f;
        if (warm > 0)
            warm = std::max (0, warm - n);
    }

    void processChunk (float* const* io, int nCh, int n, const float* const* key, int keyCh) noexcept
    {
        // 0. sanitize input, meter, feed the dry delay line; the side-chain input likewise
        for (int c = 0; c < nCh; ++c)
        {
            float* d = dryRing[c].data();
            for (int i = 0; i < n; ++i)
            {
                float v = io[c][i];
                v = std::isfinite (v) ? std::clamp (v, -1000.0f, 1000.0f) : 0.0f;
                io[c][i] = v;
                meters.inPeak[c] = std::max (meters.inPeak[c], std::abs (v));
                d[(writePos + i) & ringMask] = v;
            }
        }
        for (int c = 0; c < nCh; ++c)
        {
            const float* src = keyCh > 0 ? key[std::min (c, keyCh - 1)] : nullptr;
            float* k = keyIn[c].data();
            float* kr = keyRing[c].data();
            for (int i = 0; i < n; ++i)
            {
                float v = src != nullptr ? src[i] : 0.0f;
                v = std::isfinite (v) ? std::clamp (v, -1000.0f, 1000.0f) : 0.0f;
                k[i] = v;
                kr[(writePos + i) & ringMask] = v;
            }
        }

        // 1. structural changes
        control (n);

        // 2. Mid/Side: the stages see M = (L + R) / 2 and S = (L - R) / 2 (key likewise)
        const bool ms = active.midSide && nCh == 2;
        if (ms)
        {
            encodeMs (io[0], io[1], n);
            encodeMs (keyIn[0].data(), keyIn[1].data(), n);
        }

        // 3. oversampled processing
        const bool fetExt = params.fetExt && (fetTarget > 0.0f || fetAmt > 0.0f);
        const bool optoExt = params.optoExt && (optoTarget > 0.0f || optoAmt > 0.0f);
        const bool split = laLen > 0 || fetExt || optoExt;
        const int log2 = curOs.load (std::memory_order_relaxed);
        const int factor = 1 << log2;
        const int nS = n * factor;
        float* hi[kMaxCh] {};
        float* khi[kMaxCh] {};
        for (int c = 0; c < nCh; ++c)
            hi[c] = os.up (io[c], n, c, log2);
        if (fetExt || optoExt)
            for (int c = 0; c < nCh; ++c)
                khi[c] = osKey.up (keyIn[c].data(), n, c, log2);

        float fetPeak = 0.0f, optoPeak = 0.0f;
        float x[kMaxCh] {}, pre[kMaxCh] {}, kc[kMaxCh] {}, ke[kMaxCh] {};
        for (int s = 0; s < nS; ++s)
        {
            for (int c = 0; c < nCh; ++c)
                x[c] = hi[c][s];

            float fg = 0.0f, og = 0.0f;
            if (! split)
            {
                // 1.x path, bit for bit: each stage detects on the signal it processes
                for (int pass = 0; pass < 2; ++pass)
                {
                    const bool isFet = (pass == 0) == (active.order == 0);
                    if (isFet)
                        fg = runStage (fet, fetAmt, fetTarget, x, pre, nCh);
                    else
                        og = runStage (opto, optoAmt, optoTarget, x, pre, nCh);
                }
            }
            else
            {
                // detectors see the undelayed signal (lookahead) or the external key; the internal key of
                // the second stage is the first stage's gain applied to the internal key
                for (int c = 0; c < nCh; ++c)
                {
                    kc[c] = x[c];
                    if (fetExt || optoExt)
                        ke[c] = khi[c][s];
                    if (laLen > 0)
                    {
                        float* b = laBuf[c].data();
                        b[laPos] = x[c];
                        x[c] = b[(laPos - laLen) & laMask];
                    }
                }
                laPos = (laPos + 1) & laMask;
                for (int pass = 0; pass < 2; ++pass)
                {
                    const bool isFet = (pass == 0) == (active.order == 0);
                    if (isFet)
                        fg = runSplit (fet, fetAmt, fetTarget, fetExt ? ke : kc, kc, x, pre, nCh);
                    else
                        og = runSplit (opto, optoAmt, optoTarget, optoExt ? ke : kc, kc, x, pre, nCh);
                }
            }
            fetPeak = std::max (fetPeak, fg);
            optoPeak = std::max (optoPeak, og);
            if (probe != nullptr && probe->count < probe->capacity)
            {
                if (probe->fetGr != nullptr)  probe->fetGr[probe->count] = fg;
                if (probe->optoGr != nullptr) probe->optoGr[probe->count] = og;
                ++probe->count;
            }
            if ((s & (factor - 1)) == factor - 1)
                grBase[(size_t) (s >> log2)] = fg + og;

            for (int c = 0; c < nCh; ++c)
                hi[c][s] = x[c];
        }
        for (int c = 0; c < nCh; ++c)
            os.down (wet[c].data(), n, c, log2);
        if (ms)
            decodeMs (wet[0].data(), wet[1].data(), n);
        meters.fetGrDb = std::max (meters.fetGrDb, fetPeak);
        meters.optoGrDb = std::max (meters.optoGrDb, optoPeak);

        // 4. base rate: auto makeup, latency-aligned dry/wet mix, output gain
        const int Lmix = os.latencyFor (log2) + active.lookahead;
        for (int i = 0; i < n; ++i)
        {
            float inAbs = 0.0f;
            for (int c = 0; c < nCh; ++c)
                inAbs = std::max (inAbs, std::abs (dryRing[c][(size_t) ((writePos + i) & ringMask)]));
            gateEnv = std::max (inAbs, gateEnv * gateRelease);
            if (gateEnv > kGateLevel)
                avgGrDb += (grBase[(size_t) i] - avgGrDb) * avgK;
            makeupDb += ((params.autoMakeup ? std::min (avgGrDb, kAutoMakeupMaxDb) : 0.0f) - makeupDb) * makeupK;
            const float mk = std::abs (makeupDb) > 0.0f ? dbToGain (makeupDb) : 1.0f;

            mixCur += (mixTarget - mixCur) * smoothBase;
            outCur += (outTarget - outCur) * smoothBase;
            snapIfClose (mixCur, mixTarget);
            snapIfClose (outCur, outTarget);
            outGainBuf[(size_t) i] = outCur;
            for (int c = 0; c < nCh; ++c)
            {
                const float dry = dryRing[c][(size_t) ((writePos + i - Lmix) & ringMask)];
                io[c][i] = (dry + (wet[c][(size_t) i] * mk - dry) * mixCur) * outCur;
            }
        }

        // 5. true-peak limiter: the last stage of the processed signal
        float limPeak = 0.0f;
        if (active.limOn)
        {
            limPeak = lim.process (io, nCh, n);
            meters.limGrDb = std::max (meters.limGrDb, limPeak);
        }
        hopFet = std::max (hopFet, fetPeak);
        hopOpto = std::max (hopOpto, optoPeak);
        hopLim = std::max (hopLim, limPeak);

        // 6. side-chain listen, bypass, structural fade (everything aligned to the full latency)
        const bool keyListen = fetExt || optoExt;
        for (int i = 0; i < n; ++i)
        {
            if (listenAmt <= 0.0f && listenTarget > 0.0f)
            {
                listenHpf.resetState();
                listenEq.resetState();
                listenShelf.resetState();
            }
            listenAmt += (listenTarget - listenAmt) * switchK;
            bypassAmt += (bypassTarget - bypassAmt) * switchK;
            snapIfClose (listenAmt, listenTarget);
            snapIfClose (bypassAmt, bypassTarget);
            fadeGain = fadeTgt > fadeGain ? std::min (fadeTgt, fadeGain + fadeStep) : std::max (fadeTgt, fadeGain - fadeStep);

            const int at = (writePos + i - latency) & ringMask;
            cmpAmt += (cmpTarget - cmpAmt) * switchK;
            snapIfClose (cmpAmt, cmpTarget);
            // level-matched compare: K-weighted loudness (0.4 s) of the aligned dry and the processed signal
            double pd = 0.0, pw = 0.0;
            for (int c = 0; c < nCh; ++c)
            {
                const double d = kwDry.process (dryRing[c][(size_t) at], c), w = kwWet.process (io[c][i], c);
                pd += d * d;
                pw += w * w;
                hopIn = std::max (hopIn, std::abs (dryRing[c][(size_t) at]));
            }
            msDry += (pd - msDry) * loudK;
            msWet += (pw - msWet) * loudK;
            if (msDry > 1.0e-7) // above about -70 LUFS: track; in pauses hold the last match
            {
                const float tgt = (float) std::clamp (std::sqrt (msWet / msDry), 0.0631, 15.85); // +-24 dB
                cmpGain += (tgt - cmpGain) * cmpK;
            }
            for (int c = 0; c < nCh; ++c)
            {
                const float dry = dryRing[c][(size_t) at];
                float y = io[c][i];
                if (cmpAmt > 0.0f)
                    y += (dry * cmpGain - y) * cmpAmt;
                if (listenAmt > 0.0f)
                {
                    const float src = keyListen ? keyRing[c][(size_t) at] : dry;
                    const float lis = (float) listenShelf.process (listenEq.process (listenHpf.process (src, c), c), c);
                    y += (lis * outGainBuf[(size_t) i] - y) * listenAmt;
                }
                y += (dry - y) * bypassAmt;
                y *= fadeGain;
                io[c][i] = y;
                meters.outPeak[c] = std::max (meters.outPeak[c], std::abs (y));
                hopOut = std::max (hopOut, std::abs (y));
            }
            if (++hopCount >= hopLen)
            {
                auto toDb = [] (float a) { return a > 1.0e-6f ? gainToDb (a) : -120.0f; };
                history.push ({ toDb (hopIn), toDb (hopOut), hopFet, hopOpto, hopLim });
                hopCount = 0;
                hopIn = hopOut = hopFet = hopOpto = hopLim = 0.0f;
            }
        }
        writePos = (writePos + n) & ringMask;
        meters.makeupDb = makeupDb;
    }

    static void encodeMs (float* l, float* r, int n) noexcept
    {
        for (int i = 0; i < n; ++i)
        {
            const float m = 0.5f * (l[i] + r[i]), s = 0.5f * (l[i] - r[i]);
            l[i] = m;
            r[i] = s;
        }
    }
    static void decodeMs (float* m, float* s, int n) noexcept
    {
        for (int i = 0; i < n; ++i)
        {
            const float l = m[i] + s[i], r = m[i] - s[i];
            m[i] = l;
            s[i] = r;
        }
    }

    // runs a stage with a 10 ms on/off crossfade; a fully-off stage is skipped (and its
    // envelope reset so it starts clean when switched back on)
    template <typename Stage>
    float runStage (Stage& st, float& amt, float target, float* x, float* pre, int nCh) noexcept
    {
        if (amt <= 0.0f && target <= 0.0f)
            return 0.0f;
        if (amt <= 0.0f)
            st.resetState();
        amt += (target - amt) * stageK;
        snapIfClose (amt, target);
        if (amt >= 1.0f)
            return st.processFrame (x, nCh);
        for (int c = 0; c < nCh; ++c)
            pre[c] = x[c];
        const float g = st.processFrame (x, nCh);
        for (int c = 0; c < nCh; ++c)
            x[c] = pre[c] + (x[c] - pre[c]) * amt;
        return g * amt;
    }

    // the same with detection on `key`; `chainKey` (the internal key) is updated with the stage's gain
    template <typename Stage>
    float runSplit (Stage& st, float& amt, float target, const float* key, float* chainKey, float* x, float* pre, int nCh) noexcept
    {
        if (amt <= 0.0f && target <= 0.0f)
            return 0.0f;
        if (amt <= 0.0f)
            st.resetState();
        amt += (target - amt) * stageK;
        snapIfClose (amt, target);
        const float g = st.detectFrame (key, nCh);
        for (int c = 0; c < nCh; ++c)
        {
            chainKey[c] *= 1.0f + (st.stageGain (c) - 1.0f) * amt;
            pre[c] = x[c];
        }
        st.applyFrame (x, nCh);
        if (amt < 1.0f)
            for (int c = 0; c < nCh; ++c)
                x[c] = pre[c] + (x[c] - pre[c]) * amt;
        return g * amt;
    }

    static void snapIfClose (float& v, float target) noexcept
    {
        if (std::abs (v - target) < 1.0e-6f)
            v = target;
    }

    double fs = 44100.0;
    int numCh = 2, maxBlock = 0;
    Params params;
    Oversampler os, osKey;
    FetStage fet;
    OptoStage opto;
    TruePeakLimiter lim;
    std::atomic<int> curOs { 1 };
    Config active, want;
    int latency = 0, maxLa = 441, maxLimLook = 221;
    std::atomic<int> latencyPub { 0 };
    std::atomic<bool> offline { false };
    // latency handshake (see setHostLatency): shared with the wrapper's timer
    std::atomic<bool> hostTracking { false };
    std::atomic<std::uint64_t> latReq { 0 };
    std::atomic<std::uint32_t> beat { 0 };
    // audio thread
    int hostLat = 0, reqWait = 0, warm = 0;
    std::uint64_t reqSeq = 0;
    std::uint32_t holdBeat = 0;
    bool reqOpen = false, holding = false;

    std::vector<float> dryRing[kMaxCh], keyRing[kMaxCh], wet[kMaxCh], keyIn[kMaxCh], laBuf[kMaxCh], grBase, outGainBuf;
    int ringMask = 0, writePos = 0, laMask = 0, laPos = 0, laLen = 0;

    float fetAmt = 1.0f, fetTarget = 1.0f, optoAmt = 1.0f, optoTarget = 1.0f;
    float mixTarget = 1.0f, mixCur = 1.0f, outTarget = 1.0f, outCur = 1.0f;
    float listenTarget = 0.0f, listenAmt = 0.0f, bypassTarget = 0.0f, bypassAmt = 0.0f;
    float smoothBase = 0.001f, switchK = 0.002f, stageK = 0.001f;
    float gateEnv = 0.0f, gateRelease = 0.999f, avgGrDb = 0.0f, avgK = 0.0f, makeupDb = 0.0f, makeupK = 0.0f;
    float fadeGain = 1.0f, fadeTgt = 1.0f, fadeStep = 0.01f;
    Biquad listenHpf, listenShelf;
    SideChainFilter listenEq;
    KWeighting kwDry, kwWet;
    double msDry = 0.0, msWet = 0.0;
    float loudK = 0.001f, cmpK = 0.001f, cmpGain = 1.0f, cmpAmt = 0.0f, cmpTarget = 0.0f;
    HistoryFifo history;
    int hopLen = 240, hopCount = 0;
    float hopIn = 0.0f, hopOut = 0.0f, hopFet = 0.0f, hopOpto = 0.0f, hopLim = 0.0f;
    Meters meters;
    Probe* probe = nullptr;
};
} // namespace duo
