# Engineering decisions

Each entry: what was decided, the measurement that justified it, and any spec deviation.
All numbers come from `DuoDspTests` (see `tests/report.md`; the draft baseline is in `tests/baseline-report.md`).

## D1. The whole signal chain lives in JUCE-free headers
`Source/Engine.h` (+ `Dsp.h`, `Oversampling.h`, `Presets.h`) contains oversampling, both stages,
mix, smoothing, metering and presets. `PluginProcessor` only maps parameters and calls `Engine::process`.
**Why:** the test suite (plain g++, CLAUDE.md rule 3) then measures exactly the code that ships,
including oversampling and mix. The draft kept those in the JUCE processor, where they couldn't be tested.

## D2. Linear-phase FIR oversampling instead of JUCE's IIR halfbands
Polyphase Kaiser halfbands, 1x/2x/4x, integer latency (44.1/48 kHz: 71 samples at 2x, 79 at 4x;
>= 88.2 kHz: 25 / 33). Every filter's stopband is **measured** at design time (the Kaiser length formula
alone gave -97.5 dB for a "100 dB" design) and verified by tests: >= 100 dB stopband, <= 0.0002 dB
passband ripple, wet path flat to 0.0003 dB from 20 Hz to 20 kHz.
**Why:** a linear-phase chain is a pure delay, so the dry path can be delayed by exactly the reported
latency (see D3), and parallel mixing has no phase smear.

## D3. Mix happens at the base rate against an exactly delayed dry signal
The spec suggested keeping the mix in the oversampled domain, or relaxing the null test to -60 dB.
Instead, the dry signal goes through an integer delay line equal to the reported latency and never
passes the filters. **Result:** Mix 0 % is bit-exact with the delayed input (residual 0, i.e. below
-100 dBFS by an unlimited margin). The draft measured -22 dBFS. The Mix 50 % residual against the
delayed dry with the stages off is <= -116 dBFS (<18 kHz), so parallel blends don't comb-filter.
No spec target was relaxed.

## D4. FET detector: max(2 ms peak-hold of |x|, analytic magnitude of the >1 kHz band)
- A plain sample-peak detector (draft) reads a different peak every half cycle when there are few samples
  per cycle (up to -1.3 dB at 15 kHz / 88.2 kHz). The release then makes GR "beat", producing
  non-harmonic sidebands (estimated about -51 dBFS). That fails the 4x aliasing target (-85 dBFS) no matter how
  good the filters are.
- The magnitude of the analytic signal (IIR 90-degree allpass pair, elliptic closed-form design with exact
  elliptic nome) doesn't depend on sample phase. A full-band version (15 Hz edge) was tried first and
  measured: about 1.3 ms of envelope lag at 1 kHz and +6 % overshoot/ringing after a level step. That
  made the 800 us attack measure 1150 us (+44 %).
- Final design: the |x| peak path with a 2 ms hold (instant, gap-free from the first waveform peak), plus the
  analytic magnitude computed only on a 4th-order 1 kHz high-passed copy (12 coefficients, 500 Hz edge,
  phase error <= 0.0005 degrees at 88.2 kHz). That path runs at the base rate and is interpolated (D7).
- **Measured:** attack 20 us -> 20.3-20.6 us, 800 us -> 802-804 us; release 50 ms -> 55.6 ms,
  1100 ms -> 1176 ms (both within +-20 %; the excess is the 2 ms hold plus memory build-up during the
  0.3 s hold). Aliasing (15 kHz, -3 dBFS, 20:1, Color 100) at 44.1 kHz: -84.5 dBFS at 2x
  (target < -70), -165 dBFS at 4x (target < -85).

## D5. Ballistics time convention in the tests
A stage output sample `j` frames after a step represents the state at the end of that sample period,
i.e. time `(j+1)/fs`. This is the zero-order-hold discretisation of a continuous RC, under which a pure
one-pole with time constant tau measures exactly tau. Ballistics are measured on the stage at its 2x
internal rate with a sample-exact step, because a step that has passed the 44.1 kHz input band limit cannot
rise faster than ~23 us, which would make a 20 us attack unmeasurable. Everything else is measured through
the complete engine.

## D6. FET colour = GR-scaled 2nd + 3rd harmonic polynomial (no tanh)
A cubic polynomial can't create harmonics above the 3rd, so the colour itself doesn't feed the aliasing
measured in D4. The even term is DC-blocked (5 Hz) on its own, so the main path stays linear-phase. The amount
rises with GR (0.08 + GR/10) and Color; All-buttons is 2.5x. The amount is capped so the transfer is
provably monotonic. **Measured:** THD at 0 dB GR / Color 0 = 0.000006 %; 10 dB GR / Color 50 = 0.66 %
(target 0.3-1.5 %), 99.8 % of the distortion power is in H2+H3; All mode = 1.11 % (1.68x the 4:1 value).

## D7. Opto detector + ballistics run at the base rate; gain interpolated per oversampled sample
Nothing in an opto moves faster than ~8 ms. Running the detector at the base rate halves the stage's CPU
at 2x, and the opto behaves identically at 1x/2x/4x. The GR (dB, for meters) and the linear gain are
interpolated linearly between control points. **Measured:** release smoothness residual 0.006
(target < 0.01). No stepping (the draft updated the release every 32 samples).

## D8. Opto release model
60 % of the GR follows a 40 ms release and 40 % a light-cell release of 0.5-5 s. The light-cell time grows
with a memory that charges (4 s) from the compression *demand* and discharges over 8 s. A first version
charged from the current GR, so the memory kept charging during the release and lengthened it. The FET
program-dependent release had the same flaw, which measured +20 % at 1100 ms. Both were fixed.
**Measured:** attack 9.85 ms (~10 ms); 50 % recovery after 2 s of 10 dB GR = 71 ms (40-80);
tail time constant 1.9 s (0.5-5); 90 % recovery after 10 s vs 1 s hold = 2.5x (>= 1.5).

## D9. FET All-buttons curve
Ratio 20 with a 2 dB knee, plus an over-compression term above threshold + 10 dB (soft 6 dB knee):
output falls 0.1 dB per dB of further input, capped at 6 dB extra. Attack is 0.6x and release 0.7x the knob
setting (faster, more pumping), and the colour is 2.5x. **Measured:** the curve peaks 8-12 dB above threshold
and drops 0.6-0.7 dB to 0 dBFS input; attack is 0.60x of 20:1 at the same knob.

## D10. Program-dependent FET release
Memory charges over 2 s from the demand (0 at 2 dB, full at 10 dB GR) and slows release by up to +40 %.
**Measured:** 1.29x after 6 s at 16.5 dB GR versus after 0.3 s (target 1.15-1.40).

## D11. Base rates >= 64 kHz use a decimator whose stopband starts at Nyquist
With halfband decimators, anything in the transition band around Nyquist aliases. At 88.2 kHz the 3rd
harmonic of a 15 kHz tone (45 kHz) sits exactly there: measured -65.8 dBFS alias at 43.2 kHz, the only
failure in the extended test matrix (the spec condition, 44.1 kHz, passed). At >= 64 kHz the passband
doesn't need to reach near Nyquist, so the final decimator is a full low-pass with passband 0.30 fs
(26 kHz at 88.2 kHz) and stopband from 0.5 fs (>= 100 dB). **Measured:** 88.2 kHz alias < -150 dBFS.
At 44.1/48 kHz the halfband stays (20 kHz passband is needed there). Bonus: lower latency (25 samples)
and lower CPU at high rates.

## D12. CPU
Initial new-engine build: 2.59 % (target < 2 %). Steps, each measured: control-rate opto (D7) and FET HF
path; channel count as a compile-time constant; no exp for the opto gain; skip log/curve below the knee;
8-coefficient opto Hilbert; SC-listen filters only while listening. **Final: 1.69 %** of one core
(48 kHz / 128 / stereo / 2x, Ryzen 7 7435HS), silence costs 0.82x of program (denormal-safe).

## D13. Oversampling switching
All filters for all factors are prepared up front, so switching is RT-safe. The engine fades out over 3 ms,
switches at a chunk boundary, then fades back in. The new latency is reported to the host from a 20 Hz
message-thread timer: `setLatencySamples` never runs on the audio thread. Tested: random switching gives
no NaN/Inf, peak output stays bounded, and after switching the impulse lands exactly on the reported latency.

## D14. Factory preset gain staging
Presets are tuned with the engine on a vocal-like test signal (-18 dBFS RMS, peaks -4 dBFS).
INPUT sets the FET peak GR (lead ~7 dB, double ~10, parallel smash ~18, gentle ~3, spoken ~5).
FET OUTPUT recovers it the way it's used on a 1176. PEAK REDUCTION sets the average opto GR (3-4.5 dB)
and Opto GAIN makes that up. **Measured:** output within +-0.6 dB of input loudness for every preset at
every rate, so switching presets compares sound, not level.

## D15. Program changes from the audio thread
VST3 hosts can send program changes on the audio thread. `setCurrentProgram` therefore only
touches atomics and parameter values there. The preset *name* (a `juce::String`, which allocates)
is resolved on the message thread by the processor timer. All other access to the name is under
a lock that the audio thread never takes. Tested: a program change from a separate thread applies
its values immediately, and the name follows within one timer tick.

## D16. State restore re-applies every parameter explicitly
pluginval (strictness 10) intermittently reported "FET On not restored" (expected 1, got 0.64).
Cause: `APVTS::replaceState` skips a parameter whose snapped value already equals the stored value,
so a raw host value (0.64 on a bool) survives the restore. Every restore path (session, user preset,
A/B) now re-applies each stored value. Regression test: 50 random rounds of raw-value save/scramble/restore.
It fails without the fix and passes with it. pluginval then passed 3/3 runs with different random seeds.

## D17. Stage-order switch is faded like the oversampling switch
The two orders produce different gain trajectories, so an instant swap can click. A change now
fades out over 3 ms, swaps at a chunk boundary and fades back in. **Measured:** the largest sample
step across the switch is 1.03x the steady tone's own largest step.

## D18. Auto makeup is capped at +24 dB
With extreme settings (~60 dB of GR) uncapped makeup would add 60 dB. **Measured cap:** 23.99 dB.

---------------------------------------------------------------------------------------
# Axell Duo Compressor 2.0 (2026-10-14)

## D19. Clean FET detector = a true 25 ms running maximum
The 1.x detector holds |x| for 2 ms and then re-arms on the current sample. Below ~250 Hz half a cycle is
longer than the hold, so the level ripples at twice the signal frequency and fast settings modulate the gain:
THD+N -43.5 dB at 30 Hz and -46.4 dB at 100 Hz (4:1, 20 us / 50 ms, ~6 dB GR, Color 0). That is part of the
hardware's character and stays as **Vintage**. **Clean** replaces the hold by a running maximum over 25 ms
(monotonic deque, amortised O(1), allocated in prepare): for any steady tone down to 20 Hz the maximum over the
window is constant, so the gain does not ripple, while a transient still registers on its first sample
(attack unchanged; the onset peak differs from Vintage by 0.000 dB). The >1 kHz analytic (Hilbert) term of the
1.x detector is kept (sample-phase independence at high frequencies). Cost: the release starts up to 25 ms later.
Measured: THD+N -142 dB at 30 and 100 Hz; static curve within 0.006 dB of Vintage. A full-band analytic-signal
detector was considered first and rejected: its group delay at low frequencies lags transients (extra overshoot).

## D20. 4x is the default oversampling; old states keep theirs
4x gives -149.5 dBFS worst aliasing at 15 kHz / -3 dBFS / 20:1 / +24 dB / Color 100 (2x: -77 dBFS), at 79
samples of latency (48 kHz) and ~1.7x the CPU of 2x. New instances and all factory presets use 4x. A saved
state that has no "os" value (pre-1.0 drafts) gets 2x, the value it played with (upgradeLegacyState); every
1.0.0 session stores its value and keeps it. No other 2.0 parameter needs this: their defaults are the 1.x
behaviour.

## D21. True-peak limiter
Base rate, stereo-linked, at the end of the processed path (after mix and output gain; bypass and SC listen are
aligned around it). Detector: 8x polyphase interpolation, Kaiser-windowed sinc (beta 8, 8 phases x 128 taps)
plus parabolic refinement of every local maximum; the 8 phases are computed as one vector multiply-add per tap.
Interpolation error <= 0.003 dB up to 0.48 fs (21.2 kHz at 44.1 kHz, 23.1 kHz at 48 kHz) against an exact
band-limited reference (64 taps would reach only 0.44 fs). Gain computer: required gain min(1, ceiling / peak)
over (b - 1, b + 1) -> running minimum over the lookahead L -> release (rises with the release time constant,
falls instantly) -> 3 cascaded moving averages with combined support L. Every value entering the averages for
sample b is <= r[b], so the gain never exceeds what b and its neighbourhood need. Latency = 64 + L samples.
A fixed 0.05 dB design margin sits under the ceiling (measured use: ~0.015 dB, from the gain modulation of
band-limited material).
**Guarantee and its limit:** no sample peak above the ceiling, ever; no true peak above the ceiling for content
up to 0.48 fs (DSP suite: drum-bus program and square bursts band-limited to 0.48 fs, +12 dB into -1 dBTP:
-1.049 / -1.050 dBTP against an independent +-512-tap sinc reference; black-box audit with FFT reconstruction:
-1.048 / -1.049 dBTP at 44.1 / 48 kHz). Content between 0.48 fs and Nyquist cannot be predicted exactly by any
low-latency interpolator (its transition band would have to be arbitrarily narrow). With full-band synthetic
drums (white-noise snare and hats at full scale up to Nyquist) the measured worst true peak is +0.31 dB over the
ceiling at 2x/4x (the oversampling filters remove most of that band) and +0.61 dB at 1x; documented bounds in
the tests: +0.4 dB and +0.7 dB. Rejected: a 0.35 dB margin (costs loudness on every signal), a guard low-pass on
the output (adds latency and changes the audio above 20 kHz). Detector length: going from 64 to 128 taps moved
the accurate band from 0.44 to 0.48 fs and the worst detector error on full-band white noise from -0.59 to
-0.35 dB (prototype); longer filters were not tried - each doubling adds latency for an ever narrower band.

## D22. Detection split from the gain stage (lookahead, external key)
Each stage now has detectFrame(key) and applyFrame(audio); processFrame() = both on the same signal, which is
what 1.x did. The engine uses the 1.x path bit for bit whenever lookahead is 0 and no stage listens to the
external key (black-box audit: four 1.0.0 configurations render with difference exactly 0). Otherwise the
detectors see the undelayed oversampled signal (or the oversampled side-chain input, upsampled by its own
identical oversampler so it stays aligned) and the audio is delayed by the lookahead at the oversampled rate.
In series the second stage's internal key is the internal key times the first stage's gain (input, GR, output)
- without the first stage's Color harmonics, which are at least 40 dB down and irrelevant to a detector.
Lookahead 0-10 ms (latency + L); measured: a 20:1 / 800 us FET overshoots a tone onset by 23.4 dB without,
1.3 dB with 2 ms lookahead.

## D23. Latency handshake
Lookahead, limiter on/off, limiter lookahead and oversampling change the latency. Ported from Axell Penta EQ
2.0.1: the engine fades out (3 ms), posts a request (atomic word: state | sequence | latency), the wrapper's 20 Hz
timer reports it to the host and acknowledges; only then the new configuration is applied, run muted until every
delay line holds valid audio (latency + one host block) and faded in. Unanswered for 0.5 s: the request is
withdrawn atomically and the old configuration (what the host still compensates) plays until the message thread
shows a heartbeat again. Offline renders and tools switch at once. Black-box audit in a real-time host: 8 latency
changes (0 / 79 / 136 / 215 / 223 / 359 / 439 samples), 0 misaligned 16-sample windows.

## D24. Opto Model: Measured (hardware-calibrated) and Classic
The 1.x opto law missed the hardware LA-2A by 10-17 dB at the same knob numbers (its Peak Reduction scale was far
hotter) and its Limit mode (10:1) was much steeper than the hardware's (~3.6:1, hard knee). One law fitted jointly to
the three SignalTrain settings (docs/research/validation.md) is shipped as `measuredModel()`; it is the default for
new instances and all factory presets (Peak Reduction retuned per preset for the same opto GR on the suite's signals,
tools/retune_presets.cpp). The Peak Reduction default stays 30 (frozen 1.x default: a restored 1.x value equal to the default is not re-snapped, so 1.x sessions stay bit-identical; a default of 65 made PR 30 sessions differ by 1e-7, found by the audit). States without "optoModel" load Classic (upgradeLegacyState), so
1.x projects sound unchanged. The FET law was not changed: the single available 1176 setting cannot separate the
candidate causes (validation.md).

## D25. Mid/Side
Channel mode Stereo / Mid-Side (structural, faded): M = (L + R) / 2, S = (L - R) / 2 before the oversampler,
decoded after it; the external key is encoded the same way; dry, bypass, listen and the limiter stay L/R.
Encode + decode is exact for identical channels (difference 0) and below -130 dBFS otherwise. Stereo Link applies
between M and S.

## D26. Threshold, knee, range, hold, side-chain EQ
Per stage; defaults reproduce 1.x exactly (FET threshold -12 dB, opto -10 dB; knee Auto = 1.x ratio-dependent
knee; range 60 dB = Off; hold 0). Hold: while the demand stays within 0.05 dB of the current GR the hold timer
restarts; the release begins `hold` after the demand fell (restarting only on rising demand failed once the GR had
converged, found by the DSP suite). Side-chain EQ (both detectors): the 1.x HPF plus a 12 dB/oct LPF (20 kHz = Off)
and a bell (0 dB = Off); filters that are off are not run, so the default detector is the 1.x one.

---------------------------------------------------------------------------------------
# 2.1 (2026-10-15)

## D27. FET ALL law exposed for calibration, not yet calibrated
The ALL ("all buttons") constants (ratio 20, knee 2, attack x0.6, release x0.7, over-compression above threshold +10 dB)
were 1.0 estimates. They are now a model (`FetStage::Settings::AllModel`; defaults = 1.x) so they can be fitted to the
hardware 1176 notes with the knobs fixed at the hardware's settings (tools/validation/fet1176_all_fit.py). The fit was
stopped before it finished (user's decision to ship 2.1 without waiting); the law ships unchanged. The benchmark
shows the gap it would address: 1.46 dB vs 1.09 dB (UADx 1176) on held-out notes.

## D28. Opto onset flare: explored, not adopted
At level jumps after short dips the hardware LA-2A's gain reduction overshoots (6 -> 14 dB within 30 ms, then settles);
the Measured law does not (mean-trajectory deviation 6.7 dB; UADx LA-2A 2.8 dB). A transient term
(`onsetGain * max(0, fast(target) - slow(target))`) was fitted twice: free release law -> onsets 1.8-2.5 dB but the
10 ms gain tracking 3.7 -> 5.4-5.8 dB; release law fixed -> onsets 4.6-4.7 dB, gain tracking 4.3-4.5 dB. The rule set
before the fit (ship only if onsets improve and the overall tracking does not degrade) was not met. The term stays in
the model at 0 (no effect) for future work on a physical light-cell model.

## D29. Level-matched COMPARE
The aligned dry and the processed signal are K-weighted (BS.1770) and their mean square tracked over 0.4 s; the dry
signal is played with gain sqrt(ms_wet / ms_dry) (+-24 dB, smoothed 0.25 s, held in pauses below about -70 LUFS),
crossfaded in 10 ms. Louder is judged better; compare removes that bias (DSP suite: 0.09 dB loudness difference with
7 dB of make-up in play). A parameter (like SC Listen), not part of presets.

## D30. Display history and transfer curve
The engine pushes one frame per 5 ms (input and output peak, GR per stage) into a lock-free single-producer /
single-consumer ring; the GUI drains it at 30 Hz. The transfer curve is computed in the GUI from the stages' own static
curve functions (`staticGainReductionDb`) on GUI-owned stage objects - no shared state with the audio thread.

## D31. VOICE HOLD (3.0)
Problem: a compressor releases in every pause, so with 10-15 dB of gain reduction the room, breaths and headphone bleed
between phrases come up by the same amount (measured on a real gated rap take: pauses +23.4 dB against the voice).
A gate in front cannot fix it (its range is finite and it opens late on word starts). Solution: the Axell suite's voice
decision (Axell Gate 2.0's network, room veto and voice/pause model, in axell-core, bit-identical to Gate's, causal
with a 10 ms decision delay) marks pauses; in a pause the FET's release stops and its gain reduction glides (12.5 ms)
to the voice's recent average (0.25 s), the opto's fast part does the same and its slow memory only holds (gliding it
too carried the pause into the next phrase: +1 dB of level spread on the voice, measured). Result on the take: pauses
+5.9 dB, voice gain reduction unchanged. A pause counts only within 10 s of a voice (a long instrumental part
releases normally). Limits: the voice network never saw drums and calls a drum loop 78 % voice, so VOICE HOLD is a
vocal feature - the beat, bus, master and ducking presets turn it off. The 10 ms delay means the first 10 ms of a word
start under the held (average) gain reduction instead of the released one, which is the point. Off in states saved
before 3.0 (upgradeLegacyState).

## D32. Axell's Dojo look (3.0)
Shared with every Axell plugin through axell-core: the family emblem, the gold header rule, the title in Yuji Syuku
(a brush-written face, SIL OFL 1.1, subset to 52 KB and embedded) with the plugin's hanko seal (双, "pair"), and the
plugin's own ground - every surface colour derived from one hue (Duo: 15 degrees, ember). Text, meters and stage
colours are unchanged, so every reading keeps its contrast.


## D33. ESS HOLD (4.0)
Problem: a compressor lifts the esses above the vowels. Between a vowel and its ess the detector sees less level, the
release starts, and the ess (and the make-up gain) come up against the word it belongs to. The suite's own vocal
chains, rendered stage by stage on 28 of the artist's raw takes (the Dojo's chains through its batch renderer), showed
Duo as the largest single source of hot esses: the esses' level over the vowels of their own word (the loudest vowel
frame within +-150 ms) rose +3.28 dB in the "Amber" chain, +1.42 dB in "Rap Lead" and +1.35 dB in "Boom Bap Lead"
(medians over the takes). Velvet after it then had to cut that much more, and every compressor or limiter after
Velvet (a vocal bus, a master) added more again - in the artist's own session the esses ended 8 dB hotter than in the
dry take.

Solution: the suite's sibilance network (Axell Velvet 3.0's LIVE detector in axell-core, causal, 12 ms frames every
2 ms, its decision 6 ms behind) marks the esses. Through an ess both stages stop releasing and the gain reduction
glides (12.5 ms) up to the word's own: its recent peak reduction (a peak hold decaying with 0.3 s). Meanwhile a shadow
plays the 3.x path (same attack, release and hold); when the ess ends the gain reduction glides back to the shadow in
10 ms, so the next vowel is compressed exactly as in 3.x. The flag follows the detector's 2 ms decision directly.

Tuning (all on the 28 takes, the three chains' Duo stages, A/B against 3.0.0 on identical inputs):
- threshold 0.3 / 0.5 / 0.7 x word window 0.08 / 0.15 / 0.3 s: the longer window removes more of the lift (Amber
  +1.24 -> +0.70 dB at 0.5) but held the gain into the next vowel (vowels within 120 ms after an ess -0.9 dB);
- the shadow and 10 ms return took that to -0.39 dB; a 20 ms smoothing of the flag was the rest (it held into the
  vowel): without it -0.11 dB. Chosen: 0.5, 0.3 s, 10 ms, no flag smoothing.

| chain (Duo stage) | ess lift over its word, 3.0 -> 4.0 | vowels | vowels just after esses | pauses (absolute) |
|---|---|---|---|---|
| Amber | +3.28 -> +0.60 dB | -0.02 dB | -0.11 dB | unchanged |
| Rap Lead | +1.42 -> -0.09 dB | -0.01 dB | -0.18 dB | unchanged |
| Boom Bap Lead | +1.35 -> +0.03 dB | -0.05 dB | -0.07 dB | unchanged |

True peak, loudness range, tone (fundamental band, 5-16 kHz) unchanged within 0.05 dB. DSP suite (synthetic voice):
88-100 % of the lift removed at 44.1-96 kHz, vowels +0.03..+0.08 dB.

Off by default in the engine; on for new instances and the presets where VOICE HOLD is on (vocal presets, Init); off
for beat, bus, master and ducking presets - the network fires on hi-hats on 0.2-4 % of a beat's 2 ms frames (17 of the
artist's beats), and on 8 beats the three bus presets' output moved by -31..-50 dB with it (0.02 dB of level): small,
but a beat should not change at all. Projects saved before 4.0 load with it off (upgradeLegacyState) and render
bit-identically to 3.0.0 (independent audit). No latency; the detector adds a few % of one core at 48 kHz.
