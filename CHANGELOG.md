# Changelog

## 2.1.0 (2026-10-15)

### New
- **Level / gain-reduction history** (last 5 s): input and output level with the gain reduction of the FET, opto and
  limiter drawn from the top, and a **transfer curve** of the whole chain at the current settings with your live level.
- **COMPARE** (level-matched compare): plays the unprocessed track at the processed track's loudness (K-weighted,
  BS.1770), so you judge what the compression does, not the level change. Matches within 0.1 dB in the test suite.

### Verified
- **Benchmark against Universal Audio's UADx 1176 and UADx LA-2A** (black box, same host and analysis;
  `docs/research/benchmark.md`): 30-85 dB less aliasing, correct latency reporting (both UADx plugins report 0 while
  delaying the audio by 86-87 samples in this host), no noise, deterministic. Fidelity to the hardware recordings:
  LA-2A on par (gain tracking on music 0.68 vs 0.89 dB, loudness 0.36 vs 0.56 dB; UAD better on spectral match and
  onsets), 1176 behind (1.46 vs 1.09 dB). Open items listed in the report.
- DSP suite 2596 checks, plugin suite 125, black-box audit 26/26, pluginval strictness 10 + VST3 validator.

## 2.0.0 — Axell Duo Compressor (2026-10-14)

Duo Vocal Comp becomes **Axell Duo Compressor**. Same plugin identity: projects saved with Duo Vocal Comp 1.0 open
with every setting and render bit-identically (verified through a VST3 host). The installer replaces the old bundle.

### New
- **True-peak limiter** at the end of the chain: ceiling -24…0 dBTP, release, lookahead 0.5–5 ms, 8x true-peak
  detection, own GR meter. No sample peak ever passes the ceiling; no true peak does for content up to 0.48 fs
  (see README, "The limiter's limits").
- **Clean FET detector**: same attack and curve as the classic (Vintage) detector without its low-frequency ripple:
  THD+N -142 dB at 30 and 100 Hz with the fastest settings (Vintage: -43 / -46 dB).
- **Threshold, knee, range and hold** for both stages (defaults = the 1.x behaviour).
- **Side-chain EQ** (LPF and bell, besides the HPF) and an **external side-chain input** with a key source per stage.
- **Compressor lookahead** 0–10 ms.
- **Mid/Side** channel mode.
- **Opto Model: Measured** - the opto law calibrated on recordings of a hardware LA-2A (knob scale, ratio, knee,
  light-cell timing). Default for new instances and all factory presets; **Classic** is the 1.x law (1.x projects load
  with it). With Measured, vocals typically want Peak Reduction 55-70 (factory presets retuned).
- 2.0 presets: True-Peak Safety (-1 dBTP), Bass & 808 Clean Control, Mid/Side Glue, Lookahead Peak Control,
  Duck to Side-Chain.
- New layout: FET | OPTO | LIMITER, SIDE-CHAIN and GLOBAL panels; tooltips for every control.

### Changed
- **4x oversampling is the default** (aliasing -149.5 dBFS at heavy drive). Saved projects keep their own setting.
- Factory presets use 4x.
- Latency changes (oversampling, lookahead, limiter) take effect only after the DAW was told the new latency;
  the audio fades out and back in around the switch.
- User presets live in `~/.local/share/AxellDuoCompressor/Presets`; Duo Vocal Comp presets are still listed.

### Validation
- LA-2A (SignalTrain, held-out audio): GR envelope error 0.60 dB on music; STFT 0.449 and loudness 0.43 dB vs
  0.551-0.796 / 0.36-1.31 dB for published neural models. 1176 (one setting, 593 held-out notes): trajectory error
  1.45 dB. Blind ABX / MUSHRA kit in `tools/listening/`. Report: `docs/research/validation.md`.
- DSP suite 2537 checks, plugin suite 123, ASan/UBSan clean, pluginval strictness 10 + VST3 validator, independent
  black-box audit through a VST3 host (incl. bit-identical 1.0.0 sessions).

## 1.0.0 — Duo Vocal Comp (2026)
First release: FET and opto stages, order swap, side-chain HPF, Color, Mix, stereo link, HF emphasis, 1x/2x/4x
oversampling, auto makeup, 22 factory presets, A/B compare.
