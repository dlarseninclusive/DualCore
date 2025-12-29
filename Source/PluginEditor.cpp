#include "PluginProcessor.h"
#include "PluginEditor.h"

DualCoreAudioProcessorEditor::DualCoreAudioProcessorEditor(DualCoreAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    currentScale = audioProcessor.uiScale.load();

    // === Input Section ===
    setupSlider(inputGainSlider, inputGainLabel, "INPUT");
    setupToggle(hiBoostButton, "HI BOOST");
    setupToggle(hiCutButton, "HI CUT");
    setupToggle(limiterButton, "LIMIT");

    // === Filter 1 ===
    setupSlider(filter1FreqSlider, filter1FreqLabel, "FREQ 1");
    setupSlider(filter1ResoSlider, filter1ResoLabel, "RESO 1");

    filter1ModeBox.addItem("LP", 1);
    filter1ModeBox.addItem("HP", 2);
    filter1ModeBox.addItem("BP", 3);
    filter1ModeBox.addItem("NOTCH", 4);
    addAndMakeVisible(filter1ModeBox);

    // === Filter 2 ===
    setupSlider(filter2FreqSlider, filter2FreqLabel, "FREQ 2");
    setupSlider(filter2ResoSlider, filter2ResoLabel, "RESO 2");

    filter2ModeBox.addItem("LP", 1);
    filter2ModeBox.addItem("HP", 2);
    filter2ModeBox.addItem("BP", 3);
    filter2ModeBox.addItem("NOTCH", 4);
    addAndMakeVisible(filter2ModeBox);

    // === FM ===
    setupSlider(fmAmountSlider, fmAmountLabel, "FM");

    // === ADSR Envelope ===
    setupSlider(envAttackSlider, envAttackLabel, "A");
    setupSlider(envDecaySlider, envDecayLabel, "D");
    setupSlider(envSustainSlider, envSustainLabel, "S");
    setupSlider(envReleaseSlider, envReleaseLabel, "R");
    setupSlider(envAmountSlider, envAmountLabel, "AMT");
    setupSlider(envSensSlider, envSensLabel, "SENS");

    // === LFO ===
    setupSlider(lfoRateSlider, lfoRateLabel, "RATE");
    setupSlider(lfoDepthSlider, lfoDepthLabel, "DEPTH");

    lfoWaveBox.addItem("Sine", 1);
    lfoWaveBox.addItem("Tri", 2);
    lfoWaveBox.addItem("Sqr", 3);
    lfoWaveBox.addItem("Saw+", 4);
    lfoWaveBox.addItem("Saw-", 5);
    lfoWaveBox.addItem("Rnd", 6);
    addAndMakeVisible(lfoWaveBox);

    lfoTargetBox.addItem("F1", 1);
    lfoTargetBox.addItem("F2", 2);
    lfoTargetBox.addItem("Both", 3);
    addAndMakeVisible(lfoTargetBox);

    // === AM ===
    setupSlider(amAmountSlider, amAmountLabel, "AM");
    setupSlider(amAttackSlider, amAttackLabel, "A");
    setupSlider(amReleaseSlider, amReleaseLabel, "R");

    // === Routing ===
    setupToggle(routingButton, "PARALLEL");
    setupSlider(mixSlider, mixLabel, "MIX");

    // === Preset Browser ===
    presetBox.setTextWhenNothingSelected("-- Presets --");
    presetBox.onChange = [this] { loadSelectedPreset(); };
    addAndMakeVisible(presetBox);

    savePresetButton.setButtonText("Save");
    savePresetButton.onClick = [this] { savePresetDialog(); };
    addAndMakeVisible(savePresetButton);

    zoomInButton.setButtonText("+");
    zoomInButton.onClick = [this] { setUIScale(currentScale + 0.1f); };
    addAndMakeVisible(zoomInButton);

    zoomOutButton.setButtonText("-");
    zoomOutButton.onClick = [this] { setUIScale(currentScale - 0.1f); };
    addAndMakeVisible(zoomOutButton);

    // === Attachments ===
    inputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "inputGain", inputGainSlider);
    hiBoostAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.apvts, "hiBoost", hiBoostButton);
    hiCutAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.apvts, "hiCut", hiCutButton);
    limiterAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.apvts, "limiter", limiterButton);

    filter1FreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "filter1Freq", filter1FreqSlider);
    filter1ResoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "filter1Reso", filter1ResoSlider);
    filter1ModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "filter1Mode", filter1ModeBox);

    filter2FreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "filter2Freq", filter2FreqSlider);
    filter2ResoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "filter2Reso", filter2ResoSlider);
    filter2ModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "filter2Mode", filter2ModeBox);

    fmAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "fmAmount", fmAmountSlider);

    envAttackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "envAttack", envAttackSlider);
    envDecayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "envDecay", envDecaySlider);
    envSustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "envSustain", envSustainSlider);
    envReleaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "envRelease", envReleaseSlider);
    envAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "envAmount", envAmountSlider);
    envSensAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "envSens", envSensSlider);

    lfoRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "lfoRate", lfoRateSlider);
    lfoDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "lfoDepth", lfoDepthSlider);
    lfoWaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "lfoWave", lfoWaveBox);
    lfoTargetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "lfoTarget", lfoTargetBox);

    amAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "amAmount", amAmountSlider);
    amAttackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "amAttack", amAttackSlider);
    amReleaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "amRelease", amReleaseSlider);

    routingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.apvts, "routing", routingButton);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "mix", mixSlider);

    refreshPresetList();
    setUIScale(currentScale);
    startTimerHz(30);
}

DualCoreAudioProcessorEditor::~DualCoreAudioProcessorEditor()
{
    stopTimer();
}

void DualCoreAudioProcessorEditor::setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 16);
    slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff00aaff));
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff333355));
    addAndMakeVisible(slider);

    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.8f));
    addAndMakeVisible(label);
}

void DualCoreAudioProcessorEditor::setupToggle(juce::ToggleButton& button, const juce::String& text)
{
    button.setButtonText(text);
    button.setColour(juce::ToggleButton::textColourId, juce::Colours::white.withAlpha(0.8f));
    button.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xff00aaff));
    addAndMakeVisible(button);
}

void DualCoreAudioProcessorEditor::setUIScale(float scale)
{
    currentScale = juce::jlimit(0.7f, 1.5f, scale);
    audioProcessor.uiScale.store(currentScale);

    int width = static_cast<int>(BASE_WIDTH * currentScale);
    int height = static_cast<int>(BASE_HEIGHT * currentScale);
    setSize(width, height);
}

void DualCoreAudioProcessorEditor::timerCallback()
{
    inputLevelL = audioProcessor.inputLevelL.load();
    inputLevelR = audioProcessor.inputLevelR.load();
    outputLevelL = audioProcessor.outputLevelL.load();
    outputLevelR = audioProcessor.outputLevelR.load();
    repaint();
}

void DualCoreAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Dark background
    g.fillAll(juce::Colour(0xff121220));

    auto bounds = getLocalBounds().toFloat();
    float s = currentScale;

    // Title bar
    g.setColour(juce::Colour(0xff1a1a30));
    g.fillRect(0.0f, 0.0f, bounds.getWidth(), 40.0f * s);

    g.setColour(juce::Colours::white);
    g.setFont(juce::FontOptions(22.0f * s).withStyle("Bold"));
    g.drawText("DUALCORE", bounds.removeFromTop(40.0f * s), juce::Justification::centred);

    // Section drawing helper
    auto drawSection = [&](float x, float y, float w, float h, const juce::String& title)
    {
        juce::Rectangle<float> rect(x * s, y * s, w * s, h * s);
        g.setColour(juce::Colour(0xff1e1e35));
        g.fillRoundedRectangle(rect, 6.0f * s);
        g.setColour(juce::Colour(0xff3a3a5a));
        g.drawRoundedRectangle(rect, 6.0f * s, 1.0f);

        g.setColour(juce::Colour(0xff00aaff));
        g.setFont(juce::FontOptions(11.0f * s).withStyle("Bold"));
        g.drawText(title, rect.removeFromTop(18.0f * s), juce::Justification::centred);
    };

    // Row 1: Input, Filter 1, Filter 2
    drawSection(10, 50, 130, 180, "INPUT");
    drawSection(150, 50, 200, 180, "FILTER 1");
    drawSection(360, 50, 200, 180, "FILTER 2");
    drawSection(570, 50, 80, 180, "FM");
    drawSection(660, 50, 230, 180, "ROUTING");

    // Row 2: ADSR, LFO, AM
    drawSection(10, 240, 380, 160, "ENVELOPE");
    drawSection(400, 240, 250, 160, "LFO");
    drawSection(660, 240, 230, 160, "AM");

    // Row 3: Meters
    drawSection(10, 410, 880, 80, "OUTPUT");

    // Draw meters
    float meterY = 445.0f * s;
    float meterH = 30.0f * s;

    // Input meters
    g.setColour(juce::Colour(0xff333355));
    g.fillRect(30.0f * s, meterY, 200.0f * s, meterH);

    float inLevel = juce::jmax(inputLevelL, inputLevelR);
    g.setColour(juce::Colour(0xff00cc66));
    g.fillRect(30.0f * s, meterY, 200.0f * s * juce::jlimit(0.0f, 1.0f, inLevel), meterH);

    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.setFont(juce::FontOptions(10.0f * s));
    g.drawText("IN", 30.0f * s, meterY + meterH + 2.0f * s, 200.0f * s, 14.0f * s, juce::Justification::centred);

    // Output meters
    g.setColour(juce::Colour(0xff333355));
    g.fillRect(280.0f * s, meterY, 200.0f * s, meterH);

    float outLevel = juce::jmax(outputLevelL, outputLevelR);
    g.setColour(juce::Colour(0xff00aaff));
    g.fillRect(280.0f * s, meterY, 200.0f * s * juce::jlimit(0.0f, 1.0f, outLevel), meterH);

    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.drawText("OUT", 280.0f * s, meterY + meterH + 2.0f * s, 200.0f * s, 14.0f * s, juce::Justification::centred);
}

void DualCoreAudioProcessorEditor::resized()
{
    float s = currentScale;
    int knob = static_cast<int>(55 * s);
    int smallKnob = static_cast<int>(45 * s);
    int labelH = static_cast<int>(14 * s);
    int comboH = static_cast<int>(22 * s);
    int buttonH = static_cast<int>(24 * s);
    int margin = static_cast<int>(8 * s);

    // Header area
    int headerY = static_cast<int>(8 * s);
    presetBox.setBounds(static_cast<int>(10 * s), headerY, static_cast<int>(180 * s), static_cast<int>(24 * s));
    savePresetButton.setBounds(static_cast<int>(195 * s), headerY, static_cast<int>(50 * s), static_cast<int>(24 * s));
    zoomOutButton.setBounds(static_cast<int>(820 * s), headerY, static_cast<int>(30 * s), static_cast<int>(24 * s));
    zoomInButton.setBounds(static_cast<int>(855 * s), headerY, static_cast<int>(30 * s), static_cast<int>(24 * s));

    // Row 1 Y position
    int row1Y = static_cast<int>(70 * s);

    // Input section
    int x = static_cast<int>(20 * s);
    inputGainLabel.setBounds(x, row1Y, knob, labelH);
    inputGainSlider.setBounds(x, row1Y + labelH, knob, knob);

    int toggleY = row1Y + labelH + knob + margin;
    hiBoostButton.setBounds(x, toggleY, static_cast<int>(110 * s), buttonH);
    hiCutButton.setBounds(x, toggleY + buttonH + 2, static_cast<int>(110 * s), buttonH);
    limiterButton.setBounds(x, toggleY + (buttonH + 2) * 2, static_cast<int>(110 * s), buttonH);

    // Filter 1 section
    x = static_cast<int>(160 * s);
    filter1FreqLabel.setBounds(x, row1Y, knob, labelH);
    filter1FreqSlider.setBounds(x, row1Y + labelH, knob, knob);

    x += knob + margin;
    filter1ResoLabel.setBounds(x, row1Y, knob, labelH);
    filter1ResoSlider.setBounds(x, row1Y + labelH, knob, knob);

    filter1ModeBox.setBounds(static_cast<int>(160 * s), row1Y + labelH + knob + margin, static_cast<int>(180 * s), comboH);

    // Filter 2 section
    x = static_cast<int>(370 * s);
    filter2FreqLabel.setBounds(x, row1Y, knob, labelH);
    filter2FreqSlider.setBounds(x, row1Y + labelH, knob, knob);

    x += knob + margin;
    filter2ResoLabel.setBounds(x, row1Y, knob, labelH);
    filter2ResoSlider.setBounds(x, row1Y + labelH, knob, knob);

    filter2ModeBox.setBounds(static_cast<int>(370 * s), row1Y + labelH + knob + margin, static_cast<int>(180 * s), comboH);

    // FM section
    x = static_cast<int>(580 * s);
    fmAmountLabel.setBounds(x, row1Y, knob, labelH);
    fmAmountSlider.setBounds(x, row1Y + labelH, knob, knob);

    // Routing section
    x = static_cast<int>(680 * s);
    routingButton.setBounds(x, row1Y, static_cast<int>(100 * s), buttonH);

    mixLabel.setBounds(x, row1Y + buttonH + margin, knob, labelH);
    mixSlider.setBounds(x, row1Y + buttonH + margin + labelH, knob, knob);

    // Row 2 Y position
    int row2Y = static_cast<int>(260 * s);

    // Envelope section
    x = static_cast<int>(20 * s);
    envAttackLabel.setBounds(x, row2Y, smallKnob, labelH);
    envAttackSlider.setBounds(x, row2Y + labelH, smallKnob, smallKnob);

    x += smallKnob + margin;
    envDecayLabel.setBounds(x, row2Y, smallKnob, labelH);
    envDecaySlider.setBounds(x, row2Y + labelH, smallKnob, smallKnob);

    x += smallKnob + margin;
    envSustainLabel.setBounds(x, row2Y, smallKnob, labelH);
    envSustainSlider.setBounds(x, row2Y + labelH, smallKnob, smallKnob);

    x += smallKnob + margin;
    envReleaseLabel.setBounds(x, row2Y, smallKnob, labelH);
    envReleaseSlider.setBounds(x, row2Y + labelH, smallKnob, smallKnob);

    x += smallKnob + margin * 2;
    envAmountLabel.setBounds(x, row2Y, smallKnob, labelH);
    envAmountSlider.setBounds(x, row2Y + labelH, smallKnob, smallKnob);

    x += smallKnob + margin;
    envSensLabel.setBounds(x, row2Y, smallKnob, labelH);
    envSensSlider.setBounds(x, row2Y + labelH, smallKnob, smallKnob);

    // LFO section
    x = static_cast<int>(420 * s);
    lfoRateLabel.setBounds(x, row2Y, smallKnob, labelH);
    lfoRateSlider.setBounds(x, row2Y + labelH, smallKnob, smallKnob);

    x += smallKnob + margin;
    lfoDepthLabel.setBounds(x, row2Y, smallKnob, labelH);
    lfoDepthSlider.setBounds(x, row2Y + labelH, smallKnob, smallKnob);

    int comboY = row2Y + labelH + smallKnob + margin;
    lfoWaveBox.setBounds(static_cast<int>(420 * s), comboY, static_cast<int>(100 * s), comboH);
    lfoTargetBox.setBounds(static_cast<int>(530 * s), comboY, static_cast<int>(80 * s), comboH);

    // AM section
    x = static_cast<int>(680 * s);
    amAmountLabel.setBounds(x, row2Y, smallKnob, labelH);
    amAmountSlider.setBounds(x, row2Y + labelH, smallKnob, smallKnob);

    x += smallKnob + margin;
    amAttackLabel.setBounds(x, row2Y, smallKnob, labelH);
    amAttackSlider.setBounds(x, row2Y + labelH, smallKnob, smallKnob);

    x += smallKnob + margin;
    amReleaseLabel.setBounds(x, row2Y, smallKnob, labelH);
    amReleaseSlider.setBounds(x, row2Y + labelH, smallKnob, smallKnob);
}

void DualCoreAudioProcessorEditor::refreshPresetList()
{
    presetBox.clear();
    auto presets = audioProcessor.getPresetList();

    for (int i = 0; i < presets.size(); ++i)
        presetBox.addItem(presets[i], i + 1);
}

void DualCoreAudioProcessorEditor::savePresetDialog()
{
    auto dialog = std::make_unique<juce::AlertWindow>("Save Preset",
        "Enter a name for the preset:",
        juce::MessageBoxIconType::NoIcon);

    dialog->addTextEditor("name", "", "Preset Name:");
    dialog->addButton("Save", 1);
    dialog->addButton("Cancel", 0);

    dialog->enterModalState(true, juce::ModalCallbackFunction::create(
        [this, dialogPtr = dialog.release()](int result)
        {
            std::unique_ptr<juce::AlertWindow> d(dialogPtr);
            if (result == 1)
            {
                auto name = d->getTextEditorContents("name");
                if (name.isNotEmpty())
                {
                    audioProcessor.savePreset(name);
                    refreshPresetList();
                }
            }
        }));
}

void DualCoreAudioProcessorEditor::loadSelectedPreset()
{
    int index = presetBox.getSelectedItemIndex();
    if (index >= 0)
    {
        auto presets = audioProcessor.getPresetList();
        if (index < presets.size())
        {
            auto file = DualCoreAudioProcessor::getPresetsFolder()
                .getChildFile(presets[index] + ".dcpreset");
            audioProcessor.loadPreset(file);
        }
    }
}
