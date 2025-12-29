#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

class FilterResponseDisplay : public juce::Component
{
public:
    FilterResponseDisplay();

    void paint(juce::Graphics& g) override;
    void resized() override;

    // Update filter parameters
    void setFilter1Parameters(float frequency, float resonance, int mode, int type);
    void setFilter2Parameters(float frequency, float resonance, int mode, int type);
    void setParallelMode(bool parallel);

private:
    // Calculate magnitude response for different filter types
    float calculateFilterMagnitude(float freq, float filterFreq, float resonance, int mode, int type);

    // Convert frequency to X position (logarithmic)
    float freqToX(float freq, float width);

    // Convert magnitude (dB) to Y position
    float dbToY(float db, float height);

    // Filter parameters
    float filter1Freq = 1000.0f;
    float filter1Reso = 0.5f;
    int filter1Mode = 0;  // 0=LP, 1=HP, 2=BP, 3=Notch
    int filter1Type = 0;  // 0=SVF, 1=Ladder, 2=Diode, 3=MS20, 4=Steiner, 5=OTA

    float filter2Freq = 2000.0f;
    float filter2Reso = 0.5f;
    int filter2Mode = 0;
    int filter2Type = 0;

    bool parallelRouting = false;

    // Cached response curves
    std::vector<float> filter1Response;
    std::vector<float> filter2Response;
    std::vector<float> combinedResponse;

    void updateResponseCurves();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterResponseDisplay)
};
