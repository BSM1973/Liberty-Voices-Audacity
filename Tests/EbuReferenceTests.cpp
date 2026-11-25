#include "../Source/Mastering.h"
#include <iostream>
#include <iomanip>
#include <initializer_list>
#include <utility>
#include <cstdlib>
// EBU Tech 3341 (2023), Table 1, cases 1-5 (I only) and 15-19.
// Generated equivalent sine sequences; not official WAV-file validation.
int main(){using namespace voices;constexpr int sr=48000;constexpr double piD=3.14159265358979323846;int failures=0;std::cout<<std::fixed<<std::setprecision(4);
 auto loud=[&](int id,double expected,std::initializer_list<std::pair<double,double>> seq){LoudnessMeter m;m.prepare(sr);for(auto [seconds,db]:seq){float a=std::pow(10.,db/20);for(int i=0;i<std::lround(seconds*sr);++i){float x=a*std::sin(2*piD*1000*(i%48)/sr);m.tick(x,x,2);}}bool pass=std::abs(m.integrated-expected)<=.1;failures+=!pass;std::cout<<"EBU "<<id<<" I "<<m.integrated<<" target "<<expected<<" "<<(pass?"PASS":"FAIL")<<'\n';};
 loud(1,-23,{{20,-23}});loud(2,-33,{{20,-33}});loud(3,-23,{{10,-36},{60,-23},{10,-36}});loud(4,-23,{{10,-72},{10,-36},{60,-23},{10,-36},{10,-72}});loud(5,-23,{{20,-26},{20.1,-20},{20,-26}});
 auto peak=[&](int id,double f,double a,double phase,double expected){Interpolation p;p.prepare(4);std::array<float,17> h{};float max=0;for(int i=0;i<sr;++i){float fade=std::min({1.f,i/480.f,(sr-1-i)/480.f});float x=fade*a*std::sin(2*piD*f*i+phase*piD/180);for(int j=16;j>0;--j)h[j]=h[j-1];h[0]=x;for(int k=0;k<4;++k)max=std::max(max,std::abs(p.phase(h,k)));}double db=20*std::log10(max);bool pass=db>=expected-.4&&db<=expected+.2;failures+=!pass;std::cout<<"EBU "<<id<<" estimated peak "<<db<<" target "<<expected<<" "<<(pass?"PASS":"FAIL")<<'\n';};
 peak(15,.25,.5,0,-6);peak(16,.25,.5,45,-6);peak(17,1./6,.5,60,-6);peak(18,.125,.5,67.5,-6);peak(19,.25,1.41,45,3);return failures?1:0;}
