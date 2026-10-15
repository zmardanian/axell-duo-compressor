# Axell Duo Compressor — Engineering Spec (1.0 targets; 2.0 additions in PLAN_V2.md / DECISIONS D19-D26)

## 1. What "beat the LA-2A and 1176 at their job" means here

We cannot claim to "sound better" than the hardware — that is taste. We define winning as
**doing the vocal job the hardware does, with the hardware's useful character, while
removing its practical weaknesses**, and we prove it with measurements:

| Hardware weakness | Our requirement |
|---|---|
| Noise floor, hum | Digital silence in → silence out (< -140 dBFS) |
| 1176 has no mix knob; parallel needs a second track | Latency-aligned dry/wet mix inside the plugin |
| Low end / plosives make both units pump | Sidechain HPF on both detectors + listen mode |
| LA-2A has no attack/release control, slow to catch peaks | FET in front catches peaks first, order switchable |
| Units differ, drift with temperature | Deterministic, recallable, identical L/R |
| Aliasing in cheap emulations | Oversampling, measured aliasing below spec |
| Two units of gain-staging | Auto makeup option + I/O meters |

## 2. DSP requirements

### 2.1 FET stage (1176-style)
- Fixed threshold; INPUT drives into it, OUTPUT recovers level (as hardware).
- Attack 20 µs – 800 µs, release 50 ms – 1100 ms (time to recover 63 % of GR), measured.
- Ratios 4, 8, 12, 20, All. Knee gets harder with ratio. "All" = nuke mode:
  over-compression region (output level *drops* slightly above threshold+~10 dB),
  faster attack, more distortion, a recognisable pumping release. Document the curve.
- **Program-dependent release**: release slows slightly after long, heavy compression
  (hardware behaviour), max +40 % at sustained >10 dB GR.
- **Gain-element distortion**: FET distortion rises with gain reduction
  (mostly 2nd + 3rd harmonic). THD at 1 kHz: < 0.1 % at 0 dB GR with Color 0;
  0.3–1.5 % at 10 dB GR, Color 50; more in All mode.
- Stereo link: detector uses max of channels (current). Add "Link" param 0–100 %
  (new param, version 2) blending linked vs independent detection.

### 2.2 Opto stage (LA-2A-style)
- Attack ~10 ms (63 % to target), measured.
- Two-stage release: ~50 % of GR recovered in 40–80 ms, then 0.5–5 s for the rest,
  with the long tail getting longer the longer/harder it compressed (light-cell memory).
  Must be smooth (no stepping from the 32-sample coefficient update — interpolate or
  update every sample if CPU allows).
- Compress (~3:1 soft knee) and Limit (~10:1) modes.
- Optional HF-emphasis on the sidechain ("Emphasis" knob, new param, version 2) like
  the LA-2A R37 trim — makes it react more to sibilance/brightness.
- Tube make-up colour: predominantly 2nd harmonic, level-dependent, DC-free.

### 2.3 Global
- Oversampling selector 1x / 2x / 4x (new param, version 2; default 2x).
  Latency reported correctly for each; switching must not crash or glitch-loop.
- Mix 0–100 %, sample-accurate latency-aligned (already done in OS domain — keep it).
- Sidechain HPF 20–300 Hz on both detectors; **SC Listen** toggle (new param) outputs
  what the detector hears.
- Per-stage solo of the gain reduction is optional; per-stage bypass exists.
- **Auto makeup** toggle (new param): estimates average GR and compensates slowly
  (time constant ≥ 1 s) — off by default.
- All gain/mix changes smoothed (no zipper noise under automation).
- Denormal-safe; silent input must not raise CPU.

## 3. Measurements (automated tests, numeric pass/fail)

Run each at 44.1 / 48 / 96 kHz unless stated. Tolerances in brackets.

1. **Static curve** per FET ratio and opto mode: sweep -60..0 dBFS 1 kHz sine,
   measure output vs input after settling; slope above threshold within ±10 % of the
   nominal ratio (except All, which must show over-compression).
2. **FET attack/release** at min/max settings: step 1 kHz tone -30 → -6 dBFS; time to
   63 % GR within ±20 % of the knob value.
3. **Opto release shape**: after 2 s of 10 dB GR, 50 % recovery in 40–80 ms; after 10 s
   of 10 dB GR, full recovery tail ≥ 1.5× longer than after 1 s of 10 dB GR.
4. **THD** (FFT, 1 kHz) at the points in §2.1 / §2.2.
5. **Aliasing**: use a 15 kHz sine at -3 dBFS, FET 20:1, Color 100, 2x OS,
   44.1 kHz: largest non-harmonic (aliased) component < -70 dBFS at 2x and < -85 dBFS at 4x.
6. **Null / latency test**: Mix 0 % → output equals input delayed by reported latency,
   residual < -100 dBFS (IIR OS filters: test at Mix 0 means dry also passes the OS
   filters; compare against a 1x run or relax to < -60 dB and document why).
7. **Silence**: zeros in → |out| < 1e-7 after 1 s; no NaN/Inf ever (also test with
   +24 dBFS input and with random parameter automation every block).
8. **Block-size invariance**: same input processed at block sizes 1, 17, 512, 4096
   gives outputs equal within 1e-5.
9. **Stereo identity**: identical L/R in → identical L/R out (bit-exact).
10. **CPU**: real-time factor reported (see CLAUDE.md Phase 5).

## 4. GUI / usability

- Resizable (aspect locked), 100–200 % scale, crisp at HiDPI.
- Input meter, output meter, per-stage GR meters with peak-hold, numeric readouts.
- Tooltips on every control explaining what it does for vocals.
- Factory presets (use JUCE programs or a preset menu in the header):
  `Rap Lead`, `Boom Bap Double`, `Aggressive Parallel (All)`, `Gentle Leveling`,
  `Spoken Word`, `Init`. Plus user save/load to `~/.local/share/AxellDuoCompressor/Presets (1.x folder still read)`.
- A/B compare button.
- Double-click resets knob, Ctrl/Shift+drag for fine adjust.
- Bypass via host is fine; per-stage On toggles stay.
