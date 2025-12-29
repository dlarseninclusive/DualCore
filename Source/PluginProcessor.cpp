#include "PluginProcessor.h"
#include "PluginEditor.h"

DualCoreAudioProcessor::DualCoreAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

DualCoreAudioProcessor::~DualCoreAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout DualCoreAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // === Input Section ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{INPUT_GAIN_ID, 1},
        "Input Gain",
        juce::NormalisableRange<float>(-12.0f, 24.0f, 0.1f),
        0.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " dB"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{HI_BOOST_ID, 1},
        "Hi Boost",
        false));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{HI_CUT_ID, 1},
        "Hi Cut",
        false));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{LIMITER_ID, 1},
        "Limiter",
        false));

    // === Filter 1 ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{FILTER1_FREQ_ID, 1},
        "Filter 1 Frequency",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 0.1f, 0.3f),
        1000.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " Hz"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{FILTER1_RESO_ID, 1},
        "Filter 1 Resonance",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{FILTER1_MODE_ID, 1},
        "Filter 1 Mode",
        juce::StringArray{"Low Pass", "High Pass", "Band Pass", "Notch"},
        0));

    // === Filter 2 ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{FILTER2_FREQ_ID, 1},
        "Filter 2 Frequency",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 0.1f, 0.3f),
        2000.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " Hz"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{FILTER2_RESO_ID, 1},
        "Filter 2 Resonance",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{FILTER2_MODE_ID, 1},
        "Filter 2 Mode",
        juce::StringArray{"Low Pass", "High Pass", "Band Pass", "Notch"},
        0));

    // === FM Modulation ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{FM_AMOUNT_ID, 1},
        "FM Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.0f));

    // === ADSR Envelope ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ENV_ATTACK_ID, 1},
        "Env Attack",
        juce::NormalisableRange<float>(0.1f, 1000.0f, 0.1f, 0.4f),
        10.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " ms"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ENV_DECAY_ID, 1},
        "Env Decay",
        juce::NormalisableRange<float>(1.0f, 2000.0f, 1.0f, 0.4f),
        100.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 0) + " ms"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ENV_SUSTAIN_ID, 1},
        "Env Sustain",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.7f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ENV_RELEASE_ID, 1},
        "Env Release",
        juce::NormalisableRange<float>(1.0f, 3000.0f, 1.0f, 0.4f),
        200.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 0) + " ms"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ENV_AMOUNT_ID, 1},
        "Env Amount",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f),
        0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ENV_SENS_ID, 1},
        "Env Sensitivity",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f));

    // === LFO ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{LFO_RATE_ID, 1},
        "LFO Rate",
        juce::NormalisableRange<float>(0.01f, 20.0f, 0.01f, 0.4f),
        1.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 2) + " Hz"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{LFO_DEPTH_ID, 1},
        "LFO Depth",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.0f));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{LFO_WAVE_ID, 1},
        "LFO Waveform",
        juce::StringArray{"Sine", "Triangle", "Square", "Saw Up", "Saw Down", "Random"},
        0));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{LFO_TARGET_ID, 1},
        "LFO Target",
        juce::StringArray{"Filter 1", "Filter 2", "Both"},
        2));

    // === AM ===
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{AM_AMOUNT_ID, 1},
        "AM Amount",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{AM_ATTACK_ID, 1},
        "AM Attack",
        juce::NormalisableRange<float>(0.1f, 500.0f, 0.1f, 0.4f),
        5.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 1) + " ms"; },
        nullptr));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{AM_RELEASE_ID, 1},
        "AM Release",
        juce::NormalisableRange<float>(1.0f, 1000.0f, 1.0f, 0.4f),
        100.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 0) + " ms"; },
        nullptr));

    // === Routing ===
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ROUTING_ID, 1},
        "Parallel Routing",
        false));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{MIX_ID, 1},
        "Dry/Wet Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        100.0f,
        juce::String(),
        juce::AudioProcessorParameter::genericParameter,
        [](float value, int) { return juce::String(value, 0) + "%"; },
        nullptr));

    return {params.begin(), params.end()};
}

const juce::String DualCoreAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DualCoreAudioProcessor::acceptsMidi() const { return false; }
bool DualCoreAudioProcessor::producesMidi() const { return false; }
bool DualCoreAudioProcessor::isMidiEffect() const { return false; }
double DualCoreAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int DualCoreAudioProcessor::getNumPrograms() { return 1; }
int DualCoreAudioProcessor::getCurrentProgram() { return 0; }
void DualCoreAudioProcessor::setCurrentProgram(int /*index*/) {}
const juce::String DualCoreAudioProcessor::getProgramName(int /*index*/) { return {}; }
void DualCoreAudioProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/) {}

void DualCoreAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    dualCoreDSP.prepare(sampleRate, samplesPerBlock);
    updateDSPFromParameters();
}

void DualCoreAudioProcessor::releaseResources()
{
    dualCoreDSP.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DualCoreAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}
#endif

void DualCoreAudioProcessor::updateDSPFromParameters()
{
    // Input section
    dualCoreDSP.setInputGain(*apvts.getRawParameterValue(INPUT_GAIN_ID));
    dualCoreDSP.setHiBoost(*apvts.getRawParameterValue(HI_BOOST_ID) > 0.5f);
    dualCoreDSP.setHiCut(*apvts.getRawParameterValue(HI_CUT_ID) > 0.5f);
    dualCoreDSP.setLimiterEnabled(*apvts.getRawParameterValue(LIMITER_ID) > 0.5f);

    // Filter 1
    dualCoreDSP.setFilter1Frequency(*apvts.getRawParameterValue(FILTER1_FREQ_ID));
    dualCoreDSP.setFilter1Resonance(*apvts.getRawParameterValue(FILTER1_RESO_ID));
    dualCoreDSP.setFilter1Mode(static_cast<DualCoreDSP::FilterMode>(
        static_cast<int>(*apvts.getRawParameterValue(FILTER1_MODE_ID))));

    // Filter 2
    dualCoreDSP.setFilter2Frequency(*apvts.getRawParameterValue(FILTER2_FREQ_ID));
    dualCoreDSP.setFilter2Resonance(*apvts.getRawParameterValue(FILTER2_RESO_ID));
    dualCoreDSP.setFilter2Mode(static_cast<DualCoreDSP::FilterMode>(
        static_cast<int>(*apvts.getRawParameterValue(FILTER2_MODE_ID))));

    // FM
    dualCoreDSP.setFMAmount(*apvts.getRawParameterValue(FM_AMOUNT_ID));

    // ADSR
    dualCoreDSP.setEnvAttack(*apvts.getRawParameterValue(ENV_ATTACK_ID));
    dualCoreDSP.setEnvDecay(*apvts.getRawParameterValue(ENV_DECAY_ID));
    dualCoreDSP.setEnvSustain(*apvts.getRawParameterValue(ENV_SUSTAIN_ID));
    dualCoreDSP.setEnvRelease(*apvts.getRawParameterValue(ENV_RELEASE_ID));
    dualCoreDSP.setEnvAmount(*apvts.getRawParameterValue(ENV_AMOUNT_ID));
    dualCoreDSP.setEnvSensitivity(*apvts.getRawParameterValue(ENV_SENS_ID));

    // LFO
    dualCoreDSP.setLFORate(*apvts.getRawParameterValue(LFO_RATE_ID));
    dualCoreDSP.setLFODepth(*apvts.getRawParameterValue(LFO_DEPTH_ID));
    dualCoreDSP.setLFOWaveform(static_cast<DualCoreDSP::LFOWaveform>(
        static_cast<int>(*apvts.getRawParameterValue(LFO_WAVE_ID))));
    dualCoreDSP.setLFOTarget(static_cast<int>(*apvts.getRawParameterValue(LFO_TARGET_ID)));

    // AM
    dualCoreDSP.setAMAmount(*apvts.getRawParameterValue(AM_AMOUNT_ID));
    dualCoreDSP.setAMAttack(*apvts.getRawParameterValue(AM_ATTACK_ID));
    dualCoreDSP.setAMRelease(*apvts.getRawParameterValue(AM_RELEASE_ID));

    // Routing
    dualCoreDSP.setFilterRouting(*apvts.getRawParameterValue(ROUTING_ID) > 0.5f);
    dualCoreDSP.setDryWetMix(*apvts.getRawParameterValue(MIX_ID) / 100.0f);
}

void DualCoreAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    updateDSPFromParameters();

    // Input metering
    if (buffer.getNumChannels() > 0)
    {
        inputLevelL.store(buffer.getMagnitude(0, 0, buffer.getNumSamples()));
        if (buffer.getNumChannels() > 1)
            inputLevelR.store(buffer.getMagnitude(1, 0, buffer.getNumSamples()));
    }

    dualCoreDSP.process(buffer);

    // Output metering
    if (buffer.getNumChannels() > 0)
    {
        outputLevelL.store(buffer.getMagnitude(0, 0, buffer.getNumSamples()));
        if (buffer.getNumChannels() > 1)
            outputLevelR.store(buffer.getMagnitude(1, 0, buffer.getNumSamples()));
    }
}

bool DualCoreAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* DualCoreAudioProcessor::createEditor()
{
    return new DualCoreAudioProcessorEditor(*this);
}

void DualCoreAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty("uiScale", uiScale.load(), nullptr);
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void DualCoreAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName(apvts.state.getType()))
    {
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
        uiScale.store(static_cast<float>(apvts.state.getProperty("uiScale", 1.0f)));
        updateDSPFromParameters();
    }
}

// === Preset Management ===

juce::File DualCoreAudioProcessor::getPresetsFolder()
{
    auto appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    return appDataDir.getChildFile("DualCore").getChildFile("Presets");
}

void DualCoreAudioProcessor::ensurePresetsFolderExists()
{
    auto folder = getPresetsFolder();
    if (!folder.exists())
        folder.createDirectory();
}

void DualCoreAudioProcessor::savePreset(const juce::String& name)
{
    ensurePresetsFolderExists();
    auto file = getPresetsFolder().getChildFile(name + ".dcpreset");

    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());

    if (xml != nullptr)
        xml->writeTo(file);
}

void DualCoreAudioProcessor::loadPreset(const juce::File& presetFile)
{
    if (!presetFile.existsAsFile())
        return;

    std::unique_ptr<juce::XmlElement> xml(juce::XmlDocument::parse(presetFile));

    if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
    {
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
        updateDSPFromParameters();
    }
}

juce::StringArray DualCoreAudioProcessor::getPresetList()
{
    juce::StringArray presets;
    auto folder = getPresetsFolder();

    if (folder.exists())
    {
        for (const auto& file : folder.findChildFiles(juce::File::findFiles, false, "*.dcpreset"))
            presets.add(file.getFileNameWithoutExtension());
    }

    return presets;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DualCoreAudioProcessor();
}
