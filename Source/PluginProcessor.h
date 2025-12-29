#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "DualCoreDSP.h"

class DualCoreAudioProcessor : public juce::AudioProcessor
{
public:
    DualCoreAudioProcessor();
    ~DualCoreAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using juce::AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    // Parameter tree state
    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Level metering
    std::atomic<float> inputLevelL{0.0f};
    std::atomic<float> inputLevelR{0.0f};
    std::atomic<float> outputLevelL{0.0f};
    std::atomic<float> outputLevelR{0.0f};

    // UI scale (saved with state)
    std::atomic<float> uiScale{1.0f};

    // Preset management
    static juce::File getPresetsFolder();
    static void ensurePresetsFolderExists();
    void savePreset(const juce::String& name);
    void loadPreset(const juce::File& presetFile);
    juce::StringArray getPresetList();
    static void installFactoryPresets();

private:
    DualCoreDSP dualCoreDSP;

    // Parameter IDs - Input Section
    static constexpr const char* INPUT_GAIN_ID = "inputGain";
    static constexpr const char* HI_BOOST_ID = "hiBoost";
    static constexpr const char* HI_CUT_ID = "hiCut";
    static constexpr const char* LIMITER_ID = "limiter";

    // Filter 1
    static constexpr const char* FILTER1_FREQ_ID = "filter1Freq";
    static constexpr const char* FILTER1_RESO_ID = "filter1Reso";
    static constexpr const char* FILTER1_MODE_ID = "filter1Mode";

    // Filter 2
    static constexpr const char* FILTER2_FREQ_ID = "filter2Freq";
    static constexpr const char* FILTER2_RESO_ID = "filter2Reso";
    static constexpr const char* FILTER2_MODE_ID = "filter2Mode";

    // FM
    static constexpr const char* FM_AMOUNT_ID = "fmAmount";

    // ADSR Envelope
    static constexpr const char* ENV_ATTACK_ID = "envAttack";
    static constexpr const char* ENV_DECAY_ID = "envDecay";
    static constexpr const char* ENV_SUSTAIN_ID = "envSustain";
    static constexpr const char* ENV_RELEASE_ID = "envRelease";
    static constexpr const char* ENV_AMOUNT_ID = "envAmount";
    static constexpr const char* ENV_SENS_ID = "envSens";

    // LFO1
    static constexpr const char* LFO_RATE_ID = "lfoRate";
    static constexpr const char* LFO_DEPTH_ID = "lfoDepth";
    static constexpr const char* LFO_WAVE_ID = "lfoWave";
    static constexpr const char* LFO_TARGET_ID = "lfoTarget";

    // LFO1 Sync
    static constexpr const char* LFO_SYNC_ID = "lfoSync";
    static constexpr const char* LFO_DIV_ID = "lfoDiv";

    // LFO2
    static constexpr const char* LFO2_RATE_ID = "lfo2Rate";
    static constexpr const char* LFO2_DEPTH_ID = "lfo2Depth";
    static constexpr const char* LFO2_WAVE_ID = "lfo2Wave";
    static constexpr const char* LFO2_SYNC_ID = "lfo2Sync";
    static constexpr const char* LFO2_DIV_ID = "lfo2Div";

    // Modulation Matrix (6 slots)
    // Parameters are named: modSource1, modDest1, modAmount1, etc.

    // AM
    static constexpr const char* AM_AMOUNT_ID = "amAmount";
    static constexpr const char* AM_ATTACK_ID = "amAttack";
    static constexpr const char* AM_RELEASE_ID = "amRelease";

    // Drive
    static constexpr const char* DRIVE_AMOUNT_ID = "driveAmount";
    static constexpr const char* DRIVE_TYPE_ID = "driveType";
    static constexpr const char* DRIVE_PRE_POST_ID = "drivePrePost";

    // Routing
    static constexpr const char* ROUTING_ID = "routing";
    static constexpr const char* MIX_ID = "mix";

    void updateDSPFromParameters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DualCoreAudioProcessor)
};
