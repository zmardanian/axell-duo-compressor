import json,sys,numpy as np
R=json.load(open(sys.argv[1])); takes=list(R)
stages=[k for k in R[takes[0]] if k not in ("f0lo","f0med")]
keys=["I","TP","LRA","ess_re_vowel","fund","sub","body","hf","gap_re_voice","spread","clip"]
print("ABSOLUTE (median over %d takes; p10..p90)"%len(takes))
print("stage".ljust(18)+"".join(k[:10].rjust(16) for k in keys))
for s in stages:
    row=s.ljust(18)
    for k in keys:
        v=np.array([R[t][s][k] for t in takes if R[t][s].get(k) is not None],float)
        row+=f"{np.median(v):7.1f}[{np.percentile(v,10):4.0f},{np.percentile(v,90):3.0f}]" if len(v) else " "*16
    print(row)
print("\nCHANGE BY EACH STAGE (median over takes)")
print("stage".ljust(18)+"".join(k[:10].rjust(9) for k in keys))
for a,b in zip(stages[:-1],stages[1:]):
    row=b.ljust(18)
    for k in keys:
        v=np.array([R[t][b][k]-R[t][a][k] for t in takes if R[t][b].get(k) is not None and R[t][a].get(k) is not None],float)
        row+=f"{np.median(v):9.1f}" if len(v) else " "*9
    print(row)
