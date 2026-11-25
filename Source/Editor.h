#pragma once
#include "Plugin.h"
class VoicesEditor final:public juce::AudioProcessorEditor,private juce::Timer {
 using AP=VoicesProcessor::AP;
 struct Skin:juce::LookAndFeel_V4{Skin();void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&)override;};
 struct Knob:juce::Slider{std::function<void()> menu;void mouseDown(const juce::MouseEvent& e)override{if(e.mods.isPopupMenu()){if(menu)menu();return;}juce::Slider::mouseDown(e);}};
 struct Control{std::unique_ptr<Knob> knob;std::unique_ptr<juce::Label> label;std::unique_ptr<AP::SliderAttachment> attachment;juce::Rectangle<int> bounds;};
 VoicesProcessor& p;Skin skin;std::vector<Control> controls;
 std::vector<std::unique_ptr<juce::ToggleButton>> toggles;std::vector<std::unique_ptr<AP::ButtonAttachment>> buttons;
 std::vector<std::unique_ptr<juce::ComboBox>> combos;std::vector<std::unique_ptr<AP::ComboBoxAttachment>> choices;
 juce::TextButton browser,prev{"<"},next{">"},fav{"+"},save,midi,lang,clipIn{"CLIP"},clipOut{"CLIP"},studioPage,abA{"A"},abB{"B"},resetMeter,matchAB;std::array<float,2> abLevel{{-100,-100}};bool match=false;std::array<float,120> loudnessHistory{};int loudnessPos=0;juce::ValueTree snapshotA,snapshotB;bool studio=false;int page=0,dragBand=-1;void switchPage(int);void mouseDown(const juce::MouseEvent&)override;void mouseDrag(const juce::MouseEvent&)override;void mouseUp(const juce::MouseEvent&)override;void drawEq(juce::Graphics&);void compare(bool);juce::TextEditor search;juce::ComboBox category;juce::ToggleButton favoritesOnly;struct PresetPanel:juce::Component{void paint(juce::Graphics& g)override{g.fillAll(juce::Colour(0xff171a1d));g.setColour(juce::Colour(0xffce854a));g.drawRect(getLocalBounds(),1);}};PresetPanel presetPanel;
 static constexpr int presetPageSize=7;int presetPage=0;std::array<juce::TextButton,presetPageSize> presetRows;juce::TextButton presetBack,presetForward;juce::Label presetCount;void updatePresetPage();void activatePresetRow(int);std::vector<int> rows;std::vector<juce::File> userFiles;bool browsing=false,midiMode=false;juce::String userName;std::array<float,100> pitchHistory{},correctedHistory{};int historyPos=0;
 void addKnob(int,juce::Rectangle<int>);void addToggle(const char*,juce::Rectangle<int>);void addCombo(const char*,juce::StringArray,juce::Rectangle<int>);void refresh();void filter();void showBrowser(bool);void loadPreset(int);void savePreset();void timerCallback()override;
 juce::String tr(const char* fr,const char* en)const{return juce::String::fromUTF8(p.french?fr:en);}void layoutLanguage();
public:bool qualitySnapshot(int,bool,bool,const juce::File&,juce::StringArray&);explicit VoicesEditor(VoicesProcessor&);~VoicesEditor()override;void paint(juce::Graphics&)override;void resized()override{};
};
