#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>
#include "Dsp.h"
#include "Parameters.h"
class VoicesProcessor final:public juce::AudioProcessor,private juce::Timer {
public:
 using AP=juce::AudioProcessorValueTreeState;
 VoicesProcessor(); ~VoicesProcessor() override{stopTimer();} static AP::ParameterLayout layout();
 AP state{*this,nullptr,"VOICES_STATE",layout()};
 voices::Engine engine;
 std::array<std::atomic<int>,std::size(voices::specs)> cc;std::atomic<int> learn{-1};
 std::atomic<float> detected{0},target{0},cents{0},vocalConfidence{0},vocalProtection{0},rangeLow{0},rangeHigh{0};std::atomic<int> vocalKind{0};std::array<std::atomic<float>,2> inPeak{},outPeak{};std::atomic<bool> inClip{false},outClip{false};
 std::atomic<bool> spectrumVisible{false};std::array<std::atomic<float>,96> spectrum{};
 std::array<bool,100> favorites{};int preset=0;bool french=true;
 void prepareToPlay(double,int)override;void releaseResources()override{};void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&)override;
 bool isBusesLayoutSupported(const BusesLayout&)const override;
 const juce::String getName()const override{return "Voices Audacity";} bool acceptsMidi()const override{return true;}bool producesMidi()const override{return false;}bool isMidiEffect()const override{return false;}double getTailLengthSeconds()const override{return 8;}
 bool hasEditor()const override{return true;}juce::AudioProcessorEditor* createEditor()override;
 int getNumPrograms()override{return 100;}int getCurrentProgram()override{return preset;}void setCurrentProgram(int i)override{load(i);}const juce::String getProgramName(int i)override{return i>=0&&i<100?voices::names[i/10][i%10]:"";}void changeProgramName(int,const juce::String&)override{}
 void getStateInformation(juce::MemoryBlock&)override;void setStateInformation(const void*,int)override;
 float value(const char* id)const{return state.getRawParameterValue(id)->load();}void set(const char* id,float v);void load(int);juce::File presetDir()const;
public:std::atomic<bool> activeLive{false},transportPlaying{false};std::atomic<int> latencyStudio{0},latencyLive{0};
private:static constexpr int fftSize=4096;juce::dsp::FFT spectrumFFT{12};std::array<float,fftSize*2> spectrumData{};std::array<float,fftSize> spectrumWindow{};std::array<int,97> spectrumEdges{};int spectrumPos=0;void pushSpectrum(float);std::array<std::array<bool,128>,16> heldNotes{};double sampleRate=48000;void timerCallback()override;
};
