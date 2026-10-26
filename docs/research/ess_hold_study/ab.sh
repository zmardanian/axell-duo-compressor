#!/bin/bash
# ab.sh <sandbox name> <bundle path> <stage> <input dir> <out name>: renders a stage with a candidate plugin build (Dojo sandbox)
V=/var/home/Alex/axell-v4-work/v4; cd $V
sbx=$V/sbx/$1; mkdir -p $sbx/.vst3; ln -sfn "$2" "$sbx/.vst3/$(basename "$2")"
d=out/ab/$5; rm -rf $d; mkdir -p $d/raw
AXELL_DOJO_SANDBOX=$sbx ~/.local/share/AxellSignature/Dojo/app/axell-signature-dojo --render stages/$3.RfxChain $d/raw $4/u*.wav > $d/log.txt 2>&1
for f in $d/raw/*.wav; do b=$(basename "$f"); mv "$f" "$d/${b%% - *}.wav"; done; rmdir $d/raw; echo "$d $(ls $d/u*.wav 2>/dev/null | wc -l)"
