#pragma once
#include "Dsp.h"
#include <cstdint>
namespace voices {
// Finite windowed-sinc interpolator. All allocation/precomputation occurs at prepare.
struct Interpolation {
 static constexpr int radius=8,taps=17;
 std::array<std::array<float,taps>,8> h{};
 void prepare(int factor){for(int p=0;p<factor;++p){double total=0;for(int j=0;j<taps;++j){double x=double(j-radius)+double(p)/factor;double w=std::abs(x)<=radius?.42+.5*std::cos(pi*x/radius)+.08*std::cos(2*pi*x/radius):0;double v=std::abs(x)<1e-10?1:std::sin(pi*x)/(pi*x);h[p][j]=float(v*w);total+=h[p][j];}for(float& c:h[p])c/=float(total);}}
 float phase(const std::array<float,taps>& x,int p)const{float y=0;for(int j=0;j<taps;++j)y+=x[j]*h[p][j];return y;}
};
struct SaturationOS {
 Interpolation interpolation;std::array<float,17> history{};std::array<float,129> high{},lowpass{};int factor=1,write=0,length=1;int warmed=0;
 void prepare(int f){factor=f;interpolation.prepare(f);length=16*f+1;double total=0;for(int j=0;j<length;++j){double x=double(j-8*f)/f;double w=.42+.5*std::cos(pi*x/8)+.08*std::cos(2*pi*x/8);double t=.9*x;double v=std::abs(t)<1e-10?1:std::sin(pi*t)/(pi*t);lowpass[j]=float(w*v);total+=lowpass[j];}for(int j=0;j<length;++j)lowpass[j]/=float(total);reset();}
 void reset(){history.fill(0);high.fill(0);write=warmed=0;}
 template<class Shape> float tick(float x,Shape shape){for(int j=16;j>0;--j)history[j]=history[j-1];history[0]=x;for(int p=0;p<factor;++p){high[write]=shape(interpolation.phase(history,p));write=(write+1)%length;}float y=0; // phase 0 of this input block, not its final high-rate sample
 int index=(write-factor+length)%length;for(int j=0;j<length;++j){y+=high[index]*lowpass[j];if(--index<0)index=length-1;}++warmed;return y;}
};
// Stereo-linked anticipatory limiter. Detector is a 4x interpolated peak
// estimate; a 0.5 dB margin is reserved for reconstruction error.
struct PeakLookahead {
 Interpolation interp;std::array<std::array<float,17>,2> hist{};std::array<std::vector<float>,2> delay;
 std::vector<float> queueValue;std::vector<std::uint64_t> queueTime;
 int look=240,latency=248,write=0,head=0,tail=0,capacity=256;std::uint64_t time=0;float g=1,releaseAlpha=0,sr=48000,lastRelease=-1;
 void prepare(float rate){sr=rate;look=std::max(16,int(std::round(.005f*sr)));latency=look+8;capacity=look+3;for(auto& d:delay)d.assign(size_t(latency),0);queueValue.assign(size_t(capacity),0);queueTime.assign(size_t(capacity),0);interp.prepare(4);hist={};write=head=tail=0;time=0;g=1;lastRelease=-1;}
 void tick(float& l,float& r,int ch,bool on,float ceiling,float release){
  if(release!=lastRelease){releaseAlpha=1-std::exp(-1000/(sr*std::max(5.f,release)));lastRelease=release;}
  float x[]={l,ch>1?r:l};float peak=0;for(int k=0;k<ch;++k){for(int j=16;j>0;--j)hist[k][j]=hist[k][j-1];hist[k][0]=x[k];for(int p=0;p<4;++p)peak=std::max(peak,std::abs(interp.phase(hist[k],p)));}
  while(head!=tail&&queueTime[head]+std::uint64_t(look)<time)head=(head+1)%capacity;
  while(head!=tail){int prev=(tail+capacity-1)%capacity;if(queueValue[prev]>peak)break;tail=prev;}
  queueValue[tail]=peak;queueTime[tail]=time++;tail=(tail+1)%capacity;
  float goal=on?std::min(1.f,ceiling*.9440609f/std::max(1e-9f,queueValue[head])):1.f;
  g=goal<g?goal:g+releaseAlpha*(goal-g);
  for(int k=0;k<ch;++k){float y=delay[k][write];delay[k][write]=x[k];if(k==0)l=y*(on?g:1);else r=y*(on?g:1);}write=(write+1)%latency;
 }
};
}
