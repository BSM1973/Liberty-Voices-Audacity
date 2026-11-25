#pragma once
#include <array>
namespace voices {
struct Spec{const char* id;const char* en;const char* fr;float min,max,def,step;const char* unit;};
inline constexpr Spec specs[]={
{"input","Input gain","Gain d'entrée",-24,24,0,.1," dB"},{"output","Gain","Gain",-24,24,0,.1," dB"},{"mix","Mix","Mix",0,100,100,.1," %"},
{"speed","Speed","Vitesse",1,250,35,.1," ms"},{"human","Humanize","Humanisation",0,100,45,.1," %"},{"flex","Flex / Tolerance","Tolérance",0,50,8,.1," cents"},{"transition","Transition","Transition",0,250,35,.1," ms"},{"amount","Correction","Correction",0,100,100,.1," %"},{"formant","Formants","Formants",-12,12,0,.1,""},
{"drive","Drive","Drive",0,24,4,.1," dB"},{"satMix","Mix","Mix",0,100,25,.1," %"},{"hp","High pass","Coupe-bas",20,300,70,1," Hz"},{"body","Body","Corps",-12,12,0,.1," dB"},{"presence","Presence","Présence",-12,12,0,.1," dB"},{"air","Air","Air",-12,12,0,.1," dB"},
{"doubleAmount","Amount","Intensité",0,100,20,.1," %"},{"spread","Spread","Largeur",0,100,70,.1," %"},{"detune","Detune","Désaccord",0,30,7,.1," cents"},{"chorusRate","Rate","Vitesse",.05,5,.35,.01," Hz"},{"chorusDepth","Depth","Profondeur",0,10,3,.1," ms"},{"chorusMix","Mix","Mix",0,100,15,.1," %"},{"delayMs","Time","Temps",10,2000,280,1," ms"},{"feedback","Feedback","Feedback",0,90,22,.1," %"},{"delayMix","Mix","Mix",0,100,12,.1," %"},{"size","Size","Taille",0,1,.35,.01,""},{"decay","Decay","Déclin",0,.92,.42,.01,""},{"tone","Tone","Tonalité",1000,12000,6500,10," Hz"},{"reverbMix","Mix","Mix",0,100,12,.1," %"}};
inline constexpr const char* switches[]={"power","tune","sat","eq","doubler","chorus","delay","reverb","sync"};
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
}
