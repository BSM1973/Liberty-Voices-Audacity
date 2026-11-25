#include "../Source/Mastering.h"
#include <iostream>
#include <chrono>
#include <cstdlib>
void check(bool b,const char* s){if(!b){std::cerr<<s<<'\n';std::exit(1);}}
int main(){using namespace voices;
 for(float sr:{44100.f,48000.f,96000.f}){
  for(int mode=0;mode<4;++mode){MasterEngine m;m.prepare(sr);MasterParams p;p.oversampling=mode;int at=-1;for(int i=0;i<m.latency()+20;++i){float l=i==0?1:0,r=l;m.tick(l,r,2,0,p);if(std::abs(l)>.9f)at=i;}check(at==m.latency(),"constant bypass latency");}
  MasterEngine m;m.prepare(sr);MasterParams p;p.limiter=true;p.ceiling=-1;Interpolation estimate;estimate.prepare(4);std::array<float,17> h{};float peak=0;
  for(int i=0;i<int(sr*.5f);++i){float l=1.2f*std::sin(2*pi*.24f*i+.7f),r=l;m.tick(l,r,2,0,p);for(int j=16;j>0;--j)h[j]=h[j-1];h[0]=l;if(i>m.latency()+32)for(int k=0;k<4;++k)peak=std::max(peak,std::abs(estimate.phase(h,k)));}check(peak<=gain(-1)+.01f,"reconstructed sine peak");std::cout<<"Interpolated peak "<<sr<<" "<<20*std::log10(peak)<<" dB\n";
  for(int mode=0;mode<4;++mode){m.prepare(sr);p=MasterParams{};p.oversampling=mode;p.saturation=true;p.compressor=true;p.limiter=true;p.dynamicEq=true;p.dynRange[0]=-4;double sum=0,max=0;int blocks=0;for(int i=0;i<int(sr)/128;++i){auto start=std::chrono::steady_clock::now();for(int j=0;j<128;++j){float l=.4f*std::sin(2*pi*200*(i*128+j)/sr),r=l;m.tick(l,r,2,0,p);check(std::isfinite(l)&&std::isfinite(r),"master finite");}double v=100*std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count()*sr/128;sum+=v;max=std::max(max,v);++blocks;}std::cout<<"MASTER_CPU "<<sr<<" mode="<<mode<<" mean="<<sum/blocks<<"% peak="<<max<<"%\n";}
 }
}
