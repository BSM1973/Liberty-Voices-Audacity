#pragma once
#include "Dsp.h"
#include "Precision.h"
#include <cstdint>
namespace voices {
// Linked master processing. The final stage uses an experimental reconstructed-peak lookahead limiter.
struct MasterParams {
 bool compressor=false,saturation=false,limiter=false,dynamicEq=false,external=false,power=true;
 int model=0,color=0,oversampling=0;float comparisonGain=1;float threshold=-18,ratio=3,makeup=0,drive=4,mix=.25f,ceiling=-1,release=80;
 std::array<float,6> freq{{2500,120,400,1000,5000,10000}},q{{1,1,1,1,1,1}},dynThreshold{{-24,-24,-24,-24,-24,-24}},dynRange{};
};
struct BandDetector {
 float b0=0,b2=0,a1=0,a2=0,z1=0,z2=0;
 void configure(float sr,float f,float q){float w=2*pi*clamp(f,30,sr*.45f)/sr,alpha=std::sin(w)/(2*std::max(.2f,q)),a=1+alpha;b0=alpha/a;b2=-b0;a1=-2*std::cos(w)/a;a2=(1-alpha)/a;}
 float tick(float x){float y=b0*x+z1;z1=-a1*y+z2;z2=b2*x-a2*y;return y;}
};
struct MasterEngine {
 float sr=48000,env=0,compGain=1,limGain=1,compMix=0,satMix=0,limMix=0,eqMix=0,smooth=0,atk=0,rel=0,limRel=0,drive=1,makeup=1,ceilGain=1,ratio=3,threshold=-18;int count=0,model=0,color=0;
 std::array<std::array<BandDetector,6>,2> detector;
 std::array<std::array<StudioBell,6>,2> filters;
 std::array<float,6> bandEnv{},bandGain{};
 float dynAttack=0,dynRelease=0;std::array<std::array<SaturationOS,3>,2> os;std::array<std::array<float,16>,2> dry{};int dryWrite=0,osMode=0;bool osRunning=false;float osFade=0,abGain=1,preEnergy=0,energyAlpha=0;PeakLookahead lookahead;int latency()const{return 16+lookahead.latency;}
 void prepare(float s){*this=MasterEngine{};sr=s;smooth=1-std::exp(-1/(sr*.008f));dynAttack=1-std::exp(-1/(sr*.008f));dynRelease=1-std::exp(-1/(sr*.09f));energyAlpha=1-std::exp(-1/(sr*.4f));lookahead.prepare(sr);for(auto& c:os)for(int i=0;i<3;++i)c[i].prepare(2<<i);}
 static float shape(float x,int c){switch(c){case 1:return x/(1+std::abs(x));case 2:return std::tanh(x+.18f*x*x);default:return std::tanh(x);}}
 void tick(float& left,float& right,int channels,float external,const MasterParams& p){
  float x[2]={left,channels>1?right:left};
  compMix+=smooth*((p.power&&p.compressor?1.f:0.f)-compMix);satMix+=smooth*((p.power&&p.saturation?p.mix:0.f)-satMix);limMix+=smooth*((p.power&&p.limiter?1.f:0.f)-limMix);eqMix+=smooth*((p.power&&p.dynamicEq?1.f:0.f)-eqMix);
  count=(count+1)&31;
  if(count==1){model=std::clamp(p.model,0,3);color=std::clamp(p.color,0,2);const float attacks[]={18,1,8,30},releases[]={220,55,100,350};atk=1-std::exp(-1000/(sr*attacks[model]));rel=1-std::exp(-1000/(sr*releases[model]));limRel=1-std::exp(-1000/(sr*std::max(5.f,p.release)));drive=gain(p.drive);makeup=gain(p.makeup);ceilGain=gain(p.ceiling);ratio=std::max(1.f,p.ratio);threshold=p.threshold;
   if(eqMix>.0001f||p.dynamicEq)for(int k=0;k<channels;++k)for(int b=0;b<6;++b){detector[k][b].configure(sr,p.freq[b],p.q[b]);filters[k][b].configure(sr,p.freq[b],bandGain[b],p.q[b]);}}
  if(eqMix>.0001f){float levels[6]{};for(int k=0;k<channels;++k)for(int b=0;b<6;++b)levels[b]=std::max(levels[b],std::abs(detector[k][b].tick(x[k])));
   for(int b=0;b<6;++b){bandEnv[b]+=(levels[b]>bandEnv[b]?dynAttack:dynRelease)*(levels[b]-bandEnv[b]);if(count==1){float db=20*std::log10(std::max(1e-6f,bandEnv[b]));float goal=p.dynRange[b]*clamp((db-p.dynThreshold[b])/12,0,1);bandGain[b]+=.25f*(goal-bandGain[b]);}}
   for(int k=0;k<channels;++k){float y=x[k];for(int b=0;b<6;++b)y=filters[k][b].tick(y);x[k]+=eqMix*(y-x[k]);}}
  if(compMix>.0001f){float v=p.external?std::abs(external):std::max(std::abs(x[0]),std::abs(x[channels-1]));env+=(v>env?atk:rel)*(v-env);float db=20*std::log10(std::max(1e-6f,env)),over=std::max(0.f,db-threshold);float slope=1-1/ratio;if(model==0||model==3)slope*=clamp(over/8,0,1);float goal=gain(-over*slope);compGain+=(goal<compGain?atk:rel)*(goal-compGain);for(int k=0;k<channels;++k)x[k]*=1+compMix*(compGain*makeup-1);}
  int mode=std::clamp(p.oversampling,0,3);bool runOS=mode>0&&p.power&&(p.saturation||satMix>.0001f);if(mode!=osMode||runOS!=osRunning){osMode=mode;osRunning=runOS;osFade=0;if(runOS)for(auto& c:os)c[mode-1].reset();}
  const float norm=std::max(1.f,std::sqrt(drive));auto shaping=[&](float v){return shape(v*drive,color)/norm;};
  for(int k=0;k<channels;++k){float v=dry[k][dryWrite];dry[k][dryWrite]=x[k];if(runOS){float wet=os[k][mode-1].tick(x[k],shaping);x[k]=v+satMix*osFade*(wet-v);}else if(mode==0&&satMix>.0001f)x[k]=v+satMix*(shaping(v)-v);else x[k]=v;}
  dryWrite=(dryWrite+1)%16;if(runOS)osFade=std::min(1.f,osFade+1/(sr*.025f));
  float e=0;for(int k=0;k<channels;++k)e+=x[k]*x[k]/channels;preEnergy+=energyAlpha*(e-preEnergy);
  abGain+=smooth*((p.power?p.comparisonGain:1.f)-abGain);for(int k=0;k<channels;++k)x[k]*=abGain;
  lookahead.tick(x[0],x[1],channels,p.power&&p.limiter,ceilGain,p.release);
  left=std::isfinite(x[0])?x[0]:0;if(channels>1)right=std::isfinite(x[1])?x[1]:0;
 }
};
// Approximate K-weighted gated integrated loudness; fixed histogram, no heap.
// Not a certified BS.1770 / EBU R128 measurement.
struct WeightFilter {
 double b0=1,b1=0,b2=0,a1=0,a2=0,z1=0,z2=0;
 void shelf(double sr){double k=std::tan(3.141592653589793*1681.974450955533/sr),vh=std::pow(10.,3.99984385397/20),vb=std::pow(vh,.499666774155),q=.707175236955,a=1+k/q+k*k;b0=(vh+vb*k/q+k*k)/a;b1=2*(k*k-vh)/a;b2=(vh-vb*k/q+k*k)/a;a1=2*(k*k-1)/a;a2=(1-k/q+k*k)/a;z1=z2=0;}
 void highpass(double sr){double k=std::tan(3.141592653589793*38.1354708760244/sr),q=.500327037323877,a=1+k/q+k*k;b0=1;b1=-2;b2=1;a1=2*(k*k-1)/a;a2=(1-k/q+k*k)/a;z1=z2=0;}
 double tick(double x){double y=b0*x+z1;z1=b1*x-a1*y+z2;z2=b2*x-a2*y;return y;}
};
struct LoudnessMeter {
 std::array<WeightFilter,2> shelf,hp;std::array<double,4> recent{};std::array<double,901> sums{};std::array<std::uint64_t,901> counts{};
 double energy=0;int samples=0,hop=4800,blocks=0,pos=0;float momentary=-100,integrated=-100;
 static float db(double e){return float(-.691+10*std::log10(std::max(1e-12,e)));}
 void reset(){recent.fill(0);sums.fill(0);counts.fill(0);energy=0;samples=blocks=pos=0;momentary=integrated=-100;for(auto& f:shelf)f.z1=f.z2=0;for(auto& f:hp)f.z1=f.z2=0;}
 void prepare(double sr){hop=std::max(1,int(std::round(sr*.1)));for(auto& f:shelf)f.shelf(sr);for(auto& f:hp)f.highpass(sr);reset();}
 void tick(float l,float r,int ch){float in[]={l,r};for(int k=0;k<ch;++k){double x=hp[k].tick(shelf[k].tick(in[k]));energy+=x*x;}if(++samples<hop)return;recent[pos]=energy/hop;pos=(pos+1)%4;samples=0;energy=0;++blocks;if(blocks<4)return;double e=0;for(double v:recent)e+=v*.25;momentary=db(e);if(momentary>=-70){int b=std::clamp(int(std::round((momentary+70)*10)),0,900);sums[b]+=e;++counts[b];}double total=0;std::uint64_t n=0;for(int i=0;i<=900;++i){total+=sums[i];n+=counts[i];}if(!n){integrated=-100;return;}float gate=std::max(-70.f,db(total/double(n))-10);total=0;n=0;for(int i=std::clamp(int(std::ceil((gate+70)*10)),0,900);i<=900;++i){total+=sums[i];n+=counts[i];}integrated=n?db(total/double(n)):-100;}
};
}
