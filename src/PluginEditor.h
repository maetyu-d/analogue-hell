#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include <array>
#include <memory>
#include "PluginProcessor.h"

class AnalogueHellAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit AnalogueHellAudioProcessorEditor(AnalogueHellAudioProcessor&);
    ~AnalogueHellAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    class HellLookAndFeel;
    struct Preset;

    void addSlider(juce::Slider& slider, juce::Label& label, const char* paramID, const char* title);
    void buildPresetMenu();
    void applyPreset(int presetIndex);
    void selectPresetRelative(int delta);

    AnalogueHellAudioProcessor& processorRef;

    juce::Slider drive, mix, depth, rate, character, noise, unstable, stereo;
    juce::Label driveL, mixL, depthL, rateL, characterL, noiseL, unstableL, stereoL;

    juce::Label pluginTitle;
    juce::Label pluginSubtitle;
    juce::Label sectionMacro;
    juce::Label sectionGlobal;

    juce::ComboBox presetBox;
    juce::TextButton prevPreset { "<" };
    juce::TextButton nextPreset { ">" };
    juce::ToggleButton reactiveToggle { "Reactive" };

    std::array<juce::Slider*, 8> sliders;
    std::array<juce::Label*, 8> labels;

    using Attach = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttach = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::vector<std::unique_ptr<Attach>> attachments;
    std::unique_ptr<ButtonAttach> reactiveAttachment;

    std::unique_ptr<HellLookAndFeel> lookAndFeel;
    std::vector<Preset> presets;
    bool presetChangeInternal = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AnalogueHellAudioProcessorEditor)
};
