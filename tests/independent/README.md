# Independent black-box audit

`duo_audit.py` drives the built `Axell Duo Compressor.vst3` through `DuoAuditHost` (JUCE's VST3 *hosting* code; the
plugin's own classes are never linked) and analyses the rendered audio with numpy/scipy written from textbook
definitions: silence, transparency and reported latency, bit-identical rendering of Duo Vocal Comp 1.0.0 settings
(through the 1.0.0 bundle itself), Clean-detector distortion, aliasing, true-peak ceiling against an FFT
reconstruction, lookahead latency and overshoot, the external side-chain bus, Mid/Side, live latency changes in a
real-time host (output always aligned with the host's compensation) and host block-size invariance.

    cmake --build build --target DuoAuditHost AxellDuoCompressor_VST3
    cd tests/independent && DUO_AUDIT_WORK=/some/disk/dir python3 duo_audit.py   # exit code = failures

Environment: DUO_AUDIT_HOST, DUO_VST3, DUO_V1_VST3 (default: ~/duo-vocal-comp-release/... 1.0.0 bundle), DUO_AUDIT_WORK.
