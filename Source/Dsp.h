#pragma once
#include <array>
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
namespace voices {
constexpr float pi=3.14159265358979323846f;
inline float clamp(float x,float a,float b){return std::max(a,std::min(b,x));}
inline float gain(float db){return std::pow(10.f,db/20.f);}
inline float midi(float hz){return 69.f+12.f*std::log2(hz/440.f);}
inline int nearest(float n,int key,int scale,unsigned mask){
 constexpr unsigned scales[]={4095,2741,1453,661,1193,1709,4095};
 unsigned allowed=scale==6?mask:scales[std::clamp(scale,0,6)]; if(!allowed) return int(std::round(n));
 int center=int(std::round(n)),best=center; float dist=1e9f;
 for(int i=center-12;i<=center+12;++i){int pc=((i-(scale==6?0:key))%12+12)%12;if((allowed&(1u<<pc))&&std::abs(i-n)<dist){dist=std::abs(i-n);best=i;}}
 return best;
}
struct Detector {
 std::array<float,1024> ring{},frame{}; int pos=0,filled=0,count=0,decim=1,phase=0; float sr=12000,hz=0,confidence=0,lp=0,alpha=.3f;
 void prepare(double sampleRate){*this=Detector{};decim=std::max(1,int(std::round(sampleRate/12000)));sr=float(sampleRate/decim);alpha=1-std::exp(-2*pi*std::min(2500.f,sr*.2f)/float(sampleRate));}
 void push(float x){lp+=alpha*(x-lp);if(++phase<decim)return;phase=0;ring[pos]=lp;pos=(pos+1)%1024;filled=std::min(1024,filled+1);if(++count<128||filled<1024)return;count=0;analyse();}
 void analyse(){for(int i=0;i<1024;++i)frame[i]=ring[(pos+i)%1024];float e=0;for(float x:frame)e+=x*x;if(e<.000002f*1024){hz=confidence=0;return;}
  int lo=std::max(2,int(sr/900)),hi=std::min(300,int(sr/65)); std::array<float,302> cm{}; float sum=0; cm[0]=1;
  for(int t=1;t<=hi+1;++t){float d=0;for(int i=0;i<700;++i){float a=frame[i]-frame[i+t];d+=a*a;}sum+=d;cm[t]=sum>1e-12f?d*t/sum:1;}
  int best=lo; for(int t=lo;t<=hi;++t){if(cm[t]<cm[best])best=t;if(cm[t]<.16f&&cm[t]<=cm[t+1]&&cm[t]<cm[t-1]){best=t;break;}}
  confidence=1-cm[best];if(confidence<.72f){hz=0;return;}float delta=0,a=cm[best-1],b=cm[best],c=cm[best+1],den=a-2*b+c;if(std::abs(den)>1e-8f)delta=clamp(.5f*(a-c)/den,-.5f,.5f);hz=sr/(best+delta);
 }
};
struct Delay {
 std::vector<float> buf; int w=0;
 void prepare(int n){buf.assign(n,0);w=0;}
 void write(float x){buf[w]=x;w=(w+1)%int(buf.size());}
 float read(float d)const{d=clamp(d,1,float(buf.size()-2));float p=float(w)-d;while(p<0)p+=float(buf.size());int i=int(p);float f=p-i;return buf[i]+f*(buf[(i+1)%buf.size()]-buf[i]);}
};
struct Shifter {
 Delay d;float phase=0,window=480;
 void prepare(float sr){window=std::max(64.f,sr*.02f);phase=0;d.prepare(int(window*2)+16);}
 float tick(float x,float ratio){d.write(x);phase+=(1-ratio)/window;phase-=std::floor(phase);float p2=phase+.5f;p2-=std::floor(p2);float a=.5f-.5f*std::cos(2*pi*phase);return a*d.read(2+phase*window)+(1-a)*d.read(2+p2*window);}
};
struct Pole {float z=0;float low(float x,float hz,float sr){float a=1-std::exp(-2*pi*clamp(hz,10,sr*.45f)/sr);z+=a*(x-z);return z;}};
struct Params {
 float input=0,output=0,mix=1,speed=35,human=.45f,flex=8,transition=35,amount=1,formant=0,drive=4,satMix=.25f,hp=70,body=0,presence=0,air=0,doubleAmount=.2f,spread=.7f,detune=7,chorusRate=.35f,chorusDepth=3,chorusMix=.15f,delayMs=280,feedback=.22f,delayMix=.12f,size=.35f,decay=.42f,tone=6500,reverbMix=.12f;
 int key=0,scale=0,character=0;unsigned mask=4095; bool power=true,tune=true,sat=false,eq=true,doubler=false,chorus=false,delay=false,reverb=false;
};
struct Engine {
 struct Channel {Shifter pitch,dub;Delay dry,chor,echo,rev;Pole hp,body,pres,air,form,tone;float revFeedback=0;};
 std::array<Channel,2> ch; Detector det; float sr=48000,ratio=1,phase=0,target=0,detected=0,correction=0;int targetNote=-1;float lastGoal=0;Params smooth{};bool first=true;
 void prepare(double s){sr=float(s);det.prepare(s);ratio=1;phase=0;target=detected=correction=lastGoal=0;targetNote=-1;first=true;for(auto& c:ch){c=Channel{};c.pitch.prepare(sr);c.dub.prepare(sr);c.dry.prepare(int(sr*.05f)+8);c.chor.prepare(int(sr*.06f)+8);c.echo.prepare(int(sr*2.1f)+8);c.rev.prepare(int(sr*.2f)+8);}}
 void tick(float* l,float* r,int channels,const Params& p){
  float in[2]={*l,channels>1?*r:*l}; if(first){smooth=p;first=false;} float a=1-std::exp(-1.f/(sr*.008f));
  #define S(k) smooth.k+=a*(p.k-smooth.k)
  S(input);S(output);S(mix);S(amount);S(formant);S(drive);S(satMix);S(hp);S(body);S(presence);S(air);S(doubleAmount);S(spread);S(detune);S(chorusRate);S(chorusDepth);S(chorusMix);S(delayMs);S(feedback);S(delayMix);S(size);S(decay);S(tone);S(reverbMix);
  #undef S
  auto& q=smooth;det.push((in[0]+in[1])*.5f*gain(q.input));float goal=0;detected=det.hz;
  if(detected>0){float note=midi(detected);int n=nearest(note,p.key,p.scale,p.mask);if(targetNote!=n){targetNote=n;}target=440*std::pow(2.f,(n-69)/12.f);float diff=n-note;float tolerance=p.flex*.01f;if(std::abs(diff)>tolerance)goal=diff* q.amount*(p.character==1?1.f:1.f-p.human*.45f);}
  else{target=0;targetNote=-1;}
  float time=p.character==1?std::min(8.f,p.speed):p.speed;time=std::max(1.f,time+(goal*lastGoal<0?p.transition:0));lastGoal=goal;float desired=std::pow(2.f,goal/12.f);ratio+=(1-std::exp(-1000.f/(sr*time)))*(desired-ratio);correction=12*std::log2(ratio)*100;
  phase+=q.chorusRate/sr;phase-=std::floor(phase);
  for(int k=0;k<channels;++k){auto& c=ch[k];float x=in[k]*gain(q.input);c.dry.write(x);float dry=c.dry.read(c.pitch.window*.5f+2);float pitched=c.pitch.tick(x,p.tune?ratio:1);float y=p.tune&&std::abs(ratio-1)>.0005f?pitched:dry;
   // Prototype spectral colour proxy: not true independent formant shifting.
   float low=c.form.low(y,1100,sr);y+=q.formant/12.f*(y-2*low)*.35f;
   if(p.sat){float g=gain(q.drive);y+=(std::tanh(y*g)/std::max(1.f,std::sqrt(g))-y)*q.satMix;}
   if(p.eq){y-=c.hp.low(y,q.hp,sr);float b=c.body.low(y,400,sr);float m=c.pres.low(y,3500,sr);float h=y-c.air.low(y,9000,sr);y+=b*(gain(q.body)-1)+(m-b)*(gain(q.presence)-1)+h*(gain(q.air)-1);}
   float dub=c.dub.tick(y,std::pow(2.f,((k==0?-1:1)*q.detune)/1200.f));if(p.doubler)y+=(dub-y)*q.doubleAmount*(.5f+.5f*q.spread);
   c.chor.write(y);float cy=c.chor.read(sr*.001f*(12+q.chorusDepth*std::sin(2*pi*(phase+k*.25f))));if(p.chorus)y+=(cy-y)*q.chorusMix;
   float echo=c.echo.read(sr*.001f*q.delayMs);c.echo.write(y+(p.delay?echo*q.feedback:0));if(p.delay)y+=echo*q.delayMix;
   float rv=c.rev.read(sr*(.025f+q.size*.06f+k*.009f));float damp=c.tone.low(rv,q.tone,sr);c.rev.write(y+damp*clamp(q.decay,0,.92f));if(p.reverb)y+=rv*q.reverbMix;
   float out=p.power?(dry+(y-dry)*q.mix)*gain(q.output):in[k]; if(!std::isfinite(out))out=0; if(k==0)*l=out;else *r=out;
  }
 }
};
}
