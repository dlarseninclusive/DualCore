#include "FilterResponseDisplay.h"
#include <cmath>

FilterResponseDisplay::FilterResponseDisplay()
{
    filter1Response.resize(256);
    filter2Response.resize(256);
    combinedResponse.resize(256);
    updateResponseCurves();
}

void FilterResponseDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);

    // Background
    g.setColour(juce::Colour(0xff121220));
    g.fillRoundedRectangle(bounds, 4.0f);

    // Border
    g.setColour(juce::Colour(0xff3a3a5a));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

    auto graphBounds = bounds.reduced(10.0f);
    float graphWidth = graphBounds.getWidth();
    float graphHeight = graphBounds.getHeight();
    float graphX = graphBounds.getX();
    float graphY = graphBounds.getY();

    // Draw grid lines
    g.setColour(juce::Colour(0xff2a2a45));

    // Horizontal grid lines (dB)
    for (int db = -24; db <= 12; db += 6)
    {
        float y = graphY + dbToY(static_cast<float>(db), graphHeight);
        g.drawHorizontalLine(static_cast<int>(y), graphX, graphX + graphWidth);
    }

    // Vertical grid lines (frequency)
    float freqs[] = {100.0f, 1000.0f, 10000.0f};
    for (float freq : freqs)
    {
        float x = graphX + freqToX(freq, graphWidth);
        g.drawVerticalLine(static_cast<int>(x), graphY, graphY + graphHeight);
    }

    // Draw 0dB line
    g.setColour(juce::Colour(0xff4a4a6a));
    float zeroY = graphY + dbToY(0.0f, graphHeight);
    g.drawHorizontalLine(static_cast<int>(zeroY), graphX, graphX + graphWidth);

    // Draw frequency labels
    g.setColour(juce::Colours::white.withAlpha(0.7f));
    g.setFont(juce::FontOptions(11.0f));
    g.drawText("100", static_cast<int>(graphX + freqToX(100.0f, graphWidth) - 15),
               static_cast<int>(graphY + graphHeight + 2), 30, 14, juce::Justification::centred);
    g.drawText("1k", static_cast<int>(graphX + freqToX(1000.0f, graphWidth) - 12),
               static_cast<int>(graphY + graphHeight + 2), 24, 14, juce::Justification::centred);
    g.drawText("10k", static_cast<int>(graphX + freqToX(10000.0f, graphWidth) - 15),
               static_cast<int>(graphY + graphHeight + 2), 30, 14, juce::Justification::centred);

    // Create paths for filter responses
    juce::Path filter1Path;
    juce::Path filter2Path;
    juce::Path combinedPath;

    int numPoints = static_cast<int>(filter1Response.size());

    for (int i = 0; i < numPoints; ++i)
    {
        float xRatio = static_cast<float>(i) / static_cast<float>(numPoints - 1);
        float x = graphX + xRatio * graphWidth;

        float y1 = graphY + dbToY(filter1Response[static_cast<size_t>(i)], graphHeight);
        float y2 = graphY + dbToY(filter2Response[static_cast<size_t>(i)], graphHeight);
        float yc = graphY + dbToY(combinedResponse[static_cast<size_t>(i)], graphHeight);

        if (i == 0)
        {
            filter1Path.startNewSubPath(x, y1);
            filter2Path.startNewSubPath(x, y2);
            combinedPath.startNewSubPath(x, yc);
        }
        else
        {
            filter1Path.lineTo(x, y1);
            filter2Path.lineTo(x, y2);
            combinedPath.lineTo(x, yc);
        }
    }

    // Draw Filter 1 response (cyan)
    g.setColour(juce::Colour(0xff00aaff).withAlpha(0.6f));
    g.strokePath(filter1Path, juce::PathStrokeType(1.5f));

    // Draw Filter 2 response (magenta)
    g.setColour(juce::Colour(0xffff55aa).withAlpha(0.6f));
    g.strokePath(filter2Path, juce::PathStrokeType(1.5f));

    // Draw combined response (white)
    g.setColour(juce::Colours::white);
    g.strokePath(combinedPath, juce::PathStrokeType(2.0f));

    // Draw filter frequency markers
    float f1X = graphX + freqToX(filter1Freq, graphWidth);
    float f2X = graphX + freqToX(filter2Freq, graphWidth);

    g.setColour(juce::Colour(0xff00aaff).withAlpha(0.4f));
    g.drawVerticalLine(static_cast<int>(f1X), graphY, graphY + graphHeight);

    g.setColour(juce::Colour(0xffff55aa).withAlpha(0.4f));
    g.drawVerticalLine(static_cast<int>(f2X), graphY, graphY + graphHeight);
}

void FilterResponseDisplay::resized()
{
    updateResponseCurves();
}

void FilterResponseDisplay::setFilter1Parameters(float frequency, float resonance, int mode, int type)
{
    if (filter1Freq != frequency || filter1Reso != resonance || filter1Mode != mode || filter1Type != type)
    {
        filter1Freq = frequency;
        filter1Reso = resonance;
        filter1Mode = mode;
        filter1Type = type;
        updateResponseCurves();
        repaint();
    }
}

void FilterResponseDisplay::setFilter2Parameters(float frequency, float resonance, int mode, int type)
{
    if (filter2Freq != frequency || filter2Reso != resonance || filter2Mode != mode || filter2Type != type)
    {
        filter2Freq = frequency;
        filter2Reso = resonance;
        filter2Mode = mode;
        filter2Type = type;
        updateResponseCurves();
        repaint();
    }
}

void FilterResponseDisplay::setParallelMode(bool parallel)
{
    if (parallelRouting != parallel)
    {
        parallelRouting = parallel;
        updateResponseCurves();
        repaint();
    }
}

float FilterResponseDisplay::calculateFilterMagnitude(float freq, float filterFreq, float resonance, int mode, int type)
{
    // Normalized frequency
    float w = freq / filterFreq;
    float w2 = w * w;
    float w4 = w2 * w2;

    // Base Q factor from resonance
    float Q = 1.0f / (2.0f * (1.0f - resonance * 0.99f));

    float magnitude = 1.0f;

    // Filter type determines the response shape
    switch (type)
    {
        case 0:  // SVF - Clean 2-pole
        {
            float denom = std::sqrt((1.0f - w2) * (1.0f - w2) + (w / Q) * (w / Q));
            switch (mode)
            {
                case 0: magnitude = 1.0f / denom; break;              // LP
                case 1: magnitude = w2 / denom; break;                // HP
                case 2: magnitude = (w / Q) / denom; break;           // BP
                case 3: magnitude = std::abs(1.0f - w2) / denom; break; // Notch
            }
            break;
        }

        case 1:  // Ladder - 4-pole with bass loss
        {
            // 4-pole cascade with resonance feedback causing bass reduction
            float k = resonance * 4.0f;  // Feedback amount
            float bassLoss = 1.0f - (k * 0.15f);  // Bass reduction at high resonance

            // 4-pole lowpass base response
            float g = w;
            float G = g / (1.0f + g);
            float G4 = G * G * G * G;

            float denom4 = std::sqrt((1.0f - w4) * (1.0f - w4) + (w2 / (Q * Q)) * (w2 / (Q * Q)));

            switch (mode)
            {
                case 0:  // LP - 4-pole rolloff
                    magnitude = (bassLoss / denom4) * (1.0f + k * G4);
                    break;
                case 1:  // HP
                    magnitude = w4 / denom4;
                    break;
                case 2:  // BP - narrower than SVF
                    magnitude = (w2 / (Q * 0.7f)) / denom4;
                    break;
                case 3:  // Notch
                    magnitude = std::abs(1.0f - w4) / denom4;
                    break;
            }
            break;
        }

        case 2:  // Diode - Sharper, acidic
        {
            // Diode ladder - sharper resonance peak, less bass loss
            float sharpness = 1.0f + resonance * 0.5f;

            float denom4 = std::sqrt((1.0f - w4) * (1.0f - w4) + (w2 / (Q * Q * sharpness)) * (w2 / (Q * Q * sharpness)));

            switch (mode)
            {
                case 0: magnitude = 1.0f / denom4; break;
                case 1: magnitude = w4 / denom4; break;
                case 2: magnitude = (w2 / (Q * sharpness)) / denom4 * 1.2f; break;
                case 3: magnitude = std::abs(1.0f - w4) / denom4; break;
            }
            break;
        }

        case 3:  // MS-20 - Aggressive, screaming
        {
            // Very high Q at resonance, almost self-oscillating
            float aggQ = Q * (1.0f + resonance * 2.0f);
            float denom = std::sqrt((1.0f - w2) * (1.0f - w2) + (w / aggQ) * (w / aggQ));

            // Add some harmonic content near resonance
            float harmonicBoost = 1.0f + resonance * 0.3f * std::exp(-std::abs(w - 1.0f) * 5.0f);

            switch (mode)
            {
                case 0: magnitude = harmonicBoost / denom; break;
                case 1: magnitude = w2 * harmonicBoost / denom; break;
                case 2: magnitude = (w / aggQ) / denom * harmonicBoost * 1.5f; break;
                case 3: magnitude = std::abs(1.0f - w2) / denom * harmonicBoost; break;
            }
            break;
        }

        case 4:  // Steiner - Formant-like, vocal
        {
            // Asymmetric resonance with formant peaks
            float formantQ = Q * (1.0f + resonance * 0.8f);
            float asymmetry = w < 1.0f ? 1.0f : 0.85f;  // Asymmetric response

            float denom = std::sqrt((1.0f - w2) * (1.0f - w2) + (w / formantQ) * (w / formantQ));

            switch (mode)
            {
                case 0: magnitude = asymmetry / denom; break;
                case 1: magnitude = w2 * asymmetry / denom; break;
                case 2: magnitude = (w / formantQ) / denom * 1.3f; break;
                case 3: magnitude = std::abs(1.0f - w2) / denom * asymmetry; break;
            }
            break;
        }

        case 5:  // OTA - Punchy, snappy
        default:
        {
            // Similar to SVF but with slightly tighter response
            float tightQ = Q * 1.1f;
            float denom = std::sqrt((1.0f - w2) * (1.0f - w2) + (w / tightQ) * (w / tightQ));

            // Add slight punch/emphasis just below cutoff
            float punch = 1.0f + resonance * 0.15f * std::exp(-std::abs(w - 0.9f) * 8.0f);

            switch (mode)
            {
                case 0: magnitude = punch / denom; break;
                case 1: magnitude = w2 * punch / denom; break;
                case 2: magnitude = (w / tightQ) / denom; break;
                case 3: magnitude = std::abs(1.0f - w2) / denom; break;
            }
            break;
        }
    }

    return magnitude;
}

float FilterResponseDisplay::freqToX(float freq, float width)
{
    // Logarithmic mapping from 20Hz to 20kHz
    const float minFreq = 20.0f;
    const float maxFreq = 20000.0f;

    float logMin = std::log10(minFreq);
    float logMax = std::log10(maxFreq);
    float logFreq = std::log10(juce::jlimit(minFreq, maxFreq, freq));

    return width * (logFreq - logMin) / (logMax - logMin);
}

float FilterResponseDisplay::dbToY(float db, float height)
{
    // Map -30dB to +18dB to height
    const float minDb = -30.0f;
    const float maxDb = 18.0f;

    float normalized = (db - minDb) / (maxDb - minDb);
    return height * (1.0f - normalized);
}

void FilterResponseDisplay::updateResponseCurves()
{
    int numPoints = static_cast<int>(filter1Response.size());
    const float minFreq = 20.0f;
    const float maxFreq = 20000.0f;

    for (int i = 0; i < numPoints; ++i)
    {
        // Logarithmic frequency distribution
        float t = static_cast<float>(i) / static_cast<float>(numPoints - 1);
        float freq = minFreq * std::pow(maxFreq / minFreq, t);

        // Calculate magnitudes with filter type
        float mag1 = calculateFilterMagnitude(freq, filter1Freq, filter1Reso, filter1Mode, filter1Type);
        float mag2 = calculateFilterMagnitude(freq, filter2Freq, filter2Reso, filter2Mode, filter2Type);

        // Convert to dB
        float db1 = 20.0f * std::log10(juce::jmax(0.0001f, mag1));
        float db2 = 20.0f * std::log10(juce::jmax(0.0001f, mag2));

        filter1Response[static_cast<size_t>(i)] = juce::jlimit(-30.0f, 18.0f, db1);
        filter2Response[static_cast<size_t>(i)] = juce::jlimit(-30.0f, 18.0f, db2);

        // Combined response depends on routing
        float combinedMag;
        if (parallelRouting)
        {
            // Parallel: average of both
            combinedMag = (mag1 + mag2) * 0.5f;
        }
        else
        {
            // Series: multiply magnitudes
            combinedMag = mag1 * mag2;
        }

        float dbCombined = 20.0f * std::log10(juce::jmax(0.0001f, combinedMag));
        combinedResponse[static_cast<size_t>(i)] = juce::jlimit(-30.0f, 18.0f, dbCombined);
    }
}
