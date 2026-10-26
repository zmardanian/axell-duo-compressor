"""abmaster.py <in dir> <preset idx> <gains.txt|+dB> <tag> [extra MasterRender args]: renders every take through
MasterRender with --ess-hold 0 and 1 and prints the change each makes vs its input (median over takes)"""
import sys, os, subprocess, numpy as np, soundfile as sf
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from metrics import Ref, measure, mono
W=os.path.dirname(os.path.abspath(__file__)); MR=os.environ.get("MR", os.path.expanduser("~/axell-master/build/MasterRender"))
ind, preset, gspec, tag = sys.argv[1:5]; extra=sys.argv[5:]
gains={}
if gspec.startswith("+") or gspec.startswith("-"): fixed=float(gspec)
else:
    fixed=None; gains={l.split()[0]:float(l.split()[1]) for l in open(gspec)}
keys=["I","TP","LRA","ess_re_vowel","ess_local","vowel_lvl","vowel_after_ess","fund","hf","gap_re_voice","spread"]
rows={v:{k:[] for k in keys} for v in ("0","1")}; tmp=f"{W}/tmp_{tag}"; os.makedirs(tmp,exist_ok=True)
for line in open(f"{W}/takes/index.txt"):
    u=line.split("\t")[0]; R=Ref(mono(f"{W}/takes/{u}.wav"))
    x,fs=sf.read(f"{ind}/{u}.wav"); x=x if x.ndim==2 else np.stack([x,x],1)
    x.astype(np.float32).tofile(f"{tmp}/in.f32"); g=fixed if fixed is not None else gains[u+".wav"]
    m0=measure(R,x.mean(1)*10**(g/20))
    for v in ("0","1"):
        subprocess.run([MR,f"{tmp}/in.f32",f"{tmp}/out{v}.f32",str(fs),preset,str(g),"--no-codecs","--ess-hold",v]+extra,check=True,capture_output=True)
        y=np.fromfile(f"{tmp}/out{v}.f32",np.float32).reshape(-1,2).astype(np.float64)
        sf.write(f"{tmp}/{u}_ess{v}.wav",y,fs,subtype="FLOAT")
        m=measure(R,y.mean(1))
        for k in keys:
            if m[k] is not None and m0[k] is not None: rows[v][k].append(m[k]-m0[k])
print(f"{tag}: change vs its (gained) input, median [p10,p90]")
print("ess-hold".ljust(10)+"".join(k[:10].rjust(18) for k in keys))
for v in ("0","1"):
    print(v.ljust(10)+"".join(f"{np.median(a):+7.2f}[{np.percentile(a,10):+5.1f},{np.percentile(a,90):+4.1f}]" for a in (np.array(rows[v][k]) for k in keys)))
