#include "../Source/Dsp.h"
#include "../Source/Parameters.h"
#include <set>
#include <string>
#include <iostream>
#include <cstdlib>
#include <chrono>
#include <memory>
void check(bool b,const char* s){if(!b){std::cerr<<s<<'\n';std::exit(1);}}
int main(){using namespace voices;std::cout<<"DSP QC: heap-backed engine fixtures, sizeof(Engine)="<<sizeof(Engine)<<", sizeof(FormantEngine)="<<sizeof(FormantEngine)<<std::endl;for(double sr:{44100.,48000.,96000.}){Detector d;d.prepare(sr);for(float f:{110.f,220.f,440.f,660.f}){for(int i=0;i<int(sr*.4);++i)d.push(.2f*std::sin(2*pi*f*i/sr));check(std::abs(d.hz-f)<f*.025f,"pitch detection");}for(int i=0;i<int(sr*.2);++i)d.push(0);check(d.hz==0,"silence");auto engineStorage=std::make_unique<Engine>();auto& e=*engineStorage;e.prepare(sr);Params p;p.sat=p.eq=p.doubler=p.chorus=p.delay=p.reverb=true;for(int i=0;i<int(sr*.5);++i){float x=.2f*std::sin(2*pi*225*i/sr),y=x;e.tick(&x,&y,2,p);check(std::isfinite(x)&&std::isfinite(y)&&std::abs(x)<10&&std::abs(y)<10,"finite chain");}check(e.detected>220&&e.detected<230,"engine detector");p.power=false;for(int i=0;i<int(sr*.1);++i){float x=.2f,y=-.3f;e.tick(&x,&y,2,p);check(std::isfinite(x)&&std::isfinite(y),"bypass finite");}}for(double sr:{44100.,48000.,96000.}){
 Detector onset;onset.prepare(sr);int first=-1;
 for(int i=0;i<int(sr*.08);++i){float x=.18f*std::sin(2*pi*110*i/sr)+.07f*std::sin(2*pi*220*i/sr)+.03f*std::sin(2*pi*330*i/sr);onset.push(x);if(first<0&&onset.hz>0)first=i;}
 check(first>=0&&first<sr*.065,"harmonic onset below 65 ms");check(std::abs(onset.hz-110)<2,"harmonic fundamental");
 for(int i=0;i<int(sr*.2);++i)onset.push(.1f);check(onset.hz==0,"DC rejection");
 auto engineStorage=std::make_unique<Engine>();auto& e=*engineStorage;e.prepare(sr);Params p;p.eq=false;p.character=1;p.speed=2;p.flex=0;p.human=0;p.transition=0;
 for(int i=0;i<int(sr*.3);++i){float x=.2f*std::sin(2*pi*225*i/sr),y=-x;e.tick(&x,&y,2,p);}
 check(std::abs(e.detected-225)<3,"anti-phase stereo fundamental");
 check(e.targetNote==57&&e.correction<-20&&e.correction>-60,"hard tune correction direction and size");
 p.scale=6;p.mask=0;
 for(int i=0;i<int(sr*.1);++i){float x=.2f*std::sin(2*pi*225*i/sr),y=x;e.tick(&x,&y,2,p);}
 check(e.targetNote==-1&&std::abs(e.correction)<1,"empty custom mask disables correction");
 p.tune=false;float x=.1f,y=x;e.tick(&x,&y,2,p);check(e.correction==0,"bypassed tune meter");
}for(double sr:{44100.,48000.,96000.}){
 auto neutralStorage=std::make_unique<FormantEngine>();auto& neutral=*neutralStorage;neutral.prepare(float(sr));float peak=0,error=0;int peakAt=-1;
 for(int i=0;i<neutral.n*4;++i){float x=i==neutral.n?1.f:0.f;float y=neutral.tick(x,x,0,1,0,false);if(std::abs(y)>peak){peak=std::abs(y);peakAt=i;}float expected=i==neutral.n*2?1.f:0.f;error=std::max(error,std::abs(y-expected));}
 check(peakAt==neutral.n*2&&error<.0001f,"formant unity reconstruction and latency");
 auto shiftStorage=std::make_unique<FormantEngine>();auto& shift=*shiftStorage;shift.prepare(float(sr));double lo0=0,hi0=0,lo1=0,hi1=0,loq0=0,hiq0=0,loq1=0,hiq1=0;
 for(int i=0;i<int(sr);++i){float x=0;for(int h=1;h<=32;++h){float freq=h*100.f;float z=(freq-800.f)/180.f;x+=(.012f+.11f*std::exp(-.5f*z*z))*std::sin(2*pi*freq*i/sr);}float y=shift.tick(x,x,7,1,100,true);check(std::isfinite(y)&&std::abs(y)<5,"formant finite");
 if(i>=int(sr*.5)){float a=2*pi*800*i/sr,b=2*pi*1200*i/sr;lo0+=x*std::cos(a);loq0+=x*std::sin(a);hi0+=x*std::cos(b);hiq0+=x*std::sin(b);lo1+=y*std::cos(a);loq1+=y*std::sin(a);hi1+=y*std::cos(b);hiq1+=y*std::sin(b);}}
 double before=std::hypot(hi0,hiq0)/std::max(1e-8,std::hypot(lo0,loq0));double after=std::hypot(hi1,hiq1)/std::max(1e-8,std::hypot(lo1,loq1));
 check(after>before*1.15,"formant shift changes harmonic envelope");
 std::cout<<"Formant: "<<sr<<" Hz, added latency "<<shift.n<<" samples, envelope ratio "<<before<<" -> "<<after<<'\n';
}for(double sr:{44100.,48000.,96000.})for(bool live:{false,true})for(int block:{32,64,128,256}){
 auto engineStorage=std::make_unique<Engine>();auto& e=*engineStorage;e.prepare(sr);e.setMode(live);Params p;p.eq=p.tune=false;p.mix=0;int impulse=4096,peakAt=-1;float peak=0;
 auto begin=std::chrono::steady_clock::now();
 for(int base=0;base<8192;base+=block)for(int j=0;j<block;++j){int i=base+j;float x=i==impulse?1.f:0.f,y=x;e.tick(&x,&y,2,p);if(std::abs(x)>peak){peak=std::abs(x);peakAt=i;}}
 check(peakAt-impulse==e.latency()&&std::abs(peak-1.f)<.0002f,"engine dry impulse latency");
 double ms=1000.*e.latency()/sr,elapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-begin).count();
 std::cout<<"Latency "<<(live?"Live":"Studio")<<" "<<sr<<" Hz block "<<block<<": "<<e.latency()<<" samples "<<ms<<" ms; dry benchmark "<<100.*elapsed/(8192./sr)<<"% realtime budget\n";
}
for(bool live:{false,true}){auto engineStorage=std::make_unique<Engine>();auto& e=*engineStorage;e.prepare(48000);e.setMode(live);Params p;p.tune=false;p.eq=false;p.sat=p.chorus=p.doubler=p.delay=p.reverb=false;float last=0,maxStep=0;
 for(int i=0;i<48000;++i){if(i==12000||i==24000){p.sat=!p.sat;p.eq=!p.eq;p.doubler=!p.doubler;p.chorus=!p.chorus;p.delay=!p.delay;p.reverb=!p.reverb;p.power=!p.power;}float x=.15f*std::sin(2*pi*220*i/48000),y=x;e.tick(&x,&y,2,p);check(std::isfinite(x)&&std::isfinite(y)&&std::abs(x)<3,"effect toggle finite");if(i>5000)maxStep=std::max(maxStep,std::abs(x-last));last=x;}check(maxStep<.3f,"effect transition bounded");
}// Voice feature tests using reproducible synthetic signals.
for(double rate:{44100.,48000.,96000.}){
 VocalIntelligence vi;vi.prepare(float(rate));
 for(int i=0;i<int(rate*.3);++i)vi.tick(.2f*std::sin(2*pi*220*i/rate),220,.95f);
 check(vi.kind>=4&&vi.protection<.05f&&vi.rangeLow>210,"voiced classification");
 for(int i=0;i<int(rate*.12);++i)vi.tick(.15f*std::sin(2*pi*6500*i/rate),220,.95f);
 check(vi.kind==2&&vi.protection>.9f,"sibilance protection with stale pitch estimate");
 for(int i=0;i<int(rate*.2);++i)vi.tick(0,0,0);
 check(vi.kind==0&&vi.protection>.9f,"silence protection");
}
std::set<std::string> uniqueNames;
for(int index=0;index<100;++index){
 check(uniqueNames.insert(names[index/10][index%10]).second,"unique preset name");
 Params p;factoryPreset(index,[&](const char* id,float v){std::string key=id;check(std::isfinite(v),"finite preset value");
 for(auto& spec:specs)if(key==spec.id)check(v>=spec.min-.0001f&&v<=spec.max+.0001f,"preset range");
 if(key=="input")p.input=v;
if(key=="output")p.output=v;
if(key=="mix")p.mix=v*.01f;
if(key=="speed")p.speed=v;
if(key=="human")p.human=v*.01f;
if(key=="flex")p.flex=v;
if(key=="transition")p.transition=v;
if(key=="amount")p.amount=v*.01f;
if(key=="formant")p.formant=v;
if(key=="drive")p.drive=v;
if(key=="satMix")p.satMix=v*.01f;
if(key=="hp")p.hp=v;
if(key=="body")p.body=v;
if(key=="presence")p.presence=v;
if(key=="air")p.air=v;
if(key=="doubleAmount")p.doubleAmount=v*.01f;
if(key=="spread")p.spread=v*.01f;
if(key=="detune")p.detune=v;
if(key=="chorusRate")p.chorusRate=v;
if(key=="chorusDepth")p.chorusDepth=v;
if(key=="chorusMix")p.chorusMix=v*.01f;
if(key=="delayMs")p.delayMs=v;
if(key=="feedback")p.feedback=v*.01f;
if(key=="delayMix")p.delayMix=v*.01f;
if(key=="size")p.size=v;
if(key=="decay")p.decay=v;
if(key=="tone")p.tone=v;
if(key=="reverbMix")p.reverbMix=v*.01f;
 if(key=="power")p.power=v>.5f;
if(key=="tune")p.tune=v>.5f;
if(key=="sat")p.sat=v>.5f;
if(key=="eq")p.eq=v>.5f;
if(key=="doubler")p.doubler=v>.5f;
if(key=="chorus")p.chorus=v>.5f;
if(key=="delay")p.delay=v>.5f;
if(key=="reverb")p.reverb=v>.5f;
 if(key=="character")p.character=int(v);
 if(key=="key")p.key=int(v);
 if(key=="scale")p.scale=int(v);
 if(key=="deess")p.deess=v>.5f;if(key=="deThreshold")p.deThreshold=v;if(key=="compress")p.compress=v>.5f;if(key=="compThreshold")p.compThreshold=v;if(key=="compRatio")p.compRatio=v;if(key=="plosive")p.plosive=v>.5f;if(key=="plosiveAmount")p.plosiveAmount=v*.01f;if(key=="multiband")p.multiband=v>.5f;if(key=="mbRatio")p.mbRatio=v;if(key=="bell")p.bell=v>.5f;if(key=="harmony")p.harmony=v>.5f;if(key=="diatonic")p.diatonic=v>.5f;if(key=="harmMix")p.harmMix=v*.01f;for(int i=0;i<4;++i){auto n=std::to_string(i+1);if(key=="voiceGain"+n)p.voiceGain[i]=v;if(key=="voicePan"+n)p.voicePan[i]=v;if(key=="voiceFormant"+n)p.voiceFormant[i]=v;}for(int i=0;i<5;++i){auto n=std::to_string(i+2);if(key=="eqHz"+n)p.eqHz[i]=v;if(key=="eqGain"+n)p.eqGain[i]=v;if(key=="eqQ"+n)p.eqQ[i]=v;}for(int i=0;i<3;++i)if(key=="mbThreshold"+std::to_string(i+1))p.mbThreshold[i]=v;
 });
 auto engineStorage=std::make_unique<Engine>();auto& e=*engineStorage;e.prepare(48000);double energy=0;float peak=0;
 for(int i=0;i<14400;++i){float x=.13f*std::sin(2*pi*225*i/48000)+.045f*std::sin(2*pi*450*i/48000),y=x;e.tick(&x,&y,2,p);check(std::isfinite(x)&&std::isfinite(y)&&std::abs(x)<4&&std::abs(y)<4,"preset render finite");energy+=x*x;peak=std::max(peak,std::abs(x));}
 check(energy>1e-5,"preset produces output");std::cout<<"Preset "<<index+1<<" "<<names[index/10][index%10]<<" peak="<<peak<<" RMS="<<std::sqrt(energy/14400)<<'\n';
}
for(bool live:{false,true})for(int block:{32,64,128,256}){
 auto engineStorage=std::make_unique<Engine>();auto& e=*engineStorage;e.prepare(48000);e.setMode(live);Params p;p.eq=p.sat=p.doubler=p.chorus=p.delay=p.reverb=true;p.formant=5;
 std::vector<double> timings;timings.reserve(48000/block);double total=0,peak=0;int frames=0;
 for(int start=0;start+block<=48000;start+=block){auto t0=std::chrono::steady_clock::now();for(int j=0;j<block;++j){int i=start+j;float x=.12f*std::sin(2*pi*225*i/48000)+.06f*std::sin(2*pi*450*i/48000),y=x;e.tick(&x,&y,2,p);check(std::isfinite(x)&&std::isfinite(y),"full chain benchmark finite");}double elapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-t0).count();timings.push_back(elapsed);total+=elapsed;peak=std::max(peak,elapsed);frames+=block;}
 std::sort(timings.begin(),timings.end());std::cout<<"FULL_LOAD "<<(live?"Live":"Studio")<<" block="<<block<<" mean="<<100*total/(frames/48000.)<<"% p99="<<100*timings[size_t(.99*(timings.size()-1))]/(block/48000.)<<"% max="<<100*peak/(block/48000.)<<"%\n";
}for(double sr:{44100.,48000.,96000.}){
 StudioStrip strip;strip.prepare(float(sr));float loud=0;for(int i=0;i<int(sr);++i)loud=strip.tick(.5f,0,-48,30,0,-24,6500,1,-24,4,0,0,-20);
 check(loud<.2f&&loud>.001f,"studio compressor attenuates");
 strip.prepare(float(sr));float quiet=0;for(int i=0;i<int(sr);++i)quiet=strip.tick(.001f,1,-40,30,0,-24,6500,0,-18,3,0,0,-20);
 check(quiet<.0001f,"studio expander attenuates");
 StudioBell bell;bell.configure(float(sr),2000,6,1);double energy=0;for(int i=0;i<int(sr);++i){float x=.1f*std::sin(2*pi*2000*i/sr),y=bell.tick(x);if(i>sr*.5)energy+=y*y;}
 check(energy/(sr*.5)>.015,"parametric bell boosts centre");
 auto engineStorage=std::make_unique<Engine>();auto& e=*engineStorage;e.prepare(sr);Params p;p.gate=p.deess=p.compress=p.autolevel=p.harmony=p.bell=true;p.bellGain=4;
 for(int i=0;i<int(sr*.5);++i){float x=.15f*std::sin(2*pi*225*i/sr),y=x;e.tick(&x,&y,2,p);check(std::isfinite(x)&&std::isfinite(y)&&std::abs(x)<4&&std::abs(y)<4,"studio chain finite");}
}check(harmonyTarget(62,4,0,1,4095)==65,"diatonic D to F in C major");check(harmonyTarget(60,7,0,1,4095)==67,"diatonic fifth");check(harmonyTarget(71,7,0,1,4095)==77,"diminished fifth in C major");check(harmonyTarget(64,-4,0,1,4095)==60,"descending third");for(double sr:{44100.,48000.,96000.}){MultibandVocal m;m.prepare(float(sr));std::array<float,3> th{{-24,-24,-24}};double input=0,output=0;for(int i=0;i<int(sr);++i){float x=.5f*std::sin(2*pi*500*i/sr),y=m.tick(x,1,th,4,0,0);check(std::isfinite(y),"multiband finite");if(i>sr*.5){input+=x*x;output+=y*y;}}check(output<input*.75,"multiband attenuates");auto engineStorage=std::make_unique<Engine>();auto& e=*engineStorage;e.prepare(sr);Params p;p.harmony=p.diatonic=p.bell=p.multiband=p.plosive=true;p.scale=1;p.voiceFormant[0]=3;p.eqGain[2]=4;for(int i=0;i<int(sr*.4);++i){float x=.15f*std::sin(2*pi*220*i/sr),y=x;e.tick(&x,&y,2,p);check(std::isfinite(x)&&std::isfinite(y)&&std::abs(x)<4&&std::abs(y)<4,"v07 finite");}}check(noteAllowed(60,0,1,4095)&&!noteAllowed(61,0,1,4095),"scale membership");check(nearest(61,0,1,4095)!=61,"C major");check(nearest(61,0,6,1)==60,"custom mask");std::cout<<"DSP tests passed: 3 sample rates, 12 tone detections, silence, finite chain, bypass, scales, harmonic onset, DC, stereo phase, correction and empty mask\n";}
