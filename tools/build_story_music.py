"""Compose deterministic, original instrumental loops. No samples or third-party melodies."""
from pathlib import Path
import json, wave
import numpy as np
RATE=22050
ROOT=Path(__file__).resolve().parents[1]
OUT=ROOT/'App'/'Music'
OUT.mkdir(parents=True,exist_ok=True)
RNG=np.random.default_rng(20261013)
def freq(midi):return 440*2**((midi-69)/12)
def voice(midi,dur,kind):
 t=np.arange(int((dur+.35)*RATE))/RATE;f=freq(midi)
 release=np.minimum(1,np.maximum(0,(dur+.35-t)/.35))
 if kind=='flute':
  env=np.minimum(1,t/.10)*release
  phase=2*np.pi*f*t+.035*np.sin(2*np.pi*5.1*t)
  v=(np.sin(phase)+.20*np.sin(2*phase)+.06*np.sin(3*phase))*env
 elif kind=='pluck':
  v=sum((np.sin(2*np.pi*f*k*t+.06*k)*np.exp(-t*(2.4+k*.7)))/k**1.25 for k in range(1,7))
  v*=np.minimum(1,t/.006)*release
 elif kind=='brass':
  env=np.minimum(1,t/.05)*release*(.72+.28*np.exp(-t*4))
  v=sum(np.sin(2*np.pi*f*k*t)/k**1.3 for k in range(1,6))*env
 else:
  env=np.minimum(1,t/.23)*release
  v=sum((np.sin(2*np.pi*f*(1+detune)*t)+.18*np.sin(4*np.pi*f*(1+detune)*t))/3 for detune in [-.0023,0,.0023])*env
 return v

def make(name,bpm,kind):
 beat=60/bpm;duration=64*beat;n=round(duration*RATE);mix=np.zeros((n,2),dtype=np.float64)
 def add(data,at,gain,pan=.0):
  pos=round(at*RATE);stereo=np.stack((data*np.sqrt((1-pan)/2),data*np.sqrt((1+pan)/2)),axis=1)*gain
  idx=(np.arange(len(data))+pos)%n;np.add.at(mix,idx,stereo)
 # Distinct original phrases in a minor pentatonic language, developed over sixteen bars.
 roots=[50,46,48,45,50,53,48,45]*2 if kind in ['battle','suspense'] else [50,53,48,46,50,55,48,45]*2
 phrases=[[0,3,7,10,7,3,2,0],[7,10,12,10,7,5,3,7],[3,5,7,12,10,7,3,2],[7,5,3,2,0,3,2,0]]
 if kind=='victory':roots=[50,55,53,48,50,58,55,53]*2
 for bar,root in enumerate(roots):
  start=bar*4*beat;triad=[root,root+3,root+7]
  for k,note in enumerate(triad):add(voice(note,4*beat,'strings'),start,.050 if kind=='journey' else .055,(k-1)*.55)
  add(voice(root-12,3.8*beat,'strings'),start,.065)
  pattern=phrases[(bar//4)%4]
  for step in range(8):
   note=root+12+pattern[step]
   if kind=='suspense':note=root+12+[0,7,3,7,0,10,3,7][step]
   add(voice(note,beat*.4,'pluck'),start+step*beat/2,.08 if kind=='journey' else .065,(-.55 if step%2 else .55))
  # Leave breathing space in the first half, then build a larger returning theme.
  lead=phrases[bar%4];instrument='flute' if kind in ['journey','suspense'] else 'brass'
  for step in range(4):
   if kind=='suspense' and step not in [0,2]:continue
   note=root+12+lead[(step*2+bar%2)%8]+(12 if bar>=12 and kind in ['world','victory'] else 0)
   add(voice(note,beat*.82,instrument),start+step*beat,.12 if kind=='journey' else .14,.08*np.sin(bar))
  for step in range(4):
   t=np.arange(int(.45*RATE))/RATE
   env=np.exp(-t*12)*np.minimum(1,t/.003)
   drum=np.sin(2*np.pi*(63*t-48*t*t))*env
   noise=RNG.uniform(-1,1,len(t));metal=noise*np.exp(-t*44)+.2*np.sin(2*np.pi*940*t)*np.exp(-t*25)
   gain=.025 if kind=='journey' else .09 if kind=='suspense' else .12 if kind=='battle' else .085
   if step in [0,2] or kind=='battle':add(drum,start+step*beat,gain)
   if kind!='journey' and step in [1,3]:add(metal,start+step*beat,.025,(-.45 if step==1 else .45))
  if bar in [0,4,8,12] and kind!='journey':
   t=np.arange(int(2*RATE))/RATE;gong=(np.sin(2*np.pi*110*t)+.3*np.sin(2*np.pi*161.7*t)+.2*np.sin(2*np.pi*293.2*t))*np.exp(-t*2.6)
   add(gong,start,.065)
 # Small stereo echoes; periodic wrap makes reverb and releases cross the loop seam naturally.
 mix+=np.roll(mix,round(.13*RATE),axis=0)[:,::-1]*.12
 mix+=np.roll(mix,round(.27*RATE),axis=0)*.08
 mix=np.tanh(mix*1.25);mix*=.78/max(.001,np.max(np.abs(mix)))
 edge=round(.006*RATE)
 mix[:edge]*=np.linspace(0,1,edge)[:,None]
 mix[-edge:]*=np.linspace(1,0,edge)[:,None]
 pcm=np.round(mix*32767).astype('<i2')
 path=OUT/(name+'.wav')
 with wave.open(str(path),'wb') as w:w.setnchannels(2);w.setsampwidth(2);w.setframerate(RATE);w.writeframes(pcm.tobytes())
 rms=float(np.sqrt(np.mean(mix*mix)));peak=float(np.max(np.abs(mix)));seam=float(np.max(np.abs(mix[0]-mix[-1])))
 assert np.isfinite(mix).all() and peak<=.781 and rms>.01
 return {'file':path.name,'bpm':bpm,'seconds':round(n/RATE,3),'peak':round(peak,4),'rms':round(rms,4),'seam_delta':round(seam,4),'bytes':path.stat().st_size}
if __name__=='__main__':
 tracks=[('banner-of-dawn',88,'title'),('companions-road',76,'journey'),('bridge-in-the-storm',96,'suspense'),('battle-of-the-burning-river',128,'battle'),('oath-carried-forward',104,'victory'),('a-continent-under-banners',100,'world')]
 stats=[make(*track) for track in tracks]
 (OUT/'manifest.json').write_text(json.dumps(stats,indent=2),encoding='utf-8')
 print(json.dumps(stats,indent=2))
