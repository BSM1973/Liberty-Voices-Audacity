#include "../Source/Mastering.h"
#include <iostream>
#include <cstdlib>
void check(bool b,const char* label){if(!b){std::cerr<<label<<'\n';std::exit(1);}}
int main(){using namespace voices;for(float sr:{44100.f,48000.f,96000.f}){
 MasterEngine m;m.prepare(sr);MasterParams p;float l=.7f,r=-.5f;m.tick(l,r,2,0,p);check(l==0&&r==0,"master startup delay");for(int i=0;i<m.latency();++i){l=.7f;r=-.5f;m.tick(l,r,2,0,p);}check(l==.7f&&r==-.5f,"master delayed unity");
 for(int model=0;model<4;++model){m.prepare(sr);p=MasterParams{};p.compressor=true;p.model=model;p.threshold=-24;p.ratio=4;for(int i=0;i<int(sr*2);++i){l=.5f;r=.25f;m.tick(l,r,2,0,p);}check(l<.25f&&l>.01f&&std::abs(l-2*r)<1e-5f,"linked model attenuation");}
 m.prepare(sr);p=MasterParams{};p.limiter=true;p.ceiling=-3;for(int i=0;i<int(sr);++i){l=2;r=.5f;m.tick(l,r,2,0,p);}check(std::abs(l)<=gain(-3)+1e-4f&&std::abs(l-4*r)<1e-5f,"sample peak ceiling and link");
 m.prepare(sr);p=MasterParams{};p.compressor=true;p.external=true;p.threshold=-24;for(int i=0;i<int(sr*2);++i){l=.1f;r=.1f;m.tick(l,r,2,.8f,p);}check(l<.07f,"external detector");
 m.prepare(sr);p=MasterParams{};p.dynamicEq=true;p.dynRange[0]=-9;p.dynThreshold[0]=-40;double in=0,out=0;for(int i=0;i<int(sr);++i){l=.4f*std::sin(2*pi*2500*i/sr);r=l;float x=l;m.tick(l,r,2,0,p);if(i>sr*.5){in+=x*x;out+=l*l;}}check(out<in*.65,"dynamic EQ attenuates");
 LoudnessMeter meter;meter.prepare(sr);for(int i=0;i<int(sr*2);++i){float x=.1f*std::sin(2*pi*1000*i/sr);meter.tick(x,x,2);}check(meter.integrated>-23&&meter.integrated<-17,"approximate loudness for stereo tone");meter.reset();check(meter.integrated==-100,"meter reset");
 for(int color=0;color<3;++color){m.prepare(sr);p=MasterParams{};p.saturation=true;p.color=color;p.drive=12;for(int i=0;i<int(sr*.2);++i){l=.3f*std::sin(2*pi*220*i/sr);r=l;m.tick(l,r,2,0,p);check(std::isfinite(l)&&std::isfinite(r),"saturation finite");}}
 }std::cout<<"Master tests passed (synthetic)\n";}
