#include "DualCoreDSP.h"
#include <random>

void DualCoreDSP::prepare(double newSampleRate, int /*samplesPerBlock*/)
{
    sampleRate = newSampleRate;

    // Initialize filters
    filter1L.updateCoefficients(sampleRate);
    filter1R.updateCoefficients(sampleRate);
    filter2L.updateCoefficients(sampleRate);
    filter2R.updateCoefficients(sampleRate);

    // Initialize envelope followers
    inputEnvFollower.setAttack(1.0f, sampleRate);
    inputEnvFollower.setRelease(50.0f, sampleRate);
    amEnvFollower.setAttack(5.0f, sampleRate);
    amEnvFollower.setRelease(100.0f, sampleRate);

    // Initialize ADSR
    adsrEnv.setAttack(10.0f, sampleRate);
    adsrEnv.setDecay(100.0f, sampleRate);
    adsrEnv.setSustain(0.7f);
    adsrEnv.setRelease(200.0f, sampleRate);

    // Initialize LFO
    lfo.setRate(1.0f, sampleRate);

    // Initialize shelf filters
    hiBoostL.setHighShelf(3000.0f, 6.0f, sampleRate);
    hiBoostR.setHighShelf(3000.0f, 6.0f, sampleRate);
    hiCutL.setLowShelf(8000.0f, -12.0f, sampleRate);
    hiCutR.setLowShelf(8000.0f, -12.0f, sampleRate);

    reset();
}

void DualCoreDSP::reset()
{
    filter1L.reset();
    filter1R.reset();
    filter2L.reset();
    filter2R.reset();
    adsrEnv.reset();
    lfo.reset();
    inputEnvFollower.reset();
    amEnvFollower.reset();
    hiBoostL.reset();
    hiBoostR.reset();
    hiCutL.reset();
    hiCutR.reset();
    envTriggered = false;
    lastInputLevel = 0.0f;
}

void DualCoreDSP::process(juce::AudioBuffer<float>& buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Get mono input for modulation sources
        float monoInput = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
            monoInput += buffer.getSample(ch, sample);
        monoInput /= static_cast<float>(juce::jmax(1, numChannels));

        // Input envelope for ADSR triggering
        float inputLevel = inputEnvFollower.process(std::abs(monoInput * inputGain));

        // Trigger ADSR based on input level crossing threshold
        float threshold = 0.1f * (1.0f - envSensitivity) + 0.01f;
        if (inputLevel > threshold && lastInputLevel <= threshold && !envTriggered)
        {
            adsrEnv.trigger();
            envTriggered = true;
        }
        else if (inputLevel < threshold * 0.5f && envTriggered)
        {
            adsrEnv.release();
            envTriggered = false;
        }
        lastInputLevel = inputLevel;

        // Process modulation sources
        float envValue = adsrEnv.process();
        float lfoValue = lfo.process();

        // Calculate frequency modulation amounts
        float envMod = envValue * envAmount * 8000.0f;  // Up to 8kHz modulation
        float lfoMod = lfoValue * lfoDepth * 2000.0f;   // Up to 2kHz modulation

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float input = buffer.getSample(ch, sample);
            float drySignal = input;

            // Input gain
            input *= inputGain;

            // Input section processing
            if (hiBoostEnabled)
                input = (ch == 0) ? hiBoostL.process(input) : hiBoostR.process(input);
            if (hiCutEnabled)
                input = (ch == 0) ? hiCutL.process(input) : hiCutR.process(input);
            if (limiterEnabled)
                input = softLimit(input);

            // Get the appropriate filter instances
            SVFilter& f1 = (ch == 0) ? filter1L : filter1R;
            SVFilter& f2 = (ch == 0) ? filter2L : filter2R;

            // Apply modulation to filter frequencies
            float f1Freq = filter1BaseFreq;
            float f2Freq = filter2BaseFreq;

            if (lfoTarget == 0 || lfoTarget == 2)
                f1Freq += lfoMod;
            if (lfoTarget == 1 || lfoTarget == 2)
                f2Freq += lfoMod;

            f1Freq += envMod;
            f2Freq += envMod;

            // Clamp frequencies
            f1Freq = juce::jlimit(20.0f, 20000.0f, f1Freq);
            f2Freq = juce::jlimit(20.0f, 20000.0f, f2Freq);

            float output1, output2;

            if (parallelRouting)
            {
                // Parallel: both filters process input independently
                f1.frequency = f1Freq;
                f1.updateCoefficients(sampleRate);
                output1 = f1.process(input);

                // FM modulation: Filter 1 output modulates Filter 2 frequency
                float fmMod = output1 * fmAmount * 4000.0f;
                f2.frequency = juce::jlimit(20.0f, 20000.0f, f2Freq + fmMod);
                f2.updateCoefficients(sampleRate);
                output2 = f2.process(input);

                // Mix parallel outputs
                input = (output1 + output2) * 0.5f;
            }
            else
            {
                // Series: Filter 1 -> Filter 2
                f1.frequency = f1Freq;
                f1.updateCoefficients(sampleRate);
                output1 = f1.process(input);

                // FM modulation
                float fmMod = output1 * fmAmount * 4000.0f;
                f2.frequency = juce::jlimit(20.0f, 20000.0f, f2Freq + fmMod);
                f2.updateCoefficients(sampleRate);
                output2 = f2.process(output1);

                input = output2;
            }

            // AM modulation (amplitude modulation from filter output)
            if (amAmount > 0.0f)
            {
                float amEnv = amEnvFollower.process(std::abs(output2));
                float amGain = 1.0f - (amAmount * (1.0f - amEnv));
                input *= amGain;
            }

            // Dry/wet mix
            float output = drySignal * (1.0f - dryWetMix) + input * dryWetMix;

            buffer.setSample(ch, sample, output);
        }
    }
}

// === Parameter Setters ===

void DualCoreDSP::setInputGain(float gainDb)
{
    inputGain = juce::Decibels::decibelsToGain(gainDb);
}

void DualCoreDSP::setHiBoost(bool enabled)
{
    hiBoostEnabled = enabled;
}

void DualCoreDSP::setHiCut(bool enabled)
{
    hiCutEnabled = enabled;
}

void DualCoreDSP::setLimiterEnabled(bool enabled)
{
    limiterEnabled = enabled;
}

void DualCoreDSP::setFilter1Frequency(float freqHz)
{
    filter1BaseFreq = freqHz;
    filter1L.frequency = freqHz;
    filter1R.frequency = freqHz;
    filter1L.updateCoefficients(sampleRate);
    filter1R.updateCoefficients(sampleRate);
}

void DualCoreDSP::setFilter1Resonance(float resonance)
{
    filter1L.resonance = resonance;
    filter1R.resonance = resonance;
    filter1L.updateCoefficients(sampleRate);
    filter1R.updateCoefficients(sampleRate);
}

void DualCoreDSP::setFilter1Mode(FilterMode mode)
{
    filter1L.mode = mode;
    filter1R.mode = mode;
}

void DualCoreDSP::setFilter2Frequency(float freqHz)
{
    filter2BaseFreq = freqHz;
    filter2L.frequency = freqHz;
    filter2R.frequency = freqHz;
    filter2L.updateCoefficients(sampleRate);
    filter2R.updateCoefficients(sampleRate);
}

void DualCoreDSP::setFilter2Resonance(float resonance)
{
    filter2L.resonance = resonance;
    filter2R.resonance = resonance;
    filter2L.updateCoefficients(sampleRate);
    filter2R.updateCoefficients(sampleRate);
}

void DualCoreDSP::setFilter2Mode(FilterMode mode)
{
    filter2L.mode = mode;
    filter2R.mode = mode;
}

void DualCoreDSP::setFMAmount(float amount)
{
    fmAmount = amount;
}

void DualCoreDSP::setEnvAttack(float ms)
{
    adsrEnv.setAttack(ms, sampleRate);
}

void DualCoreDSP::setEnvDecay(float ms)
{
    adsrEnv.setDecay(ms, sampleRate);
}

void DualCoreDSP::setEnvSustain(float level)
{
    adsrEnv.setSustain(level);
}

void DualCoreDSP::setEnvRelease(float ms)
{
    adsrEnv.setRelease(ms, sampleRate);
}

void DualCoreDSP::setEnvAmount(float amount)
{
    envAmount = amount;
}

void DualCoreDSP::setEnvSensitivity(float sens)
{
    envSensitivity = juce::jlimit(0.0f, 1.0f, sens);
}

void DualCoreDSP::setLFORate(float hz)
{
    lfo.setRate(hz, sampleRate);
}

void DualCoreDSP::setLFODepth(float depth)
{
    lfoDepth = depth;
}

void DualCoreDSP::setLFOWaveform(LFOWaveform wave)
{
    lfo.waveform = wave;
}

void DualCoreDSP::setLFOTarget(int target)
{
    lfoTarget = juce::jlimit(0, 2, target);
}

void DualCoreDSP::setAMAmount(float amount)
{
    amAmount = amount;
}

void DualCoreDSP::setAMAttack(float ms)
{
    amEnvFollower.setAttack(ms, sampleRate);
}

void DualCoreDSP::setAMRelease(float ms)
{
    amEnvFollower.setRelease(ms, sampleRate);
}

void DualCoreDSP::setFilterRouting(bool parallel)
{
    parallelRouting = parallel;
}

void DualCoreDSP::setDryWetMix(float wet)
{
    dryWetMix = wet;
}

// === Utility Functions ===

float DualCoreDSP::saturate(float input, float drive)
{
    if (drive <= 1.0f)
        return input;

    float x = input * drive;
    return std::tanh(x) / std::tanh(drive);
}

float DualCoreDSP::softLimit(float input)
{
    // Soft knee limiter
    const float threshold = 0.8f;
    const float knee = 0.2f;

    float absInput = std::abs(input);
    if (absInput <= threshold)
        return input;

    float overThreshold = absInput - threshold;
    float compressed = threshold + knee * std::tanh(overThreshold / knee);
    return (input > 0.0f) ? compressed : -compressed;
}

// === SVFilter Implementation ===

void DualCoreDSP::SVFilter::updateCoefficients(double sr)
{
    f = 2.0f * std::sin(juce::MathConstants<float>::pi * frequency / static_cast<float>(sr));
    f = juce::jlimit(0.0f, 0.99f, f);
    q = 1.0f - resonance * 0.99f;
    q = juce::jmax(0.01f, q);
}

float DualCoreDSP::SVFilter::process(float input)
{
    // Chamberlin SVF
    lowpass += f * bandpass;
    highpass = input - lowpass - q * bandpass;
    bandpass += f * highpass;
    notch = highpass + lowpass;

    // Soft limit for stability at high resonance
    bandpass = juce::jlimit(-4.0f, 4.0f, bandpass);
    lowpass = juce::jlimit(-4.0f, 4.0f, lowpass);

    switch (mode)
    {
        case FilterMode::LowPass:  return lowpass;
        case FilterMode::HighPass: return highpass;
        case FilterMode::BandPass: return bandpass;
        case FilterMode::Notch:    return notch;
        default: return lowpass;
    }
}

void DualCoreDSP::SVFilter::reset()
{
    lowpass = bandpass = highpass = notch = 0.0f;
}

// === ADSR Implementation ===

void DualCoreDSP::ADSREnvelope::setAttack(float ms, double sr)
{
    attackRate = 1.0f / (static_cast<float>(sr) * ms * 0.001f);
}

void DualCoreDSP::ADSREnvelope::setDecay(float ms, double sr)
{
    decayRate = 1.0f / (static_cast<float>(sr) * ms * 0.001f);
}

void DualCoreDSP::ADSREnvelope::setSustain(float level)
{
    sustainLevel = juce::jlimit(0.0f, 1.0f, level);
}

void DualCoreDSP::ADSREnvelope::setRelease(float ms, double sr)
{
    releaseRate = 1.0f / (static_cast<float>(sr) * ms * 0.001f);
}

void DualCoreDSP::ADSREnvelope::trigger()
{
    state = State::Attack;
}

void DualCoreDSP::ADSREnvelope::release()
{
    if (state != State::Idle)
        state = State::Release;
}

float DualCoreDSP::ADSREnvelope::process()
{
    switch (state)
    {
        case State::Attack:
            output += attackRate;
            if (output >= 1.0f)
            {
                output = 1.0f;
                state = State::Decay;
            }
            break;

        case State::Decay:
            output -= decayRate;
            if (output <= sustainLevel)
            {
                output = sustainLevel;
                state = State::Sustain;
            }
            break;

        case State::Sustain:
            output = sustainLevel;
            break;

        case State::Release:
            output -= releaseRate;
            if (output <= 0.0f)
            {
                output = 0.0f;
                state = State::Idle;
            }
            break;

        case State::Idle:
        default:
            output = 0.0f;
            break;
    }

    return output;
}

void DualCoreDSP::ADSREnvelope::reset()
{
    state = State::Idle;
    output = 0.0f;
}

// === LFO Implementation ===

void DualCoreDSP::LFO::setRate(float hz, double sr)
{
    phaseIncrement = hz / static_cast<float>(sr);
}

float DualCoreDSP::LFO::process()
{
    float output = 0.0f;

    switch (waveform)
    {
        case LFOWaveform::Sine:
            output = std::sin(phase * juce::MathConstants<float>::twoPi);
            break;

        case LFOWaveform::Triangle:
            output = 4.0f * std::abs(phase - 0.5f) - 1.0f;
            break;

        case LFOWaveform::Square:
            output = (phase < 0.5f) ? 1.0f : -1.0f;
            break;

        case LFOWaveform::SawUp:
            output = 2.0f * phase - 1.0f;
            break;

        case LFOWaveform::SawDown:
            output = 1.0f - 2.0f * phase;
            break;

        case LFOWaveform::Random:
            if (phase < phaseIncrement)
            {
                lastRandomValue = randomValue;
                randomValue = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
            }
            // Smooth interpolation
            output = lastRandomValue + phase * (randomValue - lastRandomValue);
            break;
    }

    phase += phaseIncrement;
    if (phase >= 1.0f)
        phase -= 1.0f;

    return output;
}

void DualCoreDSP::LFO::reset()
{
    phase = 0.0f;
    randomValue = 0.0f;
    lastRandomValue = 0.0f;
}

// === EnvelopeFollower Implementation ===

void DualCoreDSP::EnvelopeFollower::setAttack(float ms, double sr)
{
    attackCoef = std::exp(-1.0f / (static_cast<float>(sr) * ms * 0.001f));
}

void DualCoreDSP::EnvelopeFollower::setRelease(float ms, double sr)
{
    releaseCoef = std::exp(-1.0f / (static_cast<float>(sr) * ms * 0.001f));
}

float DualCoreDSP::EnvelopeFollower::process(float input)
{
    float absInput = std::abs(input);

    if (absInput > envelope)
        envelope = attackCoef * (envelope - absInput) + absInput;
    else
        envelope = releaseCoef * (envelope - absInput) + absInput;

    return envelope;
}

void DualCoreDSP::EnvelopeFollower::reset()
{
    envelope = 0.0f;
}

// === ShelfFilter Implementation ===

void DualCoreDSP::ShelfFilter::setHighShelf(float freqHz, float gainDb, double sr)
{
    float A = std::pow(10.0f, gainDb / 40.0f);
    float w0 = juce::MathConstants<float>::twoPi * freqHz / static_cast<float>(sr);
    float cosw0 = std::cos(w0);
    float alpha = std::sin(w0) / 2.0f * std::sqrt(2.0f);

    float norm = (A + 1.0f) - (A - 1.0f) * cosw0 + 2.0f * std::sqrt(A) * alpha;

    a0 = (A * ((A + 1.0f) + (A - 1.0f) * cosw0 + 2.0f * std::sqrt(A) * alpha)) / norm;
    a1 = (-2.0f * A * ((A - 1.0f) + (A + 1.0f) * cosw0)) / norm;
    b1 = ((A + 1.0f) - (A - 1.0f) * cosw0 - 2.0f * std::sqrt(A) * alpha) / norm;
}

void DualCoreDSP::ShelfFilter::setLowShelf(float freqHz, float gainDb, double sr)
{
    float A = std::pow(10.0f, gainDb / 40.0f);
    float w0 = juce::MathConstants<float>::twoPi * freqHz / static_cast<float>(sr);
    float cosw0 = std::cos(w0);
    float alpha = std::sin(w0) / 2.0f * std::sqrt(2.0f);

    float norm = (A + 1.0f) + (A - 1.0f) * cosw0 + 2.0f * std::sqrt(A) * alpha;

    a0 = (A * ((A + 1.0f) - (A - 1.0f) * cosw0 + 2.0f * std::sqrt(A) * alpha)) / norm;
    a1 = (2.0f * A * ((A - 1.0f) - (A + 1.0f) * cosw0)) / norm;
    b1 = ((A + 1.0f) + (A - 1.0f) * cosw0 - 2.0f * std::sqrt(A) * alpha) / norm;
}

float DualCoreDSP::ShelfFilter::process(float input)
{
    float output = a0 * input + z1;
    z1 = a1 * input - b1 * output;
    return output;
}

void DualCoreDSP::ShelfFilter::reset()
{
    z1 = 0.0f;
}
