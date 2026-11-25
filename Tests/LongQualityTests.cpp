#include "../Source/Mastering.h"
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <memory>
#include <iomanip>
namespace voices {
struct BaselineDetector {
 static constexpr int frameSize=512,hop=64;
 std::array<float,frameSize> ring{},frame{}; int pos=0,filled=0,count=0,decim=1,phase=0; float sr=12000,hz=0,confidence=0,lp=0,alpha=.3f;
 void prepare(double sampleRate){*this=BaselineDetector{};decim=std::max(1,int(std::round(sampleRate/12000)));sr=float(sampleRate/decim);alpha=1-std::exp(-2*pi*std::min(2500.f,sr*.2f)/float(sampleRate));}
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

}
void require(bool condition,const char* label){if(!condition){std::cerr<<label<<'\n';std::exit(1);}}
int main(){using namespace voices;
 for(double sr:{44100.,48000.,96000.,192000.}){Detector d;BaselineDetector old;d.prepare(sr);old.prepare(sr);uint32_t seed=1;int count=0;for(int i=0;i<int(sr*2);++i){seed=1664525u*seed+1013904223u;float noise=float(seed>>8)/16777216.f-.5f;float f=i<sr?110.f:330.f;float x=i<sr*.2?0:.16f*std::sin(2*pi*f*i/sr)+.025f*noise;d.push(x);old.push(x);require(d.hz==old.hz&&d.confidence==old.confidence,"detector exact parity");++count;}std::cout<<"DETECTOR_PARITY "<<sr<<" samples="<<count<<" PASS\n";}
 for(bool live:{false,true})for(int block:{32,64,128,256}){constexpr int sr=48000,seconds=10;auto e=std::make_unique<Engine>();e->prepare(sr);e->setMode(live);MasterEngine m;m.prepare(sr);Params p;p.harmony=p.diatonic=p.bell=p.gate=p.deess=p.compress=p.autolevel=p.multiband=p.plosive=true;p.sat=p.eq=p.doubler=p.chorus=p.delay=p.reverb=true;p.formant=5;p.voiceFormant={{2,-3,4,-2}};p.scale=1;MasterParams mp;mp.compressor=mp.saturation=mp.limiter=mp.dynamicEq=true;mp.oversampling=3;mp.dynRange.fill(-3);LoudnessMeter meter;meter.prepare(sr);std::vector<double> times;times.reserve(sr*seconds/block);int misses=0;float peak=0;double sum=0;uint32_t seed=12345;double phase=0;
 for(int start=0;start+block<=sr*seconds;start+=block){auto t0=std::chrono::steady_clock::now();for(int j=0;j<block;++j){int i=start+j;seed=1664525u*seed+1013904223u;float noise=float(seed>>8)/16777216.f-.5f;float time=float(i)/sr;phase+=2*pi*(180+30*std::sin(time*.7))/sr;float a=.13f*std::sin(phase)+.05f*std::sin(2*phase)+.018f*noise;float l=a,r=a*.9f;e->tick(&l,&r,2,p);m.tick(l,r,2,0,mp);meter.tick(l,r,2);require(std::isfinite(l)&&std::isfinite(r)&&std::abs(l)<4&&std::abs(r)<4,"combined full-chain finite");peak=std::max({peak,std::abs(l),std::abs(r)});}double pct=100*std::chrono::duration<double>(std::chrono::steady_clock::now()-t0).count()*sr/block;times.push_back(pct);sum+=pct;if(pct>100)++misses;}
 std::sort(times.begin(),times.end());std::cout<<"COMBINED_QC "<<(live?"Live":"Studio")<<" rate="<<sr<<" buffer="<<block<<" duration="<<seconds<<"s mean="<<sum/times.size()<<"% p99="<<times[size_t(.99*(times.size()-1))]<<"% max="<<times.back()<<"% missed="<<misses<<"/"<<times.size()<<" outputPeak="<<peak<<" LUFS_est="<<meter.integrated<<'\n';}
}
