#include "../Source/Dsp.h"
#include <iostream>
#include <cstdlib>
int main(){using namespace voices;for(float sr:{44100.f,48000.f,96000.f}){for(int voice=0;voice<5;++voice){FormantEngine f;f.prepare(sr);f.count=voice*f.hop/5;float worst=0;std::vector<float> input(size_t(f.n*8));for(int i=0;i<int(input.size());++i){float x=.2f*std::sin(2*pi*220*i/sr);input[size_t(i)]=x;float y=f.tick(x,x,0,1,220,false);if(i>=2*f.n)worst=std::max(worst,std::abs(y-input[size_t(i-f.n)]));}if(worst>.0002f){std::cerr<<"neutral reconstruction "<<sr<<" "<<voice<<" "<<worst<<"\n";return 1;}std::cout<<"SPECTRAL_NEUTRAL "<<sr<<" phase="<<voice<<" error="<<worst<<"\n";}}}
