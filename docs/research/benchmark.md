# Benchmark: Axell Duo Compressor vs Universal Audio's UADx emulations (2.1)

Question (2026-10-15): where does Axell Duo Compressor stand against the best-known emulations of the same hardware?
Compared, at the user's request: only Universal Audio's certified UADx plugins (**UADx 1176 FET Compressor**,
**UADx LA-2A Tube Compressor**) and the online hardware references (the recordings in `validation.md`).

**Method.** Every plugin runs as a black box in the same VST3 host (`tests/independent/AuditHost.cpp`; the UADx
plugins through yabridge/Wine), and the same numpy/scipy analysis scores it (`tools/benchmark/bench.py`,
`hw_fidelity.py`, `la2a_onsets.py`). Nothing measured from a UADx plugin was used to tune Axell Duo (clean-room): the
UAD outputs are only compared. Each compressor is set to 4:1 (or its own law), otherwise defaults, and driven by its
own input control to the same gain reduction on a 1 kHz tone at -12 dBFS (6 dB; 10 dB for the stress tests).

## 1. Engineering scorecard (48 kHz unless noted)

| | Axell FET Clean | Axell FET Vintage (default) | UADx 1176 | Axell Opto (Measured) | UADx LA-2A |
|---|---|---|---|---|---|
| THD+N, 1 kHz, 6 dB GR | -148.4 dB | -53.8 dB | -40.3 dB | -54.9 dB | -50.1 dB |
| THD+N, 100 Hz, same setting | -142.7 dB | -50.6 dB | -39.2 dB | -53.8 dB | -37.4 dB |
| Aliasing, 15 kHz -3 dBFS, 10 dB GR, 44.1 kHz | **-161.5 dBFS** | -145.7 dBFS | -76.9 dBFS | **-166.2 dBFS** | -113.8 dBFS |
| Noise with silence in | none (exact 0) | none | none | none | -182.7 dBFS |
| Latency reported / actual | 79 / 79 | 79 / 79 | **0 / 86** | 79 / 79 | **0 / 87** |
| Same input twice / host block sizes 1-4096 | identical / identical | identical / identical | identical / identical | identical / identical | identical / identical |
| Onset overshoot, 10 dB GR | 8.0 dB | 7.9 dB | 10.7 dB | 14.4 dB | 9.6 dB |
| CPU, one core (indicative*) | 2.3 % | 2.2 % | 2.1 % | 1.3 % | 3.2 % |
| True-peak limiter, lookahead, M/S, ext. side-chain, SC EQ | yes | yes | no | yes | no |

Distortion figures of hardware emulations are character, not flaws (the FET Clean column shows what the stage does
without any). *CPU of the UADx plugins includes the Wine bridge; all at the same 48 kHz / 512 setting.
**Latency:** in this host both UADx plugins report 0 samples while delaying the audio by 86-87 samples; a DAW would
play them that much late against other tracks. The reporting may be lost in the yabridge bridge rather than be the
plugins' own; on Windows/macOS it may be correct.

## 2. Fidelity to the real hardware (held-out recordings)

**LA-2A** (SignalTrain Test split, 3 settings, everything outside Axell's calibration blocks). One global output offset
per plugin, by two methods (quiet passages / median level over the calibration frames). UADx Peak Reduction is
reported twice: at the hardware's knob values assuming a linear knob (its displayed values cannot be read through the
bridge), and fitted to the hardware on the calibration blocks (0.35 / 0.35 / 0.50 normalised) - its best case.

| | GR error, music | GR error, noise bursts | Spectral (MR-STFT) | Loudness |
|---|---|---|---|---|
| **Axell Opto, Measured law** | **0.68** / 0.76 dB | 2.45 / 2.30 dB | 0.534 / 0.521 | **0.36** / 0.47 dB |
| UADx LA-2A, knob fitted (best case) | 0.89 / **0.72** dB | **2.26** / 2.33 dB | **0.422** / **0.421** | 0.56 / 0.50 dB |
| UADx LA-2A, knob assumed linear | 8.88 / 3.88 dB | 10.27 / 2.73 dB | 1.112 / 0.548 | 9.95 / 2.20 dB |
| Axell Opto, Classic (1.x) law | 8.52 / 3.62 dB | 7.56 / 4.73 dB | 0.964 / 1.097 | 8.09 / 2.39 dB |

**LA-2A onsets** (3448 level jumps > 10 dB in the noise-burst material, mean gain trajectory over 30 ms): the hardware
over-reacts after short dips (gain reduction 6 -> 14 dB within 30 ms, then settling). UADx LA-2A follows it
(RMS deviation 2.8 dB); Axell's opto does not (6.7 dB). Two refits with an added "onset flare" term reached 1.8-2.5 dB on
onsets but made the overall gain tracking worse (3.7 -> 4.3-5.8 dB on 10 ms frames), so they were **not adopted**
(DECISIONS D28). Open item.

**1176** (Martinez Ramirez et al., ALL / 800 us / 1100 ms, 593 held-out notes, gain-trajectory error):

| | mean | median | 90th percentile |
|---|---|---|---|
| UADx 1176 (attack 1, release 1, ALL) | **1.09 dB** | **1.07 dB** | **1.32 dB** |
| Axell FET Vintage (ALL, 800 us, 1100 ms) | 1.46 dB | 1.45 dB | 1.94 dB |

The FET's ALL-mode law (ratio, knee, attack/release scaling, over-compression) was exposed for calibration on these
notes (only ALL-mode constants, the knobs fixed at the hardware's settings); the fit was stopped before it finished at
the user's request, so the 1.x/2.0 ALL law ships unchanged (DECISIONS D27). Open item.

## 3. Verdict

- **Engineering and features: ahead.** Lower aliasing by 30-85 dB, correct latency reporting, no noise, determinism,
  an optional distortion-free detector, and a true-peak limiter, lookahead, Mid/Side, external side-chain and
  side-chain EQ that neither UADx plugin has.
- **LA-2A fidelity: on par.** Better gain tracking on music and loudness; UAD better on spectral match and on
  onsets after short dips.
- **1176 fidelity: behind** (1.46 vs 1.09 dB at the one setting with public hardware data).

This supports "top-tier engineering, competitive hardware fidelity on the opto, still behind the reference on the
FET". It does not support a "top 5 worldwide" claim on its own: that also needs the listening study
(`tools/listening/`) and closing the two open items above.
