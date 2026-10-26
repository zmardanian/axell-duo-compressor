# Axell Duo Compressor - DSP test report

Generated 2026-10-26 21:18 by `DuoDspTests` in 155.3 s. Stage ballistics are measured on the stage at the default 2x internal rate with a sample-exact step; everything else runs through the complete engine (oversampling, mix, output). Build: Release.

**Result: ALL PASS** - 2211 checks passed, 0 failed (every check runs once per sample rate / channel configuration).

## Summary (worst case over all configurations)

| Section | Check | Target | Worst measured | at | Pass |
|---|---|---|---|---|---|
| 1 Static curve | FET 4:1: slope below threshold (-60..-44 dBFS) | 0.9800 .. 1.020 dB/dB | 1.0000 dB/dB | 96 k mono | PASS 10/10 |
| 1 Static curve | FET 4:1: ratio above threshold (-12..0 dBFS in) | 3.600 .. 4.400 :1 | 4.013 :1 | 96 k mono | PASS 10/10 |
| 1 Static curve | FET 8:1: slope below threshold (-60..-44 dBFS) | 0.9800 .. 1.020 dB/dB | 1.0000 dB/dB | 96 k mono | PASS 10/10 |
| 1 Static curve | FET 8:1: ratio above threshold (-12..0 dBFS in) | 7.200 .. 8.800 :1 | 8.134 :1 | 96 k mono | PASS 10/10 |
| 1 Static curve | FET 12:1: slope below threshold (-60..-44 dBFS) | 0.9800 .. 1.020 dB/dB | 1.0000 dB/dB | 96 k mono | PASS 10/10 |
| 1 Static curve | FET 12:1: ratio above threshold (-12..0 dBFS in) | 10.800 .. 13.200 :1 | 12.274 :1 | 96 k mono | PASS 10/10 |
| 1 Static curve | FET 20:1: slope below threshold (-60..-44 dBFS) | 0.9800 .. 1.020 dB/dB | 1.0000 dB/dB | 96 k mono | PASS 10/10 |
| 1 Static curve | FET 20:1: ratio above threshold (-12..0 dBFS in) | 18.000 .. 22.000 :1 | 19.124 :1 | 192 k mono | PASS 10/10 |
| 1 Static curve | FET All: slope below threshold (-60..-44 dBFS) | 0.9800 .. 1.020 dB/dB | 1.0000 dB/dB | 96 k mono | PASS 10/10 |
| 1 Static curve | FET All: over-compression, drop from curve peak to 0 dBFS in | > 0.3000 dB | 0.7314 dB | 96 k mono | PASS 10/10 |
| 1 Static curve | FET All: curve peak position above threshold | 4.000 .. 16.000 dB | 8.000 dB | 44.1 k mono | PASS 10/10 |
| 1 Static curve | FET All: drop stays 'slight' (peak to 0 dBFS in) | < 4.000 dB | 0.7479 dB | 192 k mono | PASS 10/10 |
| 1 Static curve | Opto Compress: slope below threshold (-60..-44 dBFS) | 0.9800 .. 1.020 dB/dB | 1.0000 dB/dB | 48 k mono | PASS 10/10 |
| 1 Static curve | Opto Compress: ratio above threshold (-12..0 dBFS in) | 2.700 .. 3.300 :1 | 2.999 :1 | 192 k mono | PASS 10/10 |
| 1 Static curve | Opto Limit: slope below threshold (-60..-44 dBFS) | 0.9800 .. 1.020 dB/dB | 1.0000 dB/dB | 48 k mono | PASS 10/10 |
| 1 Static curve | Opto Limit: ratio above threshold (-12..0 dBFS in) | 9.000 .. 11.000 :1 | 10.000 :1 | 96 k mono | PASS 10/10 |
| 2 FET attack/release | attack 20.000 us: time to 63 % GR | 16.000 .. 24.000 us | 20.642 us | 44.1 k mono | PASS 10/10 |
| 2 FET attack/release | attack 800.0 us: time to 63 % GR | 640.0 .. 960.0 us | 803.8 us | 48 k mono | PASS 10/10 |
| 2 FET attack/release | release 50.000 ms: time to 63 % recovery | 40.000 .. 60.000 ms | 55.574 ms | 88.2 k mono | PASS 10/10 |
| 2 FET attack/release | release 1100.0 ms: time to 63 % recovery | 880.0 .. 1320.0 ms | 1173.4 ms | 192 k mono | PASS 10/10 |
| 2 FET attack/release | program-dependent: GR during hold | (info) | 16.507 .. 16.532 dB | all | - |
| 2 FET attack/release | program-dependent: release after 6 s vs 0.3 s of >10 dB GR | 1.150 .. 1.400 x | 1.295 x | 48 k mono | PASS 10/10 |
| 2 FET attack/release | All mode: attack time relative to 20:1 (800 us knob) | < 0.8000 x | 0.6015 x | 192 k mono | PASS 10/10 |
| 3 Opto attack/release | attack: time to 63 % of 10 dB GR | 8.000 .. 12.000 ms | 9.771 ms | 192 k mono | PASS 10/10 |
| 3 Opto attack/release | release after 2 s of 10 dB GR: 50 % recovery | 40.000 .. 80.000 ms | 71.260 ms | 96 k mono | PASS 10/10 |
| 3 Opto attack/release | release tail time constant after 2 s (70->90 % recovery) | 0.5000 .. 5.000 s | 1.926 s | 48 k mono | PASS 10/10 |
| 3 Opto attack/release | 90 % recovery after 1 s of 10 dB GR | (info) | 1.918 .. 1.931 s | all | - |
| 3 Opto attack/release | 90 % recovery after 10 s of 10 dB GR | (info) | 4.864 .. 4.874 s | all | - |
| 3 Opto attack/release | light-cell memory: 90 % recovery 10 s hold / 1 s hold | > 1.500 x | 2.523 x | 88.2 k mono | PASS 10/10 |
| 3 Opto attack/release | release smoothness (slope residual vs slope, 10 s hold) | < 0.0100 ratio | 0.0055 ratio | 192 k mono | PASS 10/10 |
| 4 THD (1 kHz) | FET 0 dB GR, Color 0 | < 0.1000 % | 5.1e-06 % | 48 k mono | PASS 10/10 |
| 4 THD (1 kHz) | FET 10 dB GR, Color 50 (4:1) | 0.3000 .. 1.500 % | 0.6619 % | 192 k mono | PASS 10/10 |
| 4 THD (1 kHz) | FET 10 dB GR, Color 50: share of 2nd+3rd in distortion power | > 80.000 % | 99.830 % | 44.1 k mono | PASS 10/10 |
| 4 THD (1 kHz) | FET All 10 dB GR, Color 50 | (info) | 1.107 .. 1.110 % | all | - |
| 4 THD (1 kHz) | FET All vs 4:1 THD at 10 dB GR, Color 50 | > 1.250 x | 1.673 x | 96 k mono | PASS 10/10 |
| 4 THD (1 kHz) | Opto 10 dB GR, Color 0 (clean) | < 0.1000 % | 4.67e-05 % | 96 k mono | PASS 10/10 |
| 4 THD (1 kHz) | Opto 10 dB GR, Color 50 | (info) | 0.1348 .. 0.1348 % | all | - |
| 4 THD (1 kHz) | Opto Color 50: H2 / H3 (predominantly 2nd) | > 2.000 x | 118.6 x | 48 k mono | PASS 10/10 |
| 4 THD (1 kHz) | Opto Color 50: THD rises with level (0 dB vs -20 dB rel.) | > 1.300 x | 3.169 x | 96 k mono | PASS 10/10 |
| 4 THD (1 kHz) | Opto Color 100: DC offset | < -100.0 dBFS | -176.7 dBFS | 192 k mono | PASS 10/10 |
| 5 Aliasing (15 kHz, -3 dBFS, FET 20:1, Color 100) | FET only, 1x (no oversampling) | (info) | -117.4 .. -51.385 dBFS | all | - |
| 5 Aliasing (15 kHz, -3 dBFS, FET 20:1, Color 100) | FET only, 2x | < -70.000 dBFS | -84.383 dBFS | 48 k mono | PASS 10/10 |
| 5 Aliasing (15 kHz, -3 dBFS, FET 20:1, Color 100) | FET only, 4x | < -85.000 dBFS | -154.2 dBFS | 48 k mono | PASS 10/10 |
| 5 Aliasing (15 kHz, -3 dBFS, FET 20:1, Color 100) | FET + Opto (default), 1x (no oversampling) | (info) | -124.4 .. -54.586 dBFS | all | - |
| 5 Aliasing (15 kHz, -3 dBFS, FET 20:1, Color 100) | FET + Opto (default), 2x | < -70.000 dBFS | -91.428 dBFS | 48 k mono | PASS 10/10 |
| 5 Aliasing (15 kHz, -3 dBFS, FET 20:1, Color 100) | FET + Opto (default), 4x | < -85.000 dBFS | -135.6 dBFS | 44.1 k mono | PASS 10/10 |
| 6 Null / latency | Mix 0 %: residual vs input delayed by reported latency, 1x | < -100.0 dBFS | -600.0 dBFS | 44.1 k mono | PASS 10/10 |
| 6 Null / latency | reported latency matches factor, 1x | = 0.0000 samples | 0.0000 samples | 44.1 k mono | PASS 10/10 |
| 6 Null / latency | wet impulse peak offset - reported latency, 1x | = 0.0000 samples | 0.0000 samples | 44.1 k mono | PASS 10/10 |
| 6 Null / latency | Mix 50 %, stages off: residual vs delayed dry (<18 kHz), 1x | < -80.000 dBFS | -600.0 dBFS | 44.1 k mono | PASS 10/10 |
| 6 Null / latency | Mix 0 %: residual vs input delayed by reported latency, 2x | < -100.0 dBFS | -600.0 dBFS | 44.1 k mono | PASS 10/10 |
| 6 Null / latency | reported latency matches factor, 2x | = 0.0000 samples | 0.0000 samples | 44.1 k mono | PASS 10/10 |
| 6 Null / latency | wet impulse peak offset - reported latency, 2x | = 0.0000 samples | 0.0000 samples | 44.1 k mono | PASS 10/10 |
| 6 Null / latency | Mix 50 %, stages off: residual vs delayed dry (<18 kHz), 2x | < -80.000 dBFS | -126.7 dBFS | 96 k mono | PASS 10/10 |
| 6 Null / latency | Mix 0 %: residual vs input delayed by reported latency, 4x | < -100.0 dBFS | -600.0 dBFS | 44.1 k mono | PASS 10/10 |
| 6 Null / latency | reported latency matches factor, 4x | = 0.0000 samples | 0.0000 samples | 44.1 k mono | PASS 10/10 |
| 6 Null / latency | wet impulse peak offset - reported latency, 4x | = 0.0000 samples | 0.0000 samples | 44.1 k mono | PASS 10/10 |
| 6 Null / latency | Mix 50 %, stages off: residual vs delayed dry (<18 kHz), 4x | < -80.000 dBFS | -121.3 dBFS | 44.1 k mono | PASS 10/10 |
| 7 Silence / stability | zeros after +24 dBFS noise: max |out| after 1 s of silence | < 1e-07  | 4.01e-17  | 44.1 k mono | PASS 10/10 |
| 7 Silence / stability | zeros from fresh instance: max |out| | < 1e-07  | 0.0000  | 44.1 k mono | PASS 10/10 |
| 7 Silence / stability | random automation every block, +24 dBFS: all samples finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 10/10 |
| 7 Silence / stability | random automation: peak output (sanity bound) | < 80.000 dBFS | 39.083 dBFS | 88.2 k mono | PASS 10/10 |
| 7 Silence / stability | random automation then silence: max |out| in last 0.5 s | < 1e-07  | 1.52e-23  | 48 k mono | PASS 10/10 |
| 8 Block-size invariance | max |diff| blocks 1/17/64/2048/4096 vs 512, 1x | < 1e-05  | 0.0000  | 44.1 k mono | PASS 10/10 |
| 8 Block-size invariance | max |diff| blocks 1/17/64/2048/4096 vs 512, 2x | < 1e-05  | 0.0000  | 44.1 k mono | PASS 10/10 |
| 8 Block-size invariance | max |diff| blocks 1/17/64/2048/4096 vs 512, 4x | < 1e-05  | 0.0000  | 44.1 k mono | PASS 10/10 |
| F Features (spec section 2) | FET SC HPF: GR reduction on 60 Hz, HPF 20 -> 300 Hz | > 6.000 dB | 8.908 dB | 44.1 k mono | PASS 10/10 |
| F Features (spec section 2) | Opto SC HPF: GR reduction on 60 Hz, HPF 20 -> 300 Hz | > 6.000 dB | 17.625 dB | 44.1 k mono | PASS 10/10 |
| F Features (spec section 2) | SC Listen (HPF 300 Hz): 50 Hz relative to 3 kHz in output | < -20.000 dB | -31.129 dB | 192 k mono | PASS 10/10 |
| F Features (spec section 2) | Emphasis 100 %: extra opto GR on 8 kHz | > 4.000 dB | 7.726 dB | 192 k mono | PASS 10/10 |
| F Features (spec section 2) | Emphasis 100 %: GR change on 200 Hz (should be ~0) | < 0.5000 dB | 0.000188 dB | 192 k mono | PASS 10/10 |
| F Features (spec section 2) | Auto makeup: final makeup vs average GR (|diff|) | < 1.500 dB | 0.4186 dB | 192 k mono | PASS 10/10 |
| F Features (spec section 2) | Auto makeup: time to 63 % of final makeup (>= 1 s) | 1.000 .. 6.000 s | 2.235 s | 192 k mono | PASS 10/10 |
| F Features (spec section 2) | Auto makeup: cap under extreme GR | < 24.001 dB | 23.987 dB | 44.1 k mono | PASS 10/10 |
| F Features (spec section 2) | OS switching every 37 blocks: all samples finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 10/10 |
| F Features (spec section 2) | OS switching: peak output vs peak input | < 12.000 dB | -7.057 dB | 44.1 k mono | PASS 10/10 |
| F Features (spec section 2) | OS switched to 4x: reported latency - latency of 4x | = 0.0000 samples | 0.0000 samples | 44.1 k mono | PASS 10/10 |
| F Features (spec section 2) | OS switched to 4x: impulse peak - reported latency | = 0.0000 samples | 0.0000 samples | 44.1 k mono | PASS 10/10 |
| F Features (spec section 2) | Order switch while playing: max sample step vs steady tone | < 1.500 x | 1.030 x | 96 k mono | PASS 10/10 |
| F Features (spec section 2) | Output +12 dB step: 10-90 % transition | > 5.000 ms | 44.000 ms | 44.1 k mono | PASS 10/10 |
| F Features (spec section 2) | FET Output -20 dB step: 10-90 % transition | > 5.000 ms | 44.000 ms | 44.1 k mono | PASS 10/10 |
| F Features (spec section 2) | Opto Gain +12 dB step: 10-90 % transition | > 5.000 ms | 44.000 ms | 44.1 k mono | PASS 10/10 |
| F Features (spec section 2) | FET Input -18 dB step: 10-90 % transition | > 5.000 ms | 44.000 ms | 44.1 k mono | PASS 10/10 |
| F Features (spec section 2) | Mix 100 -> 0 % step (wet 20 dB lower): 10-90 % transition | > 5.000 ms | 44.000 ms | 44.1 k mono | PASS 10/10 |
| O Oversampling filters | 2x stage (up/down): stopband peak | < -100.0 dB | -100.3 dB | 88.2 k mono | PASS 10/10 |
| O Oversampling filters | 2x stage (up/down): passband ripple | < 0.0010 dB | 8.43e-05 dB | 88.2 k mono | PASS 10/10 |
| O Oversampling filters | 4x stage: stopband peak | < -100.0 dB | -100.4 dB | 44.1 k mono | PASS 10/10 |
| O Oversampling filters | 4x stage: passband ripple | < 0.0010 dB | 8.34e-05 dB | 44.1 k mono | PASS 10/10 |
| O Oversampling filters | latency 1x | (info) | 0.0000 samples | all | - |
| O Oversampling filters | wet path (stages off) flatness 20 Hz-20 kHz, 1x | < 0.0100 dB | 0.0000 dB | 44.1 k mono | PASS 10/10 |
| O Oversampling filters | latency 2x | (info) | 25.000 .. 71.000 samples | all | - |
| O Oversampling filters | wet path (stages off) flatness 20 Hz-20 kHz, 2x | < 0.0100 dB | 0.000157 dB | 44.1 k mono | PASS 10/10 |
| O Oversampling filters | latency 4x | (info) | 33.000 .. 79.000 samples | all | - |
| O Oversampling filters | wet path (stages off) flatness 20 Hz-20 kHz, 4x | < 0.0100 dB | 0.000205 dB | 44.1 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Rap Lead: output vs input loudness | -1.500 .. 1.500 dB | -0.5451 dB | 192 k stereo | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Rap Lead: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Rap Lead: FET peak GR | (info) | 6.720 .. 8.229 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Rap Lead: opto average GR | (info) | 2.583 .. 2.992 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Boom Bap Double: output vs input loudness | -1.500 .. 1.500 dB | -1.023 dB | 192 k stereo | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Boom Bap Double: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Boom Bap Double: FET peak GR | (info) | 9.544 .. 10.917 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Boom Bap Double: opto average GR | (info) | 4.111 .. 4.426 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Aggressive Parallel (All): output vs input loudness | -1.500 .. 1.500 dB | -0.2458 dB | 192 k stereo | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Aggressive Parallel (All): output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Aggressive Parallel (All): FET peak GR | (info) | 17.133 .. 19.173 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Aggressive Parallel (All): opto average GR | (info) | 0.5818 .. 1.416 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Gentle Leveling: output vs input loudness | -1.500 .. 1.500 dB | -0.5290 dB | 96 k stereo | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Gentle Leveling: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Gentle Leveling: FET peak GR | (info) | 2.971 .. 4.464 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Gentle Leveling: opto average GR | (info) | 3.568 .. 4.053 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Spoken Word: output vs input loudness | -1.500 .. 1.500 dB | -0.6795 dB | 96 k stereo | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Spoken Word: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Spoken Word: FET peak GR | (info) | 4.859 .. 6.375 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Spoken Word: opto average GR | (info) | 3.561 .. 3.989 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Classic Serial (FET > Opto): output vs input loudness | -1.500 .. 1.500 dB | -0.6081 dB | 96 k stereo | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Classic Serial (FET > Opto): output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Classic Serial (FET > Opto): FET peak GR | (info) | 4.866 .. 6.383 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Classic Serial (FET > Opto): opto average GR | (info) | 2.524 .. 3.070 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Opto First, FET Catch: output vs input loudness | -1.500 .. 1.500 dB | -0.7113 dB | 192 k stereo | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Opto First, FET Catch: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Opto First, FET Catch: FET peak GR | (info) | 3.594 .. 4.753 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Opto First, FET Catch: opto average GR | (info) | 3.909 .. 4.126 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: New York Parallel: output vs input loudness | -1.500 .. 1.500 dB | -0.2169 dB | 192 k stereo | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: New York Parallel: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: New York Parallel: FET peak GR | (info) | 13.638 .. 15.491 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: New York Parallel: opto average GR | (info) | 0.7638 .. 1.494 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Upfront Hip-Hop: output vs input loudness | -1.500 .. 1.500 dB | -0.7945 dB | 192 k stereo | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Upfront Hip-Hop: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Upfront Hip-Hop: FET peak GR | (info) | 7.409 .. 8.924 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Upfront Hip-Hop: opto average GR | (info) | 2.609 .. 3.013 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Ad-libs & Hype: output vs input loudness | -1.500 .. 1.500 dB | -1.327 dB | 192 k stereo | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Ad-libs & Hype: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Ad-libs & Hype: FET peak GR | (info) | 10.682 .. 12.031 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Ad-libs & Hype: opto average GR | (info) | 3.672 .. 3.958 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Sibilance-Aware Leveling: output vs input loudness | -1.500 .. 1.500 dB | -1.346 dB | 192 k stereo | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Sibilance-Aware Leveling: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Sibilance-Aware Leveling: FET peak GR | (info) | 2.598 .. 4.109 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Sibilance-Aware Leveling: opto average GR | (info) | 3.943 .. 4.636 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Transparent Peak Control: output vs input loudness | -1.500 .. 1.500 dB | -0.7624 dB | 192 k stereo | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Transparent Peak Control: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Transparent Peak Control: FET peak GR | (info) | 5.510 .. 7.018 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Transparent Peak Control: opto average GR | (info) | 0.0000 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Sung Hook: output vs input loudness | -1.500 .. 1.500 dB | -0.3960 dB | 96 k stereo | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Sung Hook: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Sung Hook: FET peak GR | (info) | 3.723 .. 5.234 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Sung Hook: opto average GR | (info) | 4.680 .. 5.071 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Lo-fi Grit (All): output vs input loudness | -1.500 .. 1.500 dB | -0.5962 dB | 192 k stereo | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Lo-fi Grit (All): output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Lo-fi Grit (All): FET peak GR | (info) | 14.814 .. 16.506 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Lo-fi Grit (All): opto average GR | (info) | 1.012 .. 1.863 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Broadcast Voice: output vs input loudness | -1.500 .. 1.500 dB | -0.7198 dB | 96 k stereo | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Broadcast Voice: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Broadcast Voice: FET peak GR | (info) | 5.651 .. 7.411 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Broadcast Voice: opto average GR | (info) | 4.526 .. 5.030 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Mix Bus: Glue: output vs input loudness | -1.500 .. 1.500 dB | -0.1132 dB | 192 k stereo | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Mix Bus: Glue: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Mix Bus: Glue: FET peak GR | (info) | 1.808 .. 2.361 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Mix Bus: Glue: opto average GR | (info) | 1.454 .. 1.511 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Mix Bus: Punch: output vs input loudness | -1.500 .. 1.500 dB | -0.1636 dB | 192 k stereo | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Mix Bus: Punch: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Mix Bus: Punch: FET peak GR | (info) | 3.674 .. 4.179 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Mix Bus: Punch: opto average GR | (info) | 0.0000 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: Transparent Leveling: output vs input loudness | -1.500 .. 1.500 dB | 0.0791 dB | 44.1 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: Transparent Leveling: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: Transparent Leveling: FET peak GR | (info) | 0.0000 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: Transparent Leveling: opto average GR | (info) | 1.984 .. 2.018 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: Peak Catch: output vs input loudness | -1.500 .. 1.500 dB | -0.0913 dB | 192 k stereo | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: Peak Catch: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: Peak Catch: FET peak GR | (info) | 1.621 .. 2.288 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: Peak Catch: opto average GR | (info) | 0.0000 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: Parallel Density: output vs input loudness | -1.500 .. 1.500 dB | -0.2627 dB | 192 k stereo | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: Parallel Density: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: Parallel Density: FET peak GR | (info) | 7.268 .. 7.952 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: Parallel Density: opto average GR | (info) | 0.0000 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: True-Peak Safety (-1 dBTP): output vs input loudness | -1.500 .. 1.500 dB | -1.126 dB | 192 k stereo | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: True-Peak Safety (-1 dBTP): output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: True-Peak Safety (-1 dBTP): FET peak GR | (info) | 0.0000 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: True-Peak Safety (-1 dBTP): opto average GR | (info) | 2.142 .. 2.181 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Bass & 808: Clean Control: output vs input loudness | -1.500 .. 1.500 dB | -1.273 dB | 192 k stereo | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Bass & 808: Clean Control: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Bass & 808: Clean Control: FET peak GR | (info) | 7.701 .. 8.156 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Bass & 808: Clean Control: opto average GR | (info) | 0.0000 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Mix Bus: Mid/Side Glue: output vs input loudness | -1.500 .. 1.500 dB | -0.7029 dB | 192 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Mix Bus: Mid/Side Glue: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Mix Bus: Mid/Side Glue: FET peak GR | (info) | 2.986 .. 3.569 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Mix Bus: Mid/Side Glue: opto average GR | (info) | 1.740 .. 1.841 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Lookahead Peak Control: output vs input loudness | -1.500 .. 1.500 dB | -1.018 dB | 96 k stereo | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Lookahead Peak Control: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Lookahead Peak Control: FET peak GR | (info) | 7.403 .. 9.000 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Lookahead Peak Control: opto average GR | (info) | 1.931 .. 2.532 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Drum Bus: Boom Bap Smash: output vs input loudness | -1.500 .. 1.500 dB | -0.3025 dB | 192 k stereo | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Drum Bus: Boom Bap Smash: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 10/10 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Drum Bus: Boom Bap Smash: FET peak GR | (info) | 14.677 .. 15.450 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Drum Bus: Boom Bap Smash: opto average GR | (info) | 0.0000 dB | all | - |
| V 2.0 features | Vintage detector (1.x): FET 4:1, 20 us / 50 ms, Color 0, 30.000 Hz -6 dBFS (+12 dB in): THD+N | (info) | -43.490 .. -43.488 dB | all | - |
| V 2.0 features | Clean detector: FET 4:1, 20 us / 50 ms, Color 0, 30.000 Hz -6 dBFS (+12 dB in): THD+N | < -90.000 dB | -142.2 dB | 48 k mono | PASS 10/10 |
| V 2.0 features | Vintage detector (1.x): FET 4:1, 20 us / 50 ms, Color 0, 100.0 Hz -6 dBFS (+12 dB in): THD+N | (info) | -46.399 .. -46.369 dB | all | - |
| V 2.0 features | Clean detector: FET 4:1, 20 us / 50 ms, Color 0, 100.0 Hz -6 dBFS (+12 dB in): THD+N | < -90.000 dB | -142.1 dB | 88.2 k mono | PASS 10/10 |
| V 2.0 features | Clean vs Vintage: steady GR at 1 kHz -6 dBFS (difference) | -0.3000 .. 0.3000 dB | 0.0055 dB | 44.1 k mono | PASS 10/10 |
| V 2.0 features | Clean vs Vintage: onset peak (first 0.5 ms, 20:1, 20 us) difference | -0.5000 .. 0.5000 dB | 0.0000 dB | 44.1 k mono | PASS 10/10 |
| V 2.0 features | 4x: worst non-harmonic component, 15 kHz -3 dBFS, 20:1 +24 dB, Color 100 | < -120.0 dBFS | -149.5 dBFS | 48 k mono | PASS 10/10 |
| V 2.0 features | FET threshold -24 dB: GR at -18 dBFS = GR of the -12 dB threshold at -6 dBFS | -0.1000 .. 0.1000 dB | 1.43e-06 dB | 96 k mono | PASS 10/10 |
| V 2.0 features | FET hard knee: 4:1, 6 dB over threshold -> GR 4.5 dB | 4.350 .. 4.650 dB | 4.529 dB | 192 k mono | PASS 10/10 |
| V 2.0 features | FET hard knee: 1 dB under threshold -> GR | < 0.0500 dB | 0.0000 dB | 44.1 k mono | PASS 10/10 |
| V 2.0 features | FET 12 dB knee: GR at threshold = slope * knee / 8 (1.125 dB) | 1.000 .. 1.250 dB | 1.136 dB | 192 k mono | PASS 10/10 |
| V 2.0 features | FET range 6 dB: GR with 20 dB of demand | 5.900 .. 6.010 dB | 6.000 dB | 44.1 k mono | PASS 10/10 |
| V 2.0 features | Opto range 3 dB: GR with heavy demand | 2.900 .. 3.010 dB | 3.000 dB | 44.1 k mono | PASS 10/10 |
| V 2.0 features | Opto threshold -20 dB: GR at -16 dBFS = GR of the -10 dB threshold at -6 dBFS | -0.1500 .. 0.1500 dB | 3.81e-06 dB | 192 k mono | PASS 10/10 |
| V 2.0 features | FET hold 100 ms: GR change 80 ms after the drop | < 0.1000 dB | 0.0000 dB | 44.1 k mono | PASS 10/10 |
| V 2.0 features | FET hold 100 ms: released 200 ms after the drop (GR fraction left) | < 0.8000 x | 0.1843 x | 44.1 k mono | PASS 10/10 |
| V 2.0 features | Opto hold 100 ms: GR change 80 ms after the drop | < 0.1000 dB | 3.81e-05 dB | 44.1 k mono | PASS 10/10 |
| V 2.0 features | Opto hold 100 ms: released 200 ms after the drop (GR fraction left) | < 0.8000 x | 0.4244 x | 44.1 k mono | PASS 10/10 |
| V 2.0 features | SC bell +12 dB at 3 kHz: GR(3 kHz) - GR(300 Hz), equal levels | > 6.000 dB | 8.887 dB | 48 k mono | PASS 10/10 |
| V 2.0 features | SC LPF 1 kHz: GR on an 8 kHz tone at -6 dBFS | < 1.500 dB | 0.0000 dB | 44.1 k mono | PASS 10/10 |
| V 2.0 features | SC LPF 1 kHz: GR on a 300 Hz tone at -6 dBFS | > 3.000 dB | 8.981 dB | 48 k mono | PASS 10/10 |
| V 2.0 features | FET external key 120 Hz -3 dBFS ducks a -30 dBFS 1 kHz track by | > 6.000 dB | 10.643 dB | 192 k mono | PASS 10/10 |
| V 2.0 features | FET external key quiet: track passes at its gain | -0.0500 .. 0.0500 dB | -9.56e-06 dB | 88.2 k mono | PASS 10/10 |
| V 2.0 features | FET external key, no side-chain input: track passes at its gain | -0.0500 .. 0.0500 dB | -9.56e-06 dB | 88.2 k mono | PASS 10/10 |
| V 2.0 features | Opto external key 120 Hz -3 dBFS ducks a -30 dBFS 1 kHz track by | > 6.000 dB | 17.486 dB | 192 k mono | PASS 10/10 |
| V 2.0 features | Opto external key quiet: track passes at its gain | -0.0500 .. 0.0500 dB | -9.86e-06 dB | 88.2 k mono | PASS 10/10 |
| V 2.0 features | Opto external key, no side-chain input: track passes at its gain | -0.0500 .. 0.0500 dB | -9.86e-06 dB | 88.2 k mono | PASS 10/10 |
| V 2.0 features | lookahead 5 ms: latency = 4x + 5 ms | = 300.0 samples | 300.0 samples | 44.1 k mono | PASS 10/10 |
| V 2.0 features | lookahead 5 ms, stages off: output = input delayed by the latency (<18 kHz) | < -80.000 dBFS | -115.5 dBFS | 44.1 k mono | PASS 10/10 |
| V 2.0 features | lookahead 5 ms, stages off: = the engine without lookahead, delayed 5 ms | < -140.0 dBFS | -600.0 dBFS | 44.1 k mono | PASS 10/10 |
| V 2.0 features | FET 20:1, 800 us attack: onset overshoot without lookahead | (info) | 23.340 .. 23.485 dB | all | - |
| V 2.0 features | FET 20:1, 800 us attack: onset overshoot with 2 ms lookahead | < 20.340 dB | 1.329 dB | 44.1 k mono | PASS 10/10 |
| V 2.0 features | limiter on (1.5 ms): latency = 4x + 64 (true-peak detector) + lookahead | = 209.0 samples | 209.0 samples | 44.1 k mono | PASS 10/10 |
| V 2.0 features | limiter -1 dBTP, +12 dB, drum-bus program, content <= 0.48 fs: sample peak | < -1.000 dBFS | -1.050 dBFS | 192 k mono | PASS 10/10 |
| V 2.0 features | limiter -1 dBTP, +12 dB, drum-bus program, content <= 0.48 fs: true peak (independent long-sinc reference) | < -1.000 dBTP | -1.050 dBTP | 192 k mono | PASS 10/10 |
| V 2.0 features | limiter, drum-bus program, content <= 0.48 fs: GR meter | > 3.000 dB | 13.543 dB | 48 k mono | PASS 10/10 |
| V 2.0 features | limiter -1 dBTP, +12 dB, square bursts, content <= 0.48 fs: sample peak | < -1.000 dBFS | -1.354 dBFS | 88.2 k mono | PASS 10/10 |
| V 2.0 features | limiter -1 dBTP, +12 dB, square bursts, content <= 0.48 fs: true peak (independent long-sinc reference) | < -1.000 dBTP | -1.049 dBTP | 192 k mono | PASS 10/10 |
| V 2.0 features | limiter, square bursts, content <= 0.48 fs: GR meter | > 3.000 dB | 3.189 dB | 88.2 k mono | PASS 10/10 |
| V 2.0 features | limiter -1 dBTP, +12 dB, drum-bus program, full band: sample peak | < -1.000 dBFS | -1.050 dBFS | 192 k mono | PASS 10/10 |
| V 2.0 features | limiter -1 dBTP, +12 dB, drum-bus program, full band: true peak (reference), documented bound +0.4 dB | < -0.6000 dBTP | -0.9463 dBTP | 48 k mono | PASS 10/10 |
| V 2.0 features | limiter -1 dBTP, +12 dB, square bursts, full band: sample peak | < -1.000 dBFS | -1.354 dBFS | 88.2 k mono | PASS 10/10 |
| V 2.0 features | limiter -1 dBTP, +12 dB, square bursts, full band: true peak (reference), documented bound +0.4 dB | < -0.6000 dBTP | -0.8309 dBTP | 44.1 k mono | PASS 10/10 |
| V 2.0 features | limiter below the ceiling: = the engine without limiter, delayed by its latency (null) | < -140.0 dBFS | -600.0 dBFS | 44.1 k mono | PASS 10/10 |
| V 2.0 features | handshake: unacknowledged latency change keeps the old latency | = 79.000 samples | 79.000 samples | 44.1 k mono | PASS 10/10 |
| V 2.0 features | handshake: muted while waiting for the host | < -120.0 dBFS | -600.0 dBFS | 44.1 k mono | PASS 10/10 |
| V 2.0 features | handshake: a request with the new latency is posted | yes | 1.000  | 44.1 k mono | PASS 10/10 |
| V 2.0 features | handshake: after the acknowledgement the new latency plays (impulse position - input) | = 211.0 samples | 211.0 samples | 44.1 k mono | PASS 10/10 |
| V 2.0 features | handshake: engine latency = the acknowledged one | = 211.0 samples | 211.0 samples | 44.1 k mono | PASS 10/10 |
| V 2.0 features | handshake timeout: latency unchanged | = 211.0 samples | 211.0 samples | 44.1 k mono | PASS 10/10 |
| V 2.0 features | handshake timeout: the old configuration plays again after 0.5 s | > -7.000 dBFS | -6.021 dBFS | 88.2 k mono | PASS 10/10 |
| V 2.0 features | compare: processed signal louder than the input by | (info) | 6.239 .. 7.432 dB | all | - |
| V 2.0 features | compare: unprocessed signal played at the processed signal's loudness (difference) | -1.000 .. 1.000 dB | 0.0860 dB | 44.1 k stereo | PASS 10/10 |
| V 2.0 features | compare: output is the unprocessed input (residual after one gain) | < -40.000 dB | -42.636 dB | 48 k mono | PASS 10/10 |
| V 2.0 features | history: frames per second of audio | 195.0 .. 205.0  | 199.0  | 44.1 k mono | PASS 10/10 |
| V 2.0 features | history: FET gain reduction in the last frame vs the stage's steady GR | -0.3000 .. 0.3000 dB | -0.0049 dB | 44.1 k mono | PASS 10/10 |
| V 2.0 features | all 2.0 features: block 512 vs 37 (difference) | < -140.0 dBFS | -600.0 dBFS | 44.1 k mono | PASS 10/10 |
| V 2.0 features | all 2.0 features: output finite | yes | 1.000  | 44.1 k mono | PASS 10/10 |
| V 2.0 features | all 2.0 features: heap allocations inside process() | = 0.0000  | 0.0000  | 44.1 k mono | PASS 10/10 |
| V11 Voice hold (3.0) | factory presets: VOICE HOLD on exactly for the vocal presets and Init | yes | 1.000  | 44.1 k mono | PASS 10/10 |
| V11 Voice hold (3.0) | engine default: VOICE HOLD off (the 2.x path; the plugin turns it on for new instances) | yes | 1.000  | 44.1 k mono | PASS 10/10 |
| V11 Voice hold (3.0) | FET: gain reduction on the loud tone | (info) | 17.257 .. 17.308 dB | all | - |
| V11 Voice hold (3.0) | FET without pause: share of the GR left 0.4 s after the drop (released) | < 0.1500  | 0.0996  | 88.2 k mono | PASS 10/10 |
| V11 Voice hold (3.0) | FET in a pause: gain reduction re the voice's (held) | -2.000 .. 0.5000 dB | 0.0000 dB | 44.1 k mono | PASS 10/10 |
| V11 Voice hold (3.0) | opto: gain reduction on the loud tone | (info) | 21.333 .. 21.335 dB | all | - |
| V11 Voice hold (3.0) | opto in a pause: holds more than it would release | > 10.667 dB | 14.462 dB | 88.2 k mono | PASS 10/10 |
| V11 Voice hold (3.0) | opto in a pause: gain reduction re the voice's (held) | -3.000 .. 0.5000 dB | 3.81e-05 dB | 48 k mono | PASS 10/10 |
| V11 Voice hold (3.0) | pauses re voice, off: lift | (info) | 18.699 .. 19.460 dB | all | - |
| V11 Voice hold (3.0) | pauses re voice, on: lift | (info) | 0.5698 .. 0.7449 dB | all | - |
| V11 Voice hold (3.0) | VOICE HOLD lowers the pauses' lift by | > 8.000 dB | 18.081 dB | 48 k mono | PASS 10/10 |
| V11 Voice hold (3.0) | VOICE HOLD: voice level unchanged | -0.7500 .. 0.7500 dB | -0.0459 dB | 48 k mono | PASS 10/10 |
| V11 Voice hold (3.0) | VOICE HOLD: latency unchanged | = 0.0000 samples | 0.0000 samples | 44.1 k mono | PASS 10/10 |
| V11 Voice hold (3.0) | drum loop: VOICE HOLD on vs off (peak difference; beat/bus presets turn it off) | (info) | -13.504 .. -8.993 dBFS | all | - |
| V11 Voice hold (3.0) | VOICE HOLD: block 512 vs 37 (difference) | < -140.0 dBFS | -600.0 dBFS | 44.1 k mono | PASS 10/10 |
| V11 Voice hold (3.0) | VOICE HOLD: output finite | yes | 1.000  | 44.1 k mono | PASS 10/10 |
| V11 Voice hold (3.0) | VOICE HOLD: heap allocations inside process() | = 0.0000  | 0.0000  | 44.1 k mono | PASS 10/10 |
| V12 Ess hold (4.0) | factory presets: ESS HOLD on exactly where VOICE HOLD is (the vocal presets and Init) | yes | 1.000  | 44.1 k mono | PASS 10/10 |
| V12 Ess hold (4.0) | engine default: ESS HOLD off (the 3.x path; the plugin turns it on for new instances) | yes | 1.000  | 44.1 k mono | PASS 10/10 |
| V12 Ess hold (4.0) | FET: gain reduction at the ess's start | (info) | 13.021 .. 13.024 dB | all | - |
| V12 Ess hold (4.0) | FET without ESS HOLD: share of that GR left at the end of the 80 ms ess (releasing) | < 0.7500  | 0.6184  | 88.2 k mono | PASS 10/10 |
| V12 Ess hold (4.0) | FET with ESS HOLD: GR at the end of the ess re its start (held, up to the word's peak) | -0.2500 .. 6.000 dB | 3.069 dB | 96 k mono | PASS 10/10 |
| V12 Ess hold (4.0) | opto: gain reduction at the ess's start | (info) | 17.748 .. 17.751 dB | all | - |
| V12 Ess hold (4.0) | opto with ESS HOLD: holds more than it would release | > 0.5000 dB | 10.318 dB | 96 k mono | PASS 10/10 |
| V12 Ess hold (4.0) | opto with ESS HOLD: GR at the end of the ess re its start (held, up to the word's peak) | -0.2500 .. 6.000 dB | 1.887 dB | 96 k mono | PASS 10/10 |
| V12 Ess hold (4.0) | esses re vowels, off: lift | (info) | -0.7708 .. 2.985 dB | all | - |
| V12 Ess hold (4.0) | esses re vowels, on: lift | (info) | -1.589 .. 0.3359 dB | all | - |
| V12 Ess hold (4.0) | ESS HOLD removes of the esses' lift (dB, against 60 % of the lift without it) | > 0.8933 dB | 1.526 dB | 96 k mono | PASS 10/10 |
| V12 Ess hold (4.0) | ESS HOLD never lifts the esses more | < 0.0000 dB | -0.8187 dB | 192 k mono | PASS 10/10 |
| V12 Ess hold (4.0) | ESS HOLD: vowel level unchanged | -0.5000 .. 0.5000 dB | 0.0760 dB | 48 k mono | PASS 10/10 |
| V12 Ess hold (4.0) | ESS HOLD: latency unchanged | = 0.0000 samples | 0.0000 samples | 44.1 k mono | PASS 10/10 |
| V12 Ess hold (4.0) | ESS HOLD off: deterministic (two engines, same input) | = 0.0000  | 0.0000  | 44.1 k mono | PASS 10/10 |
| V12 Ess hold (4.0) | ESS HOLD: block 512 vs 37 (difference) | < -140.0 dBFS | -600.0 dBFS | 44.1 k mono | PASS 10/10 |
| V12 Ess hold (4.0) | ESS HOLD: output finite | yes | 1.000  | 44.1 k mono | PASS 10/10 |
| V12 Ess hold (4.0) | ESS HOLD: heap allocations inside process() | = 0.0000  | 0.0000  | 44.1 k mono | PASS 10/10 |
| 9 Stereo identity | identical L/R in: differing output samples (defaults) | = 0.0000 samples | 0.0000 samples | 44.1 k stereo | PASS 5/5 |
| 9 Stereo identity | identical L/R in: differing samples (4x, link 0, All, auto makeup) | = 0.0000 samples | 0.0000 samples | 44.1 k stereo | PASS 5/5 |
| 9 Stereo identity | mono instance vs dual-mono stereo: max |diff| | < 1e-06  | 0.0000  | 44.1 k stereo | PASS 5/5 |
| F Features (spec section 2) | Link 100 %: quiet side follows loud side's GR (|GR_L - GR_R|) | < 0.5000 dB | 2.29e-07 dB | 192 k stereo | PASS 5/5 |
| F Features (spec section 2) | Link 0 %: quiet side GR (independent detection) | < 0.2000 dB | 7.36e-06 dB | 88.2 k stereo | PASS 5/5 |
| V 2.0 features | Mid/Side, stages off: = stereo mode (M/S encode + decode is exact) | < -130.0 dBFS | -136.5 dBFS | 96 k stereo | PASS 5/5 |
| V 2.0 features | Mid/Side on identical channels = stereo mode (difference) | < -100.0 dBFS | -600.0 dBFS | 44.1 k stereo | PASS 5/5 |
| V 2.0 features | Mid/Side, link 0: quiet mid passes at its gain (6 dB input) | -0.0500 .. 0.0500 dB | 1.17e-05 dB | 44.1 k stereo | PASS 5/5 |
| V 2.0 features | Mid/Side, link 0: loud side compressed by | > 3.000 dB | 8.993 dB | 44.1 k stereo | PASS 5/5 |
| O Oversampling filters | 2x decimator (stopband from base Nyquist): stopband peak | < -100.0 dB | -100.2 dB | 88.2 k mono | PASS 6/6 |
| O Oversampling filters | 2x decimator (stopband from base Nyquist): passband ripple | < 0.0010 dB | 0.000101 dB | 88.2 k mono | PASS 6/6 |
| 10 CPU | 48 kHz / 128 / stereo / 2x, 60 s program: CPU of one core | < 2.000 % | 1.903 % | 48 k stereo | PASS 1/1 |
| 10 CPU | 48 kHz / 128 / stereo / 2x: real-time factor | (info) | 52.544 x | all | - |
| 10 CPU | silence after program (denormal safety): CPU silence / CPU program | < 1.500 x | 0.7937 x | 48 k stereo | PASS 1/1 |
| 10 CPU | 48 kHz / 128 / stereo / 1x: CPU of one core | (info) | 1.256 % | all | - |
| 10 CPU | 48 kHz / 128 / stereo / 4x: CPU of one core | (info) | 3.194 % | all | - |
| 10 CPU | 48 kHz / 128 / stereo / 4x, every 2.0 feature on (limiter, lookahead, M/S, Clean, SC EQ): CPU of one core | < 8.000 % | 6.427 % | 48 k stereo | PASS 1/1 |
| 10 CPU | 48 kHz / 128 / stereo / 4x, a new instance (VOICE HOLD + ESS HOLD on): CPU of one core | < 8.000 % | 5.565 % | 48 k stereo | PASS 1/1 |
| 10 CPU | ESS HOLD's own cost (its sibilance detector): CPU of one core | (info) | 1.099 % | all | - |
| 10 CPU | stereo / 128 / 2x at 44.100 kHz: CPU of one core | (info) | 1.751 % | all | - |
| 10 CPU | stereo / 128 / 2x at 96.000 kHz: CPU of one core | (info) | 3.740 % | all | - |
| 10 CPU | stereo / 128 / 2x at 192.0 kHz: CPU of one core | (info) | 7.461 % | all | - |

## Detail per configuration

| Check | 44.1 k mono | 44.1 k stereo | 48 k mono | 48 k stereo | 88.2 k mono | 88.2 k stereo | 96 k mono | 96 k stereo | 192 k mono | 192 k stereo |
|---|---|---|---|---|---|---|---|---|---|---|
| 1 Static curve: FET 4:1: slope below threshold (-60..-44 dBFS) | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 |
| 1 Static curve: FET 4:1: ratio above threshold (-12..0 dBFS in) | 4.006 | 4.006 | 4.002 | 4.002 | 4.008 | 4.008 | 4.013 | 4.013 | 4.009 | 4.009 |
| 1 Static curve: FET 8:1: slope below threshold (-60..-44 dBFS) | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 |
| 1 Static curve: FET 8:1: ratio above threshold (-12..0 dBFS in) | 8.030 | 8.030 | 8.031 | 8.031 | 8.100 | 8.100 | 8.134 | 8.134 | 8.033 | 8.033 |
| 1 Static curve: FET 12:1: slope below threshold (-60..-44 dBFS) | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 |
| 1 Static curve: FET 12:1: ratio above threshold (-12..0 dBFS in) | 12.057 | 12.057 | 12.069 | 12.069 | 12.191 | 12.191 | 12.274 | 12.274 | 11.910 | 11.910 |
| 1 Static curve: FET 20:1: slope below threshold (-60..-44 dBFS) | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 |
| 1 Static curve: FET 20:1: ratio above threshold (-12..0 dBFS in) | 20.132 | 20.132 | 20.183 | 20.183 | 20.402 | 20.402 | 20.641 | 20.641 | 19.124 | 19.124 |
| 1 Static curve: FET All: slope below threshold (-60..-44 dBFS) | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 |
| 1 Static curve: FET All: over-compression, drop from curve peak to 0 dBFS in | 0.7380 | 0.7380 | 0.7422 | 0.7422 | 0.7353 | 0.7353 | 0.7314 | 0.7314 | 0.7479 | 0.7479 |
| 1 Static curve: FET All: curve peak position above threshold | 8.000 | 8.000 | 8.000 | 8.000 | 8.000 | 8.000 | 8.000 | 8.000 | 8.000 | 8.000 |
| 1 Static curve: FET All: drop stays 'slight' (peak to 0 dBFS in) | 0.7380 | 0.7380 | 0.7422 | 0.7422 | 0.7353 | 0.7353 | 0.7314 | 0.7314 | 0.7479 | 0.7479 |
| 1 Static curve: Opto Compress: slope below threshold (-60..-44 dBFS) | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 |
| 1 Static curve: Opto Compress: ratio above threshold (-12..0 dBFS in) | 3.000 | 3.000 | 3.000 | 3.000 | 3.000 | 3.000 | 3.000 | 3.000 | 2.999 | 2.999 |
| 1 Static curve: Opto Limit: slope below threshold (-60..-44 dBFS) | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 | 1.0000 |
| 1 Static curve: Opto Limit: ratio above threshold (-12..0 dBFS in) | 10.000 | 10.000 | 10.000 | 10.000 | 10.000 | 10.000 | 10.000 | 10.000 | 10.000 | 10.000 |
| 2 FET attack/release: attack 20.000 us: time to 63 % GR | 20.642 | 20.642 | 20.274 | 20.274 | 20.277 | 20.277 | 20.177 | 20.177 | 20.161 | 20.161 |
| 2 FET attack/release: attack 800.0 us: time to 63 % GR | 803.6 | 803.6 | 803.8 | 803.8 | 802.2 | 802.2 | 802.6 | 802.6 | 801.3 | 801.3 |
| 2 FET attack/release: release 50.000 ms: time to 63 % recovery | 55.169 | 55.169 | 55.204 | 55.204 | 55.574 | 55.574 | 55.554 | 55.554 | 55.573 | 55.573 |
| 2 FET attack/release: release 1100.0 ms: time to 63 % recovery | 1164.3 | 1164.3 | 1165.3 | 1165.3 | 1172.6 | 1172.6 | 1172.7 | 1172.7 | 1173.4 | 1173.4 |
| 2 FET attack/release: program-dependent: GR during hold | 16.507 | 16.507 | 16.513 | 16.513 | 16.520 | 16.520 | 16.526 | 16.526 | 16.532 | 16.532 |
| 2 FET attack/release: program-dependent: release after 6 s vs 0.3 s of >10 dB GR | 1.295 | 1.295 | 1.295 | 1.295 | 1.295 | 1.295 | 1.295 | 1.295 | 1.294 | 1.294 |
| 2 FET attack/release: All mode: attack time relative to 20:1 (800 us knob) | 0.6008 | 0.6008 | 0.6005 | 0.6005 | 0.6002 | 0.6002 | 0.6004 | 0.6004 | 0.6015 | 0.6015 |
| 3 Opto attack/release: attack: time to 63 % of 10 dB GR | 9.798 | 9.798 | 9.796 | 9.796 | 9.787 | 9.787 | 9.786 | 9.786 | 9.771 | 9.771 |
| 3 Opto attack/release: release after 2 s of 10 dB GR: 50 % recovery | 71.245 | 71.245 | 71.233 | 71.233 | 71.255 | 71.255 | 71.260 | 71.260 | 71.218 | 71.218 |
| 3 Opto attack/release: release tail time constant after 2 s (70->90 % recovery) | 1.928 | 1.928 | 1.926 | 1.926 | 1.937 | 1.937 | 1.936 | 1.936 | 1.937 | 1.937 |
| 3 Opto attack/release: 90 % recovery after 1 s of 10 dB GR | 1.921 | 1.921 | 1.918 | 1.918 | 1.931 | 1.931 | 1.929 | 1.929 | 1.929 | 1.929 |
| 3 Opto attack/release: 90 % recovery after 10 s of 10 dB GR | 4.864 | 4.864 | 4.864 | 4.864 | 4.871 | 4.871 | 4.874 | 4.874 | 4.866 | 4.866 |
| 3 Opto attack/release: light-cell memory: 90 % recovery 10 s hold / 1 s hold | 2.533 | 2.533 | 2.537 | 2.537 | 2.523 | 2.523 | 2.527 | 2.527 | 2.523 | 2.523 |
| 3 Opto attack/release: release smoothness (slope residual vs slope, 10 s hold) | 0.0026 | 0.0026 | 0.0026 | 0.0026 | 0.0033 | 0.0033 | 0.0035 | 0.0035 | 0.0055 | 0.0055 |
| 4 THD (1 kHz): FET 0 dB GR, Color 0 | 1.72e-06 | 1.72e-06 | 5.1e-06 | 5.1e-06 | 1.26e-06 | 1.26e-06 | 4.94e-06 | 4.94e-06 | 3.62e-06 | 3.62e-06 |
| 4 THD (1 kHz): FET 10 dB GR, Color 50 (4:1) | 0.6635 | 0.6635 | 0.6633 | 0.6633 | 0.6633 | 0.6633 | 0.6630 | 0.6630 | 0.6619 | 0.6619 |
| 4 THD (1 kHz): FET 10 dB GR, Color 50: share of 2nd+3rd in distortion power | 99.830 | 99.830 | 99.830 | 99.830 | 99.830 | 99.830 | 99.830 | 99.830 | 99.832 | 99.832 |
| 4 THD (1 kHz): FET All 10 dB GR, Color 50 | 1.110 | 1.110 | 1.110 | 1.110 | 1.110 | 1.110 | 1.109 | 1.109 | 1.107 | 1.107 |
| 4 THD (1 kHz): FET All vs 4:1 THD at 10 dB GR, Color 50 | 1.673 | 1.673 | 1.673 | 1.673 | 1.673 | 1.673 | 1.673 | 1.673 | 1.673 | 1.673 |
| 4 THD (1 kHz): Opto 10 dB GR, Color 0 (clean) | 2.19e-06 | 2.19e-06 | 7.7e-06 | 7.7e-06 | 3.51e-05 | 3.51e-05 | 4.67e-05 | 4.67e-05 | 2.47e-06 | 2.47e-06 |
| 4 THD (1 kHz): Opto 10 dB GR, Color 50 | 0.1348 | 0.1348 | 0.1348 | 0.1348 | 0.1348 | 0.1348 | 0.1348 | 0.1348 | 0.1348 | 0.1348 |
| 4 THD (1 kHz): Opto Color 50: H2 / H3 (predominantly 2nd) | 118.7 | 118.7 | 118.6 | 118.6 | 122.4 | 122.4 | 123.4 | 123.4 | 118.7 | 118.7 |
| 4 THD (1 kHz): Opto Color 50: THD rises with level (0 dB vs -20 dB rel.) | 3.169 | 3.169 | 3.169 | 3.169 | 3.169 | 3.169 | 3.169 | 3.169 | 3.169 | 3.169 |
| 4 THD (1 kHz): Opto Color 100: DC offset | -203.7 | -203.7 | -191.0 | -191.0 | -206.8 | -206.8 | -177.7 | -177.7 | -176.7 | -176.7 |
| 5 Aliasing (15 kHz, -3 dBFS, FET 20:1, Color 100): FET only, 1x (no oversampling) | -51.574 | -51.574 | -51.385 | -51.385 | -53.498 | -53.498 | -84.380 | -84.380 | -117.4 | -117.4 |
| 5 Aliasing (15 kHz, -3 dBFS, FET 20:1, Color 100): FET only, 2x | -84.472 | -84.472 | -84.383 | -84.383 | -156.3 | -156.3 | -163.4 | -163.4 | -164.8 | -164.8 |
| 5 Aliasing (15 kHz, -3 dBFS, FET 20:1, Color 100): FET only, 4x | -157.2 | -157.2 | -154.2 | -154.2 | -156.7 | -156.7 | -159.2 | -159.2 | -160.8 | -160.8 |
| 5 Aliasing (15 kHz, -3 dBFS, FET 20:1, Color 100): FET + Opto (default), 1x (no oversampling) | -54.778 | -54.778 | -54.586 | -54.586 | -60.529 | -60.529 | -89.443 | -89.443 | -124.4 | -124.4 |
| 5 Aliasing (15 kHz, -3 dBFS, FET 20:1, Color 100): FET + Opto (default), 2x | -91.519 | -91.519 | -91.428 | -91.428 | -163.7 | -163.7 | -160.5 | -160.5 | -165.4 | -165.4 |
| 5 Aliasing (15 kHz, -3 dBFS, FET 20:1, Color 100): FET + Opto (default), 4x | -135.6 | -135.6 | -152.9 | -152.9 | -164.2 | -164.2 | -163.8 | -163.8 | -161.4 | -161.4 |
| 6 Null / latency: Mix 0 %: residual vs input delayed by reported latency, 1x | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 |
| 6 Null / latency: reported latency matches factor, 1x | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| 6 Null / latency: wet impulse peak offset - reported latency, 1x | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| 6 Null / latency: Mix 50 %, stages off: residual vs delayed dry (<18 kHz), 1x | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 |
| 6 Null / latency: Mix 0 %: residual vs input delayed by reported latency, 2x | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 |
| 6 Null / latency: reported latency matches factor, 2x | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| 6 Null / latency: wet impulse peak offset - reported latency, 2x | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| 6 Null / latency: Mix 50 %, stages off: residual vs delayed dry (<18 kHz), 2x | -131.9 | -131.9 | -132.5 | -132.5 | -129.1 | -129.1 | -126.7 | -126.7 | -130.3 | -130.3 |
| 6 Null / latency: Mix 0 %: residual vs input delayed by reported latency, 4x | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 |
| 6 Null / latency: reported latency matches factor, 4x | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| 6 Null / latency: wet impulse peak offset - reported latency, 4x | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| 6 Null / latency: Mix 50 %, stages off: residual vs delayed dry (<18 kHz), 4x | -121.3 | -121.3 | -121.4 | -121.4 | -123.3 | -123.3 | -123.7 | -123.7 | -127.0 | -127.0 |
| 7 Silence / stability: zeros after +24 dBFS noise: max |out| after 1 s of silence | 4.01e-17 | 3.5e-17 | 3.59e-17 | 3.37e-17 | 3.41e-17 | 3.35e-17 | 3.72e-17 | 3.55e-17 | 3.3e-17 | 3.13e-17 |
| 7 Silence / stability: zeros from fresh instance: max |out| | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| 7 Silence / stability: random automation every block, +24 dBFS: all samples finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| 7 Silence / stability: random automation: peak output (sanity bound) | 38.825 | 38.825 | 37.715 | 37.703 | 39.083 | 39.082 | 36.292 | 36.292 | 35.913 | 35.913 |
| 7 Silence / stability: random automation then silence: max |out| in last 0.5 s | 1.05e-26 | 9.66e-27 | 1.52e-23 | 1.42e-23 | 1.19e-24 | 1.13e-24 | 3.86e-24 | 3.71e-24 | 2.52e-26 | 2.43e-26 |
| 8 Block-size invariance: max |diff| blocks 1/17/64/2048/4096 vs 512, 1x | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| 8 Block-size invariance: max |diff| blocks 1/17/64/2048/4096 vs 512, 2x | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| 8 Block-size invariance: max |diff| blocks 1/17/64/2048/4096 vs 512, 4x | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| F Features (spec section 2): FET SC HPF: GR reduction on 60 Hz, HPF 20 -> 300 Hz | 8.908 | 8.908 | 8.909 | 8.909 | 8.909 | 8.909 | 8.909 | 8.909 | 8.909 | 8.909 |
| F Features (spec section 2): Opto SC HPF: GR reduction on 60 Hz, HPF 20 -> 300 Hz | 17.625 | 17.625 | 17.626 | 17.626 | 17.632 | 17.632 | 17.633 | 17.633 | 17.640 | 17.640 |
| F Features (spec section 2): SC Listen (HPF 300 Hz): 50 Hz relative to 3 kHz in output | -31.132 | -31.132 | -31.131 | -31.131 | -31.130 | -31.130 | -31.130 | -31.130 | -31.129 | -31.129 |
| F Features (spec section 2): Emphasis 100 %: extra opto GR on 8 kHz | 7.790 | 7.790 | 7.780 | 7.780 | 7.741 | 7.741 | 7.738 | 7.738 | 7.726 | 7.726 |
| F Features (spec section 2): Emphasis 100 %: GR change on 200 Hz (should be ~0) | 0.000173 | 0.000173 | 0.000176 | 0.000176 | 0.000181 | 0.000181 | 0.000182 | 0.000182 | 0.000188 | 0.000188 |
| F Features (spec section 2): Auto makeup: final makeup vs average GR (|diff|) | 0.0968 | 0.0968 | 0.1052 | 0.1052 | 0.1936 | 0.1936 | 0.2113 | 0.2113 | 0.4186 | 0.4186 |
| F Features (spec section 2): Auto makeup: time to 63 % of final makeup (>= 1 s) | 2.312 | 2.312 | 2.305 | 2.305 | 2.292 | 2.292 | 2.295 | 2.295 | 2.235 | 2.235 |
| F Features (spec section 2): Auto makeup: cap under extreme GR | 23.987 | 23.987 | 23.986 | 23.986 | 23.975 | 23.975 | 23.973 | 23.973 | 23.945 | 23.945 |
| F Features (spec section 2): OS switching every 37 blocks: all samples finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| F Features (spec section 2): OS switching: peak output vs peak input | -7.057 | -7.394 | -7.315 | -7.724 | -7.190 | -7.347 | -7.726 | -8.250 | -8.354 | -8.473 |
| F Features (spec section 2): OS switched to 4x: reported latency - latency of 4x | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| F Features (spec section 2): OS switched to 4x: impulse peak - reported latency | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| F Features (spec section 2): Order switch while playing: max sample step vs steady tone | 0.9999 | 0.9999 | 1.0000 | 1.0000 | 1.000 | 1.000 | 1.030 | 1.030 | 1.029 | 1.029 |
| F Features (spec section 2): Output +12 dB step: 10-90 % transition | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 |
| F Features (spec section 2): FET Output -20 dB step: 10-90 % transition | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 |
| F Features (spec section 2): Opto Gain +12 dB step: 10-90 % transition | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 |
| F Features (spec section 2): FET Input -18 dB step: 10-90 % transition | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 |
| F Features (spec section 2): Mix 100 -> 0 % step (wet 20 dB lower): 10-90 % transition | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 | 44.000 |
| O Oversampling filters: 2x stage (up/down): stopband peak | -100.9 | -100.9 | -100.9 | -100.9 | -100.3 | -100.3 | -100.3 | -100.3 | -100.3 | -100.3 |
| O Oversampling filters: 2x stage (up/down): passband ripple | 7.82e-05 | 7.82e-05 | 7.82e-05 | 7.82e-05 | 8.43e-05 | 8.43e-05 | 8.43e-05 | 8.43e-05 | 8.43e-05 | 8.43e-05 |
| O Oversampling filters: 4x stage: stopband peak | -100.4 | -100.4 | -100.4 | -100.4 | -100.4 | -100.4 | -100.4 | -100.4 | -100.4 | -100.4 |
| O Oversampling filters: 4x stage: passband ripple | 8.34e-05 | 8.34e-05 | 8.34e-05 | 8.34e-05 | 8.34e-05 | 8.34e-05 | 8.34e-05 | 8.34e-05 | 8.34e-05 | 8.34e-05 |
| O Oversampling filters: latency 1x | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| O Oversampling filters: wet path (stages off) flatness 20 Hz-20 kHz, 1x | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| O Oversampling filters: latency 2x | 71.000 | 71.000 | 71.000 | 71.000 | 25.000 | 25.000 | 25.000 | 25.000 | 25.000 | 25.000 |
| O Oversampling filters: wet path (stages off) flatness 20 Hz-20 kHz, 2x | 0.000157 | 0.000157 | 0.000102 | 0.000102 | 7.09e-05 | 7.09e-05 | 6.93e-05 | 6.93e-05 | 5.01e-05 | 5.01e-05 |
| O Oversampling filters: latency 4x | 79.000 | 79.000 | 79.000 | 79.000 | 33.000 | 33.000 | 33.000 | 33.000 | 33.000 | 33.000 |
| O Oversampling filters: wet path (stages off) flatness 20 Hz-20 kHz, 4x | 0.000205 | 0.000205 | 0.000138 | 0.000138 | 0.000123 | 0.000123 | 0.000123 | 0.000123 | 0.000123 | 0.000123 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Rap Lead: output vs input loudness | 0.0037 | -0.2747 | -0.0836 | -0.3370 | -0.2068 | -0.4000 | -0.2793 | -0.5010 | -0.3779 | -0.5451 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Rap Lead: output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Rap Lead: FET peak GR | 6.720 | 7.254 | 7.121 | 7.121 | 7.533 | 7.533 | 7.783 | 8.229 | 7.577 | 7.820 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Rap Lead: opto average GR | 2.991 | 2.992 | 2.957 | 2.954 | 2.881 | 2.814 | 2.854 | 2.759 | 2.644 | 2.583 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Boom Bap Double: output vs input loudness | -0.1783 | -0.4882 | -0.2614 | -0.5165 | -0.5349 | -0.7214 | -0.6371 | -0.8583 | -0.8596 | -1.023 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Boom Bap Double: output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Boom Bap Double: FET peak GR | 9.544 | 9.863 | 9.912 | 9.912 | 10.755 | 10.755 | 10.165 | 10.917 | 10.311 | 10.464 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Boom Bap Double: opto average GR | 4.426 | 4.365 | 4.409 | 4.342 | 4.356 | 4.291 | 4.355 | 4.208 | 4.206 | 4.111 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Aggressive Parallel (All): output vs input loudness | -0.0404 | -0.1298 | -0.0588 | -0.1336 | -0.1111 | -0.1746 | -0.1442 | -0.2076 | -0.1838 | -0.2458 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Aggressive Parallel (All): output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Aggressive Parallel (All): FET peak GR | 17.133 | 17.556 | 17.740 | 17.740 | 18.549 | 18.549 | 18.376 | 19.173 | 18.337 | 18.591 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Aggressive Parallel (All): opto average GR | 1.416 | 1.327 | 1.330 | 1.214 | 1.013 | 0.9135 | 1.007 | 0.8687 | 0.7029 | 0.5818 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Gentle Leveling: output vs input loudness | -0.1148 | -0.4184 | -0.1816 | -0.4473 | -0.2188 | -0.4222 | -0.2654 | -0.5290 | -0.2963 | -0.4738 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Gentle Leveling: output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Gentle Leveling: FET peak GR | 2.971 | 3.591 | 3.225 | 3.257 | 3.767 | 3.782 | 4.115 | 4.464 | 3.718 | 4.052 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Gentle Leveling: opto average GR | 4.002 | 4.053 | 3.923 | 4.003 | 3.877 | 3.845 | 3.828 | 3.738 | 3.587 | 3.568 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Spoken Word: output vs input loudness | -0.2197 | -0.5141 | -0.2922 | -0.5596 | -0.3691 | -0.5706 | -0.4321 | -0.6795 | -0.4972 | -0.6726 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Spoken Word: output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Spoken Word: FET peak GR | 4.859 | 5.452 | 5.199 | 5.201 | 5.682 | 5.683 | 5.978 | 6.375 | 5.672 | 5.961 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Spoken Word: opto average GR | 3.961 | 3.989 | 3.906 | 3.948 | 3.858 | 3.809 | 3.822 | 3.723 | 3.585 | 3.561 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Classic Serial (FET > Opto): output vs input loudness | -0.2242 | -0.4987 | -0.2984 | -0.5558 | -0.3282 | -0.5250 | -0.3898 | -0.6081 | -0.3996 | -0.5706 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Classic Serial (FET > Opto): output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Classic Serial (FET > Opto): FET peak GR | 4.866 | 5.457 | 5.207 | 5.208 | 5.690 | 5.690 | 5.986 | 6.383 | 5.678 | 5.966 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Classic Serial (FET > Opto): opto average GR | 3.057 | 3.070 | 3.010 | 3.030 | 2.876 | 2.832 | 2.847 | 2.764 | 2.571 | 2.524 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Opto First, FET Catch: output vs input loudness | 0.2275 | -0.0440 | 0.0638 | -0.1661 | -0.0672 | -0.3044 | -0.1674 | -0.4792 | -0.4831 | -0.7113 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Opto First, FET Catch: output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Opto First, FET Catch: FET peak GR | 3.667 | 3.649 | 3.762 | 3.594 | 4.393 | 4.155 | 4.459 | 4.753 | 4.357 | 4.357 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Opto First, FET Catch: opto average GR | 3.939 | 4.126 | 3.937 | 4.109 | 3.940 | 4.041 | 3.957 | 4.066 | 3.909 | 4.004 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: New York Parallel: output vs input loudness | -0.0452 | -0.1300 | -0.0671 | -0.1410 | -0.1022 | -0.1674 | -0.1351 | -0.2014 | -0.1556 | -0.2169 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: New York Parallel: output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: New York Parallel: FET peak GR | 13.638 | 14.227 | 14.211 | 14.211 | 14.787 | 14.787 | 14.887 | 15.491 | 14.750 | 14.999 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: New York Parallel: opto average GR | 1.494 | 1.431 | 1.442 | 1.358 | 1.172 | 1.095 | 1.169 | 1.061 | 0.8573 | 0.7638 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Upfront Hip-Hop: output vs input loudness | -0.1761 | -0.4309 | -0.2551 | -0.4757 | -0.4242 | -0.6102 | -0.5075 | -0.6937 | -0.6190 | -0.7945 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Upfront Hip-Hop: output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Upfront Hip-Hop: FET peak GR | 7.409 | 7.891 | 7.877 | 7.877 | 8.349 | 8.349 | 8.431 | 8.924 | 8.318 | 8.522 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Upfront Hip-Hop: opto average GR | 3.013 | 2.989 | 2.988 | 2.946 | 2.881 | 2.818 | 2.881 | 2.784 | 2.684 | 2.609 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Ad-libs & Hype: output vs input loudness | -0.2282 | -0.5132 | -0.2759 | -0.5211 | -0.6988 | -0.8817 | -0.8035 | -0.9796 | -1.116 | -1.327 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Ad-libs & Hype: output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Ad-libs & Hype: FET peak GR | 10.682 | 11.192 | 10.969 | 10.969 | 12.031 | 12.031 | 11.127 | 11.874 | 11.371 | 11.528 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Ad-libs & Hype: opto average GR | 3.958 | 3.876 | 3.907 | 3.828 | 3.850 | 3.778 | 3.895 | 3.725 | 3.804 | 3.672 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Sibilance-Aware Leveling: output vs input loudness | 0.1230 | -0.1121 | 0.0176 | -0.1927 | -0.5247 | -0.6811 | -0.6437 | -0.8534 | -1.194 | -1.346 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Sibilance-Aware Leveling: output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Sibilance-Aware Leveling: FET peak GR | 2.598 | 3.192 | 2.935 | 2.938 | 3.414 | 3.419 | 3.710 | 4.109 | 3.410 | 3.698 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Sibilance-Aware Leveling: opto average GR | 3.964 | 3.977 | 3.943 | 3.979 | 4.371 | 4.299 | 4.384 | 4.279 | 4.636 | 4.609 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Transparent Peak Control: output vs input loudness | 0.1904 | -0.0738 | 0.0645 | -0.1676 | -0.1152 | -0.3520 | -0.2182 | -0.5397 | -0.5439 | -0.7624 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Transparent Peak Control: output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Transparent Peak Control: FET peak GR | 5.510 | 6.129 | 5.850 | 5.852 | 6.308 | 6.312 | 6.607 | 7.018 | 6.340 | 6.625 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Transparent Peak Control: opto average GR | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Sung Hook: output vs input loudness | 0.0607 | -0.2308 | -0.0032 | -0.2676 | -0.0879 | -0.2902 | -0.1479 | -0.3960 | -0.2155 | -0.3880 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Sung Hook: output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Sung Hook: FET peak GR | 3.723 | 4.317 | 4.060 | 4.062 | 4.539 | 4.545 | 4.835 | 5.234 | 4.535 | 4.824 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Sung Hook: opto average GR | 5.033 | 5.071 | 4.961 | 5.029 | 4.963 | 4.910 | 4.914 | 4.814 | 4.702 | 4.680 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Lo-fi Grit (All): output vs input loudness | -0.1546 | -0.3546 | -0.1364 | -0.3216 | -0.3067 | -0.4450 | -0.3576 | -0.4812 | -0.4418 | -0.5962 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Lo-fi Grit (All): output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Lo-fi Grit (All): FET peak GR | 14.871 | 15.605 | 14.814 | 14.814 | 16.506 | 16.506 | 15.098 | 15.667 | 15.634 | 15.634 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Lo-fi Grit (All): opto average GR | 1.863 | 1.745 | 1.758 | 1.664 | 1.426 | 1.386 | 1.439 | 1.304 | 1.143 | 1.012 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Broadcast Voice: output vs input loudness | -0.2214 | -0.5175 | -0.3015 | -0.5667 | -0.4013 | -0.6043 | -0.4715 | -0.7198 | -0.5438 | -0.7176 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Broadcast Voice: output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Broadcast Voice: FET peak GR | 5.651 | 6.275 | 6.119 | 6.120 | 6.599 | 6.599 | 6.892 | 7.411 | 6.651 | 6.934 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Broadcast Voice: opto average GR | 5.030 | 5.026 | 4.985 | 4.975 | 4.899 | 4.815 | 4.863 | 4.738 | 4.595 | 4.526 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Mix Bus: Glue: output vs input loudness | 0.0614 | 0.0056 | 0.0154 | -0.0541 | -0.0242 | -0.0690 | -0.0273 | -0.0867 | -0.0755 | -0.1132 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Mix Bus: Glue: output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Mix Bus: Glue: FET peak GR | 1.922 | 1.926 | 1.846 | 2.021 | 1.808 | 1.810 | 2.359 | 2.361 | 2.038 | 2.053 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Mix Bus: Glue: opto average GR | 1.510 | 1.511 | 1.486 | 1.476 | 1.494 | 1.484 | 1.471 | 1.468 | 1.469 | 1.454 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Mix Bus: Punch: output vs input loudness | 0.0447 | -0.0365 | 0.0080 | -0.0773 | -0.0628 | -0.1158 | -0.0582 | -0.1341 | -0.1059 | -0.1636 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Mix Bus: Punch: output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Mix Bus: Punch: FET peak GR | 3.674 | 3.732 | 3.858 | 4.052 | 3.994 | 4.003 | 4.173 | 4.179 | 4.057 | 4.059 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Mix Bus: Punch: opto average GR | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: Transparent Leveling: output vs input loudness | 0.0791 | 0.0537 | 0.0765 | 0.0600 | 0.0503 | 0.0339 | 0.0407 | 0.0298 | 0.0390 | 0.0300 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: Transparent Leveling: output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: Transparent Leveling: FET peak GR | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: Transparent Leveling: opto average GR | 1.993 | 2.018 | 1.992 | 2.009 | 1.984 | 2.002 | 1.997 | 2.005 | 1.992 | 2.002 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: Peak Catch: output vs input loudness | 0.0417 | 0.0122 | 0.0040 | -0.0331 | -0.0320 | -0.0497 | -0.0254 | -0.0807 | -0.0690 | -0.0913 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: Peak Catch: output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: Peak Catch: FET peak GR | 1.621 | 1.859 | 1.890 | 2.135 | 1.884 | 1.978 | 2.288 | 2.288 | 2.145 | 2.145 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: Peak Catch: opto average GR | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: Parallel Density: output vs input loudness | -0.2117 | -0.2302 | -0.2193 | -0.2377 | -0.2351 | -0.2476 | -0.2331 | -0.2525 | -0.2476 | -0.2627 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: Parallel Density: output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: Parallel Density: FET peak GR | 7.268 | 7.399 | 7.511 | 7.798 | 7.682 | 7.682 | 7.952 | 7.952 | 7.752 | 7.752 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: Parallel Density: opto average GR | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: True-Peak Safety (-1 dBTP): output vs input loudness | -1.024 | -1.080 | -1.012 | -1.055 | -1.056 | -1.081 | -1.047 | -1.107 | -1.098 | -1.126 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: True-Peak Safety (-1 dBTP): output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: True-Peak Safety (-1 dBTP): FET peak GR | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: True-Peak Safety (-1 dBTP): opto average GR | 2.153 | 2.181 | 2.151 | 2.172 | 2.142 | 2.163 | 2.156 | 2.166 | 2.150 | 2.162 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Bass & 808: Clean Control: output vs input loudness | -0.9331 | -1.039 | -0.9268 | -1.058 | -1.051 | -1.143 | -1.061 | -1.180 | -1.154 | -1.273 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Bass & 808: Clean Control: output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Bass & 808: Clean Control: FET peak GR | 7.959 | 7.959 | 7.701 | 8.156 | 8.077 | 8.077 | 8.023 | 8.023 | 8.138 | 8.138 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Bass & 808: Clean Control: opto average GR | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Mix Bus: Mid/Side Glue: output vs input loudness | -0.5409 | -0.3495 | -0.5980 | -0.3802 | -0.6493 | -0.3836 | -0.6500 | -0.4434 | -0.7029 | -0.4544 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Mix Bus: Mid/Side Glue: output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Mix Bus: Mid/Side Glue: FET peak GR | 3.072 | 2.986 | 3.310 | 3.176 | 3.356 | 3.212 | 3.569 | 3.429 | 3.511 | 3.494 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Mix Bus: Mid/Side Glue: opto average GR | 1.793 | 1.841 | 1.775 | 1.822 | 1.760 | 1.841 | 1.740 | 1.809 | 1.741 | 1.825 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Lookahead Peak Control: output vs input loudness | -0.5372 | -0.8420 | -0.6338 | -0.8975 | -0.6708 | -0.8818 | -0.7514 | -1.018 | -0.7987 | -0.9668 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Lookahead Peak Control: output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Lookahead Peak Control: FET peak GR | 7.403 | 8.026 | 7.871 | 7.871 | 8.351 | 8.351 | 8.644 | 9.000 | 8.403 | 8.686 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Lookahead Peak Control: opto average GR | 2.517 | 2.532 | 2.467 | 2.487 | 2.330 | 2.277 | 2.287 | 2.181 | 1.981 | 1.931 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Drum Bus: Boom Bap Smash: output vs input loudness | -0.1719 | -0.2205 | -0.1871 | -0.2371 | -0.2168 | -0.2631 | -0.2243 | -0.2727 | -0.2619 | -0.3025 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Drum Bus: Boom Bap Smash: output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Drum Bus: Boom Bap Smash: FET peak GR | 14.677 | 14.952 | 14.989 | 15.272 | 14.982 | 15.090 | 15.450 | 15.450 | 15.284 | 15.284 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Drum Bus: Boom Bap Smash: opto average GR | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| V 2.0 features: Vintage detector (1.x): FET 4:1, 20 us / 50 ms, Color 0, 30.000 Hz -6 dBFS (+12 dB in): THD+N | -43.488 | -43.488 | -43.488 | -43.488 | -43.489 | -43.489 | -43.489 | -43.489 | -43.490 | -43.490 |
| V 2.0 features: Clean detector: FET 4:1, 20 us / 50 ms, Color 0, 30.000 Hz -6 dBFS (+12 dB in): THD+N | -142.5 | -142.5 | -142.2 | -142.2 | -143.0 | -143.0 | -142.4 | -142.4 | -142.6 | -142.6 |
| V 2.0 features: Vintage detector (1.x): FET 4:1, 20 us / 50 ms, Color 0, 100.0 Hz -6 dBFS (+12 dB in): THD+N | -46.372 | -46.372 | -46.369 | -46.369 | -46.384 | -46.384 | -46.397 | -46.397 | -46.399 | -46.399 |
| V 2.0 features: Clean detector: FET 4:1, 20 us / 50 ms, Color 0, 100.0 Hz -6 dBFS (+12 dB in): THD+N | -143.5 | -143.5 | -143.4 | -143.4 | -142.1 | -142.1 | -143.6 | -143.6 | -142.6 | -142.6 |
| V 2.0 features: Clean vs Vintage: steady GR at 1 kHz -6 dBFS (difference) | 0.0055 | 0.0055 | 0.0037 | 0.0037 | 0.0040 | 0.0040 | 0.0022 | 0.0022 | 0.000213 | 0.000213 |
| V 2.0 features: Clean vs Vintage: onset peak (first 0.5 ms, 20:1, 20 us) difference | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| V 2.0 features: 4x: worst non-harmonic component, 15 kHz -3 dBFS, 20:1 +24 dB, Color 100 | -152.4 | -152.4 | -149.5 | -149.5 | -164.4 | -164.4 | -160.0 | -160.0 | -168.3 | -168.3 |
| V 2.0 features: FET threshold -24 dB: GR at -18 dBFS = GR of the -12 dB threshold at -6 dBFS | 4.77e-07 | 4.77e-07 | 0.0000 | 0.0000 | 4.77e-07 | 4.77e-07 | 1.43e-06 | 1.43e-06 | 1.4e-06 | 1.4e-06 |
| V 2.0 features: FET hard knee: 4:1, 6 dB over threshold -> GR 4.5 dB | 4.521 | 4.521 | 4.521 | 4.521 | 4.522 | 4.522 | 4.523 | 4.523 | 4.529 | 4.529 |
| V 2.0 features: FET hard knee: 1 dB under threshold -> GR | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| V 2.0 features: FET 12 dB knee: GR at threshold = slope * knee / 8 (1.125 dB) | 1.135 | 1.135 | 1.136 | 1.136 | 1.136 | 1.136 | 1.136 | 1.136 | 1.136 | 1.136 |
| V 2.0 features: FET range 6 dB: GR with 20 dB of demand | 6.000 | 6.000 | 6.000 | 6.000 | 6.000 | 6.000 | 6.000 | 6.000 | 6.000 | 6.000 |
| V 2.0 features: Opto range 3 dB: GR with heavy demand | 3.000 | 3.000 | 3.000 | 3.000 | 3.000 | 3.000 | 3.000 | 3.000 | 3.000 | 3.000 |
| V 2.0 features: Opto threshold -20 dB: GR at -16 dBFS = GR of the -10 dB threshold at -6 dBFS | 0.0000 | 0.0000 | -1.91e-06 | -1.91e-06 | 1.63e-06 | 1.63e-06 | 1.43e-06 | 1.43e-06 | 3.81e-06 | 3.81e-06 |
| V 2.0 features: FET hold 100 ms: GR change 80 ms after the drop | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| V 2.0 features: FET hold 100 ms: released 200 ms after the drop (GR fraction left) | 0.1843 | 0.1843 | 0.1824 | 0.1824 | 0.1803 | 0.1803 | 0.1818 | 0.1818 | 0.1818 | 0.1818 |
| V 2.0 features: Opto hold 100 ms: GR change 80 ms after the drop | 3.81e-05 | 3.81e-05 | 3.81e-05 | 3.81e-05 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| V 2.0 features: Opto hold 100 ms: released 200 ms after the drop (GR fraction left) | 0.4244 | 0.4244 | 0.4243 | 0.4243 | 0.4235 | 0.4235 | 0.4235 | 0.4235 | 0.4234 | 0.4234 |
| V 2.0 features: SC bell +12 dB at 3 kHz: GR(3 kHz) - GR(300 Hz), equal levels | 8.911 | 8.911 | 8.887 | 8.887 | 8.897 | 8.897 | 8.899 | 8.899 | 8.892 | 8.892 |
| V 2.0 features: SC LPF 1 kHz: GR on an 8 kHz tone at -6 dBFS | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| V 2.0 features: SC LPF 1 kHz: GR on a 300 Hz tone at -6 dBFS | 8.981 | 8.981 | 8.981 | 8.981 | 8.986 | 8.986 | 8.988 | 8.988 | 9.015 | 9.015 |
| V 2.0 features: FET external key 120 Hz -3 dBFS ducks a -30 dBFS 1 kHz track by | 10.643 | 10.643 | 10.643 | 10.643 | 10.643 | 10.643 | 10.643 | 10.643 | 10.643 | 10.643 |
| V 2.0 features: FET external key quiet: track passes at its gain | -5.64e-06 | -5.64e-06 | -1.35e-06 | -1.35e-06 | -9.56e-06 | -9.56e-06 | -8.12e-06 | -8.12e-06 | -1.51e-06 | -1.51e-06 |
| V 2.0 features: FET external key, no side-chain input: track passes at its gain | -5.64e-06 | -5.64e-06 | -1.35e-06 | -1.35e-06 | -9.56e-06 | -9.56e-06 | -8.12e-06 | -8.12e-06 | -1.51e-06 | -1.51e-06 |
| V 2.0 features: Opto external key 120 Hz -3 dBFS ducks a -30 dBFS 1 kHz track by | 17.487 | 17.487 | 17.488 | 17.488 | 17.491 | 17.491 | 17.491 | 17.491 | 17.486 | 17.486 |
| V 2.0 features: Opto external key quiet: track passes at its gain | -5.95e-06 | -5.95e-06 | -1.54e-06 | -1.54e-06 | -9.86e-06 | -9.86e-06 | -8.55e-06 | -8.55e-06 | -1.85e-06 | -1.85e-06 |
| V 2.0 features: Opto external key, no side-chain input: track passes at its gain | -5.95e-06 | -5.95e-06 | -1.54e-06 | -1.54e-06 | -9.86e-06 | -9.86e-06 | -8.55e-06 | -8.55e-06 | -1.85e-06 | -1.85e-06 |
| V 2.0 features: lookahead 5 ms: latency = 4x + 5 ms | 300.0 | 300.0 | 319.0 | 319.0 | 474.0 | 474.0 | 513.0 | 513.0 | 993.0 | 993.0 |
| V 2.0 features: lookahead 5 ms, stages off: output = input delayed by the latency (<18 kHz) | -115.5 | -115.5 | -115.8 | -115.8 | -117.5 | -117.5 | -118.2 | -118.2 | -121.6 | -121.6 |
| V 2.0 features: lookahead 5 ms, stages off: = the engine without lookahead, delayed 5 ms | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 |
| V 2.0 features: FET 20:1, 800 us attack: onset overshoot without lookahead | 23.340 | 23.340 | 23.357 | 23.357 | 23.404 | 23.404 | 23.412 | 23.412 | 23.485 | 23.485 |
| V 2.0 features: FET 20:1, 800 us attack: onset overshoot with 2 ms lookahead | 1.329 | 1.329 | 1.333 | 1.333 | 1.370 | 1.370 | 1.367 | 1.367 | 1.424 | 1.424 |
| V 2.0 features: limiter on (1.5 ms): latency = 4x + 64 (true-peak detector) + lookahead | 209.0 | 209.0 | 215.0 | 215.0 | 229.0 | 229.0 | 241.0 | 241.0 | 385.0 | 385.0 |
| V 2.0 features: limiter -1 dBTP, +12 dB, drum-bus program, content <= 0.48 fs: sample peak | -1.050 | -1.050 | -1.050 | -1.050 | -1.050 | -1.050 | -1.050 | -1.050 | -1.050 | -1.050 |
| V 2.0 features: limiter -1 dBTP, +12 dB, drum-bus program, content <= 0.48 fs: true peak (independent long-sinc reference) | -1.050 | -1.050 | -1.050 | -1.050 | -1.050 | -1.050 | -1.050 | -1.050 | -1.050 | -1.050 |
| V 2.0 features: limiter, drum-bus program, content <= 0.48 fs: GR meter | 13.555 | 13.555 | 13.543 | 13.570 | 13.552 | 13.552 | 13.687 | 13.687 | 13.880 | 13.880 |
| V 2.0 features: limiter -1 dBTP, +12 dB, square bursts, content <= 0.48 fs: sample peak | -1.928 | -1.928 | -1.933 | -1.933 | -1.354 | -1.354 | -1.354 | -1.354 | -1.354 | -1.354 |
| V 2.0 features: limiter -1 dBTP, +12 dB, square bursts, content <= 0.48 fs: true peak (independent long-sinc reference) | -1.053 | -1.053 | -1.053 | -1.053 | -1.049 | -1.049 | -1.049 | -1.049 | -1.049 | -1.049 |
| V 2.0 features: limiter, square bursts, content <= 0.48 fs: GR meter | 3.590 | 3.590 | 3.590 | 3.590 | 3.189 | 3.189 | 3.189 | 3.189 | 3.189 | 3.189 |
| V 2.0 features: limiter -1 dBTP, +12 dB, drum-bus program, full band: sample peak | -1.050 | -1.050 | -1.051 | -1.050 | -1.050 | -1.050 | -1.050 | -1.050 | -1.050 | -1.050 |
| V 2.0 features: limiter -1 dBTP, +12 dB, drum-bus program, full band: true peak (reference), documented bound +0.4 dB | -1.014 | -1.007 | -0.9463 | -0.9903 | -1.050 | -1.050 | -1.050 | -1.050 | -1.050 | -1.050 |
| V 2.0 features: limiter -1 dBTP, +12 dB, square bursts, full band: sample peak | -2.463 | -2.463 | -2.475 | -2.475 | -1.354 | -1.354 | -1.354 | -1.354 | -1.354 | -1.354 |
| V 2.0 features: limiter -1 dBTP, +12 dB, square bursts, full band: true peak (reference), documented bound +0.4 dB | -0.8309 | -0.8309 | -0.8321 | -0.8321 | -1.049 | -1.049 | -1.049 | -1.049 | -1.049 | -1.049 |
| V 2.0 features: limiter below the ceiling: = the engine without limiter, delayed by its latency (null) | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 |
| V 2.0 features: handshake: unacknowledged latency change keeps the old latency | 79.000 | 79.000 | 79.000 | 79.000 | 33.000 | 33.000 | 33.000 | 33.000 | 33.000 | 33.000 |
| V 2.0 features: handshake: muted while waiting for the host | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 |
| V 2.0 features: handshake: a request with the new latency is posted | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| V 2.0 features: handshake: after the acknowledgement the new latency plays (impulse position - input) | 211.0 | 211.0 | 223.0 | 223.0 | 298.0 | 298.0 | 321.0 | 321.0 | 609.0 | 609.0 |
| V 2.0 features: handshake: engine latency = the acknowledged one | 211.0 | 211.0 | 223.0 | 223.0 | 298.0 | 298.0 | 321.0 | 321.0 | 609.0 | 609.0 |
| V 2.0 features: handshake timeout: latency unchanged | 211.0 | 211.0 | 223.0 | 223.0 | 298.0 | 298.0 | 321.0 | 321.0 | 609.0 | 609.0 |
| V 2.0 features: handshake timeout: the old configuration plays again after 0.5 s | -6.021 | -6.021 | -6.021 | -6.021 | -6.021 | -6.021 | -6.021 | -6.021 | -6.021 | -6.021 |
| V 2.0 features: compare: processed signal louder than the input by | 7.314 | 6.956 | 7.432 | 6.922 | 6.862 | 6.583 | 6.812 | 6.566 | 6.560 | 6.239 |
| V 2.0 features: compare: unprocessed signal played at the processed signal's loudness (difference) | 0.0575 | 0.0860 | -0.0065 | -0.0056 | 0.0113 | 0.0065 | 0.0799 | 0.0804 | 0.0224 | 0.0209 |
| V 2.0 features: compare: output is the unprocessed input (residual after one gain) | -49.131 | -49.090 | -42.636 | -54.295 | -48.737 | -44.339 | -49.104 | -48.043 | -48.668 | -48.290 |
| V 2.0 features: history: frames per second of audio | 199.0 | 199.0 | 200.0 | 200.0 | 200.0 | 200.0 | 200.0 | 200.0 | 200.0 | 200.0 |
| V 2.0 features: history: FET gain reduction in the last frame vs the stage's steady GR | -0.0049 | -0.0049 | -0.0016 | -0.0016 | -0.000454 | -0.000454 | -0.00045 | -0.00045 | -0.00045 | -0.00045 |
| V 2.0 features: all 2.0 features: block 512 vs 37 (difference) | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 |
| V 2.0 features: all 2.0 features: output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| V 2.0 features: all 2.0 features: heap allocations inside process() | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| V11 Voice hold (3.0): factory presets: VOICE HOLD on exactly for the vocal presets and Init | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| V11 Voice hold (3.0): engine default: VOICE HOLD off (the 2.x path; the plugin turns it on for new instances) | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| V11 Voice hold (3.0): FET: gain reduction on the loud tone | 17.257 | 17.257 | 17.264 | 17.264 | 17.271 | 17.271 | 17.279 | 17.279 | 17.308 | 17.308 |
| V11 Voice hold (3.0): FET without pause: share of the GR left 0.4 s after the drop (released) | 0.0994 | 0.0994 | 0.0993 | 0.0993 | 0.0996 | 0.0996 | 0.0995 | 0.0995 | 0.0995 | 0.0995 |
| V11 Voice hold (3.0): FET in a pause: gain reduction re the voice's (held) | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| V11 Voice hold (3.0): opto: gain reduction on the loud tone | 21.333 | 21.333 | 21.333 | 21.333 | 21.335 | 21.335 | 21.335 | 21.335 | 21.333 | 21.333 |
| V11 Voice hold (3.0): opto in a pause: holds more than it would release | 14.471 | 14.471 | 14.474 | 14.474 | 14.462 | 14.462 | 14.465 | 14.465 | 14.464 | 14.464 |
| V11 Voice hold (3.0): opto in a pause: gain reduction re the voice's (held) | 3.43e-05 | 3.43e-05 | 3.81e-05 | 3.81e-05 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| V11 Voice hold (3.0): pauses re voice, off: lift | 18.699 | 18.699 | 18.703 | 18.703 | 18.759 | 18.759 | 18.911 | 18.911 | 19.460 | 19.460 |
| V11 Voice hold (3.0): pauses re voice, on: lift | 0.6557 | 0.6557 | 0.6681 | 0.6681 | 0.5698 | 0.5698 | 0.6067 | 0.6067 | 0.7449 | 0.7449 |
| V11 Voice hold (3.0): VOICE HOLD lowers the pauses' lift by | 18.089 | 18.089 | 18.081 | 18.081 | 18.224 | 18.224 | 18.337 | 18.337 | 18.740 | 18.740 |
| V11 Voice hold (3.0): VOICE HOLD: voice level unchanged | -0.0449 | -0.0449 | -0.0459 | -0.0459 | -0.0350 | -0.0350 | -0.0327 | -0.0327 | -0.0249 | -0.0249 |
| V11 Voice hold (3.0): VOICE HOLD: latency unchanged | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| V11 Voice hold (3.0): drum loop: VOICE HOLD on vs off (peak difference; beat/bus presets turn it off) | -11.360 | -11.360 | -11.413 | -11.413 | -13.504 | -13.504 | -8.993 | -8.993 | -9.404 | -9.404 |
| V11 Voice hold (3.0): VOICE HOLD: block 512 vs 37 (difference) | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 |
| V11 Voice hold (3.0): VOICE HOLD: output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| V11 Voice hold (3.0): VOICE HOLD: heap allocations inside process() | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| V12 Ess hold (4.0): factory presets: ESS HOLD on exactly where VOICE HOLD is (the vocal presets and Init) | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| V12 Ess hold (4.0): engine default: ESS HOLD off (the 3.x path; the plugin turns it on for new instances) | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| V12 Ess hold (4.0): FET: gain reduction at the ess's start | 13.021 | 13.021 | 13.021 | 13.021 | 13.023 | 13.023 | 13.024 | 13.024 | 13.024 | 13.024 |
| V12 Ess hold (4.0): FET without ESS HOLD: share of that GR left at the end of the 80 ms ess (releasing) | 0.6181 | 0.6181 | 0.6181 | 0.6181 | 0.6184 | 0.6184 | 0.6183 | 0.6183 | 0.6183 | 0.6183 |
| V12 Ess hold (4.0): FET with ESS HOLD: GR at the end of the ess re its start (held, up to the word's peak) | 3.060 | 3.060 | 3.059 | 3.059 | 3.065 | 3.065 | 3.069 | 3.069 | 3.062 | 3.062 |
| V12 Ess hold (4.0): opto: gain reduction at the ess's start | 17.748 | 17.748 | 17.748 | 17.748 | 17.751 | 17.751 | 17.750 | 17.750 | 17.749 | 17.749 |
| V12 Ess hold (4.0): opto with ESS HOLD: holds more than it would release | 10.319 | 10.319 | 10.320 | 10.320 | 10.319 | 10.319 | 10.318 | 10.318 | 10.319 | 10.319 |
| V12 Ess hold (4.0): opto with ESS HOLD: GR at the end of the ess re its start (held, up to the word's peak) | 1.887 | 1.887 | 1.887 | 1.887 | 1.888 | 1.888 | 1.887 | 1.887 | 1.888 | 1.888 |
| V12 Ess hold (4.0): esses re vowels, off: lift | 2.909 | 2.909 | 2.985 | 2.985 | 1.754 | 1.754 | 1.489 | 1.489 | -0.7708 | -0.7708 |
| V12 Ess hold (4.0): esses re vowels, on: lift | 0.3359 | 0.3359 | 0.2819 | 0.2819 | -0.0116 | -0.0116 | -0.0376 | -0.0376 | -1.589 | -1.589 |
| V12 Ess hold (4.0): ESS HOLD removes of the esses' lift (dB, against 60 % of the lift without it) | 2.573 | 2.573 | 2.703 | 2.703 | 1.765 | 1.765 | 1.526 | 1.526 | 0.8187 | 0.8187 |
| V12 Ess hold (4.0): ESS HOLD never lifts the esses more | -2.573 | -2.573 | -2.703 | -2.703 | -1.765 | -1.765 | -1.526 | -1.526 | -0.8187 | -0.8187 |
| V12 Ess hold (4.0): ESS HOLD: vowel level unchanged | 0.0697 | 0.0697 | 0.0760 | 0.0760 | 0.0551 | 0.0551 | 0.0445 | 0.0445 | 0.0270 | 0.0270 |
| V12 Ess hold (4.0): ESS HOLD: latency unchanged | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| V12 Ess hold (4.0): ESS HOLD off: deterministic (two engines, same input) | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| V12 Ess hold (4.0): ESS HOLD: block 512 vs 37 (difference) | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 | -600.0 |
| V12 Ess hold (4.0): ESS HOLD: output finite | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 | 1.000 |
| V12 Ess hold (4.0): ESS HOLD: heap allocations inside process() | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| 9 Stereo identity: identical L/R in: differing output samples (defaults) | - | 0.0000 | - | 0.0000 | - | 0.0000 | - | 0.0000 | - | 0.0000 |
| 9 Stereo identity: identical L/R in: differing samples (4x, link 0, All, auto makeup) | - | 0.0000 | - | 0.0000 | - | 0.0000 | - | 0.0000 | - | 0.0000 |
| 9 Stereo identity: mono instance vs dual-mono stereo: max |diff| | - | 0.0000 | - | 0.0000 | - | 0.0000 | - | 0.0000 | - | 0.0000 |
| F Features (spec section 2): Link 100 %: quiet side follows loud side's GR (|GR_L - GR_R|) | - | 6.11e-08 | - | 2.1e-07 | - | 1.93e-08 | - | 2.28e-07 | - | 2.29e-07 |
| F Features (spec section 2): Link 0 %: quiet side GR (independent detection) | - | -4.08e-06 | - | -7e-06 | - | 7.36e-06 | - | 6.39e-06 | - | 1.42e-06 |
| V 2.0 features: Mid/Side, stages off: = stereo mode (M/S encode + decode is exact) | - | -138.5 | - | -138.5 | - | -138.5 | - | -136.5 | - | -136.5 |
| V 2.0 features: Mid/Side on identical channels = stereo mode (difference) | - | -600.0 | - | -600.0 | - | -600.0 | - | -600.0 | - | -600.0 |
| V 2.0 features: Mid/Side, link 0: quiet mid passes at its gain (6 dB input) | - | 1.17e-05 | - | 1.13e-05 | - | -1.73e-06 | - | -1.43e-06 | - | -2.88e-07 |
| V 2.0 features: Mid/Side, link 0: loud side compressed by | - | 8.993 | - | 9.000 | - | 9.005 | - | 9.014 | - | 9.043 |
| O Oversampling filters: 2x decimator (stopband from base Nyquist): stopband peak | - | - | - | - | -100.2 | -100.2 | -100.2 | -100.2 | -100.2 | -100.2 |
| O Oversampling filters: 2x decimator (stopband from base Nyquist): passband ripple | - | - | - | - | 0.000101 | 0.000101 | 0.000101 | 0.000101 | 0.000101 | 0.000101 |
| 10 CPU: 48 kHz / 128 / stereo / 2x, 60 s program: CPU of one core | - | - | - | 1.903 | - | - | - | - | - | - |
| 10 CPU: 48 kHz / 128 / stereo / 2x: real-time factor | - | - | - | 52.544 | - | - | - | - | - | - |
| 10 CPU: silence after program (denormal safety): CPU silence / CPU program | - | - | - | 0.7937 | - | - | - | - | - | - |
| 10 CPU: 48 kHz / 128 / stereo / 1x: CPU of one core | - | - | - | 1.256 | - | - | - | - | - | - |
| 10 CPU: 48 kHz / 128 / stereo / 4x: CPU of one core | - | - | - | 3.194 | - | - | - | - | - | - |
| 10 CPU: 48 kHz / 128 / stereo / 4x, every 2.0 feature on (limiter, lookahead, M/S, Clean, SC EQ): CPU of one core | - | - | - | 6.427 | - | - | - | - | - | - |
| 10 CPU: 48 kHz / 128 / stereo / 4x, a new instance (VOICE HOLD + ESS HOLD on): CPU of one core | - | - | - | 5.565 | - | - | - | - | - | - |
| 10 CPU: ESS HOLD's own cost (its sibilance detector): CPU of one core | - | - | - | 1.099 | - | - | - | - | - | - |
| 10 CPU: stereo / 128 / 2x at 44.100 kHz: CPU of one core | - | - | - | 1.751 | - | - | - | - | - | - |
| 10 CPU: stereo / 128 / 2x at 96.000 kHz: CPU of one core | - | - | - | 3.740 | - | - | - | - | - | - |
| 10 CPU: stereo / 128 / 2x at 192.0 kHz: CPU of one core | - | - | - | 7.461 | - | - | - | - | - | - |
