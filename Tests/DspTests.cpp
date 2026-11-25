#include "../Source/Dsp.h"
#include <iostream>
#include <cstdlib>
void check(bool b,const char* s){if(!b){std::cerr<<s<<'\n';std::exit(1);}}
int main(){using namespace voices;for(double sr:{44100.,48000.,96000.}){Detector d;d.prepare(sr);for(float f:{110.f,220.f,440.f,660.f}){for(int i=0;i<int(sr*.4);++i)d.push(.2f*std::sin(2*pi*f*i/sr));check(std::abs(d.hz-f)<f*.025f,"pitch detection");}for(int i=0;i<int(sr*.2);++i)d.push(0);check(d.hz==0,"silence");Engine e;e.prepare(sr);Params p;p.sat=p.eq=p.doubler=p.chorus=p.delay=p.reverb=true;for(int i=0;i<int(sr*.5);++i){float x=.2f*std::sin(2*pi*225*i/sr),y=x;e.tick(&x,&y,2,p);check(std::isfinite(x)&&std::isfinite(y)&&std::abs(x)<10&&std::abs(y)<10,"finite chain");}check(e.detected>220&&e.detected<230,"engine detector");p.power=false;float x=.2f,y=-.3f;e.tick(&x,&y,2,p);check(x==.2f&&y==-.3f,"bypass");}for(double sr:{44100.,48000.,96000.}){
 Detector onset;onset.prepare(sr);int first=-1;
 for(int i=0;i<int(sr*.08);++i){float x=.18f*std::sin(2*pi*110*i/sr)+.07f*std::sin(2*pi*220*i/sr)+.03f*std::sin(2*pi*330*i/sr);onset.push(x);if(first<0&&onset.hz>0)first=i;}
 check(first>=0&&first<sr*.065,"harmonic onset below 65 ms");check(std::abs(onset.hz-110)<2,"harmonic fundamental");
 for(int i=0;i<int(sr*.2);++i)onset.push(.1f);check(onset.hz==0,"DC rejection");
 Engine e;e.prepare(sr);Params p;p.eq=false;p.character=1;p.speed=2;p.flex=0;p.human=0;p.transition=0;
 for(int i=0;i<int(sr*.3);++i){float x=.2f*std::sin(2*pi*225*i/sr),y=-x;e.tick(&x,&y,2,p);}
 check(std::abs(e.detected-225)<3,"anti-phase stereo fundamental");
 check(e.targetNote==57&&e.correction<-20&&e.correction>-60,"hard tune correction direction and size");
 p.scale=6;p.mask=0;
 for(int i=0;i<int(sr*.1);++i){float x=.2f*std::sin(2*pi*225*i/sr),y=x;e.tick(&x,&y,2,p);}
 check(e.targetNote==-1&&std::abs(e.correction)<1,"empty custom mask disables correction");
 p.tune=false;float x=.1f,y=x;e.tick(&x,&y,2,p);check(e.correction==0,"bypassed tune meter");
}check(noteAllowed(60,0,1,4095)&&!noteAllowed(61,0,1,4095),"scale membership");check(nearest(61,0,1,4095)!=61,"C major");check(nearest(61,0,6,1)==60,"custom mask");std::cout<<"DSP tests passed: 3 sample rates, 12 tone detections, silence, finite chain, bypass, scales, harmonic onset, DC, stereo phase, correction and empty mask\n";}
