"""mres.py <tmp dirs...>: ESS HOLD on minus off, per measure, median [p10,p90] over the takes (from abmaster.py renders)"""
import sys, os, glob, numpy as np, soundfile as sf
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from metrics import Ref, measure, mono
W=os.path.dirname(os.path.abspath(__file__))
keys=["ess_local","ess_re_vowel","vowel_lvl","vowel_after_ess","gap_abs","I","TP","LRA","fund","hf"]
print("on - off".ljust(26)+"".join(k[:14].rjust(18) for k in keys))
for d in sys.argv[1:]:
    rows={k:[] for k in keys}
    for line in open(f"{W}/takes/index.txt"):
        u=line.split("\t")[0]; R=Ref(mono(f"{W}/takes/{u}.wav"))
        a=measure(R,mono(f"{d}/{u}_ess0.wav")); b=measure(R,mono(f"{d}/{u}_ess1.wav"))
        for k in keys:
            if a[k] is not None and b[k] is not None: rows[k].append(b[k]-a[k])
    print(os.path.basename(d)[:26].ljust(26)+"".join(f"{np.median(v):+7.2f}[{np.percentile(v,10):+5.1f},{np.percentile(v,90):+4.1f}]" for v in (np.array(rows[k]) for k in keys)), flush=True)
