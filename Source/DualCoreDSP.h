#pragma once

#include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <array>

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

    enum class NoteDivision
    {
        Div_4_1 = 0,   // 4 bars
        Div_2_1,       // 2 bars
        Div_1_1,       // 1 bar
        Div_1_2,       // 1/2
        Div_1_4,       // 1/4 (quarter note)
        Div_1_8,       // 1/8
        Div_1_16,      // 1/16
        Div_1_32,      // 1/32
        Div_1_2T,      // 1/2 triplet
        Div_1_4T,      // 1/4 triplet
        Div_1_8T,      // 1/8 triplet
        Div_1_16T,     // 1/16 triplet
        Div_1_2D,      // 1/2 dotted
        Div_1_4D,      // 1/4 dotted
        Div_1_8D,      // 1/8 dotted
        Div_1_16D      // 1/16 dotted
    };

    enum class DriveType
    {
        Soft = 0,      // Gentle tanh saturation
        Tube,          // Asymmetric tube-style
        Tape,          // Tape saturation with compression
        Hard,          // Hard clipping
        Fuzz           // Aggressive fuzz/distortion
    };

    enum class FilterType
    {
        SVF = 0,       // Clean state-variable filter (default)
        Ladder,        // Moog-style transistor ladder - warm, fat
        Diode,         // Diode ladder (303-style) - acidic, sharp
        MS20,          // Korg MS-20 style - aggressive, screaming
        Steiner,       // Steiner-Parker - vocal, rubbery
        OTA            // OTA-based (80s polysynth) - punchy, snappy
    };

    // Modulation Matrix
    enum class ModSource
    {
        None = 0,
        LFO1,
        LFO2,
        Envelope,
        InputFollower
    };

    enum class ModDestination
    {
        None = 0,
        Filter1Freq,
        Filter1Reso,
        Filter2Freq,
        Filter2Reso,
        FMAmount,
        DriveAmount,
        LFO1Rate,
        LFO2Rate,
        Mix,
        AMAmount
    };

    struct ModulationSlot
    {
        ModSource source = ModSource::None;
        ModDestination destination = ModDestination::None;
        float amount = 0.0f;  // -1.0 to 1.0 (bipolar)
    };

    static constexpr int NUM_MOD_SLOTS = 6;

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
    void setFilter1Type(FilterType type);

    // === Filter 2 Parameters ===
    void setFilter2Frequency(float freqHz);
    void setFilter2Resonance(float resonance);
    void setFilter2Mode(FilterMode mode);
    void setFilter2Type(FilterType type);

    // === FM Modulation (Filter 1 -> Filter 2 frequency) ===
    void setFMAmount(float amount);  // 0.0 to 1.0

    // === ADSR Envelope -> Filter Frequency ===
    void setEnvAttack(float ms);
    void setEnvDecay(float ms);
    void setEnvSustain(float level);  // 0.0 to 1.0
    void setEnvRelease(float ms);
    void setEnvAmount(float amount);  // -1.0 to 1.0 (can invert)
    void setEnvSensitivity(float sens);  // Input sensitivity for triggering

    // === LFO1 -> Filter Frequency ===
    void setLFORate(float hz);
    void setLFODepth(float depth);  // 0.0 to 1.0
    void setLFOWaveform(LFOWaveform wave);
    void setLFOTarget(int target);  // 0=Filter1, 1=Filter2, 2=Both
    void setLFOSync(bool sync);
    void setLFODivision(NoteDivision div);

    // === LFO2 ===
    void setLFO2Rate(float hz);
    void setLFO2Depth(float depth);
    void setLFO2Waveform(LFOWaveform wave);
    void setLFO2Sync(bool sync);
    void setLFO2Division(NoteDivision div);

    // === Host Tempo ===
    void setHostBPM(double bpm);

    // === Modulation Matrix ===
    void setModSlot(int slotIndex, ModSource source, ModDestination dest, float amount);

    // === AM (Amplitude Modulation from Filter 2) ===
    void setAMAmount(float amount);  // 0.0 to 1.0
    void setAMAttack(float ms);
    void setAMRelease(float ms);

    // === Drive/Saturation ===
    void setDriveAmount(float amount);     // 0.0 to 1.0
    void setDriveType(DriveType type);
    void setDrivePrePost(bool post);       // false = pre-filter, true = post-filter

    // === Routing ===
    void setFilterRouting(bool parallel);  // true = parallel, false = series
    void setDryWetMix(float wet);  // 0.0 to 1.0

private:
    // Multi-type Filter with multiple outputs
    struct SVFilter
    {
        // SVF state variables
        float lowpass = 0.0f;
        float bandpass = 0.0f;
        float highpass = 0.0f;
        float notch = 0.0f;

        // Ladder filter state (4 poles)
        float stage[4] = {0.0f, 0.0f, 0.0f, 0.0f};
        float delay[4] = {0.0f, 0.0f, 0.0f, 0.0f};

        // MS-20/Steiner state
        float s1 = 0.0f, s2 = 0.0f;

        // Parameters
        float frequency = 1000.0f;
        float resonance = 0.5f;
        FilterMode mode = FilterMode::LowPass;
        FilterType type = FilterType::SVF;

        // Coefficients
        float f = 0.0f;      // Normalized frequency
        float q = 0.0f;      // Q factor / feedback
        float g = 0.0f;      // Gain coefficient for ladder
        float k = 0.0f;      // Resonance coefficient

        void updateCoefficients(double sampleRate);
        float process(float input);
        void reset();

    private:
        float processSVF(float input);
        float processLadder(float input);
        float processDiode(float input);
        float processMS20(float input);
        float processSteiner(float input);
        float processOTA(float input);

        // Saturation helpers
        float tanhApprox(float x);
        float softClip(float x);
        float diodeClip(float x);
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

    // Saturation/Drive processing
    float processDrive(float input);
    float saturateSoft(float input, float drive);
    float saturateTube(float input, float drive);
    float saturateTape(float input, float drive);
    float saturateHard(float input, float drive);
    float saturateFuzz(float input, float drive);
    float softLimit(float input);

    // Oversampling
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;

    // DSP components
    SVFilter filter1L, filter1R;
    SVFilter filter2L, filter2R;
    ADSREnvelope adsrEnv;
    LFO lfo;
    LFO lfo2;
    EnvelopeFollower inputEnvFollower;
    EnvelopeFollower amEnvFollower;
    ShelfFilter hiBoostL, hiBoostR;
    ShelfFilter hiCutL, hiCutR;

    // Modulation Matrix
    std::array<ModulationSlot, NUM_MOD_SLOTS> modSlots;

    // Parameters
    float inputGain = 1.0f;
    bool hiBoostEnabled = false;
    bool hiCutEnabled = false;
    bool limiterEnabled = false;

    float filter1BaseFreq = 1000.0f;
    float filter1BaseReso = 0.5f;
    float filter2BaseFreq = 2000.0f;
    float filter2BaseReso = 0.5f;

    float fmAmount = 0.0f;
    float envAmount = 0.0f;
    float envSensitivity = 0.5f;

    float lfoDepth = 0.0f;
    float lfoBaseRate = 1.0f;
    int lfoTarget = 2;  // Both filters
    bool lfoSyncEnabled = false;
    NoteDivision lfoDivision = NoteDivision::Div_1_4;

    float lfo2Depth = 0.0f;
    float lfo2BaseRate = 1.0f;
    bool lfo2SyncEnabled = false;
    NoteDivision lfo2Division = NoteDivision::Div_1_4;

    double hostBPM = 120.0;

    float amAmount = 0.0f;

    float driveAmount = 0.0f;
    DriveType driveType = DriveType::Soft;
    bool drivePost = true;  // Post-filter by default

    bool parallelRouting = false;
    float dryWetMix = 1.0f;

    // State
    double sampleRate = 44100.0;
    bool envTriggered = false;
    float lastInputLevel = 0.0f;
};
