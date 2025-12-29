#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class DualCoreAudioProcessorEditor : public juce::AudioProcessorEditor,
                                      private juce::Timer
{
public:
    explicit DualCoreAudioProcessorEditor(DualCoreAudioProcessor&);
    ~DualCoreAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void setUIScale(float scale);
    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& text);
    void setupToggle(juce::ToggleButton& button, const juce::String& text);

    DualCoreAudioProcessor& audioProcessor;

    // Base dimensions
    static constexpr int BASE_WIDTH = 900;
    static constexpr int BASE_HEIGHT = 720;
    float currentScale = 1.0f;

    // === Input Section ===
    juce::Slider inputGainSlider;
    juce::Label inputGainLabel;
    juce::ToggleButton hiBoostButton;
    juce::ToggleButton hiCutButton;
    juce::ToggleButton limiterButton;

    // === Filter 1 ===
    juce::Slider filter1FreqSlider;
    juce::Slider filter1ResoSlider;
    juce::ComboBox filter1ModeBox;
    juce::Label filter1FreqLabel;
    juce::Label filter1ResoLabel;

    // === Filter 2 ===
    juce::Slider filter2FreqSlider;
    juce::Slider filter2ResoSlider;
    juce::ComboBox filter2ModeBox;
    juce::Label filter2FreqLabel;
    juce::Label filter2ResoLabel;

    // === FM ===
    juce::Slider fmAmountSlider;
    juce::Label fmAmountLabel;

    // === ADSR Envelope ===
    juce::Slider envAttackSlider;
    juce::Slider envDecaySlider;
    juce::Slider envSustainSlider;
    juce::Slider envReleaseSlider;
    juce::Slider envAmountSlider;
    juce::Slider envSensSlider;
    juce::Label envAttackLabel;
    juce::Label envDecayLabel;
    juce::Label envSustainLabel;
    juce::Label envReleaseLabel;
    juce::Label envAmountLabel;
    juce::Label envSensLabel;

    // === LFO1 ===
    juce::Slider lfoRateSlider;
    juce::Slider lfoDepthSlider;
    juce::ComboBox lfoWaveBox;
    juce::ComboBox lfoTargetBox;
    juce::Label lfoRateLabel;
    juce::Label lfoDepthLabel;
    juce::ToggleButton lfoSyncButton;
    juce::ComboBox lfoDivBox;

    // === LFO2 ===
    juce::Slider lfo2RateSlider;
    juce::Slider lfo2DepthSlider;
    juce::ComboBox lfo2WaveBox;
    juce::Label lfo2RateLabel;
    juce::Label lfo2DepthLabel;
    juce::ToggleButton lfo2SyncButton;
    juce::ComboBox lfo2DivBox;

    // === Modulation Matrix ===
    struct ModSlotUI
    {
        juce::ComboBox sourceBox;
        juce::ComboBox destBox;
        juce::Slider amountSlider;
    };
    std::array<ModSlotUI, 6> modSlotUIs;

    // === AM ===
    juce::Slider amAmountSlider;
    juce::Slider amAttackSlider;
    juce::Slider amReleaseSlider;
    juce::Label amAmountLabel;
    juce::Label amAttackLabel;
    juce::Label amReleaseLabel;

    // === Drive ===
    juce::Slider driveAmountSlider;
    juce::Label driveAmountLabel;
    juce::ComboBox driveTypeBox;
    juce::ToggleButton drivePrePostButton;

    // === Routing ===
    juce::ToggleButton routingButton;
    juce::Slider mixSlider;
    juce::Label mixLabel;

    // === Preset Browser ===
    juce::ComboBox presetBox;
    juce::TextButton savePresetButton;
    juce::TextButton zoomInButton;
    juce::TextButton zoomOutButton;

    // === Parameter Attachments ===
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> hiBoostAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> hiCutAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> limiterAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filter1FreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filter1ResoAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filter1ModeAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filter2FreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filter2ResoAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filter2ModeAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> fmAmountAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> envAttackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> envDecayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> envSustainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> envReleaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> envAmountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> envSensAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoDepthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfoWaveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfoTargetAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lfoSyncAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfoDivAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfo2RateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfo2DepthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfo2WaveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lfo2SyncAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfo2DivAttachment;

    // Modulation Matrix Attachments
    struct ModSlotAttachments
    {
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> sourceAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> destAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> amountAttachment;
    };
    std::array<ModSlotAttachments, 6> modSlotAttachments;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> amAmountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> amAttackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> amReleaseAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAmountAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> driveTypeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> drivePrePostAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> routingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;

    // Level meters
    float inputLevelL = 0.0f;
    float inputLevelR = 0.0f;
    float outputLevelL = 0.0f;
    float outputLevelR = 0.0f;

    void refreshPresetList();
    void savePresetDialog();
    void loadSelectedPreset();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DualCoreAudioProcessorEditor)
};
