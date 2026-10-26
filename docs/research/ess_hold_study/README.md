# ESS HOLD study (Duo 4.0, Master 4.0) - DECISIONS D33

How the suite's vocal chains were measured stage by stage on the artist's own takes, and how ESS HOLD was A/B-tested
against the 3.0 releases. The takes themselves are personal recordings and are not in the repository.

- `pipe.sh` renders every take (`takes/u*.wav`, an index in `takes/index.txt`) through a chain split into single
  stages (one `.RfxChain` per plugin) with the Axell Signature Dojo's batch renderer (`--render`), keeping every step.
- `metrics.py` measures one render against the dry take: BS.1770 loudness / LRA / true peak, the esses' level over the
  vowels of their own word (`ess_local`: each ess frame's 4-12 kHz level against the loudest vowel frame within
  +-150 ms; frames classified on the dry take), vowel level, vowels just after esses, pauses (absolute), fundamental
  band and 5-16 kHz tone. `summ.py` prints what each stage changes.
- `ab.sh` renders a stage with a candidate build (Dojo sandbox: `AXELL_DOJO_SANDBOX` with the candidate bundle in
  `.vst3`), `abm.py` compares candidates with the released stage on the same inputs.
- `learn.py` sets Axell Master's input per take so its output lands on the preset's target, as LEARN does;
  `abmaster.py` / `mres.py` A/B Master's ESS HOLD through `tools/MasterRender` of Axell Master.

Paths in the scripts point at the work folder used for the study; adjust `W` / `V` before running.
