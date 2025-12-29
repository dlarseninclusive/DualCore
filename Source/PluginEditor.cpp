#include "PluginProcessor.h"
#include "PluginEditor.h"

DualCoreAudioProcessorEditor::DualCoreAudioProcessorEditor(DualCoreAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&customLookAndFeel);
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

    // === LFO1 ===
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

    setupToggle(lfoSyncButton, "SYNC");

    lfoDivBox.addItem("4/1", 1);
    lfoDivBox.addItem("2/1", 2);
    lfoDivBox.addItem("1/1", 3);
    lfoDivBox.addItem("1/2", 4);
    lfoDivBox.addItem("1/4", 5);
    lfoDivBox.addItem("1/8", 6);
    lfoDivBox.addItem("1/16", 7);
    lfoDivBox.addItem("1/32", 8);
    lfoDivBox.addItem("1/2T", 9);
    lfoDivBox.addItem("1/4T", 10);
    lfoDivBox.addItem("1/8T", 11);
    lfoDivBox.addItem("1/16T", 12);
    lfoDivBox.addItem("1/2D", 13);
    lfoDivBox.addItem("1/4D", 14);
    lfoDivBox.addItem("1/8D", 15);
    lfoDivBox.addItem("1/16D", 16);
    addAndMakeVisible(lfoDivBox);

    // === LFO2 ===
    setupSlider(lfo2RateSlider, lfo2RateLabel, "RATE");
    setupSlider(lfo2DepthSlider, lfo2DepthLabel, "DEPTH");

    lfo2WaveBox.addItem("Sine", 1);
    lfo2WaveBox.addItem("Tri", 2);
    lfo2WaveBox.addItem("Sqr", 3);
    lfo2WaveBox.addItem("Saw+", 4);
    lfo2WaveBox.addItem("Saw-", 5);
    lfo2WaveBox.addItem("Rnd", 6);
    addAndMakeVisible(lfo2WaveBox);

    setupToggle(lfo2SyncButton, "SYNC");

    lfo2DivBox.addItem("4/1", 1);
    lfo2DivBox.addItem("2/1", 2);
    lfo2DivBox.addItem("1/1", 3);
    lfo2DivBox.addItem("1/2", 4);
    lfo2DivBox.addItem("1/4", 5);
    lfo2DivBox.addItem("1/8", 6);
    lfo2DivBox.addItem("1/16", 7);
    lfo2DivBox.addItem("1/32", 8);
    lfo2DivBox.addItem("1/2T", 9);
    lfo2DivBox.addItem("1/4T", 10);
    lfo2DivBox.addItem("1/8T", 11);
    lfo2DivBox.addItem("1/16T", 12);
    lfo2DivBox.addItem("1/2D", 13);
    lfo2DivBox.addItem("1/4D", 14);
    lfo2DivBox.addItem("1/8D", 15);
    lfo2DivBox.addItem("1/16D", 16);
    addAndMakeVisible(lfo2DivBox);

    // === Modulation Matrix ===
    juce::StringArray modSources{"None", "LFO1", "LFO2", "Env", "Input"};
    juce::StringArray modDests{"None", "F1 Freq", "F1 Reso", "F2 Freq", "F2 Reso",
                               "FM", "Drive", "LFO1 Rate", "LFO2 Rate", "Mix", "AM"};

    for (int i = 0; i < 6; ++i)
    {
        auto& slot = modSlotUIs[i];

        for (const auto& src : modSources)
            slot.sourceBox.addItem(src, slot.sourceBox.getNumItems() + 1);
        addAndMakeVisible(slot.sourceBox);

        for (const auto& dst : modDests)
            slot.destBox.addItem(dst, slot.destBox.getNumItems() + 1);
        addAndMakeVisible(slot.destBox);

        slot.amountSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        slot.amountSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 40, 16);
        slot.amountSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff00aaff));
        addAndMakeVisible(slot.amountSlider);
    }

    // === AM ===
    setupSlider(amAmountSlider, amAmountLabel, "AM");
    setupSlider(amAttackSlider, amAttackLabel, "A");
    setupSlider(amReleaseSlider, amReleaseLabel, "R");

    // === Drive ===
    setupSlider(driveAmountSlider, driveAmountLabel, "DRIVE");

    driveTypeBox.addItem("Soft", 1);
    driveTypeBox.addItem("Tube", 2);
    driveTypeBox.addItem("Tape", 3);
    driveTypeBox.addItem("Hard", 4);
    driveTypeBox.addItem("Fuzz", 5);
    addAndMakeVisible(driveTypeBox);

    setupToggle(drivePrePostButton, "POST");

    // === Routing ===
    setupToggle(routingButton, "PARALLEL");
    setupSlider(mixSlider, mixLabel, "MIX");

    // === Filter Response Display ===
    addAndMakeVisible(filterResponseDisplay);

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
    lfoSyncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.apvts, "lfoSync", lfoSyncButton);
    lfoDivAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "lfoDiv", lfoDivBox);

    lfo2RateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "lfo2Rate", lfo2RateSlider);
    lfo2DepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "lfo2Depth", lfo2DepthSlider);
    lfo2WaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "lfo2Wave", lfo2WaveBox);
    lfo2SyncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.apvts, "lfo2Sync", lfo2SyncButton);
    lfo2DivAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "lfo2Div", lfo2DivBox);

    // Modulation Matrix Attachments
    for (int i = 0; i < 6; ++i)
    {
        juce::String slotNum(i + 1);
        modSlotAttachments[i].sourceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            audioProcessor.apvts, "modSource" + slotNum, modSlotUIs[i].sourceBox);
        modSlotAttachments[i].destAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            audioProcessor.apvts, "modDest" + slotNum, modSlotUIs[i].destBox);
        modSlotAttachments[i].amountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.apvts, "modAmount" + slotNum, modSlotUIs[i].amountSlider);
    }

    amAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "amAmount", amAmountSlider);
    amAttackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "amAttack", amAttackSlider);
    amReleaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "amRelease", amReleaseSlider);

    driveAmountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "driveAmount", driveAmountSlider);
    driveTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "driveType", driveTypeBox);
    drivePrePostAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.apvts, "drivePrePost", drivePrePostButton);

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
    setLookAndFeel(nullptr);
}

void DualCoreAudioProcessorEditor::setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& text)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 16);
    slider.setTextValueSuffix("");
    addAndMakeVisible(slider);

    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

void DualCoreAudioProcessorEditor::setupToggle(juce::ToggleButton& button, const juce::String& text)
{
    button.setButtonText(text);
    addAndMakeVisible(button);
}

void DualCoreAudioProcessorEditor::setUIScale(float scale)
{
    currentScale = juce::jlimit(0.7f, 2.0f, scale);
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

    // Update filter response display
    float f1Freq = *audioProcessor.apvts.getRawParameterValue("filter1Freq");
    float f1Reso = *audioProcessor.apvts.getRawParameterValue("filter1Reso");
    int f1Mode = static_cast<int>(*audioProcessor.apvts.getRawParameterValue("filter1Mode"));

    float f2Freq = *audioProcessor.apvts.getRawParameterValue("filter2Freq");
    float f2Reso = *audioProcessor.apvts.getRawParameterValue("filter2Reso");
    int f2Mode = static_cast<int>(*audioProcessor.apvts.getRawParameterValue("filter2Mode"));

    bool parallel = *audioProcessor.apvts.getRawParameterValue("routing") > 0.5f;

    filterResponseDisplay.setFilter1Parameters(f1Freq, f1Reso, f1Mode);
    filterResponseDisplay.setFilter2Parameters(f2Freq, f2Reso, f2Mode);
    filterResponseDisplay.setParallelMode(parallel);

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

    // Row 1: Input, Filter 1, Filter 2, FM, Drive, Routing
    drawSection(10, 50, 130, 185, "INPUT");
    drawSection(150, 50, 190, 185, "FILTER 1");
    drawSection(350, 50, 190, 185, "FILTER 2");
    drawSection(550, 50, 80, 185, "FM");
    drawSection(640, 50, 100, 185, "DRIVE");
    drawSection(750, 50, 160, 185, "ROUTING");

    // Row 2: ADSR, LFO1, LFO2, AM
    drawSection(10, 245, 320, 170, "ENVELOPE");
    drawSection(340, 245, 175, 170, "LFO1");
    drawSection(525, 245, 175, 170, "LFO2");
    drawSection(710, 245, 200, 170, "AM");

    // Row 3: Modulation Matrix
    drawSection(10, 425, 900, 115, "MODULATION MATRIX");

    // Row 4: Output with meters and filter display
    drawSection(10, 550, 900, 120, "OUTPUT");

    // Draw meters
    float meterY = 580.0f * s;
    float meterH = 35.0f * s;

    // Input meters
    g.setColour(juce::Colour(0xff333355));
    g.fillRect(30.0f * s, meterY, 180.0f * s, meterH);

    float inLevel = juce::jmax(inputLevelL, inputLevelR);
    g.setColour(juce::Colour(0xff00cc66));
    g.fillRect(30.0f * s, meterY, 180.0f * s * juce::jlimit(0.0f, 1.0f, inLevel), meterH);

    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.setFont(juce::FontOptions(10.0f * s));
    g.drawText("IN", 30.0f * s, meterY + meterH + 4.0f * s, 180.0f * s, 14.0f * s, juce::Justification::centred);

    // Output meters
    g.setColour(juce::Colour(0xff333355));
    g.fillRect(230.0f * s, meterY, 180.0f * s, meterH);

    float outLevel = juce::jmax(outputLevelL, outputLevelR);
    g.setColour(juce::Colour(0xff00aaff));
    g.fillRect(230.0f * s, meterY, 180.0f * s * juce::jlimit(0.0f, 1.0f, outLevel), meterH);

    g.setColour(juce::Colours::white.withAlpha(0.6f));
    g.drawText("OUT", 230.0f * s, meterY + meterH + 4.0f * s, 180.0f * s, 14.0f * s, juce::Justification::centred);
}

void DualCoreAudioProcessorEditor::resized()
{
    float s = currentScale;
    int knob = static_cast<int>(55 * s);
    int smallKnob = static_cast<int>(45 * s);
    int labelH = static_cast<int>(14 * s);
    int comboH = static_cast<int>(22 * s);
    int buttonH = static_cast<int>(24 * s);
    int margin = static_cast<int>(10 * s);

    // Header area
    int headerY = static_cast<int>(10 * s);
    presetBox.setBounds(static_cast<int>(10 * s), headerY, static_cast<int>(180 * s), static_cast<int>(24 * s));
    savePresetButton.setBounds(static_cast<int>(195 * s), headerY, static_cast<int>(50 * s), static_cast<int>(24 * s));
    zoomOutButton.setBounds(static_cast<int>(850 * s), headerY, static_cast<int>(30 * s), static_cast<int>(24 * s));
    zoomInButton.setBounds(static_cast<int>(885 * s), headerY, static_cast<int>(30 * s), static_cast<int>(24 * s));

    // Row 1 Y position
    int row1Y = static_cast<int>(70 * s);

    // Input section
    int x = static_cast<int>(20 * s);
    inputGainLabel.setBounds(x, row1Y, knob, labelH);
    inputGainSlider.setBounds(x, row1Y + labelH, knob, knob);

    int toggleY = row1Y + labelH + knob + margin;
    hiBoostButton.setBounds(x, toggleY, static_cast<int>(110 * s), buttonH);
    hiCutButton.setBounds(x, toggleY + buttonH + 4, static_cast<int>(110 * s), buttonH);
    limiterButton.setBounds(x, toggleY + (buttonH + 4) * 2, static_cast<int>(110 * s), buttonH);

    // Filter 1 section
    x = static_cast<int>(160 * s);
    filter1FreqLabel.setBounds(x, row1Y, knob, labelH);
    filter1FreqSlider.setBounds(x, row1Y + labelH, knob, knob);

    x += knob + margin;
    filter1ResoLabel.setBounds(x, row1Y, knob, labelH);
    filter1ResoSlider.setBounds(x, row1Y + labelH, knob, knob);

    filter1ModeBox.setBounds(static_cast<int>(160 * s), row1Y + labelH + knob + margin, static_cast<int>(170 * s), comboH);

    // Filter 2 section
    x = static_cast<int>(360 * s);
    filter2FreqLabel.setBounds(x, row1Y, knob, labelH);
    filter2FreqSlider.setBounds(x, row1Y + labelH, knob, knob);

    x += knob + margin;
    filter2ResoLabel.setBounds(x, row1Y, knob, labelH);
    filter2ResoSlider.setBounds(x, row1Y + labelH, knob, knob);

    filter2ModeBox.setBounds(static_cast<int>(360 * s), row1Y + labelH + knob + margin, static_cast<int>(170 * s), comboH);

    // FM section
    x = static_cast<int>(565 * s);
    fmAmountLabel.setBounds(x, row1Y, knob, labelH);
    fmAmountSlider.setBounds(x, row1Y + labelH, knob, knob);

    // Drive section
    x = static_cast<int>(655 * s);
    driveAmountLabel.setBounds(x, row1Y, knob, labelH);
    driveAmountSlider.setBounds(x, row1Y + labelH, knob, knob);

    int driveComboY = row1Y + labelH + knob + margin;
    driveTypeBox.setBounds(static_cast<int>(655 * s), driveComboY, static_cast<int>(80 * s), comboH);
    drivePrePostButton.setBounds(static_cast<int>(655 * s), driveComboY + comboH + 4, static_cast<int>(80 * s), buttonH);

    // Routing section
    x = static_cast<int>(760 * s);
    routingButton.setBounds(x, row1Y, static_cast<int>(140 * s), buttonH);

    mixLabel.setBounds(x + static_cast<int>(35 * s), row1Y + buttonH + margin, knob, labelH);
    mixSlider.setBounds(x + static_cast<int>(35 * s), row1Y + buttonH + margin + labelH, knob, knob);

    // Row 2 Y position
    int row2Y = static_cast<int>(265 * s);

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

    x += smallKnob + margin;
    envAmountLabel.setBounds(x, row2Y, smallKnob, labelH);
    envAmountSlider.setBounds(x, row2Y + labelH, smallKnob, smallKnob);

    x += smallKnob + margin;
    envSensLabel.setBounds(x, row2Y, smallKnob, labelH);
    envSensSlider.setBounds(x, row2Y + labelH, smallKnob, smallKnob);

    // LFO1 section
    x = static_cast<int>(355 * s);
    lfoRateLabel.setBounds(x, row2Y, smallKnob, labelH);
    lfoRateSlider.setBounds(x, row2Y + labelH, smallKnob, smallKnob);

    x += smallKnob + margin;
    lfoDepthLabel.setBounds(x, row2Y, smallKnob, labelH);
    lfoDepthSlider.setBounds(x, row2Y + labelH, smallKnob, smallKnob);

    int comboY = row2Y + labelH + smallKnob + margin;
    lfoWaveBox.setBounds(static_cast<int>(355 * s), comboY, static_cast<int>(75 * s), comboH);
    lfoTargetBox.setBounds(static_cast<int>(435 * s), comboY, static_cast<int>(60 * s), comboH);

    int syncY = comboY + comboH + 6;
    lfoSyncButton.setBounds(static_cast<int>(355 * s), syncY, static_cast<int>(55 * s), buttonH);
    lfoDivBox.setBounds(static_cast<int>(415 * s), syncY, static_cast<int>(75 * s), comboH);

    // LFO2 section
    x = static_cast<int>(540 * s);
    lfo2RateLabel.setBounds(x, row2Y, smallKnob, labelH);
    lfo2RateSlider.setBounds(x, row2Y + labelH, smallKnob, smallKnob);

    x += smallKnob + margin;
    lfo2DepthLabel.setBounds(x, row2Y, smallKnob, labelH);
    lfo2DepthSlider.setBounds(x, row2Y + labelH, smallKnob, smallKnob);

    lfo2WaveBox.setBounds(static_cast<int>(540 * s), comboY, static_cast<int>(140 * s), comboH);

    lfo2SyncButton.setBounds(static_cast<int>(540 * s), syncY, static_cast<int>(55 * s), buttonH);
    lfo2DivBox.setBounds(static_cast<int>(600 * s), syncY, static_cast<int>(75 * s), comboH);

    // AM section
    x = static_cast<int>(725 * s);
    amAmountLabel.setBounds(x, row2Y, smallKnob, labelH);
    amAmountSlider.setBounds(x, row2Y + labelH, smallKnob, smallKnob);

    x += smallKnob + margin;
    amAttackLabel.setBounds(x, row2Y, smallKnob, labelH);
    amAttackSlider.setBounds(x, row2Y + labelH, smallKnob, smallKnob);

    x += smallKnob + margin;
    amReleaseLabel.setBounds(x, row2Y, smallKnob, labelH);
    amReleaseSlider.setBounds(x, row2Y + labelH, smallKnob, smallKnob);

    // Row 3: Modulation Matrix
    int row3Y = static_cast<int>(448 * s);
    int slotWidth = static_cast<int>(290 * s);
    int slotSpacing = static_cast<int>(8 * s);
    int srcComboW = static_cast<int>(60 * s);
    int dstComboW = static_cast<int>(75 * s);
    int amtSliderW = static_cast<int>(130 * s);
    int slotH = static_cast<int>(28 * s);

    for (int i = 0; i < 6; ++i)
    {
        int col = i % 3;
        int row = i / 3;

        int slotX = static_cast<int>(20 * s) + col * (slotWidth + slotSpacing);
        int slotY = row3Y + row * (slotH + static_cast<int>(8 * s));

        auto& slot = modSlotUIs[i];
        slot.sourceBox.setBounds(slotX, slotY, srcComboW, comboH);
        slot.destBox.setBounds(slotX + srcComboW + 5, slotY, dstComboW, comboH);
        slot.amountSlider.setBounds(slotX + srcComboW + dstComboW + 10, slotY, amtSliderW, comboH);
    }

    // Row 4: Output section - filter response display
    filterResponseDisplay.setBounds(static_cast<int>(430 * s), static_cast<int>(565 * s),
                                    static_cast<int>(470 * s), static_cast<int>(90 * s));
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
