"""abm.py <ref stage dir (input)> <A dir> <B dir> ... : per-take change made by each candidate vs its input, medians"""
import sys, os, glob, numpy as np
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from metrics import Ref, measure, mono
W=os.path.dirname(os.path.abspath(__file__))
ins=sys.argv[1]; cands=sys.argv[2:]; keys0=["I","TP","LRA","ess_re_vowel","ess_local","vowel_lvl","vowel_after_ess","fund","body","hf","gap_re_voice","spread"]
keys=os.environ.get("ABM_KEYS","").split(",") if os.environ.get("ABM_KEYS") else keys0
rows={c:{k:[] for k in keys} for c in cands}
for line in open(f"{W}/takes/index.txt"):
    u=line.split("\t")[0]; R=Ref(mono(f"{W}/takes/{u}.wav")); m0=measure(R,mono(f"{ins}/{u}.wav"))
    for c in cands:
        m=measure(R,mono(f"{c}/{u}.wav"))
        for k in keys:
            if m[k] is not None and m0[k] is not None: rows[c][k].append(m[k]-m0[k])
print("change vs input (median [p10,p90])".ljust(34)+"".join(k[:14].rjust(18) for k in keys))
for c in cands:
    print(os.path.basename(c.rstrip('/'))[:34].ljust(34)+"".join(f"{np.median(v):+7.2f}[{np.percentile(v,10):+5.1f},{np.percentile(v,90):+4.1f}]" for v in (np.array(rows[c][k]) for k in keys)))
