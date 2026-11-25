#pragma once
#include <array>
#include <algorithm>
#include <cstring>
namespace voices {
struct Spec{const char* id;const char* en;const char* fr;float min,max,def,step;const char* unit;};
inline constexpr Spec specs[]={
{"input","Input gain","Gain d'entrée",-24,24,0,.1," dB"},{"output","Gain","Gain",-24,24,0,.1," dB"},{"mix","Mix","Mix",0,100,100,.1," %"},
{"speed","Speed","Vitesse",1,250,35,.1," ms"},{"human","Humanize","Humanisation",0,100,45,.1," %"},{"flex","Flex / Tolerance","Tolérance",0,50,8,.1," cents"},{"transition","Transition","Transition",0,250,35,.1," ms"},{"amount","Correction","Correction",0,100,100,.1," %"},{"formant","Formants","Formants",-12,12,0,.1,""},
{"drive","Drive","Drive",0,24,4,.1," dB"},{"satMix","Mix","Mix",0,100,25,.1," %"},{"hp","High pass","Coupe-bas",20,300,70,1," Hz"},{"body","Body","Corps",-12,12,0,.1," dB"},{"presence","Presence","Présence",-12,12,0,.1," dB"},{"air","Air","Air",-12,12,0,.1," dB"},
{"doubleAmount","Amount","Intensité",0,100,20,.1," %"},{"spread","Spread","Largeur",0,100,70,.1," %"},{"detune","Detune","Désaccord",0,30,7,.1," cents"},{"chorusRate","Rate","Vitesse",.05,5,.35,.01," Hz"},{"chorusDepth","Depth","Profondeur",0,10,3,.1," ms"},{"chorusMix","Mix","Mix",0,100,15,.1," %"},{"delayMs","Time","Temps",10,2000,280,1," ms"},{"feedback","Feedback","Feedback",0,90,22,.1," %"},{"delayMix","Mix","Mix",0,100,12,.1," %"},{"size","Size","Taille",0,1,.35,.01,""},{"decay","Decay","Déclin",0,.92,.42,.01,""},{"tone","Tone","Tonalité",1000,12000,6500,10," Hz"},{"reverbMix","Mix","Mix",0,100,12,.1," %"},
{"gateThreshold","Threshold","Seuil",-80,0,-48,.1," dB"},{"gateRange","Range","Atténuation",0,60,30,.1," dB"},
{"deThreshold","Threshold","Seuil",-60,0,-24,.1," dB"},{"deHz","Frequency","Fréquence",2000,10000,6500,10," Hz"},
{"compThreshold","Threshold","Seuil",-60,0,-18,.1," dB"},{"compRatio","Ratio","Ratio",1,12,3,.1,":1"},{"makeup","Makeup","Gain",0,18,0,.1," dB"},{"autoTarget","Target","Cible",-36,-8,-20,.1," dB"},
{"interval1","Voice 1","Voix 1",-12,12,4,1," st"},{"interval2","Voice 2","Voix 2",-12,12,7,1," st"},{"interval3","Voice 3","Voix 3",-12,12,12,1," st"},{"interval4","Voice 4","Voix 4",-12,12,-12,1," st"},{"harmMix","Harmony mix","Harmonies",0,100,25,.1," %"},
{"bellHz","Frequency","Fréquence",80,12000,2500,1," Hz"},{"bellGain","Gain","Gain",-18,18,0,.1," dB"},{"bellQ","Q","Q",.2,8,1,.01,""},
{"voiceGain1","Level 1","Volume 1",0,100,100,0.1," %"},
{"voicePan1","Pan 1","Pan 1",-100,100,0,0.1," %"},
{"voiceFormant1","Formant 1","Formant 1",-12,12,0,0.1," st"},
{"voiceGain2","Level 2","Volume 2",0,100,100,0.1," %"},
{"voicePan2","Pan 2","Pan 2",-100,100,0,0.1," %"},
{"voiceFormant2","Formant 2","Formant 2",-12,12,0,0.1," st"},
{"voiceGain3","Level 3","Volume 3",0,100,100,0.1," %"},
{"voicePan3","Pan 3","Pan 3",-100,100,0,0.1," %"},
{"voiceFormant3","Formant 3","Formant 3",-12,12,0,0.1," st"},
{"voiceGain4","Level 4","Volume 4",0,100,100,0.1," %"},
{"voicePan4","Pan 4","Pan 4",-100,100,0,0.1," %"},
{"voiceFormant4","Formant 4","Formant 4",-12,12,0,0.1," st"},
{"eqHz2","Freq 2","Fréq 2",80,12000,120,1," Hz"},
{"eqGain2","Gain 2","Gain 2",-18,18,0,0.1," dB"},
{"eqQ2","Q 2","Q 2",0.2,8,1,0.01,""},
{"eqHz3","Freq 3","Fréq 3",80,12000,400,1," Hz"},
{"eqGain3","Gain 3","Gain 3",-18,18,0,0.1," dB"},
{"eqQ3","Q 3","Q 3",0.2,8,1,0.01,""},
{"eqHz4","Freq 4","Fréq 4",80,12000,1000,1," Hz"},
{"eqGain4","Gain 4","Gain 4",-18,18,0,0.1," dB"},
{"eqQ4","Q 4","Q 4",0.2,8,1,0.01,""},
{"eqHz5","Freq 5","Fréq 5",80,12000,5000,1," Hz"},
{"eqGain5","Gain 5","Gain 5",-18,18,0,0.1," dB"},
{"eqQ5","Q 5","Q 5",0.2,8,1,0.01,""},
{"eqHz6","Freq 6","Fréq 6",80,12000,10000,1," Hz"},
{"eqGain6","Gain 6","Gain 6",-18,18,0,0.1," dB"},
{"eqQ6","Q 6","Q 6",0.2,8,1,0.01,""},
{"mbThreshold1","Low","Graves",-60,0,-18,0.1," dB"},
{"mbThreshold2","Mid","Médiums",-60,0,-18,0.1," dB"},
{"mbThreshold3","High","Aigus",-60,0,-18,0.1," dB"},
{"mbRatio","Ratio","Ratio",1,12,3,0.1,":1"},
{"plosiveAmount","Reduction","Réduction",0,100,65,0.1," %"}};
inline constexpr const char* switches[]={"power","tune","sat","eq","doubler","chorus","delay","reverb","sync","gate","deess","compress","autolevel","harmony","bell","harmonyMidi","diatonic","multiband","plosive"};
inline constexpr const char* notes[]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
inline constexpr const char* categories[]={"Natural Correction","Pop Lead","Modern Pop","R&B","Rap and Hip-Hop","Rock Vocals","Electronic and Robotic","Harmonies and Doubles","Voice-Over","Creative Effects"};
inline constexpr const char* names[10][10]={
{"Invisible Touch","Gentle Lead","Studio Clean","Soft Ballad","Acoustic Intimacy","Vibrato Keeper","Subtle Tenor","Warm Alto","Slow Correction","Transparent Voice"},
{"Radio Lead","Bright Hook","Pop Clarity","Velvet Verse","Wide Chorus Lead","Shining Vocal","Tight Lead","Polished Ballad","Front and Centre","Arena Pop"},
{"Modern Shine","Hyper Pop Lead","Glass Top","Silky Topline","Stereo Dream","Urban Pop","Midnight Hook","Neon Lead","Airy Anthem","Digital Polish"},
{"Velvet Soul","Smooth Runs","Silk Harmony","Late Night R&B","Soul Glow","Warm Falsetto","Dreamy Adlibs","Soft Pocket","Honey Lead","Deep Velvet"},
{"Trap Tune","Tight Rap","Dry Bars","Hook Machine","Melodic Flow","Street Lead","Dark Adlibs","Wide Trap","Clean Drill","Fast Lock"},
{"Rock Edge","Grit Lead","Arena Voice","Indie Warmth","Alternative Lead","Power Chorus","Vintage Rock","Hard Edge","Soft Rock","Live Rock"},
{"Robot Lock","Neon Steps","Cyber Vocal","Digital Snap","Laser Lead","Hard Grid","Synth Voice","Chrome Robot","Electro Hook","Machine Melody"},
{"Double Track","Wide Double","Soft Unison","Stereo Layers","Chorus Stack","Octave Colour","Silk Double","Room Doubles","Wide Adlibs","Dream Stack"},
{"Narrator Clean","Podcast Warm","Broadcast Clear","Dialogue Polish","Documentary Voice","Soft Spoken","Presence Boost","Voice Actor","Interview Clean","Dry Narration"},
{"Copper Echo","Dream Tunnel","Ghost Voice","Space Drift","Broken Robot","Shimmer Colour","Wide Hall","Dark Chamber","Rhythmic Trail","Alien Radio"}};

// Shared factory values, applied by the real host loader and portable tests.
template<class Setter> void factoryPreset(int index,Setter set){
 int preset=std::clamp(index,0,99),c=preset/10,j=preset%10;
 for(auto& s:specs)set(s.id,s.def);
 for(auto id:switches)set(id,std::strcmp(id,"power")==0||std::strcmp(id,"tune")==0||std::strcmp(id,"eq")==0);
 set("key",0);set("scale",0);set("character",c==6||(c==4&&j<3));
 for(int k=0;k<12;++k){char id[8]={'n','o','t','e',0,0,0,0};if(k<10)id[4]=char('0'+k);else{id[4]='1';id[5]=char('0'+k-10);}set(id,1);}
 const float speeds[]={65,30,15,45,8,40,2,28,90,12};set("speed",speeds[c]+(c==6?.2f:2.f)*j);set("human",c==6?0:std::min(95,25+c*3+j*4));set("amount",c==8?0:c==0?55+j*4:100);set("flex",c==6?0:3+j*2);set("hp",60+c*5+j*3);set("presence",float(j%5)*.7f);set("air",float((c+j)%5)*.8f);set("sat",c==5||c==4||c==9&&j%2);set("drive",3+j*1.2f);set("doubler",c==7||c==2&&j%2);set("doubleAmount",15+j*6);set("chorus",c==7&&j>=5||c==9&&j%2==0);set("chorusMix",10+j*4);set("delay",c==9||c==4&&j>=4);set("delayMs",110+j*60);set("delayMix",10+j*3);set("reverb",c==1||c==3||c==7||c==9);set("reverbMix",7+j*3);set("size",.2f+j*.07f);set("decay",.25f+j*.06f);
 set("deess",c!=6&&c!=9);set("deThreshold",-26+j*.5f);set("compress",c!=0&&c!=6);set("compThreshold",-22+j*.6f);set("compRatio",2+j*.2f);
 set("plosive",c==8||c==0);set("plosiveAmount",35+j*3);set("multiband",c==1||c==4||c==5);set("mbRatio",2+j*.1f);set("mbThreshold1",-20+j*.4f);
 set("bell",c!=6);set("eqGain2",c==8?-2.f:-.5f);set("eqGain5",c==1?1+j*.2f:.5f);
 if(c==7){set("harmony",1);set("diatonic",1);set("scale",j%2?2:1);set("harmMix",15+j*3);set("voicePan1",-65);set("voicePan2",65);set("voicePan3",-30);set("voicePan4",30);set("voiceGain3",55);set("voiceGain4",45);}
}
}
