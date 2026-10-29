#!/bin/bash
# pipe.sh <name> <stage files...>: renders every take through the stages cumulatively, keeping each step
W=${AXELL_STUDY_DIR:-$HOME/axell-v4-work/v4}; cd $W
DOJO=${DOJO:-$HOME/.local/share/AxellSignature/Dojo/app/axell-signature-dojo}
name=$1; shift; prev=takes; k=0
for st in "$@"; do
  k=$((k+1)); d=out/$name/$(printf %02d $k)-$(basename $st .RfxChain); rm -rf "$d"; mkdir -p "$d/raw"
  $DOJO --render "$st" "$d/raw" $prev/u*.wav > "$d/log.txt" 2>&1 || echo "FAIL $d"
  for f in "$d"/raw/*.wav; do b=$(basename "$f"); mv "$f" "$d/${b%% - *}.wav"; done; rmdir "$d/raw"
  echo "$d $(ls $d/u*.wav | wc -l)"; prev=$d
done
