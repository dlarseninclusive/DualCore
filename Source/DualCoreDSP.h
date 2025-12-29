#pragma once

#include <juce_dsp/juce_dsp.h>
#include <cmath>

// DualCore Filter DSP - Dual multimode filters with modulation
class DualCoreDSP
{
public:
    enum class FilterMode
    {
        LowPass = 0,
        HighPass,
        BandPass,
        Notch          // LP + HP combined
    };

    enum class LFOWaveform
    {
        Sine = 0,
        Triangle,
        Square,
        SawUp,
        SawDown,
        Random
    };

    DualCoreDSP() = default;

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();
    void process(juce::AudioBuffer<float>& buffer);

    // === Input Section ===
    void setInputGain(float gainDb);
    void setHiBoost(bool enabled);
    void setHiCut(bool enabled);
    void setLimiterEnabled(bool enabled);

    // === Filter 1 Parameters ===
    void setFilter1Frequency(float freqHz);
    void setFilter1Resonance(float resonance);  // 0.0 to 1.0
    void setFilter1Mode(FilterMode mode);

    // === Filter 2 Parameters ===
    void setFilter2Frequency(float freqHz);
    void setFilter2Resonance(float resonance);
    void setFilter2Mode(FilterMode mode);

    // === FM Modulation (Filter 1 -> Filter 2 frequency) ===
    void setFMAmount(float amount);  // 0.0 to 1.0

    // === ADSR Envelope -> Filter Frequency ===
    void setEnvAttack(float ms);
    void setEnvDecay(float ms);
    void setEnvSustain(float level);  // 0.0 to 1.0
    void setEnvRelease(float ms);
    void setEnvAmount(float amount);  // -1.0 to 1.0 (can invert)
    void setEnvSensitivity(float sens);  // Input sensitivity for triggering

    // === LFO -> Filter Frequency ===
    void setLFORate(float hz);
    void setLFODepth(float depth);  // 0.0 to 1.0
    void setLFOWaveform(LFOWaveform wave);
    void setLFOTarget(int target);  // 0=Filter1, 1=Filter2, 2=Both

    // === AM (Amplitude Modulation from Filter 2) ===
    void setAMAmount(float amount);  // 0.0 to 1.0
    void setAMAttack(float ms);
    void setAMRelease(float ms);

    // === Routing ===
    void setFilterRouting(bool parallel);  // true = parallel, false = series
    void setDryWetMix(float wet);  // 0.0 to 1.0

private:
    // State Variable Filter with multiple outputs
    struct SVFilter
    {
        float lowpass = 0.0f;
        float bandpass = 0.0f;
        float highpass = 0.0f;
        float notch = 0.0f;

        float frequency = 1000.0f;
        float resonance = 0.5f;
        FilterMode mode = FilterMode::LowPass;

        float f = 0.0f;
        float q = 0.0f;

        void updateCoefficients(double sampleRate);
        float process(float input);
        void reset();
    };

    // ADSR Envelope
    struct ADSREnvelope
    {
        enum class State { Idle, Attack, Decay, Sustain, Release };

        State state = State::Idle;
        float output = 0.0f;
        float attackRate = 0.0f;
        float decayRate = 0.0f;
        float sustainLevel = 0.7f;
        float releaseRate = 0.0f;

        void setAttack(float ms, double sampleRate);
        void setDecay(float ms, double sampleRate);
        void setSustain(float level);
        void setRelease(float ms, double sampleRate);
        void trigger();
        void release();
        float process();
        void reset();
    };

    // LFO
    struct LFO
    {
        float phase = 0.0f;
        float phaseIncrement = 0.0f;
        LFOWaveform waveform = LFOWaveform::Sine;
        float randomValue = 0.0f;
        float lastRandomValue = 0.0f;

        void setRate(float hz, double sampleRate);
        float process();
        void reset();
    };

    // Envelope Follower for input triggering
    struct EnvelopeFollower
    {
        float envelope = 0.0f;
        float attackCoef = 0.0f;
        float releaseCoef = 0.0f;

        void setAttack(float ms, double sampleRate);
        void setRelease(float ms, double sampleRate);
        float process(float input);
        void reset();
    };

    // Hi/Lo shelf filters for input section
    struct ShelfFilter
    {
        float z1 = 0.0f;
        float a0 = 1.0f, a1 = 0.0f, b1 = 0.0f;
        void setHighShelf(float freqHz, float gainDb, double sampleRate);
        void setLowShelf(float freqHz, float gainDb, double sampleRate);
        float process(float input);
        void reset();
    };

    // Soft clipping
    float saturate(float input, float drive);
    float softLimit(float input);

    // DSP components
    SVFilter filter1L, filter1R;
    SVFilter filter2L, filter2R;
    ADSREnvelope adsrEnv;
    LFO lfo;
    EnvelopeFollower inputEnvFollower;
    EnvelopeFollower amEnvFollower;
    ShelfFilter hiBoostL, hiBoostR;
    ShelfFilter hiCutL, hiCutR;

    // Parameters
    float inputGain = 1.0f;
    bool hiBoostEnabled = false;
    bool hiCutEnabled = false;
    bool limiterEnabled = false;

    float filter1BaseFreq = 1000.0f;
    float filter2BaseFreq = 2000.0f;

    float fmAmount = 0.0f;
    float envAmount = 0.0f;
    float envSensitivity = 0.5f;

    float lfoDepth = 0.0f;
    int lfoTarget = 2;  // Both filters

    float amAmount = 0.0f;

    bool parallelRouting = false;
    float dryWetMix = 1.0f;

    // State
    double sampleRate = 44100.0;
    bool envTriggered = false;
    float lastInputLevel = 0.0f;
};
