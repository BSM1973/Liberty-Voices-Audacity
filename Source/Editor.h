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
 juce::TextButton browser,prev{"<"},next{">"},fav{"+"},save,midi,lang,clipIn{"CLIP"},clipOut{"CLIP"};juce::TextEditor search;juce::ComboBox category;juce::ToggleButton favoritesOnly;juce::ListBox list;
 struct BrowserModel:juce::ListBoxModel{VoicesEditor& e;explicit BrowserModel(VoicesEditor& a):e(a){}int getNumRows()override;void paintListBoxItem(int,juce::Graphics&,int,int,bool)override;void listBoxItemClicked(int,const juce::MouseEvent&)override;};BrowserModel model{*this};std::vector<int> rows;std::vector<juce::File> userFiles;bool browsing=false,midiMode=false;juce::String userName;
 void addKnob(int,juce::Rectangle<int>);void addToggle(const char*,juce::Rectangle<int>);void addCombo(const char*,juce::StringArray,juce::Rectangle<int>);void refresh();void filter();void showBrowser(bool);void loadPreset(int);void savePreset();void timerCallback()override;
 juce::String tr(const char* fr,const char* en)const{return juce::String::fromUTF8(p.french?fr:en);}void layoutLanguage();
public:explicit VoicesEditor(VoicesProcessor&);~VoicesEditor()override;void paint(juce::Graphics&)override;void resized()override{};
};
