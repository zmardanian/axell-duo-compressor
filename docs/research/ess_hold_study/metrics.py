"""metrics.py <pipeline>  -> per take and stage: loudness, true peak, LRA, ess balance, fundamental loss, HF tilt, gaps"""
import numpy as np, soundfile as sf, sys, os, glob, json, math
from scipy import signal as sg
W=os.path.dirname(os.path.abspath(__file__)); fs=48000
def mono(p):
    x,r=sf.read(p); assert r==fs; return x if x.ndim==1 else x.mean(1)
def kw(x):
    f0,G,Q=1681.974450955533,3.999843853973347,0.7071752369554196
    K=math.tan(math.pi*f0/fs); Vh=10**(G/20); Vb=Vh**0.4996667741545416; a0=1+K/Q+K*K
    b1=[(Vh+Vb*K/Q+K*K)/a0,2*(K*K-Vh)/a0,(Vh-Vb*K/Q+K*K)/a0]; a1=[1,2*(K*K-1)/a0,(1-K/Q+K*K)/a0]
    f0,Q=38.13547087602444,0.5003270373238773; K=math.tan(math.pi*f0/fs)
    a2=[1,2*(K*K-1)/(1+K/Q+K*K),(1-K/Q+K*K)/(1+K/Q+K*K)]
    return sg.lfilter([1,-2,1],a2,sg.lfilter(b1,a1,x))
def loud(x):
    z=kw(x)**2; blk,hop=int(.4*fs),int(.1*fs); c=np.concatenate([[0],np.cumsum(z)])
    ms=np.array([(c[i+blk]-c[i])/blk for i in range(0,len(z)-blk,hop)]); L=-0.691+10*np.log10(ms+1e-20)
    g=ms[L>-70]; 
    if len(g)==0: return -99,0
    I1=-0.691+10*np.log10(g.mean()); I=-0.691+10*np.log10(g[-0.691+10*np.log10(g)>I1-10].mean())
    st=int(3*fs); s=np.array([(c[i+st]-c[i])/st for i in range(0,len(z)-st,hop)]); S=-0.691+10*np.log10(s+1e-20)
    S=S[(S>-70)&(S>I-20)]; lra=np.percentile(S,95)-np.percentile(S,10) if len(S)>2 else 0
    return I,lra
def tp(x): return 20*np.log10(np.max(np.abs(sg.resample_poly(x,4,1)))+1e-12)
def f0track(x):
    # simple YIN-like f0 on 40 ms frames (60-400 Hz); returns per-frame f0 (0 = unvoiced)
    y=sg.decimate(x,4); r=12000; n=480; hop=240; out=[]
    for i in range(0,len(y)-n-400,hop):
        fr=y[i:i+n+400]; a=fr[:n]
        if np.mean(a**2)<1e-7: out.append(0); continue
        d=np.array([np.sum((a-fr[t:t+n])**2) for t in range(30,201)]); cm=d/(np.cumsum(d)/np.arange(1,len(d)+1)+1e-12)
        k=np.argmin(cm); out.append(r/(k+30) if cm[k]<0.25 else 0)
    return np.array(out)
class Ref:
    def __init__(s,dry):
        s.x=dry; f,_,Z=sg.stft(dry,fs,nperseg=1024,noverlap=512); s.f=f; P=np.abs(Z)**2; s.P=P
        b=s.band; tot=b(P,100,18000); top=np.percentile(tot,95); s.voiced=tot>top-20
        s.ess=s.voiced&(b(P,5000,10000)-b(P,200,1000)>3); s.vowel=s.voiced&(b(P,5000,10000)-b(P,200,1000)<-15)
        s.gap=tot<top-45
        f0=f0track(dry); v=f0[f0>0]; s.f0lo=np.percentile(v,10) if len(v) else 120; s.f0med=np.median(v) if len(v) else 150
    def band(s,P,lo,hi): return 10*np.log10(P[(s.f>=lo)&(s.f<hi)].sum(0)+1e-20)
def align(x,ref):
    n=min(len(ref),fs*20); c=sg.fftconvolve(x[:n],ref[:n][::-1]); lag=int(np.argmax(np.abs(c))-(n-1))
    x=np.roll(x,-lag); x=np.pad(x,(0,max(0,len(ref)-len(x))))[:len(ref)]; return x,lag
def measure(R,x):
    x,lag=align(x,R.x); _,_,Z=sg.stft(x,fs,nperseg=1024,noverlap=512); P=np.abs(Z)**2; b=R.band
    I,lra=loud(x); m={"lag":lag,"I":I,"LRA":lra,"TP":tp(x)}
    m["ess_re_vowel"]=float(np.percentile(b(P,4000,12000)[R.ess],95)-np.percentile(b(P,200,18000)[R.vowel],95)) if R.ess.sum()>10 else None
    # tone on vowels, re 1-3 kHz: fundamental band (f0lo..f0med), body 200-500, HF 5-16k
    ref=b(P,1000,3000)[R.vowel]
    m["fund"]=float(np.median(b(P,R.f0lo/1.12,R.f0med*1.12)[R.vowel]-ref)); m["sub"]=float(np.median(b(P,50,R.f0lo/1.12)[R.vowel]-ref))
    m["body"]=float(np.median(b(P,200,500)[R.vowel]-ref)); m["hf"]=float(np.median(b(P,5000,16000)[R.vowel]-ref))
    # local ess balance: each ess frame's 4-12 kHz level against the loudest vowel frame within +-150 ms (its word)
    hb=b(P,4000,12000); tb=b(P,200,18000); loc=[]
    vi=np.where(R.vowel)[0]
    for k in np.where(R.ess)[0]:
        near=vi[(vi>=k-14)&(vi<=k+14)]
        if len(near): loc.append(hb[k]-tb[near].max())
    m["ess_local"]=float(np.median(loc)) if len(loc)>10 else None
    m["gap_abs"]=float(np.median(b(P,100,18000)[R.gap])) if R.gap.sum()>20 else None
    m["vowel_lvl"]=float(np.median(b(P,100,18000)[R.vowel]))
    # vowels right after an ess (within 120 ms): where a hold could linger
    ei=np.where(R.ess)[0]; after=np.zeros_like(R.vowel)
    for k in ei: after[k+1:k+12]=True
    av=after&R.vowel; m["vowel_after_ess"]=float(np.median(b(P,100,18000)[av])) if av.sum()>10 else None
    m["gap_re_voice"]=float(np.median(b(P,100,18000)[R.gap])-np.percentile(b(P,100,18000)[R.voiced],50)) if R.gap.sum()>20 else None
    fr=x[:len(x)//480*480].reshape(-1,480); l=10*np.log10(np.mean(fr**2,1)+1e-20); vv=np.repeat(R.voiced,1)[:len(l)]
    lv=l[l>np.percentile(l,95)-20]; m["spread"]=float(np.percentile(lv,90)-np.percentile(lv,10))
    m["clip"]=int(np.sum(np.abs(x)>0.999))
    return m
if __name__=="__main__":
    pipe=sys.argv[1]; res={}
    for line in open(f"{W}/takes/index.txt"):
        u,p=line.rstrip("\n").split("\t"); dry=mono(f"{W}/takes/{u}.wav"); R=Ref(dry)
        res[u]={"f0lo":R.f0lo,"f0med":R.f0med,"dry":measure(R,dry)}
        for d in sorted(glob.glob(f"{W}/out/{pipe}/*/")):
            st=os.path.basename(d.rstrip("/")); fp=f"{d}{u}.wav"
            if os.path.exists(fp): res[u][st]=measure(R,mono(fp))
        print(u, f"f0 {R.f0lo:.0f}-{R.f0med:.0f} Hz", flush=True)
    json.dump(res,open(f"{W}/metrics_{pipe}.json","w"),indent=1)
