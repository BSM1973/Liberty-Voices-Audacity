#include "../Source/Plugin.h"
#include "../Source/Editor.h"
#include <iostream>
int main(int argc,char** argv){
 juce::ScopedJuceInitialiser_GUI gui;
 VoicesProcessor p;p.prepareToPlay(48000,128);
 juce::AudioBuffer<float> b(2,128);b.clear();juce::MidiBuffer midi;
 p.learn=0;midi.addEvent(juce::MidiMessage::controllerEvent(1,21,127),0);p.processBlock(b,midi);
 if(p.cc[0].load()!=21||std::abs(p.value("input")-24.f)>.01f){std::cerr<<"MIDI Learn failed\n";return 1;}
 juce::MemoryBlock state;p.getStateInformation(state);p.cc[0]=-1;p.set("input",0);p.setStateInformation(state.getData(),int(state.getSize()));if(p.cc[0].load()!=21||std::abs(p.value("input")-24.f)>.01f){std::cerr<<"State recall failed\n";return 1;}
 p.cc[0]=-1;p.load(0);
 auto editor=std::make_unique<VoicesEditor>(p);juce::File dir=argc>1?juce::File::getCurrentWorkingDirectory().getChildFile(juce::String::fromUTF8(argv[1])):juce::File::getCurrentWorkingDirectory().getChildFile("ui-qc");dir.createDirectory();
 juce::StringArray issues;for(bool fr:{true,false})for(int pg=0;pg<7;++pg){juce::StringArray local;auto stem=juce::String(fr?"FR":"EN")+"-"+juce::String(pg+1);if(!editor->qualitySnapshot(pg,fr,false,dir.getChildFile(stem+".png"),local))issues.add(stem+" image failed");for(auto& v:local)issues.add(stem+" "+v);}for(bool fr:{true,false}){juce::StringArray local;auto stem=juce::String(fr?"FR":"EN")+"-browser";if(!editor->qualitySnapshot(0,fr,true,dir.getChildFile(stem+".png"),local))issues.add(stem+" image failed");for(auto& v:local)issues.add(stem+" "+v);}
 for(auto& issue:issues)std::cerr<<issue.toStdString()<<"\n";std::cout<<"UI_QC issues="<<issues.size()<<" MIDI learn and state recall PASS\n";return issues.isEmpty()?0:1;
}
