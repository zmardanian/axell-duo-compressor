"""learn.py <in_dir> <stage.RfxChain> <out_dir> <target LUFS>: per take, gain the input so the Master stage's output lands on target (2 passes), like LEARN"""
import sys, os, glob, subprocess, shutil, numpy as np, soundfile as sf
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from metrics import loud, mono
DOJO=os.path.expanduser("~/.local/share/AxellSignature/Dojo/app/axell-signature-dojo")
src,stage,dst,target=sys.argv[1],sys.argv[2],sys.argv[3],float(sys.argv[4])
os.makedirs(dst,exist_ok=True); tmp=dst+"/_tmp"; gains={}
files=sorted(glob.glob(src+"/u*.wav")); g={os.path.basename(f):0.0 for f in files}
for it in range(3):
    shutil.rmtree(tmp,ignore_errors=True); os.makedirs(tmp+"/in"); os.makedirs(tmp+"/out")
    for f in files:
        x,fs=sf.read(f); sf.write(f"{tmp}/in/{os.path.basename(f)}", x*10**(g[os.path.basename(f)]/20), fs, subtype="FLOAT")
    subprocess.run([DOJO,"--render",stage,tmp+"/out"]+sorted(glob.glob(tmp+"/in/*.wav")),capture_output=True)
    err=[]
    for f in sorted(glob.glob(tmp+"/out/*.wav")):
        b=os.path.basename(f).split(" - ")[0]+".wav"; I,_=loud(mono(f)); e=target-I; err.append(e); g[b]+=e*0.9
        if it==2: shutil.move(f, f"{dst}/{b}")
    print(f"pass {it}: median error {np.median(np.abs(err)):.2f} dB, max {np.max(np.abs(err)):.2f}", flush=True)
shutil.rmtree(tmp); open(dst+"/gains.txt","w").write("\n".join(f"{k}\t{v:.2f}" for k,v in g.items()))
