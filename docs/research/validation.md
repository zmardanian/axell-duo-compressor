# Validation against hardware recordings and published listening tests (2.0)

Request (2026-10-14): validate Axell Duo Compressor against hardware recordings and blind listening tests, using
recordings and tests from studios / researchers found online, downloading only what is needed.

## Data used (only what was needed)

| Hardware | Source | Licence | Used |
|---|---|---|---|
| Teletronix LA-2A (opto) | SignalTrain LA-2A dataset v1.1, Hawley, Colburn, Mimilakis; Zenodo 3824876 | CC BY 4.0 | the 3 input/target pairs of its **Test split** (15 min each, 44.1 kHz): Peak Reduction 65 compress, 65 limit, 80 limit. The archive is one 20.7 GB gzip with the Test split first; 1.57 GB of it was downloaded |
| Universal Audio 1176LN (FET; limiter section of a 6176 strip) | Martínez Ramírez, Benetos, Reiss, "Deep Learning for Black-Box Modeling of Audio Effects", Appl. Sci. 10(2) 638, 2020; Zenodo 3562442 | CC BY 4.0 | the limiter + dry folders (1186 guitar/bass notes each, ~400 MB of the 968 MB zip, ranged reads). One setting: attack 800 µs, release 1100 ms, input 4, output 7, ratio ALL |

No public multi-setting 1176 recordings were found.

## Published listening tests and metrics used as the yardstick

- **Steinmetz & Reiss 2021** (AES; arXiv 2102.06200), neural models of the LA-2A on the SignalTrain test set:
  MAE 7.66e-3 … 1.2e-1, multi-resolution STFT error 0.551 … 0.796, loudness error 0.361 … 1.305 dB (Table 1).
  MUSHRA with 18 screened engineers: the best models (LSTM-32, TCN-300-C) rated slightly but significantly below the
  hardware reference; listeners noted they "let some transients pass through more than the analog LA-2A".
- **Simionato & Fasciani 2024** (arXiv 2408.12549): state-space models of LA-2A / CL 1B, MUSHRA.
- **Moore (Huddersfield)**, "Objective Analysis and Perceptual Evaluation of LA-2A Compressors": six hardware units
  measured, ABX with 17 listeners - differences between real units are subtle and only sometimes audible.

## LA-2A: the opto stage

**Method.** `tools/duo_render` runs the shipping engine on the hardware inputs. One opto law (knob scale, ratio and
knee per mode, attack, two-stage release) was fitted jointly to the three settings (`tools/validation/
la2a_joint_fit.py`: Nelder-Mead, 1801 renders) on calibration blocks only (minutes 3-7 of each file, every fourth
20 s block). Evaluation (`la2a_report.py`) uses everything else, at the hardware's own knob settings, with one fixed
output gain (the hardware's measured quiet-signal gain, -0.38 dB): nothing is fitted per file.

**Result** (held-out audio; GR envelope = output level minus input level on 50 ms frames):

| | Measured law (2.0 default) | Classic (1.x) law, same knobs | Published neural models |
|---|---|---|---|
| GR envelope error, music | **0.60 dB** RMS (0.42 / 0.57 / 0.82 per file) | 14.1 dB | - |
| GR envelope error, noise bursts | 1.87 dB | 14.9 dB | - |
| Median GR bias | -0.16 / -0.31 / -0.09 dB | -10.5 / -16.8 / -17.2 dB | - |
| MAE | 0.030 | 0.050 | 0.0077 - 0.12 |
| Multi-resolution STFT error | **0.449** | 1.36 | 0.551 - 0.796 |
| Loudness error (BS.1770) | 0.43 dB | 14.8 dB | 0.361 - 1.305 dB |

Notes: the noise-burst material (minutes 5-15) jumps in level every few hundred ms; the hardware's own 10 ms gain
estimate jitters by 3.4 dB there (spectral and noise effects, measured as the deviation from its 50 ms average),
which sets the floor of that row. The published numbers were computed by their authors on the same test split
with models trained on separate training data; our evaluation excludes the calibration minutes. Conditions are
close, not identical. MAE depends on the waveform (the hardware's transformers and frequency response are not
modelled), which is why it sits mid-range while STFT and loudness are at or beyond the best published models.

**Calibrated law** (`tools/validation/la2a_measured_model.json`, `OptoStage::measuredModel()`): Peak Reduction
0.443 dB per step - 20.2 dB into the cell; ratio 3.66 (compress) / 3.60 (limit) - the hardware's switch changes
the knee (3 dB vs hard) more than the ratio; attack 1.3 ms; fast release 38.5 ms carrying 56 % of the GR; slow
tail 1.0-1.6 s. Shipped as **Opto Model: Measured** (default for new instances, all factory presets retuned to the
same opto GR); **Classic** keeps the 1.x law, and 1.x projects load with it.

## 1176: the FET stage

**Method** (`fet1176_compare.py`): the files are peak-normalised, so the gain trajectory over each note is compared
(5 ms frames, per-note constant removed). One global input gain was fitted on the even-numbered notes; the 593
odd-numbered notes are held out. Model: FET, ALL, 800 µs, 1100 ms, Color 30.

**Result:** trajectory error 1.45 dB RMS (median 1.44, 90th percentile 1.94) against 3.68 dB of trajectory variation
in the hardware (the error a model without dynamics would make). Average over held-out notes: the hardware reaches
about 11 dB of GR (relative to the note's end) within 15 ms and keeps ~4.8 dB at 1 s; the model reaches ~9.4 dB and
keeps ~2.4 dB. At equal loudness the hardware output's peaks sit 4.7-10.7 dB lower than the model's: at output 7 and
ALL the recorded unit flattens peaks far harder (its recordings already held full-scale flat tops before
normalisation, which points at output-stage saturation).

**Decision (no change to the FET law):** a diagnostic free fit (`fet1176_fit.py`) reaches 0.6-0.8 dB, but only with
the release at the bottom of its range (50 ms) while the hardware knob was at 1100 ms, and with ratio/knee choices
that trade off against it. One hardware setting cannot separate a release-law difference from a static-curve or
saturation difference; changing the 1176 law on that basis would be guesswork. Needed: recordings at several
attack/release/ratio settings.

## Blind listening test kit

`tools/listening/` (published as a private page: https://claude.ai/artifact/G1wtiZd6uj8wYkunoPjkYx). 10 ABX trials
(6 LA-2A excerpts, 4 trials of three 1176 notes) and 3 MUSHRA-style screens (hidden hardware reference, Measured law,
Classic law). Every version of a trial is loudness-matched (BS.1770) and shares one gain (-20 LUFS, peaks <= -1 dBFS:
no clipping cue). Excerpts come from audio not used for calibration. Each listener's answers are stored in the page's
database (only the owner and editors read them); the owner view shows ABX scores with binomial p-values and rating
medians. Listening results are not yet collected - Claude cannot listen; share the page with listeners (contributor
access) to run it. Expect the 1176 trials to be distinguishable (see above).
