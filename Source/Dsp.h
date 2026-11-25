#pragma once
#include <array>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <complex>
namespace voices {
constexpr float pi=3.14159265358979323846f;
inline float clamp(float x,float a,float b){return std::max(a,std::min(b,x));}
inline float gain(float db){return std::pow(10.f,db/20.f);}
inline float midi(float hz){return 69.f+12.f*std::log2(hz/440.f);}
inline unsigned scaleMask(int scale,unsigned mask){constexpr unsigned scales[]={4095,2741,1453,661,1193,1709,4095};return scale==6?(mask&4095u):scales[std::clamp(scale,0,6)];}
inline bool noteAllowed(int note,int key,int scale,unsigned mask){int pc=((note-(scale==6?0:key))%12+12)%12;return (scaleMask(scale,mask)&(1u<<pc))!=0;}
inline int nearest(float n,int key,int scale,unsigned mask){
 unsigned allowed=scaleMask(scale,mask); if(!allowed) return int(std::round(n));
 int center=int(std::round(n)),best=center; float dist=1e9f;
 for(int i=center-12;i<=center+12;++i){int pc=((i-(scale==6?0:key))%12+12)%12;if((allowed&(1u<<pc))&&std::abs(i-n)<dist){dist=std::abs(i-n);best=i;}}
 return best;
}
struct Detector {
 static constexpr int frameSize=512,hop=64;
 std::array<float,frameSize> ring{},frame{}; int pos=0,filled=0,count=0,decim=1,phase=0; float sr=12000,hz=0,confidence=0,lp=0,alpha=.3f;
 void prepare(double sampleRate){*this=Detector{};decim=std::max(1,int(std::round(sampleRate/12000)));sr=float(sampleRate/decim);alpha=1-std::exp(-2*pi*std::min(2500.f,sr*.2f)/float(sampleRate));}
 void push(float x){lp+=alpha*(x-lp);if(++phase<decim)return;phase=0;ring[pos]=lp;pos=(pos+1)%frameSize;filled=std::min(frameSize,filled+1);if(++count<hop||filled<frameSize)return;count=0;analyse();}
 void analyse(){
  float mean=0;for(int i=0;i<frameSize;++i){frame[i]=ring[(pos+i)%frameSize];mean+=frame[i];}mean/=frameSize;
  float energy=0;for(float& x:frame){x-=mean;energy+=x*x;}
  if(energy<.000002f*frameSize){hz=confidence=0;return;}
  const int lo=std::max(2,int(sr/900)),hi=std::min(300,int(sr/65)),span=frameSize-hi-1;
  std::array<float,302> cm{};float sum=0;cm[0]=1;
  for(int t=1;t<=hi+1;++t){float diff=0;for(int i=0;i<span;++i){float a=frame[i]-frame[i+t];diff+=a*a;}sum+=diff;cm[t]=sum>1e-12f?diff*t/sum:1;}
  int best=lo;for(int t=lo;t<=hi;++t){if(cm[t]<cm[best])best=t;if(cm[t]<.16f&&cm[t]<=cm[t+1]&&cm[t]<cm[t-1]){best=t;break;}}
  confidence=clamp(1-cm[best],0,1);if(confidence<.72f){hz=0;return;}
  float delta=0,a=cm[best-1],b=cm[best],c=cm[best+1],den=a-2*b+c;if(std::abs(den)>1e-8f)delta=clamp(.5f*(a-c)/den,-.5f,.5f);
  hz=sr/(best+delta);
 }
};
struct Delay {
 std::vector<float> buf; int w=0;
 void prepare(int n){buf.assign(n,0);w=0;}
 void write(float x){buf[w]=x;w=(w+1)%int(buf.size());}
 float cubic(float d)const{d=clamp(d,2,float(buf.size()-3));float x=float(w)-d;while(x<0)x+=float(buf.size());int i=int(x),n=int(buf.size());float t=x-i,a=buf[(i+n-1)%n],b=buf[i],c=buf[(i+1)%n],e=buf[(i+2)%n];return b+.5f*t*(c-a+t*(2*a-5*b+4*c-e+t*(3*(b-c)+e-a)));}
 float read(float d)const{d=clamp(d,1,float(buf.size()-2));float p=float(w)-d;while(p<0)p+=float(buf.size());int i=int(p);float f=p-i;return buf[i]+f*(buf[(i+1)%buf.size()]-buf[i]);}
};
struct Shifter {
 Delay d;float phase=0,window=480;
 void prepare(float sr){window=std::max(64.f,float(std::round(sr*.02f/2)*2));phase=0;d.prepare(int(window*2)+16);}
 float tick(float x,float ratio){d.write(x);phase+=(1-ratio)/window;phase-=std::floor(phase);float p2=phase+.5f;p2-=std::floor(p2);float a=.5f-.5f*std::cos(2*pi*phase);return a*d.cubic(2+phase*window)+(1-a)*d.cubic(2+p2*window);}
};
// Fixed-size, allocation-free weighted overlap-add spectral envelope processor.
// This changes the harmonic amplitude envelope, not the harmonic frequencies.
struct FormantEngine {
 static constexpr int maxN=2048,maxBins=maxN/2+1;
 using Complex=std::complex<float>;
 std::array<float,maxN> reference{},wet{},ola{},window{};
 std::array<Complex,maxN> r{},w{};
 std::array<Complex,maxN/2> roots{};
 std::array<float,maxBins> logRef{},logWet{},envRef{},envWet{},tmp{},gains{};
 std::array<float,maxBins+1> prefix{};
 int n=1024,hop=256,pos=0,count=0;float sr=48000;
 void prepare(float sampleRate){
  sr=sampleRate;n=sr>64000?2048:1024;hop=n/4;pos=count=0;
  reference.fill(0);wet.fill(0);ola.fill(0);gains.fill(0);
  for(int i=0;i<n;++i)window[i]=std::sqrt(std::max(0.f,.5f-.5f*std::cos(2*pi*i/n)));
  for(int i=0;i<n/2;++i)roots[i]=Complex(std::cos(-2*pi*i/n),std::sin(-2*pi*i/n));
 }
 void fft(std::array<Complex,maxN>& a,bool inverse)const{
  for(int i=1,j=0;i<n;++i){int bit=n>>1;for(;j&bit;bit>>=1)j^=bit;j^=bit;if(i<j)std::swap(a[i],a[j]);}
  for(int len=2;len<=n;len*=2){int half=len/2,step=n/len;for(int i=0;i<n;i+=len)for(int j=0;j<half;++j){auto root=roots[j*step];if(inverse)root=std::conj(root);auto u=a[i+j],v=a[i+j+half]*root;a[i+j]=u+v;a[i+j+half]=u-v;}}
  if(inverse)for(int i=0;i<n;++i)a[i]/=float(n);
 }
 void average(const std::array<float,maxBins>& input,std::array<float,maxBins>& output,int radius){
  const int bins=n/2+1;prefix[0]=0;for(int i=0;i<bins;++i)prefix[i+1]=prefix[i]+input[i];
  for(int i=0;i<bins;++i){int lo=std::max(0,i-radius),hi=std::min(bins-1,i+radius);output[i]=(prefix[hi+1]-prefix[lo])/float(hi-lo+1);}
 }
 void envelope(const std::array<Complex,maxN>& spectrum,std::array<float,maxBins>& logMag,std::array<float,maxBins>& env,int radius){
  float peak=0;for(int i=0;i<=n/2;++i)peak=std::max(peak,std::abs(spectrum[i]));
  float floor=std::max(1e-7f,peak*.001f);
  for(int i=0;i<=n/2;++i)logMag[i]=std::log(std::max(floor,std::abs(spectrum[i])));
  average(logMag,tmp,radius);average(tmp,env,radius);
 }
 float tick(float ref,float signal,float semitones,float pitchRatio,float fundamental,bool enabled){
  float out=ola[pos];ola[pos]=0;reference[pos]=ref;wet[pos]=signal;pos=(pos+1)%n;
  if(++count<hop)return out;count=0;
  const bool active=enabled&&(std::abs(semitones)>.01f||std::abs(pitchRatio-1)>.002f)&&fundamental>50;
  for(int i=0;i<n;++i){int ix=(pos+i)%n;w[i]=Complex(wet[ix]*window[i],0);if(active)r[i]=Complex(reference[ix]*window[i],0);}
  fft(w,false);
  if(active){
   fft(r,false);const int radius=std::clamp(int(std::ceil(std::max(160.f,fundamental*1.1f)*n/sr)),2,n/12);
   envelope(r,logRef,envRef,radius);envelope(w,logWet,envWet,radius);
   const float scale=std::pow(2.f,semitones/12.f);
   for(int i=0;i<=n/2;++i){
    float source=clamp(i/scale,0,float(n/2));int lo=int(source),hi=std::min(n/2,lo+1);
    float desired=envRef[lo]+(source-lo)*(envRef[hi]-envRef[lo]);
    float freq=float(i)*sr/n;
    float taper=clamp(freq/180.f,0,1)*clamp((std::min(10000.f,sr*.47f)-freq)/1800.f,0,1);
    float goal=clamp(desired-envWet[i],-1.38f,1.38f)*taper;
    gains[i]+=.35f*(goal-gains[i]);
   }
  }else for(int i=0;i<=n/2;++i)gains[i]*=.65f;
  for(int i=0;i<=n/2;++i){float g=std::exp(gains[i]);w[i]*=g;if(i>0&&i<n/2)w[n-i]*=g;}
  fft(w,true);for(int i=0;i<n;++i)ola[(pos+i)%n]+=.5f*window[i]*w[i].real();
  return out;
 }
};
struct Pole {float z=0;float low(float x,float hz,float sr){float a=1-std::exp(-2*pi*clamp(hz,10,sr*.45f)/sr);z+=a*(x-z);return z;}};
struct Params {
 float input=0,output=0,mix=1,speed=35,human=.45f,flex=8,transition=35,amount=1,formant=0,drive=4,satMix=.25f,hp=70,body=0,presence=0,air=0,doubleAmount=.2f,spread=.7f,detune=7,chorusRate=.35f,chorusDepth=3,chorusMix=.15f,delayMs=280,feedback=.22f,delayMix=.12f,size=.35f,decay=.42f,tone=6500,reverbMix=.12f;
 int key=0,scale=0,character=0;unsigned mask=4095; bool power=true,tune=true,sat=false,eq=true,doubler=false,chorus=false,delay=false,reverb=false;
};
struct VocalRoom {
 std::array<Delay,4> lines;std::array<Pole,4> damp;
 void prepare(float sr){for(auto& line:lines)line.prepare(int(sr*.14f)+8);damp={};}
 float tick(float x,float size,float decay,float tone,float sr,int channel){
  constexpr float lengths[]={.0297f,.0371f,.0411f,.0437f};float wet=0;
  for(int i=0;i<4;++i){float delay=sr*(lengths[i]*(.65f+size*1.35f)+channel*.0017f*(i+1));float tap=lines[i].read(delay);float filtered=damp[i].low(tap,tone,sr);lines[i].write(x*.5f+filtered*clamp(decay,0,.92f));wet+=tap*.5f;}
  return wet;
 }
};
// Lightweight causal vocal features; no model, allocations or lookahead.
// Classes: 0 quiet, 1 breath/noise, 2 sibilant, 3 transient, 4 voiced,
// 5 stable/singing-like, 6 variable/speech-like. Last two are heuristics.
struct VocalIntelligence {
 float sr=48000,low=0,fast=0,slow=0,high=0,energy=0,protection=0,centre=0,variation=0,stable=0,rangeLow=0,rangeHigh=0,last=0;
 float aFast=0,aSlow=0,aLow=0,aTrack=0,aProtect=0,aRelease=0;
 int kind=0,hold=0,clock=0;bool wasVoiced=false;
 void prepare(float rate){*this=VocalIntelligence{};sr=rate;aFast=1-std::exp(-1.f/(sr*.003f));aSlow=1-std::exp(-1.f/(sr*.04f));aLow=1-std::exp(-2*pi*2200.f/sr);aTrack=1-std::exp(-16.f/(sr*.22f));aProtect=1-std::exp(-1.f/(sr*.0015f));aRelease=1-std::exp(-1.f/(sr*.025f));}
 void tick(float x,float hz,float confidence){
  low+=aLow*(x-low);float hf=x-low,xx=x*x;fast+=aFast*(xx-fast);slow+=aSlow*(xx-slow);high+=aFast*(hf*hf-high);energy=fast;
  float brightness=high/std::max(fast,1e-9f);
  bool voiced=hz>50&&confidence>.72f&&fast>2e-6f;
  if(fast>1e-4f&&fast>slow*4.5f&&(!voiced||brightness>.35f))hold=int(sr*.012f);
  if(hold>0)--hold;
  if(fast<2e-6f)kind=0;
  else if(brightness>.48f)kind=2;
  else if(hold>0)kind=3;
  else if(!voiced)kind=1;
  else kind=stable>.55f?5:variation>.32f?6:4;
  const bool protect=kind<4;
  protection+=(protect?aProtect:aRelease)*((protect?1.f:0.f)-protection);
  clock=(clock+1)&15;
  if(clock==1){
   if(voiced&&brightness<.48f&&hold==0){
    float note=midi(hz);
    if(!wasVoiced){centre=note;last=note;variation=0;}
    float delta=std::abs(note-centre);centre+=aTrack*(note-centre);
    variation+=aTrack*(delta-variation);stable+=aTrack*((delta<.45f?1.f:0.f)-stable);
    if(rangeLow==0){rangeLow=rangeHigh=hz;}
    else{rangeLow=std::min(rangeLow,hz);rangeHigh=std::max(rangeHigh,hz);}
    last=note;
   }else stable+=aTrack*(0-stable);
   wasVoiced=voiced;
  }
 }
};
struct Engine {
 struct Channel {Shifter pitch,dub,dubOther;FormantEngine formants;Delay dry,alignedDry,chor,echo;VocalRoom room;Pole hp,body,pres,air;float pitchBlend=0;};
 std::array<Channel,2> ch; Detector det; VocalIntelligence intelligence;float smoothAlpha=0; float sr=48000,ratio=1,phase=0,target=0,detected=0,correction=0;int targetNote=-1,transitionRemaining=0;float leftPower=0,rightPower=0,midPower=0,detectorBlend=0;Params smooth{};bool first=true,live=false;float noteCentre=0,fade=1;std::array<float,8> enabled{};int control=0;float desiredRatio=1,ratioAlpha=.001f;
 int latency(bool isLive)const{return int(std::round(sr*(isLive?.004f:.01f)))+1+(isLive?0:ch[0].formants.n);}
 int latency()const{return latency(live);}
 void setMode(bool mode){if(live==mode)return;live=mode;fade=0;for(auto& c:ch){c.pitch.window=float(2*(latency(mode)-1-(mode?0:c.formants.n)));c.pitch.phase=0;c.pitchBlend=0;c.formants.prepare(sr);std::fill(c.alignedDry.buf.begin(),c.alignedDry.buf.end(),0.f);c.alignedDry.w=0;}}
 void prepare(double s){sr=float(s);live=false;noteCentre=0;fade=1;control=0;desiredRatio=1;ratioAlpha=.001f;enabled.fill(0);det.prepare(s);intelligence.prepare(sr);smoothAlpha=1-std::exp(-1.f/(sr*.008f));ratio=1;phase=0;target=detected=correction=0;targetNote=-1;transitionRemaining=0;leftPower=rightPower=midPower=detectorBlend=0;first=true;for(auto& c:ch){c=Channel{};c.pitch.prepare(sr);c.dub.prepare(sr);c.dubOther.prepare(sr);c.formants.prepare(sr);c.alignedDry.prepare(c.formants.n+8);c.dry.prepare(int(sr*.05f)+8);c.chor.prepare(int(sr*.06f)+8);c.echo.prepare(int(sr*2.1f)+8);c.room.prepare(sr);}}
 void tick(float* l,float* r,int channels,const Params& p){
  float in[2]={*l,channels>1?*r:*l}; const std::array<bool,8> switches={p.power,p.tune,p.sat,p.eq,p.doubler,p.chorus,p.delay,p.reverb};if(first){smooth=p;for(int i=0;i<8;++i)enabled[i]=switches[i]?1.f:0.f;first=false;} const float a=smoothAlpha;for(int i=0;i<8;++i)enabled[i]+=a*((switches[i]?1.f:0.f)-enabled[i]);fade=std::min(1.f,fade+1.f/(sr*.012f));
  #define S(k) smooth.k+=a*(p.k-smooth.k)
  S(input);S(output);S(mix);S(speed);S(human);S(flex);S(transition);S(amount);S(formant);S(drive);S(satMix);S(hp);S(body);S(presence);S(air);S(doubleAmount);S(spread);S(detune);S(chorusRate);S(chorusDepth);S(chorusMix);S(delayMs);S(feedback);S(delayMix);S(size);S(decay);S(tone);S(reverbMix);
  #undef S
  auto& q=smooth;
  const float mid=(in[0]+in[1])*.5f;
  leftPower+=a*(in[0]*in[0]-leftPower);rightPower+=a*(in[1]*in[1]-rightPower);midPower+=a*(mid*mid-midPower);
  const float strongest=std::max(leftPower,rightPower);
  const float fallback=channels>1&&strongest>1e-8f?clamp(1-midPower/(.15f*strongest),0,1):0;
  detectorBlend+=a*(fallback-detectorBlend);
  const float side=leftPower>=rightPower*.9f?in[0]:in[1];
  const float analysisInput=(mid+(side-mid)*detectorBlend)*gain(q.input);det.push(analysisInput);float goal=0;detected=det.hz;intelligence.tick(analysisInput,detected,det.confidence);
  const int previous=targetNote;
  control=(control+1)&15;if(control==1){
  if(detected>0&&scaleMask(p.scale,p.mask)!=0){
   const float note=midi(detected);int n=nearest(note,p.key,p.scale,p.mask);
   if(p.character==0&&previous>=0&&noteAllowed(previous,p.key,p.scale,p.mask)&&std::abs(note-previous)<=std::abs(note-n)+(.08f+.12f*q.human))n=previous;
   targetNote=n;target=440*std::pow(2.f,(n-69)/12.f);
   if(previous!=n||noteCentre==0)noteCentre=note;
   else noteCentre+=(1-std::exp(-16.f/(sr*.045f)))*(note-noteCentre);
   const float tracked=p.character==1?note:note+(noteCentre-note)*q.human;
   const float diff=n-tracked,tolerance=q.flex*.01f;
   goal=std::copysign(std::max(0.f,std::abs(diff)-tolerance),diff)*q.amount*(p.character==1?1.f:1.f-q.human*.45f);
  }else{target=0;targetNote=-1;noteCentre=0;}
  if(previous>=0&&targetNote>=0&&previous!=targetNote)transitionRemaining=int(sr*.001f*p.transition);
  if(targetNote<0)transitionRemaining=0;
  float time=p.character==1?std::min(8.f,p.speed):p.speed;
  if(transitionRemaining>0){time=std::max(time,p.transition);transitionRemaining=std::max(0,transitionRemaining-16);}
  time=std::max(1.f,time);
  desiredRatio=std::pow(2.f,(p.tune&&p.power?goal:0.f)/12.f);ratioAlpha=1-std::exp(-1000.f/(sr*time));}
  ratio+=ratioAlpha*(desiredRatio-ratio);
  correction=p.tune&&p.power?12*std::log2(ratio)*100*(1-intelligence.protection):0;
  phase+=q.chorusRate/sr;phase-=std::floor(phase);
  const float inputGain=gain(q.input),outputGain=gain(q.output),driveGain=gain(q.drive),bodyGain=gain(q.body)-1,presGain=gain(q.presence)-1,airGain=gain(q.air)-1;
  const float dubUp=std::pow(2.f,q.detune/1200.f),dubDown=1.f/dubUp;
  for(int k=0;k<channels;++k){
   auto& c=ch[k];c.dry.write(in[k]);float raw=c.dry.read(c.pitch.window*.5f+2);float dry=raw*inputGain;
   // Gain applied after the pitch delay to keep the neutral and shifted paths aligned.
   float pitched=c.pitch.tick(in[k],ratio)*inputGain;
   float blend=enabled[1]*clamp(std::abs(ratio-1)/.002f,0,1);
   c.pitchBlend+=a*(blend-c.pitchBlend);float y=dry+(pitched-dry)*c.pitchBlend*(1-intelligence.protection);
   if(!live){y=c.formants.tick(dry,y,q.formant*enabled[1],ratio,detected,enabled[1]>.001f&&enabled[0]>.001f&&intelligence.protection<.5f);c.alignedDry.write(raw);raw=c.alignedDry.read(float(c.formants.n+1));dry=raw*inputGain;}
   y+=(std::tanh(y*driveGain)/std::max(1.f,std::sqrt(driveGain))-y)*q.satMix*enabled[2];
   float eqY=y-c.hp.low(y,q.hp,sr);float b=c.body.low(eqY,400,sr),m=c.pres.low(eqY,3500,sr),hi=eqY-c.air.low(eqY,9000,sr);
   eqY+=b*bodyGain+(m-b)*presGain+hi*airGain;y+=(eqY-y)*enabled[3];
   float lowDub=c.dub.tick(y,dubDown),highDub=c.dubOther.tick(y,dubUp);
   float pan=(channels==1?.5f:(k==0?.5f-.5f*q.spread:.5f+.5f*q.spread));
   float doubled=lowDub+(highDub-lowDub)*pan;y+=(doubled-y)*q.doubleAmount*enabled[4];
   c.chor.write(y);float cy=c.chor.read(sr*.001f*(12+q.chorusDepth*std::sin(2*pi*(phase+k*.25f))));y+=(cy-y)*q.chorusMix*enabled[5];
   float echo=c.echo.read(sr*.001f*q.delayMs);c.echo.write(y+echo*q.feedback*enabled[6]);y+=echo*q.delayMix*enabled[6];
   float rv=c.room.tick(y,q.size,q.decay,q.tone,sr,k);y+=rv*q.reverbMix*enabled[7];
   float processed=(dry+(y-dry)*q.mix)*outputGain;
   float out=(raw+(processed-raw)*enabled[0])*fade;if(!std::isfinite(out))out=0;if(k==0)*l=out;else *r=out;
  }
 }
};
}
