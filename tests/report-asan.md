# Axell Duo Compressor - DSP test report

Generated 2026-10-15 01:51 by `DuoDspTests` in 99.104 s. Stage ballistics are measured on the stage at the default 2x internal rate with a sample-exact step; everything else runs through the complete engine (oversampling, mix, output). Build: RelWithDebInfo+ASan+UBSan.

**Result: ALL PASS** - 758 checks passed, 0 failed (every check runs once per sample rate / channel configuration).

## Summary (worst case over all configurations)

| Section | Check | Target | Worst measured | at | Pass |
|---|---|---|---|---|---|
| 1 Static curve | FET 4:1: slope below threshold (-60..-44 dBFS) | 0.9800 .. 1.020 dB/dB | 1.0000 dB/dB | 44.1 k mono | PASS 4/4 |
| 1 Static curve | FET 4:1: ratio above threshold (-12..0 dBFS in) | 3.600 .. 4.400 :1 | 4.006 :1 | 44.1 k mono | PASS 4/4 |
| 1 Static curve | FET 8:1: slope below threshold (-60..-44 dBFS) | 0.9800 .. 1.020 dB/dB | 1.0000 dB/dB | 44.1 k mono | PASS 4/4 |
| 1 Static curve | FET 8:1: ratio above threshold (-12..0 dBFS in) | 7.200 .. 8.800 :1 | 8.031 :1 | 48 k mono | PASS 4/4 |
| 1 Static curve | FET 12:1: slope below threshold (-60..-44 dBFS) | 0.9800 .. 1.020 dB/dB | 1.0000 dB/dB | 44.1 k mono | PASS 4/4 |
| 1 Static curve | FET 12:1: ratio above threshold (-12..0 dBFS in) | 10.800 .. 13.200 :1 | 12.069 :1 | 48 k mono | PASS 4/4 |
| 1 Static curve | FET 20:1: slope below threshold (-60..-44 dBFS) | 0.9800 .. 1.020 dB/dB | 1.0000 dB/dB | 44.1 k mono | PASS 4/4 |
| 1 Static curve | FET 20:1: ratio above threshold (-12..0 dBFS in) | 18.000 .. 22.000 :1 | 20.183 :1 | 48 k mono | PASS 4/4 |
| 1 Static curve | FET All: slope below threshold (-60..-44 dBFS) | 0.9800 .. 1.020 dB/dB | 1.0000 dB/dB | 44.1 k mono | PASS 4/4 |
| 1 Static curve | FET All: over-compression, drop from curve peak to 0 dBFS in | > 0.3000 dB | 0.7380 dB | 44.1 k mono | PASS 4/4 |
| 1 Static curve | FET All: curve peak position above threshold | 4.000 .. 16.000 dB | 8.000 dB | 44.1 k mono | PASS 4/4 |
| 1 Static curve | FET All: drop stays 'slight' (peak to 0 dBFS in) | < 4.000 dB | 0.7422 dB | 48 k mono | PASS 4/4 |
| 1 Static curve | Opto Compress: slope below threshold (-60..-44 dBFS) | 0.9800 .. 1.020 dB/dB | 1.0000 dB/dB | 48 k mono | PASS 4/4 |
| 1 Static curve | Opto Compress: ratio above threshold (-12..0 dBFS in) | 2.700 .. 3.300 :1 | 3.000 :1 | 48 k mono | PASS 4/4 |
| 1 Static curve | Opto Limit: slope below threshold (-60..-44 dBFS) | 0.9800 .. 1.020 dB/dB | 1.0000 dB/dB | 48 k mono | PASS 4/4 |
| 1 Static curve | Opto Limit: ratio above threshold (-12..0 dBFS in) | 9.000 .. 11.000 :1 | 10.000 :1 | 44.1 k mono | PASS 4/4 |
| 2 FET attack/release | attack 20.000 us: time to 63 % GR | 16.000 .. 24.000 us | 20.642 us | 44.1 k mono | PASS 4/4 |
| 2 FET attack/release | attack 800.0 us: time to 63 % GR | 640.0 .. 960.0 us | 803.8 us | 48 k mono | PASS 4/4 |
| 2 FET attack/release | release 50.000 ms: time to 63 % recovery | 40.000 .. 60.000 ms | 55.204 ms | 48 k mono | PASS 4/4 |
| 2 FET attack/release | release 1100.0 ms: time to 63 % recovery | 880.0 .. 1320.0 ms | 1165.3 ms | 48 k mono | PASS 4/4 |
| 2 FET attack/release | program-dependent: GR during hold | (info) | 16.507 .. 16.513 dB | all | - |
| 2 FET attack/release | program-dependent: release after 6 s vs 0.3 s of >10 dB GR | 1.150 .. 1.400 x | 1.295 x | 48 k mono | PASS 4/4 |
| 2 FET attack/release | All mode: attack time relative to 20:1 (800 us knob) | < 0.8000 x | 0.6008 x | 44.1 k mono | PASS 4/4 |
| 3 Opto attack/release | attack: time to 63 % of 10 dB GR | 8.000 .. 12.000 ms | 9.796 ms | 48 k mono | PASS 4/4 |
| 3 Opto attack/release | release after 2 s of 10 dB GR: 50 % recovery | 40.000 .. 80.000 ms | 71.245 ms | 44.1 k mono | PASS 4/4 |
| 3 Opto attack/release | release tail time constant after 2 s (70->90 % recovery) | 0.5000 .. 5.000 s | 1.926 s | 48 k mono | PASS 4/4 |
| 3 Opto attack/release | 90 % recovery after 1 s of 10 dB GR | (info) | 1.918 .. 1.921 s | all | - |
| 3 Opto attack/release | 90 % recovery after 10 s of 10 dB GR | (info) | 4.864 .. 4.864 s | all | - |
| 3 Opto attack/release | light-cell memory: 90 % recovery 10 s hold / 1 s hold | > 1.500 x | 2.533 x | 44.1 k mono | PASS 4/4 |
| 3 Opto attack/release | release smoothness (slope residual vs slope, 10 s hold) | < 0.0100 ratio | 0.0026 ratio | 44.1 k mono | PASS 4/4 |
| 4 THD (1 kHz) | FET 0 dB GR, Color 0 | < 0.1000 % | 5.1e-06 % | 48 k mono | PASS 4/4 |
| 4 THD (1 kHz) | FET 10 dB GR, Color 50 (4:1) | 0.3000 .. 1.500 % | 0.6633 % | 48 k mono | PASS 4/4 |
| 4 THD (1 kHz) | FET 10 dB GR, Color 50: share of 2nd+3rd in distortion power | > 80.000 % | 99.830 % | 44.1 k mono | PASS 4/4 |
| 4 THD (1 kHz) | FET All 10 dB GR, Color 50 | (info) | 1.110 .. 1.110 % | all | - |
| 4 THD (1 kHz) | FET All vs 4:1 THD at 10 dB GR, Color 50 | > 1.250 x | 1.673 x | 48 k mono | PASS 4/4 |
| 4 THD (1 kHz) | Opto 10 dB GR, Color 0 (clean) | < 0.1000 % | 7.7e-06 % | 48 k mono | PASS 4/4 |
| 4 THD (1 kHz) | Opto 10 dB GR, Color 50 | (info) | 0.1348 .. 0.1348 % | all | - |
| 4 THD (1 kHz) | Opto Color 50: H2 / H3 (predominantly 2nd) | > 2.000 x | 118.6 x | 48 k mono | PASS 4/4 |
| 4 THD (1 kHz) | Opto Color 50: THD rises with level (0 dB vs -20 dB rel.) | > 1.300 x | 3.169 x | 48 k mono | PASS 4/4 |
| 4 THD (1 kHz) | Opto Color 100: DC offset | < -100.0 dBFS | -191.0 dBFS | 48 k mono | PASS 4/4 |
| 5 Aliasing (15 kHz, -3 dBFS, FET 20:1, Color 100) | FET only, 1x (no oversampling) | (info) | -51.574 .. -51.385 dBFS | all | - |
| 5 Aliasing (15 kHz, -3 dBFS, FET 20:1, Color 100) | FET only, 2x | < -70.000 dBFS | -84.383 dBFS | 48 k mono | PASS 4/4 |
| 5 Aliasing (15 kHz, -3 dBFS, FET 20:1, Color 100) | FET only, 4x | < -85.000 dBFS | -154.2 dBFS | 48 k mono | PASS 4/4 |
| 5 Aliasing (15 kHz, -3 dBFS, FET 20:1, Color 100) | FET + Opto (default), 1x (no oversampling) | (info) | -54.778 .. -54.586 dBFS | all | - |
| 5 Aliasing (15 kHz, -3 dBFS, FET 20:1, Color 100) | FET + Opto (default), 2x | < -70.000 dBFS | -91.428 dBFS | 48 k mono | PASS 4/4 |
| 5 Aliasing (15 kHz, -3 dBFS, FET 20:1, Color 100) | FET + Opto (default), 4x | < -85.000 dBFS | -135.6 dBFS | 44.1 k mono | PASS 4/4 |
| 6 Null / latency | Mix 0 %: residual vs input delayed by reported latency, 1x | < -100.0 dBFS | -600.0 dBFS | 44.1 k mono | PASS 4/4 |
| 6 Null / latency | reported latency matches factor, 1x | = 0.0000 samples | 0.0000 samples | 44.1 k mono | PASS 4/4 |
| 6 Null / latency | wet impulse peak offset - reported latency, 1x | = 0.0000 samples | 0.0000 samples | 44.1 k mono | PASS 4/4 |
| 6 Null / latency | Mix 50 %, stages off: residual vs delayed dry (<18 kHz), 1x | < -80.000 dBFS | -600.0 dBFS | 44.1 k mono | PASS 4/4 |
| 6 Null / latency | Mix 0 %: residual vs input delayed by reported latency, 2x | < -100.0 dBFS | -600.0 dBFS | 44.1 k mono | PASS 4/4 |
| 6 Null / latency | reported latency matches factor, 2x | = 0.0000 samples | 0.0000 samples | 44.1 k mono | PASS 4/4 |
| 6 Null / latency | wet impulse peak offset - reported latency, 2x | = 0.0000 samples | 0.0000 samples | 44.1 k mono | PASS 4/4 |
| 6 Null / latency | Mix 50 %, stages off: residual vs delayed dry (<18 kHz), 2x | < -80.000 dBFS | -131.9 dBFS | 44.1 k mono | PASS 4/4 |
| 6 Null / latency | Mix 0 %: residual vs input delayed by reported latency, 4x | < -100.0 dBFS | -600.0 dBFS | 44.1 k mono | PASS 4/4 |
| 6 Null / latency | reported latency matches factor, 4x | = 0.0000 samples | 0.0000 samples | 44.1 k mono | PASS 4/4 |
| 6 Null / latency | wet impulse peak offset - reported latency, 4x | = 0.0000 samples | 0.0000 samples | 44.1 k mono | PASS 4/4 |
| 6 Null / latency | Mix 50 %, stages off: residual vs delayed dry (<18 kHz), 4x | < -80.000 dBFS | -121.3 dBFS | 44.1 k mono | PASS 4/4 |
| 7 Silence / stability | zeros after +24 dBFS noise: max |out| after 1 s of silence | < 1e-07  | 4.01e-17  | 44.1 k mono | PASS 4/4 |
| 7 Silence / stability | zeros from fresh instance: max |out| | < 1e-07  | 0.0000  | 44.1 k mono | PASS 4/4 |
| 7 Silence / stability | random automation every block, +24 dBFS: all samples finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 4/4 |
| 7 Silence / stability | random automation: peak output (sanity bound) | < 80.000 dBFS | 38.825 dBFS | 44.1 k stereo | PASS 4/4 |
| 7 Silence / stability | random automation then silence: max |out| in last 0.5 s | < 1e-07  | 1.52e-23  | 48 k mono | PASS 4/4 |
| 8 Block-size invariance | max |diff| blocks 1/17/64/2048/4096 vs 512, 1x | < 1e-05  | 0.0000  | 44.1 k mono | PASS 4/4 |
| 8 Block-size invariance | max |diff| blocks 1/17/64/2048/4096 vs 512, 2x | < 1e-05  | 0.0000  | 44.1 k mono | PASS 4/4 |
| 8 Block-size invariance | max |diff| blocks 1/17/64/2048/4096 vs 512, 4x | < 1e-05  | 0.0000  | 44.1 k mono | PASS 4/4 |
| F Features (spec section 2) | FET SC HPF: GR reduction on 60 Hz, HPF 20 -> 300 Hz | > 6.000 dB | 8.908 dB | 44.1 k mono | PASS 4/4 |
| F Features (spec section 2) | Opto SC HPF: GR reduction on 60 Hz, HPF 20 -> 300 Hz | > 6.000 dB | 17.625 dB | 44.1 k mono | PASS 4/4 |
| F Features (spec section 2) | SC Listen (HPF 300 Hz): 50 Hz relative to 3 kHz in output | < -20.000 dB | -31.131 dB | 48 k mono | PASS 4/4 |
| F Features (spec section 2) | Emphasis 100 %: extra opto GR on 8 kHz | > 4.000 dB | 7.780 dB | 48 k mono | PASS 4/4 |
| F Features (spec section 2) | Emphasis 100 %: GR change on 200 Hz (should be ~0) | < 0.5000 dB | 0.000176 dB | 48 k mono | PASS 4/4 |
| F Features (spec section 2) | Auto makeup: final makeup vs average GR (|diff|) | < 1.500 dB | 0.1052 dB | 48 k mono | PASS 4/4 |
| F Features (spec section 2) | Auto makeup: time to 63 % of final makeup (>= 1 s) | 1.000 .. 6.000 s | 2.305 s | 48 k mono | PASS 4/4 |
| F Features (spec section 2) | Auto makeup: cap under extreme GR | < 24.001 dB | 23.987 dB | 44.1 k mono | PASS 4/4 |
| F Features (spec section 2) | OS switching every 37 blocks: all samples finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 4/4 |
| F Features (spec section 2) | OS switching: peak output vs peak input | < 12.000 dB | -7.057 dB | 44.1 k mono | PASS 4/4 |
| F Features (spec section 2) | OS switched to 4x: reported latency - latency of 4x | = 0.0000 samples | 0.0000 samples | 44.1 k mono | PASS 4/4 |
| F Features (spec section 2) | OS switched to 4x: impulse peak - reported latency | = 0.0000 samples | 0.0000 samples | 44.1 k mono | PASS 4/4 |
| F Features (spec section 2) | Order switch while playing: max sample step vs steady tone | < 1.500 x | 1.0000 x | 48 k mono | PASS 4/4 |
| F Features (spec section 2) | Output +12 dB step: 10-90 % transition | > 5.000 ms | 44.000 ms | 44.1 k mono | PASS 4/4 |
| F Features (spec section 2) | FET Output -20 dB step: 10-90 % transition | > 5.000 ms | 44.000 ms | 44.1 k mono | PASS 4/4 |
| F Features (spec section 2) | Opto Gain +12 dB step: 10-90 % transition | > 5.000 ms | 44.000 ms | 44.1 k mono | PASS 4/4 |
| F Features (spec section 2) | FET Input -18 dB step: 10-90 % transition | > 5.000 ms | 44.000 ms | 44.1 k mono | PASS 4/4 |
| F Features (spec section 2) | Mix 100 -> 0 % step (wet 20 dB lower): 10-90 % transition | > 5.000 ms | 44.000 ms | 44.1 k mono | PASS 4/4 |
| O Oversampling filters | 2x stage (up/down): stopband peak | < -100.0 dB | -100.9 dB | 44.1 k mono | PASS 4/4 |
| O Oversampling filters | 2x stage (up/down): passband ripple | < 0.0010 dB | 7.82e-05 dB | 44.1 k mono | PASS 4/4 |
| O Oversampling filters | 4x stage: stopband peak | < -100.0 dB | -100.4 dB | 44.1 k mono | PASS 4/4 |
| O Oversampling filters | 4x stage: passband ripple | < 0.0010 dB | 8.34e-05 dB | 44.1 k mono | PASS 4/4 |
| O Oversampling filters | latency 1x | (info) | 0.0000 samples | all | - |
| O Oversampling filters | wet path (stages off) flatness 20 Hz-20 kHz, 1x | < 0.0100 dB | 0.0000 dB | 44.1 k mono | PASS 4/4 |
| O Oversampling filters | latency 2x | (info) | 71.000 samples | all | - |
| O Oversampling filters | wet path (stages off) flatness 20 Hz-20 kHz, 2x | < 0.0100 dB | 0.000157 dB | 44.1 k mono | PASS 4/4 |
| O Oversampling filters | latency 4x | (info) | 79.000 samples | all | - |
| O Oversampling filters | wet path (stages off) flatness 20 Hz-20 kHz, 4x | < 0.0100 dB | 0.000205 dB | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Rap Lead: output vs input loudness | -1.500 .. 1.500 dB | 0.3678 dB | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Rap Lead: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Rap Lead: FET peak GR | (info) | 6.720 .. 7.254 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Rap Lead: opto average GR | (info) | 2.996 .. 3.046 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Boom Bap Double: output vs input loudness | -1.500 .. 1.500 dB | -0.1869 dB | 48 k stereo | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Boom Bap Double: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Boom Bap Double: FET peak GR | (info) | 9.544 .. 9.912 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Boom Bap Double: opto average GR | (info) | 4.468 .. 4.595 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Aggressive Parallel (All): output vs input loudness | -1.500 .. 1.500 dB | -0.1231 dB | 48 k stereo | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Aggressive Parallel (All): output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Aggressive Parallel (All): FET peak GR | (info) | 17.133 .. 17.740 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Aggressive Parallel (All): opto average GR | (info) | 1.498 .. 1.733 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Gentle Leveling: output vs input loudness | -1.500 .. 1.500 dB | -0.2269 dB | 48 k stereo | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Gentle Leveling: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Gentle Leveling: FET peak GR | (info) | 2.961 .. 3.591 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Gentle Leveling: opto average GR | (info) | 3.907 .. 4.024 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Spoken Word: output vs input loudness | -1.500 .. 1.500 dB | -0.3080 dB | 48 k stereo | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Spoken Word: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Spoken Word: FET peak GR | (info) | 4.859 .. 5.452 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Spoken Word: opto average GR | (info) | 3.936 .. 4.012 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Classic Serial (FET > Opto): output vs input loudness | -1.500 .. 1.500 dB | -0.2367 dB | 48 k stereo | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Classic Serial (FET > Opto): output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Classic Serial (FET > Opto): FET peak GR | (info) | 4.866 .. 5.457 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Classic Serial (FET > Opto): opto average GR | (info) | 3.035 .. 3.087 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Opto First, FET Catch: output vs input loudness | -1.500 .. 1.500 dB | 0.5990 dB | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Opto First, FET Catch: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Opto First, FET Catch: FET peak GR | (info) | 3.656 .. 3.819 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Opto First, FET Catch: opto average GR | (info) | 3.747 .. 3.966 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: New York Parallel: output vs input loudness | -1.500 .. 1.500 dB | -0.0917 dB | 48 k stereo | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: New York Parallel: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: New York Parallel: FET peak GR | (info) | 13.638 .. 14.227 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: New York Parallel: opto average GR | (info) | 1.480 .. 1.630 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Upfront Hip-Hop: output vs input loudness | -1.500 .. 1.500 dB | 0.1828 dB | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Upfront Hip-Hop: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Upfront Hip-Hop: FET peak GR | (info) | 7.409 .. 7.891 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Upfront Hip-Hop: opto average GR | (info) | 2.985 .. 3.051 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Ad-libs & Hype: output vs input loudness | -1.500 .. 1.500 dB | -0.2504 dB | 48 k stereo | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Ad-libs & Hype: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Ad-libs & Hype: FET peak GR | (info) | 10.682 .. 11.192 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Ad-libs & Hype: opto average GR | (info) | 3.989 .. 4.145 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Sibilance-Aware Leveling: output vs input loudness | -1.500 .. 1.500 dB | -0.2784 dB | 48 k stereo | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Sibilance-Aware Leveling: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Sibilance-Aware Leveling: FET peak GR | (info) | 2.597 .. 3.192 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Sibilance-Aware Leveling: opto average GR | (info) | 3.922 .. 3.962 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Transparent Peak Control: output vs input loudness | -1.500 .. 1.500 dB | 0.5690 dB | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Transparent Peak Control: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Transparent Peak Control: FET peak GR | (info) | 5.507 .. 6.129 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Transparent Peak Control: opto average GR | (info) | 0.0000 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Sung Hook: output vs input loudness | -1.500 .. 1.500 dB | 0.3233 dB | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Sung Hook: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Sung Hook: FET peak GR | (info) | 3.720 .. 4.317 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Sung Hook: opto average GR | (info) | 4.974 .. 5.072 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Lo-fi Grit (All): output vs input loudness | -1.500 .. 1.500 dB | -0.2610 dB | 44.1 k stereo | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Lo-fi Grit (All): output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Lo-fi Grit (All): FET peak GR | (info) | 14.814 .. 15.605 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Lo-fi Grit (All): opto average GR | (info) | 1.936 .. 2.187 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Broadcast Voice: output vs input loudness | -1.500 .. 1.500 dB | -0.2937 dB | 48 k stereo | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Broadcast Voice: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Broadcast Voice: FET peak GR | (info) | 5.651 .. 6.275 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Broadcast Voice: opto average GR | (info) | 5.072 .. 5.127 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Mix Bus: Glue: output vs input loudness | -1.500 .. 1.500 dB | 0.0614 dB | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Mix Bus: Glue: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Mix Bus: Glue: FET peak GR | (info) | 1.846 .. 2.021 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Mix Bus: Glue: opto average GR | (info) | 1.476 .. 1.511 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Mix Bus: Punch: output vs input loudness | -1.500 .. 1.500 dB | -0.0773 dB | 48 k stereo | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Mix Bus: Punch: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Mix Bus: Punch: FET peak GR | (info) | 3.674 .. 4.052 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Mix Bus: Punch: opto average GR | (info) | 0.0000 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: Transparent Leveling: output vs input loudness | -1.500 .. 1.500 dB | 0.0791 dB | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: Transparent Leveling: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: Transparent Leveling: FET peak GR | (info) | 0.0000 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: Transparent Leveling: opto average GR | (info) | 1.992 .. 2.018 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: Peak Catch: output vs input loudness | -1.500 .. 1.500 dB | 0.0417 dB | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: Peak Catch: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: Peak Catch: FET peak GR | (info) | 1.621 .. 2.135 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: Peak Catch: opto average GR | (info) | 0.0000 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: Parallel Density: output vs input loudness | -1.500 .. 1.500 dB | -0.2377 dB | 48 k stereo | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: Parallel Density: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: Parallel Density: FET peak GR | (info) | 7.268 .. 7.798 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: Parallel Density: opto average GR | (info) | 0.0000 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: True-Peak Safety (-1 dBTP): output vs input loudness | -1.500 .. 1.500 dB | -1.080 dB | 44.1 k stereo | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: True-Peak Safety (-1 dBTP): output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: True-Peak Safety (-1 dBTP): FET peak GR | (info) | 0.0000 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Master: True-Peak Safety (-1 dBTP): opto average GR | (info) | 2.151 .. 2.181 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Bass & 808: Clean Control: output vs input loudness | -1.500 .. 1.500 dB | -1.058 dB | 48 k stereo | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Bass & 808: Clean Control: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Bass & 808: Clean Control: FET peak GR | (info) | 7.701 .. 8.156 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Bass & 808: Clean Control: opto average GR | (info) | 0.0000 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Mix Bus: Mid/Side Glue: output vs input loudness | -1.500 .. 1.500 dB | -0.5980 dB | 48 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Mix Bus: Mid/Side Glue: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Mix Bus: Mid/Side Glue: FET peak GR | (info) | 2.986 .. 3.310 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Mix Bus: Mid/Side Glue: opto average GR | (info) | 1.775 .. 1.841 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Lookahead Peak Control: output vs input loudness | -1.500 .. 1.500 dB | -0.5931 dB | 48 k stereo | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Lookahead Peak Control: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Lookahead Peak Control: FET peak GR | (info) | 7.403 .. 8.026 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Vocal: Lookahead Peak Control: opto average GR | (info) | 2.662 .. 2.734 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Drum Bus: Boom Bap Smash: output vs input loudness | -1.500 .. 1.500 dB | -0.2371 dB | 48 k stereo | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Drum Bus: Boom Bap Smash: output finite | no NaN/Inf | 1.000  | 44.1 k mono | PASS 4/4 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Drum Bus: Boom Bap Smash: FET peak GR | (info) | 14.677 .. 15.272 dB | all | - |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS) | Drum Bus: Boom Bap Smash: opto average GR | (info) | 0.0000 dB | all | - |
| V 2.0 features | Vintage detector (1.x): FET 4:1, 20 us / 50 ms, Color 0, 30.000 Hz -6 dBFS (+12 dB in): THD+N | (info) | -43.488 .. -43.488 dB | all | - |
| V 2.0 features | Clean detector: FET 4:1, 20 us / 50 ms, Color 0, 30.000 Hz -6 dBFS (+12 dB in): THD+N | < -90.000 dB | -142.2 dB | 48 k mono | PASS 4/4 |
| V 2.0 features | Vintage detector (1.x): FET 4:1, 20 us / 50 ms, Color 0, 100.0 Hz -6 dBFS (+12 dB in): THD+N | (info) | -46.372 .. -46.369 dB | all | - |
| V 2.0 features | Clean detector: FET 4:1, 20 us / 50 ms, Color 0, 100.0 Hz -6 dBFS (+12 dB in): THD+N | < -90.000 dB | -143.4 dB | 48 k mono | PASS 4/4 |
| V 2.0 features | Clean vs Vintage: steady GR at 1 kHz -6 dBFS (difference) | -0.3000 .. 0.3000 dB | 0.0055 dB | 44.1 k mono | PASS 4/4 |
| V 2.0 features | Clean vs Vintage: onset peak (first 0.5 ms, 20:1, 20 us) difference | -0.5000 .. 0.5000 dB | 0.0000 dB | 44.1 k mono | PASS 4/4 |
| V 2.0 features | 4x: worst non-harmonic component, 15 kHz -3 dBFS, 20:1 +24 dB, Color 100 | < -120.0 dBFS | -149.5 dBFS | 48 k mono | PASS 4/4 |
| V 2.0 features | FET threshold -24 dB: GR at -18 dBFS = GR of the -12 dB threshold at -6 dBFS | -0.1000 .. 0.1000 dB | 4.77e-07 dB | 44.1 k mono | PASS 4/4 |
| V 2.0 features | FET hard knee: 4:1, 6 dB over threshold -> GR 4.5 dB | 4.350 .. 4.650 dB | 4.521 dB | 48 k mono | PASS 4/4 |
| V 2.0 features | FET hard knee: 1 dB under threshold -> GR | < 0.0500 dB | 0.0000 dB | 44.1 k mono | PASS 4/4 |
| V 2.0 features | FET 12 dB knee: GR at threshold = slope * knee / 8 (1.125 dB) | 1.000 .. 1.250 dB | 1.136 dB | 48 k mono | PASS 4/4 |
| V 2.0 features | FET range 6 dB: GR with 20 dB of demand | 5.900 .. 6.010 dB | 6.000 dB | 44.1 k mono | PASS 4/4 |
| V 2.0 features | Opto range 3 dB: GR with heavy demand | 2.900 .. 3.010 dB | 3.000 dB | 44.1 k mono | PASS 4/4 |
| V 2.0 features | Opto threshold -20 dB: GR at -16 dBFS = GR of the -10 dB threshold at -6 dBFS | -0.1500 .. 0.1500 dB | -1.91e-06 dB | 48 k mono | PASS 4/4 |
| V 2.0 features | FET hold 100 ms: GR change 80 ms after the drop | < 0.1000 dB | 0.0000 dB | 44.1 k mono | PASS 4/4 |
| V 2.0 features | FET hold 100 ms: released 200 ms after the drop (GR fraction left) | < 0.8000 x | 0.1843 x | 44.1 k mono | PASS 4/4 |
| V 2.0 features | Opto hold 100 ms: GR change 80 ms after the drop | < 0.1000 dB | 3.81e-05 dB | 44.1 k mono | PASS 4/4 |
| V 2.0 features | Opto hold 100 ms: released 200 ms after the drop (GR fraction left) | < 0.8000 x | 0.4244 x | 44.1 k mono | PASS 4/4 |
| V 2.0 features | SC bell +12 dB at 3 kHz: GR(3 kHz) - GR(300 Hz), equal levels | > 6.000 dB | 8.887 dB | 48 k mono | PASS 4/4 |
| V 2.0 features | SC LPF 1 kHz: GR on an 8 kHz tone at -6 dBFS | < 1.500 dB | 0.0000 dB | 44.1 k mono | PASS 4/4 |
| V 2.0 features | SC LPF 1 kHz: GR on a 300 Hz tone at -6 dBFS | > 3.000 dB | 8.981 dB | 48 k mono | PASS 4/4 |
| V 2.0 features | FET external key 120 Hz -3 dBFS ducks a -30 dBFS 1 kHz track by | > 6.000 dB | 10.643 dB | 48 k mono | PASS 4/4 |
| V 2.0 features | FET external key quiet: track passes at its gain | -0.0500 .. 0.0500 dB | -5.64e-06 dB | 44.1 k mono | PASS 4/4 |
| V 2.0 features | FET external key, no side-chain input: track passes at its gain | -0.0500 .. 0.0500 dB | -5.64e-06 dB | 44.1 k mono | PASS 4/4 |
| V 2.0 features | Opto external key 120 Hz -3 dBFS ducks a -30 dBFS 1 kHz track by | > 6.000 dB | 17.487 dB | 44.1 k mono | PASS 4/4 |
| V 2.0 features | Opto external key quiet: track passes at its gain | -0.0500 .. 0.0500 dB | -5.95e-06 dB | 44.1 k mono | PASS 4/4 |
| V 2.0 features | Opto external key, no side-chain input: track passes at its gain | -0.0500 .. 0.0500 dB | -5.95e-06 dB | 44.1 k mono | PASS 4/4 |
| V 2.0 features | lookahead 5 ms: latency = 4x + 5 ms | = 300.0 samples | 300.0 samples | 44.1 k mono | PASS 4/4 |
| V 2.0 features | lookahead 5 ms, stages off: output = input delayed by the latency (<18 kHz) | < -80.000 dBFS | -115.5 dBFS | 44.1 k mono | PASS 4/4 |
| V 2.0 features | lookahead 5 ms, stages off: = the engine without lookahead, delayed 5 ms | < -140.0 dBFS | -600.0 dBFS | 44.1 k mono | PASS 4/4 |
| V 2.0 features | FET 20:1, 800 us attack: onset overshoot without lookahead | (info) | 23.340 .. 23.357 dB | all | - |
| V 2.0 features | FET 20:1, 800 us attack: onset overshoot with 2 ms lookahead | < 20.340 dB | 1.329 dB | 44.1 k mono | PASS 4/4 |
| V 2.0 features | limiter on (1.5 ms): latency = 4x + 64 (true-peak detector) + lookahead | = 209.0 samples | 209.0 samples | 44.1 k mono | PASS 4/4 |
| V 2.0 features | limiter -1 dBTP, +12 dB, drum-bus program, content <= 0.48 fs: sample peak | < -1.000 dBFS | -1.050 dBFS | 44.1 k mono | PASS 4/4 |
| V 2.0 features | limiter -1 dBTP, +12 dB, drum-bus program, content <= 0.48 fs: true peak (independent long-sinc reference) | < -1.000 dBTP | -1.050 dBTP | 44.1 k mono | PASS 4/4 |
| V 2.0 features | limiter, drum-bus program, content <= 0.48 fs: GR meter | > 3.000 dB | 13.543 dB | 48 k mono | PASS 4/4 |
| V 2.0 features | limiter -1 dBTP, +12 dB, square bursts, content <= 0.48 fs: sample peak | < -1.000 dBFS | -1.928 dBFS | 44.1 k mono | PASS 4/4 |
| V 2.0 features | limiter -1 dBTP, +12 dB, square bursts, content <= 0.48 fs: true peak (independent long-sinc reference) | < -1.000 dBTP | -1.053 dBTP | 48 k mono | PASS 4/4 |
| V 2.0 features | limiter, square bursts, content <= 0.48 fs: GR meter | > 3.000 dB | 3.590 dB | 44.1 k mono | PASS 4/4 |
| V 2.0 features | limiter -1 dBTP, +12 dB, drum-bus program, full band: sample peak | < -1.000 dBFS | -1.050 dBFS | 44.1 k stereo | PASS 4/4 |
| V 2.0 features | limiter -1 dBTP, +12 dB, drum-bus program, full band: true peak (reference), documented bound +0.4 dB | < -0.6000 dBTP | -0.9463 dBTP | 48 k mono | PASS 4/4 |
| V 2.0 features | limiter -1 dBTP, +12 dB, square bursts, full band: sample peak | < -1.000 dBFS | -2.463 dBFS | 44.1 k mono | PASS 4/4 |
| V 2.0 features | limiter -1 dBTP, +12 dB, square bursts, full band: true peak (reference), documented bound +0.4 dB | < -0.6000 dBTP | -0.8309 dBTP | 44.1 k mono | PASS 4/4 |
| V 2.0 features | limiter below the ceiling: = the engine without limiter, delayed by its latency (null) | < -140.0 dBFS | -600.0 dBFS | 44.1 k mono | PASS 4/4 |
| V 2.0 features | handshake: unacknowledged latency change keeps the old latency | = 79.000 samples | 79.000 samples | 44.1 k mono | PASS 4/4 |
| V 2.0 features | handshake: muted while waiting for the host | < -120.0 dBFS | -600.0 dBFS | 44.1 k mono | PASS 4/4 |
| V 2.0 features | handshake: a request with the new latency is posted | yes | 1.000  | 44.1 k mono | PASS 4/4 |
| V 2.0 features | handshake: after the acknowledgement the new latency plays (impulse position - input) | = 211.0 samples | 211.0 samples | 44.1 k mono | PASS 4/4 |
| V 2.0 features | handshake: engine latency = the acknowledged one | = 211.0 samples | 211.0 samples | 44.1 k mono | PASS 4/4 |
| V 2.0 features | handshake timeout: latency unchanged | = 211.0 samples | 211.0 samples | 44.1 k mono | PASS 4/4 |
| V 2.0 features | handshake timeout: the old configuration plays again after 0.5 s | > -7.000 dBFS | -6.021 dBFS | 44.1 k mono | PASS 4/4 |
| V 2.0 features | all 2.0 features: block 512 vs 37 (difference) | < -140.0 dBFS | -600.0 dBFS | 44.1 k mono | PASS 4/4 |
| V 2.0 features | all 2.0 features: output finite | yes | 1.000  | 44.1 k mono | PASS 4/4 |
| V 2.0 features | all 2.0 features: heap allocations inside process() | = 0.0000  | 0.0000  | 44.1 k mono | PASS 4/4 |
| 9 Stereo identity | identical L/R in: differing output samples (defaults) | = 0.0000 samples | 0.0000 samples | 44.1 k stereo | PASS 2/2 |
| 9 Stereo identity | identical L/R in: differing samples (4x, link 0, All, auto makeup) | = 0.0000 samples | 0.0000 samples | 44.1 k stereo | PASS 2/2 |
| 9 Stereo identity | mono instance vs dual-mono stereo: max |diff| | < 1e-06  | 0.0000  | 44.1 k stereo | PASS 2/2 |
| F Features (spec section 2) | Link 100 %: quiet side follows loud side's GR (|GR_L - GR_R|) | < 0.5000 dB | 2.1e-07 dB | 48 k stereo | PASS 2/2 |
| F Features (spec section 2) | Link 0 %: quiet side GR (independent detection) | < 0.2000 dB | -4.08e-06 dB | 44.1 k stereo | PASS 2/2 |
| V 2.0 features | Mid/Side, stages off: = stereo mode (M/S encode + decode is exact) | < -130.0 dBFS | -138.5 dBFS | 44.1 k stereo | PASS 2/2 |
| V 2.0 features | Mid/Side on identical channels = stereo mode (difference) | < -100.0 dBFS | -600.0 dBFS | 44.1 k stereo | PASS 2/2 |
| V 2.0 features | Mid/Side, link 0: quiet mid passes at its gain (6 dB input) | -0.0500 .. 0.0500 dB | 1.17e-05 dB | 44.1 k stereo | PASS 2/2 |
| V 2.0 features | Mid/Side, link 0: loud side compressed by | > 3.000 dB | 8.993 dB | 44.1 k stereo | PASS 2/2 |

## Detail per configuration

| Check | 44.1 k mono | 44.1 k stereo | 48 k mono | 48 k stereo |
|---|---|---|---|---|
| 1 Static curve: FET 4:1: slope below threshold (-60..-44 dBFS) | 1.0000 | 1.0000 | 1.0000 | 1.0000 |
| 1 Static curve: FET 4:1: ratio above threshold (-12..0 dBFS in) | 4.006 | 4.006 | 4.002 | 4.002 |
| 1 Static curve: FET 8:1: slope below threshold (-60..-44 dBFS) | 1.0000 | 1.0000 | 1.0000 | 1.0000 |
| 1 Static curve: FET 8:1: ratio above threshold (-12..0 dBFS in) | 8.030 | 8.030 | 8.031 | 8.031 |
| 1 Static curve: FET 12:1: slope below threshold (-60..-44 dBFS) | 1.0000 | 1.0000 | 1.0000 | 1.0000 |
| 1 Static curve: FET 12:1: ratio above threshold (-12..0 dBFS in) | 12.057 | 12.057 | 12.069 | 12.069 |
| 1 Static curve: FET 20:1: slope below threshold (-60..-44 dBFS) | 1.0000 | 1.0000 | 1.0000 | 1.0000 |
| 1 Static curve: FET 20:1: ratio above threshold (-12..0 dBFS in) | 20.132 | 20.132 | 20.183 | 20.183 |
| 1 Static curve: FET All: slope below threshold (-60..-44 dBFS) | 1.0000 | 1.0000 | 1.0000 | 1.0000 |
| 1 Static curve: FET All: over-compression, drop from curve peak to 0 dBFS in | 0.7380 | 0.7380 | 0.7422 | 0.7422 |
| 1 Static curve: FET All: curve peak position above threshold | 8.000 | 8.000 | 8.000 | 8.000 |
| 1 Static curve: FET All: drop stays 'slight' (peak to 0 dBFS in) | 0.7380 | 0.7380 | 0.7422 | 0.7422 |
| 1 Static curve: Opto Compress: slope below threshold (-60..-44 dBFS) | 1.0000 | 1.0000 | 1.0000 | 1.0000 |
| 1 Static curve: Opto Compress: ratio above threshold (-12..0 dBFS in) | 3.000 | 3.000 | 3.000 | 3.000 |
| 1 Static curve: Opto Limit: slope below threshold (-60..-44 dBFS) | 1.0000 | 1.0000 | 1.0000 | 1.0000 |
| 1 Static curve: Opto Limit: ratio above threshold (-12..0 dBFS in) | 10.000 | 10.000 | 10.000 | 10.000 |
| 2 FET attack/release: attack 20.000 us: time to 63 % GR | 20.642 | 20.642 | 20.274 | 20.274 |
| 2 FET attack/release: attack 800.0 us: time to 63 % GR | 803.6 | 803.6 | 803.8 | 803.8 |
| 2 FET attack/release: release 50.000 ms: time to 63 % recovery | 55.169 | 55.169 | 55.204 | 55.204 |
| 2 FET attack/release: release 1100.0 ms: time to 63 % recovery | 1164.3 | 1164.3 | 1165.3 | 1165.3 |
| 2 FET attack/release: program-dependent: GR during hold | 16.507 | 16.507 | 16.513 | 16.513 |
| 2 FET attack/release: program-dependent: release after 6 s vs 0.3 s of >10 dB GR | 1.295 | 1.295 | 1.295 | 1.295 |
| 2 FET attack/release: All mode: attack time relative to 20:1 (800 us knob) | 0.6008 | 0.6008 | 0.6005 | 0.6005 |
| 3 Opto attack/release: attack: time to 63 % of 10 dB GR | 9.798 | 9.798 | 9.796 | 9.796 |
| 3 Opto attack/release: release after 2 s of 10 dB GR: 50 % recovery | 71.245 | 71.245 | 71.233 | 71.233 |
| 3 Opto attack/release: release tail time constant after 2 s (70->90 % recovery) | 1.928 | 1.928 | 1.926 | 1.926 |
| 3 Opto attack/release: 90 % recovery after 1 s of 10 dB GR | 1.921 | 1.921 | 1.918 | 1.918 |
| 3 Opto attack/release: 90 % recovery after 10 s of 10 dB GR | 4.864 | 4.864 | 4.864 | 4.864 |
| 3 Opto attack/release: light-cell memory: 90 % recovery 10 s hold / 1 s hold | 2.533 | 2.533 | 2.537 | 2.537 |
| 3 Opto attack/release: release smoothness (slope residual vs slope, 10 s hold) | 0.0026 | 0.0026 | 0.0026 | 0.0026 |
| 4 THD (1 kHz): FET 0 dB GR, Color 0 | 1.72e-06 | 1.72e-06 | 5.1e-06 | 5.1e-06 |
| 4 THD (1 kHz): FET 10 dB GR, Color 50 (4:1) | 0.6635 | 0.6635 | 0.6633 | 0.6633 |
| 4 THD (1 kHz): FET 10 dB GR, Color 50: share of 2nd+3rd in distortion power | 99.830 | 99.830 | 99.830 | 99.830 |
| 4 THD (1 kHz): FET All 10 dB GR, Color 50 | 1.110 | 1.110 | 1.110 | 1.110 |
| 4 THD (1 kHz): FET All vs 4:1 THD at 10 dB GR, Color 50 | 1.673 | 1.673 | 1.673 | 1.673 |
| 4 THD (1 kHz): Opto 10 dB GR, Color 0 (clean) | 2.19e-06 | 2.19e-06 | 7.7e-06 | 7.7e-06 |
| 4 THD (1 kHz): Opto 10 dB GR, Color 50 | 0.1348 | 0.1348 | 0.1348 | 0.1348 |
| 4 THD (1 kHz): Opto Color 50: H2 / H3 (predominantly 2nd) | 118.7 | 118.7 | 118.6 | 118.6 |
| 4 THD (1 kHz): Opto Color 50: THD rises with level (0 dB vs -20 dB rel.) | 3.169 | 3.169 | 3.169 | 3.169 |
| 4 THD (1 kHz): Opto Color 100: DC offset | -203.7 | -203.7 | -191.0 | -191.0 |
| 5 Aliasing (15 kHz, -3 dBFS, FET 20:1, Color 100): FET only, 1x (no oversampling) | -51.574 | -51.574 | -51.385 | -51.385 |
| 5 Aliasing (15 kHz, -3 dBFS, FET 20:1, Color 100): FET only, 2x | -84.472 | -84.472 | -84.383 | -84.383 |
| 5 Aliasing (15 kHz, -3 dBFS, FET 20:1, Color 100): FET only, 4x | -157.2 | -157.2 | -154.2 | -154.2 |
| 5 Aliasing (15 kHz, -3 dBFS, FET 20:1, Color 100): FET + Opto (default), 1x (no oversampling) | -54.778 | -54.778 | -54.586 | -54.586 |
| 5 Aliasing (15 kHz, -3 dBFS, FET 20:1, Color 100): FET + Opto (default), 2x | -91.519 | -91.519 | -91.428 | -91.428 |
| 5 Aliasing (15 kHz, -3 dBFS, FET 20:1, Color 100): FET + Opto (default), 4x | -135.6 | -135.6 | -152.9 | -152.9 |
| 6 Null / latency: Mix 0 %: residual vs input delayed by reported latency, 1x | -600.0 | -600.0 | -600.0 | -600.0 |
| 6 Null / latency: reported latency matches factor, 1x | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| 6 Null / latency: wet impulse peak offset - reported latency, 1x | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| 6 Null / latency: Mix 50 %, stages off: residual vs delayed dry (<18 kHz), 1x | -600.0 | -600.0 | -600.0 | -600.0 |
| 6 Null / latency: Mix 0 %: residual vs input delayed by reported latency, 2x | -600.0 | -600.0 | -600.0 | -600.0 |
| 6 Null / latency: reported latency matches factor, 2x | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| 6 Null / latency: wet impulse peak offset - reported latency, 2x | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| 6 Null / latency: Mix 50 %, stages off: residual vs delayed dry (<18 kHz), 2x | -131.9 | -131.9 | -132.5 | -132.5 |
| 6 Null / latency: Mix 0 %: residual vs input delayed by reported latency, 4x | -600.0 | -600.0 | -600.0 | -600.0 |
| 6 Null / latency: reported latency matches factor, 4x | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| 6 Null / latency: wet impulse peak offset - reported latency, 4x | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| 6 Null / latency: Mix 50 %, stages off: residual vs delayed dry (<18 kHz), 4x | -121.3 | -121.3 | -121.4 | -121.4 |
| 7 Silence / stability: zeros after +24 dBFS noise: max |out| after 1 s of silence | 4.01e-17 | 3.5e-17 | 3.59e-17 | 3.37e-17 |
| 7 Silence / stability: zeros from fresh instance: max |out| | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| 7 Silence / stability: random automation every block, +24 dBFS: all samples finite | 1.000 | 1.000 | 1.000 | 1.000 |
| 7 Silence / stability: random automation: peak output (sanity bound) | 38.825 | 38.825 | 37.715 | 37.703 |
| 7 Silence / stability: random automation then silence: max |out| in last 0.5 s | 1.05e-26 | 9.66e-27 | 1.52e-23 | 1.42e-23 |
| 8 Block-size invariance: max |diff| blocks 1/17/64/2048/4096 vs 512, 1x | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| 8 Block-size invariance: max |diff| blocks 1/17/64/2048/4096 vs 512, 2x | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| 8 Block-size invariance: max |diff| blocks 1/17/64/2048/4096 vs 512, 4x | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| F Features (spec section 2): FET SC HPF: GR reduction on 60 Hz, HPF 20 -> 300 Hz | 8.908 | 8.908 | 8.909 | 8.909 |
| F Features (spec section 2): Opto SC HPF: GR reduction on 60 Hz, HPF 20 -> 300 Hz | 17.625 | 17.625 | 17.626 | 17.626 |
| F Features (spec section 2): SC Listen (HPF 300 Hz): 50 Hz relative to 3 kHz in output | -31.132 | -31.132 | -31.131 | -31.131 |
| F Features (spec section 2): Emphasis 100 %: extra opto GR on 8 kHz | 7.790 | 7.790 | 7.780 | 7.780 |
| F Features (spec section 2): Emphasis 100 %: GR change on 200 Hz (should be ~0) | 0.000173 | 0.000173 | 0.000176 | 0.000176 |
| F Features (spec section 2): Auto makeup: final makeup vs average GR (|diff|) | 0.0968 | 0.0968 | 0.1052 | 0.1052 |
| F Features (spec section 2): Auto makeup: time to 63 % of final makeup (>= 1 s) | 2.312 | 2.312 | 2.305 | 2.305 |
| F Features (spec section 2): Auto makeup: cap under extreme GR | 23.987 | 23.987 | 23.986 | 23.986 |
| F Features (spec section 2): OS switching every 37 blocks: all samples finite | 1.000 | 1.000 | 1.000 | 1.000 |
| F Features (spec section 2): OS switching: peak output vs peak input | -7.057 | -7.394 | -7.315 | -7.724 |
| F Features (spec section 2): OS switched to 4x: reported latency - latency of 4x | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| F Features (spec section 2): OS switched to 4x: impulse peak - reported latency | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| F Features (spec section 2): Order switch while playing: max sample step vs steady tone | 0.9999 | 0.9999 | 1.0000 | 1.0000 |
| F Features (spec section 2): Output +12 dB step: 10-90 % transition | 44.000 | 44.000 | 44.000 | 44.000 |
| F Features (spec section 2): FET Output -20 dB step: 10-90 % transition | 44.000 | 44.000 | 44.000 | 44.000 |
| F Features (spec section 2): Opto Gain +12 dB step: 10-90 % transition | 44.000 | 44.000 | 44.000 | 44.000 |
| F Features (spec section 2): FET Input -18 dB step: 10-90 % transition | 44.000 | 44.000 | 44.000 | 44.000 |
| F Features (spec section 2): Mix 100 -> 0 % step (wet 20 dB lower): 10-90 % transition | 44.000 | 44.000 | 44.000 | 44.000 |
| O Oversampling filters: 2x stage (up/down): stopband peak | -100.9 | -100.9 | -100.9 | -100.9 |
| O Oversampling filters: 2x stage (up/down): passband ripple | 7.82e-05 | 7.82e-05 | 7.82e-05 | 7.82e-05 |
| O Oversampling filters: 4x stage: stopband peak | -100.4 | -100.4 | -100.4 | -100.4 |
| O Oversampling filters: 4x stage: passband ripple | 8.34e-05 | 8.34e-05 | 8.34e-05 | 8.34e-05 |
| O Oversampling filters: latency 1x | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| O Oversampling filters: wet path (stages off) flatness 20 Hz-20 kHz, 1x | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| O Oversampling filters: latency 2x | 71.000 | 71.000 | 71.000 | 71.000 |
| O Oversampling filters: wet path (stages off) flatness 20 Hz-20 kHz, 2x | 0.000157 | 0.000157 | 0.000102 | 0.000102 |
| O Oversampling filters: latency 4x | 79.000 | 79.000 | 79.000 | 79.000 |
| O Oversampling filters: wet path (stages off) flatness 20 Hz-20 kHz, 4x | 0.000205 | 0.000205 | 0.000138 | 0.000138 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Rap Lead: output vs input loudness | 0.3678 | 0.0713 | 0.3023 | 0.0149 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Rap Lead: output finite | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Rap Lead: FET peak GR | 6.720 | 7.254 | 7.121 | 7.121 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Rap Lead: opto average GR | 3.046 | 3.043 | 3.014 | 2.996 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Boom Bap Double: output vs input loudness | 0.1654 | -0.1668 | 0.1101 | -0.1869 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Boom Bap Double: output finite | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Boom Bap Double: FET peak GR | 9.544 | 9.863 | 9.912 | 9.912 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Boom Bap Double: opto average GR | 4.595 | 4.537 | 4.536 | 4.468 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Aggressive Parallel (All): output vs input loudness | -0.0074 | -0.1123 | -0.0227 | -0.1231 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Aggressive Parallel (All): output finite | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Aggressive Parallel (All): FET peak GR | 17.133 | 17.556 | 17.740 | 17.740 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Aggressive Parallel (All): opto average GR | 1.733 | 1.603 | 1.643 | 1.498 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Gentle Leveling: output vs input loudness | 0.1461 | -0.1742 | 0.0821 | -0.2269 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Gentle Leveling: output finite | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Gentle Leveling: FET peak GR | 2.961 | 3.591 | 3.225 | 3.241 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Gentle Leveling: opto average GR | 3.961 | 4.024 | 3.907 | 3.990 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Spoken Word: output vs input loudness | 0.0661 | -0.2402 | -0.0025 | -0.3080 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Spoken Word: output finite | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Spoken Word: FET peak GR | 4.859 | 5.452 | 5.199 | 5.201 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Spoken Word: opto average GR | 3.991 | 4.012 | 3.936 | 3.983 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Classic Serial (FET > Opto): output vs input loudness | 0.1081 | -0.1790 | 0.0465 | -0.2367 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Classic Serial (FET > Opto): output finite | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Classic Serial (FET > Opto): FET peak GR | 4.866 | 5.457 | 5.207 | 5.208 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Classic Serial (FET > Opto): opto average GR | 3.072 | 3.087 | 3.035 | 3.046 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Opto First, FET Catch: output vs input loudness | 0.5990 | 0.2958 | 0.4789 | 0.1877 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Opto First, FET Catch: output finite | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Opto First, FET Catch: FET peak GR | 3.783 | 3.665 | 3.819 | 3.656 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Opto First, FET Catch: opto average GR | 3.756 | 3.966 | 3.747 | 3.949 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: New York Parallel: output vs input loudness | 0.0287 | -0.0733 | 0.0082 | -0.0917 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: New York Parallel: output finite | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: New York Parallel: FET peak GR | 13.638 | 14.227 | 14.211 | 14.211 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: New York Parallel: opto average GR | 1.630 | 1.542 | 1.574 | 1.480 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Upfront Hip-Hop: output vs input loudness | 0.1828 | -0.1221 | 0.1222 | -0.1634 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Upfront Hip-Hop: output finite | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Upfront Hip-Hop: FET peak GR | 7.409 | 7.891 | 7.877 | 7.877 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Upfront Hip-Hop: opto average GR | 3.051 | 3.030 | 3.021 | 2.985 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Ad-libs & Hype: output vs input loudness | 0.1054 | -0.2332 | 0.0747 | -0.2504 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Ad-libs & Hype: output finite | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Ad-libs & Hype: FET peak GR | 10.682 | 11.192 | 10.969 | 10.969 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Ad-libs & Hype: opto average GR | 4.145 | 4.049 | 4.064 | 3.989 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Sibilance-Aware Leveling: output vs input loudness | 0.0674 | -0.1834 | -0.0408 | -0.2784 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Sibilance-Aware Leveling: output finite | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Sibilance-Aware Leveling: FET peak GR | 2.597 | 3.192 | 2.931 | 2.937 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Sibilance-Aware Leveling: opto average GR | 3.926 | 3.951 | 3.922 | 3.962 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Transparent Peak Control: output vs input loudness | 0.5690 | 0.2909 | 0.4429 | 0.1693 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Transparent Peak Control: output finite | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Transparent Peak Control: FET peak GR | 5.507 | 6.129 | 5.849 | 5.851 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Transparent Peak Control: opto average GR | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Sung Hook: output vs input loudness | 0.3233 | 0.0061 | 0.2571 | -0.0509 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Sung Hook: output finite | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Sung Hook: FET peak GR | 3.720 | 4.317 | 4.055 | 4.062 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Sung Hook: opto average GR | 5.027 | 5.072 | 4.974 | 5.045 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Lo-fi Grit (All): output vs input loudness | -0.0385 | -0.2610 | -0.0116 | -0.2358 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Lo-fi Grit (All): output finite | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Lo-fi Grit (All): FET peak GR | 14.871 | 15.605 | 14.814 | 14.814 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Lo-fi Grit (All): opto average GR | 2.187 | 2.045 | 2.041 | 1.936 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Broadcast Voice: output vs input loudness | 0.0821 | -0.2309 | 0.0063 | -0.2937 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Broadcast Voice: output finite | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Broadcast Voice: FET peak GR | 5.651 | 6.275 | 6.119 | 6.120 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Broadcast Voice: opto average GR | 5.125 | 5.127 | 5.085 | 5.072 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Mix Bus: Glue: output vs input loudness | 0.0614 | 0.0056 | 0.0154 | -0.0541 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Mix Bus: Glue: output finite | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Mix Bus: Glue: FET peak GR | 1.922 | 1.926 | 1.846 | 2.021 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Mix Bus: Glue: opto average GR | 1.510 | 1.511 | 1.486 | 1.476 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Mix Bus: Punch: output vs input loudness | 0.0447 | -0.0365 | 0.0080 | -0.0773 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Mix Bus: Punch: output finite | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Mix Bus: Punch: FET peak GR | 3.674 | 3.732 | 3.858 | 4.052 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Mix Bus: Punch: opto average GR | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: Transparent Leveling: output vs input loudness | 0.0791 | 0.0537 | 0.0765 | 0.0600 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: Transparent Leveling: output finite | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: Transparent Leveling: FET peak GR | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: Transparent Leveling: opto average GR | 1.993 | 2.018 | 1.992 | 2.009 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: Peak Catch: output vs input loudness | 0.0417 | 0.0122 | 0.0040 | -0.0331 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: Peak Catch: output finite | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: Peak Catch: FET peak GR | 1.621 | 1.859 | 1.890 | 2.135 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: Peak Catch: opto average GR | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: Parallel Density: output vs input loudness | -0.2117 | -0.2302 | -0.2193 | -0.2377 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: Parallel Density: output finite | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: Parallel Density: FET peak GR | 7.268 | 7.399 | 7.511 | 7.798 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: Parallel Density: opto average GR | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: True-Peak Safety (-1 dBTP): output vs input loudness | -1.024 | -1.080 | -1.012 | -1.055 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: True-Peak Safety (-1 dBTP): output finite | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: True-Peak Safety (-1 dBTP): FET peak GR | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Master: True-Peak Safety (-1 dBTP): opto average GR | 2.153 | 2.181 | 2.151 | 2.172 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Bass & 808: Clean Control: output vs input loudness | -0.9331 | -1.039 | -0.9268 | -1.058 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Bass & 808: Clean Control: output finite | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Bass & 808: Clean Control: FET peak GR | 7.959 | 7.959 | 7.701 | 8.156 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Bass & 808: Clean Control: opto average GR | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Mix Bus: Mid/Side Glue: output vs input loudness | -0.5409 | -0.3495 | -0.5980 | -0.3802 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Mix Bus: Mid/Side Glue: output finite | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Mix Bus: Mid/Side Glue: FET peak GR | 3.072 | 2.986 | 3.310 | 3.176 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Mix Bus: Mid/Side Glue: opto average GR | 1.793 | 1.841 | 1.775 | 1.822 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Lookahead Peak Control: output vs input loudness | -0.2172 | -0.5293 | -0.3050 | -0.5931 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Lookahead Peak Control: output finite | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Lookahead Peak Control: FET peak GR | 7.403 | 8.026 | 7.871 | 7.871 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Vocal: Lookahead Peak Control: opto average GR | 2.734 | 2.723 | 2.684 | 2.662 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Drum Bus: Boom Bap Smash: output vs input loudness | -0.1719 | -0.2205 | -0.1871 | -0.2371 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Drum Bus: Boom Bap Smash: output finite | 1.000 | 1.000 | 1.000 | 1.000 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Drum Bus: Boom Bap Smash: FET peak GR | 14.677 | 14.952 | 14.989 | 15.272 |
| P Factory presets (vocal -18 dBFS RMS / mix -14 dBFS RMS): Drum Bus: Boom Bap Smash: opto average GR | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| V 2.0 features: Vintage detector (1.x): FET 4:1, 20 us / 50 ms, Color 0, 30.000 Hz -6 dBFS (+12 dB in): THD+N | -43.488 | -43.488 | -43.488 | -43.488 |
| V 2.0 features: Clean detector: FET 4:1, 20 us / 50 ms, Color 0, 30.000 Hz -6 dBFS (+12 dB in): THD+N | -142.5 | -142.5 | -142.2 | -142.2 |
| V 2.0 features: Vintage detector (1.x): FET 4:1, 20 us / 50 ms, Color 0, 100.0 Hz -6 dBFS (+12 dB in): THD+N | -46.372 | -46.372 | -46.369 | -46.369 |
| V 2.0 features: Clean detector: FET 4:1, 20 us / 50 ms, Color 0, 100.0 Hz -6 dBFS (+12 dB in): THD+N | -143.5 | -143.5 | -143.4 | -143.4 |
| V 2.0 features: Clean vs Vintage: steady GR at 1 kHz -6 dBFS (difference) | 0.0055 | 0.0055 | 0.0037 | 0.0037 |
| V 2.0 features: Clean vs Vintage: onset peak (first 0.5 ms, 20:1, 20 us) difference | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| V 2.0 features: 4x: worst non-harmonic component, 15 kHz -3 dBFS, 20:1 +24 dB, Color 100 | -152.4 | -152.4 | -149.5 | -149.5 |
| V 2.0 features: FET threshold -24 dB: GR at -18 dBFS = GR of the -12 dB threshold at -6 dBFS | 4.77e-07 | 4.77e-07 | 0.0000 | 0.0000 |
| V 2.0 features: FET hard knee: 4:1, 6 dB over threshold -> GR 4.5 dB | 4.521 | 4.521 | 4.521 | 4.521 |
| V 2.0 features: FET hard knee: 1 dB under threshold -> GR | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| V 2.0 features: FET 12 dB knee: GR at threshold = slope * knee / 8 (1.125 dB) | 1.135 | 1.135 | 1.136 | 1.136 |
| V 2.0 features: FET range 6 dB: GR with 20 dB of demand | 6.000 | 6.000 | 6.000 | 6.000 |
| V 2.0 features: Opto range 3 dB: GR with heavy demand | 3.000 | 3.000 | 3.000 | 3.000 |
| V 2.0 features: Opto threshold -20 dB: GR at -16 dBFS = GR of the -10 dB threshold at -6 dBFS | 0.0000 | 0.0000 | -1.91e-06 | -1.91e-06 |
| V 2.0 features: FET hold 100 ms: GR change 80 ms after the drop | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| V 2.0 features: FET hold 100 ms: released 200 ms after the drop (GR fraction left) | 0.1843 | 0.1843 | 0.1824 | 0.1824 |
| V 2.0 features: Opto hold 100 ms: GR change 80 ms after the drop | 3.81e-05 | 3.81e-05 | 3.81e-05 | 3.81e-05 |
| V 2.0 features: Opto hold 100 ms: released 200 ms after the drop (GR fraction left) | 0.4244 | 0.4244 | 0.4243 | 0.4243 |
| V 2.0 features: SC bell +12 dB at 3 kHz: GR(3 kHz) - GR(300 Hz), equal levels | 8.911 | 8.911 | 8.887 | 8.887 |
| V 2.0 features: SC LPF 1 kHz: GR on an 8 kHz tone at -6 dBFS | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| V 2.0 features: SC LPF 1 kHz: GR on a 300 Hz tone at -6 dBFS | 8.981 | 8.981 | 8.981 | 8.981 |
| V 2.0 features: FET external key 120 Hz -3 dBFS ducks a -30 dBFS 1 kHz track by | 10.643 | 10.643 | 10.643 | 10.643 |
| V 2.0 features: FET external key quiet: track passes at its gain | -5.64e-06 | -5.64e-06 | -1.35e-06 | -1.35e-06 |
| V 2.0 features: FET external key, no side-chain input: track passes at its gain | -5.64e-06 | -5.64e-06 | -1.35e-06 | -1.35e-06 |
| V 2.0 features: Opto external key 120 Hz -3 dBFS ducks a -30 dBFS 1 kHz track by | 17.487 | 17.487 | 17.488 | 17.488 |
| V 2.0 features: Opto external key quiet: track passes at its gain | -5.95e-06 | -5.95e-06 | -1.54e-06 | -1.54e-06 |
| V 2.0 features: Opto external key, no side-chain input: track passes at its gain | -5.95e-06 | -5.95e-06 | -1.54e-06 | -1.54e-06 |
| V 2.0 features: lookahead 5 ms: latency = 4x + 5 ms | 300.0 | 300.0 | 319.0 | 319.0 |
| V 2.0 features: lookahead 5 ms, stages off: output = input delayed by the latency (<18 kHz) | -115.5 | -115.5 | -115.8 | -115.8 |
| V 2.0 features: lookahead 5 ms, stages off: = the engine without lookahead, delayed 5 ms | -600.0 | -600.0 | -600.0 | -600.0 |
| V 2.0 features: FET 20:1, 800 us attack: onset overshoot without lookahead | 23.340 | 23.340 | 23.357 | 23.357 |
| V 2.0 features: FET 20:1, 800 us attack: onset overshoot with 2 ms lookahead | 1.329 | 1.329 | 1.333 | 1.333 |
| V 2.0 features: limiter on (1.5 ms): latency = 4x + 64 (true-peak detector) + lookahead | 209.0 | 209.0 | 215.0 | 215.0 |
| V 2.0 features: limiter -1 dBTP, +12 dB, drum-bus program, content <= 0.48 fs: sample peak | -1.050 | -1.050 | -1.050 | -1.050 |
| V 2.0 features: limiter -1 dBTP, +12 dB, drum-bus program, content <= 0.48 fs: true peak (independent long-sinc reference) | -1.050 | -1.050 | -1.050 | -1.050 |
| V 2.0 features: limiter, drum-bus program, content <= 0.48 fs: GR meter | 13.555 | 13.555 | 13.543 | 13.570 |
| V 2.0 features: limiter -1 dBTP, +12 dB, square bursts, content <= 0.48 fs: sample peak | -1.928 | -1.928 | -1.933 | -1.933 |
| V 2.0 features: limiter -1 dBTP, +12 dB, square bursts, content <= 0.48 fs: true peak (independent long-sinc reference) | -1.053 | -1.053 | -1.053 | -1.053 |
| V 2.0 features: limiter, square bursts, content <= 0.48 fs: GR meter | 3.590 | 3.590 | 3.590 | 3.590 |
| V 2.0 features: limiter -1 dBTP, +12 dB, drum-bus program, full band: sample peak | -1.050 | -1.050 | -1.051 | -1.050 |
| V 2.0 features: limiter -1 dBTP, +12 dB, drum-bus program, full band: true peak (reference), documented bound +0.4 dB | -1.014 | -1.007 | -0.9463 | -0.9903 |
| V 2.0 features: limiter -1 dBTP, +12 dB, square bursts, full band: sample peak | -2.463 | -2.463 | -2.475 | -2.475 |
| V 2.0 features: limiter -1 dBTP, +12 dB, square bursts, full band: true peak (reference), documented bound +0.4 dB | -0.8309 | -0.8309 | -0.8321 | -0.8321 |
| V 2.0 features: limiter below the ceiling: = the engine without limiter, delayed by its latency (null) | -600.0 | -600.0 | -600.0 | -600.0 |
| V 2.0 features: handshake: unacknowledged latency change keeps the old latency | 79.000 | 79.000 | 79.000 | 79.000 |
| V 2.0 features: handshake: muted while waiting for the host | -600.0 | -600.0 | -600.0 | -600.0 |
| V 2.0 features: handshake: a request with the new latency is posted | 1.000 | 1.000 | 1.000 | 1.000 |
| V 2.0 features: handshake: after the acknowledgement the new latency plays (impulse position - input) | 211.0 | 211.0 | 223.0 | 223.0 |
| V 2.0 features: handshake: engine latency = the acknowledged one | 211.0 | 211.0 | 223.0 | 223.0 |
| V 2.0 features: handshake timeout: latency unchanged | 211.0 | 211.0 | 223.0 | 223.0 |
| V 2.0 features: handshake timeout: the old configuration plays again after 0.5 s | -6.021 | -6.021 | -6.021 | -6.021 |
| V 2.0 features: all 2.0 features: block 512 vs 37 (difference) | -600.0 | -600.0 | -600.0 | -600.0 |
| V 2.0 features: all 2.0 features: output finite | 1.000 | 1.000 | 1.000 | 1.000 |
| V 2.0 features: all 2.0 features: heap allocations inside process() | 0.0000 | 0.0000 | 0.0000 | 0.0000 |
| 9 Stereo identity: identical L/R in: differing output samples (defaults) | - | 0.0000 | - | 0.0000 |
| 9 Stereo identity: identical L/R in: differing samples (4x, link 0, All, auto makeup) | - | 0.0000 | - | 0.0000 |
| 9 Stereo identity: mono instance vs dual-mono stereo: max |diff| | - | 0.0000 | - | 0.0000 |
| F Features (spec section 2): Link 100 %: quiet side follows loud side's GR (|GR_L - GR_R|) | - | 6.11e-08 | - | 2.1e-07 |
| F Features (spec section 2): Link 0 %: quiet side GR (independent detection) | - | -4.08e-06 | - | -7e-06 |
| V 2.0 features: Mid/Side, stages off: = stereo mode (M/S encode + decode is exact) | - | -138.5 | - | -138.5 |
| V 2.0 features: Mid/Side on identical channels = stereo mode (difference) | - | -600.0 | - | -600.0 |
| V 2.0 features: Mid/Side, link 0: quiet mid passes at its gain (6 dB input) | - | 1.17e-05 | - | 1.13e-05 |
| V 2.0 features: Mid/Side, link 0: loud side compressed by | - | 8.993 | - | 9.000 |
