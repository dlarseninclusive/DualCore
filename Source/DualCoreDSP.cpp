#include "DualCoreDSP.h"
#include <random>

void DualCoreDSP::prepare(double newSampleRate, int samplesPerBlock)
{
    sampleRate = newSampleRate;

    // Initialize 2x oversampling for drive section
    oversampling = std::make_unique<juce::dsp::Oversampling<float>>(
        2,  // numChannels
        1,  // oversampling factor (2^1 = 2x)
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
        true  // isMaxQuality
    );
    oversampling->initProcessing(static_cast<size_t>(samplesPerBlock));

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

    // Initialize LFOs
    lfo.setRate(1.0f, sampleRate);
    lfo2.setRate(1.0f, sampleRate);

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
    lfo2.reset();
    inputEnvFollower.reset();
    amEnvFollower.reset();
    hiBoostL.reset();
    hiBoostR.reset();
    hiCutL.reset();
    hiCutR.reset();
    envTriggered = false;
    lastInputLevel = 0.0f;

    if (oversampling)
        oversampling->reset();
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

        // === Compute all modulation sources ===
        float envValue = adsrEnv.process();
        float lfo1Value = lfo.process();
        float lfo2Value = lfo2.process();

        // === Calculate legacy modulation (backward compatible) ===
        float envMod = envValue * envAmount * 8000.0f;
        float lfoMod = lfo1Value * lfoDepth * 2000.0f;

        // === Modulation Matrix Processing ===
        float modFilter1Freq = 0.0f;
        float modFilter1Reso = 0.0f;
        float modFilter2Freq = 0.0f;
        float modFilter2Reso = 0.0f;
        float modFMAmount = 0.0f;
        float modDriveAmount = 0.0f;
        float modLFO1Rate = 0.0f;
        float modLFO2Rate = 0.0f;
        float modMix = 0.0f;
        float modAMAmount = 0.0f;

        for (int slot = 0; slot < NUM_MOD_SLOTS; ++slot)
        {
            const auto& modSlot = modSlots[slot];
            if (modSlot.source == ModSource::None || modSlot.destination == ModDestination::None)
                continue;

            // Get source value (-1 to 1)
            float sourceValue = 0.0f;
            switch (modSlot.source)
            {
                case ModSource::LFO1:
                    sourceValue = lfo1Value;
                    break;
                case ModSource::LFO2:
                    sourceValue = lfo2Value;
                    break;
                case ModSource::Envelope:
                    sourceValue = envValue * 2.0f - 1.0f;  // Convert 0-1 to -1 to 1
                    break;
                case ModSource::InputFollower:
                    sourceValue = inputLevel * 2.0f - 1.0f;
                    break;
                default:
                    break;
            }

            // Apply amount (source * amount gives modulation value)
            float modValue = sourceValue * modSlot.amount;

            // Accumulate to destination
            switch (modSlot.destination)
            {
                case ModDestination::Filter1Freq:
                    modFilter1Freq += modValue * 8000.0f;
                    break;
                case ModDestination::Filter1Reso:
                    modFilter1Reso += modValue * 0.5f;
                    break;
                case ModDestination::Filter2Freq:
                    modFilter2Freq += modValue * 8000.0f;
                    break;
                case ModDestination::Filter2Reso:
                    modFilter2Reso += modValue * 0.5f;
                    break;
                case ModDestination::FMAmount:
                    modFMAmount += modValue;
                    break;
                case ModDestination::DriveAmount:
                    modDriveAmount += modValue;
                    break;
                case ModDestination::LFO1Rate:
                    modLFO1Rate += modValue * 10.0f;
                    break;
                case ModDestination::LFO2Rate:
                    modLFO2Rate += modValue * 10.0f;
                    break;
                case ModDestination::Mix:
                    modMix += modValue;
                    break;
                case ModDestination::AMAmount:
                    modAMAmount += modValue;
                    break;
                default:
                    break;
            }
        }

        // Apply LFO rate modulation if needed
        if (modLFO1Rate != 0.0f)
            lfo.setRate(juce::jlimit(0.01f, 20.0f, lfoBaseRate + modLFO1Rate), sampleRate);
        if (modLFO2Rate != 0.0f)
            lfo2.setRate(juce::jlimit(0.01f, 20.0f, lfo2BaseRate + modLFO2Rate), sampleRate);

        // Calculate modulated parameter values
        float modulatedFM = juce::jlimit(0.0f, 1.0f, fmAmount + modFMAmount);
        float modulatedDrive = juce::jlimit(0.0f, 1.0f, driveAmount + modDriveAmount);
        float modulatedMix = juce::jlimit(0.0f, 1.0f, dryWetMix + modMix);
        float modulatedAM = juce::jlimit(0.0f, 1.0f, amAmount + modAMAmount);

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

            // Pre-filter drive (with modulation)
            if (!drivePost && modulatedDrive > 0.0f)
            {
                float savedDrive = driveAmount;
                driveAmount = modulatedDrive;
                input = processDrive(input);
                driveAmount = savedDrive;
            }

            // Get the appropriate filter instances
            SVFilter& f1 = (ch == 0) ? filter1L : filter1R;
            SVFilter& f2 = (ch == 0) ? filter2L : filter2R;

            // Apply modulation to filter frequencies (legacy + matrix)
            float f1Freq = filter1BaseFreq + modFilter1Freq;
            float f2Freq = filter2BaseFreq + modFilter2Freq;
            float f1Reso = filter1BaseReso + modFilter1Reso;
            float f2Reso = filter2BaseReso + modFilter2Reso;

            // Add legacy LFO modulation
            if (lfoTarget == 0 || lfoTarget == 2)
                f1Freq += lfoMod;
            if (lfoTarget == 1 || lfoTarget == 2)
                f2Freq += lfoMod;

            // Add legacy envelope modulation
            f1Freq += envMod;
            f2Freq += envMod;

            // Clamp values
            f1Freq = juce::jlimit(20.0f, 20000.0f, f1Freq);
            f2Freq = juce::jlimit(20.0f, 20000.0f, f2Freq);
            f1Reso = juce::jlimit(0.0f, 1.0f, f1Reso);
            f2Reso = juce::jlimit(0.0f, 1.0f, f2Reso);

            float output1, output2;

            if (parallelRouting)
            {
                // Parallel: both filters process input independently
                f1.frequency = f1Freq;
                f1.resonance = f1Reso;
                f1.updateCoefficients(sampleRate);
                output1 = f1.process(input);

                // FM modulation: Filter 1 output modulates Filter 2 frequency
                float fmMod = output1 * modulatedFM * 4000.0f;
                f2.frequency = juce::jlimit(20.0f, 20000.0f, f2Freq + fmMod);
                f2.resonance = f2Reso;
                f2.updateCoefficients(sampleRate);
                output2 = f2.process(input);

                // Mix parallel outputs
                input = (output1 + output2) * 0.5f;
            }
            else
            {
                // Series: Filter 1 -> Filter 2
                f1.frequency = f1Freq;
                f1.resonance = f1Reso;
                f1.updateCoefficients(sampleRate);
                output1 = f1.process(input);

                // FM modulation
                float fmMod = output1 * modulatedFM * 4000.0f;
                f2.frequency = juce::jlimit(20.0f, 20000.0f, f2Freq + fmMod);
                f2.resonance = f2Reso;
                f2.updateCoefficients(sampleRate);
                output2 = f2.process(output1);

                input = output2;
            }

            // AM modulation (with matrix modulation)
            if (modulatedAM > 0.0f)
            {
                float amEnv = amEnvFollower.process(std::abs(output2));
                float amGain = 1.0f - (modulatedAM * (1.0f - amEnv));
                input *= amGain;
            }

            // Post-filter drive (with modulation)
            if (drivePost && modulatedDrive > 0.0f)
            {
                float savedDrive = driveAmount;
                driveAmount = modulatedDrive;
                input = processDrive(input);
                driveAmount = savedDrive;
            }

            // Dry/wet mix (with modulation)
            float output = drySignal * (1.0f - modulatedMix) + input * modulatedMix;

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
    filter1BaseReso = resonance;
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
    filter2BaseReso = resonance;
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
    lfoBaseRate = hz;
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

void DualCoreDSP::setLFO2Rate(float hz)
{
    lfo2BaseRate = hz;
    lfo2.setRate(hz, sampleRate);
}

void DualCoreDSP::setLFO2Depth(float depth)
{
    lfo2Depth = depth;
}

void DualCoreDSP::setLFO2Waveform(LFOWaveform wave)
{
    lfo2.waveform = wave;
}

void DualCoreDSP::setLFOSync(bool sync)
{
    lfoSyncEnabled = sync;
    if (sync)
    {
        // Recalculate rate from BPM and division
        setLFODivision(lfoDivision);
    }
    else
    {
        // Restore free-running rate
        lfo.setRate(lfoBaseRate, sampleRate);
    }
}

void DualCoreDSP::setLFODivision(NoteDivision div)
{
    lfoDivision = div;
    if (lfoSyncEnabled)
    {
        // Calculate rate from BPM and note division
        // Quarter note = 1 beat, so 1/4 note frequency = BPM/60
        float quarterNoteHz = static_cast<float>(hostBPM) / 60.0f;
        float rate = quarterNoteHz;

        switch (div)
        {
            case NoteDivision::Div_4_1:  rate = quarterNoteHz / 16.0f; break;  // 4 bars
            case NoteDivision::Div_2_1:  rate = quarterNoteHz / 8.0f;  break;  // 2 bars
            case NoteDivision::Div_1_1:  rate = quarterNoteHz / 4.0f;  break;  // 1 bar
            case NoteDivision::Div_1_2:  rate = quarterNoteHz / 2.0f;  break;
            case NoteDivision::Div_1_4:  rate = quarterNoteHz;         break;
            case NoteDivision::Div_1_8:  rate = quarterNoteHz * 2.0f;  break;
            case NoteDivision::Div_1_16: rate = quarterNoteHz * 4.0f;  break;
            case NoteDivision::Div_1_32: rate = quarterNoteHz * 8.0f;  break;
            case NoteDivision::Div_1_2T: rate = quarterNoteHz / 2.0f * 1.5f;  break;  // Triplet
            case NoteDivision::Div_1_4T: rate = quarterNoteHz * 1.5f;         break;
            case NoteDivision::Div_1_8T: rate = quarterNoteHz * 2.0f * 1.5f;  break;
            case NoteDivision::Div_1_16T: rate = quarterNoteHz * 4.0f * 1.5f; break;
            case NoteDivision::Div_1_2D: rate = quarterNoteHz / 2.0f / 1.5f;  break;  // Dotted
            case NoteDivision::Div_1_4D: rate = quarterNoteHz / 1.5f;         break;
            case NoteDivision::Div_1_8D: rate = quarterNoteHz * 2.0f / 1.5f;  break;
            case NoteDivision::Div_1_16D: rate = quarterNoteHz * 4.0f / 1.5f; break;
        }

        lfo.setRate(rate, sampleRate);
    }
}

void DualCoreDSP::setLFO2Sync(bool sync)
{
    lfo2SyncEnabled = sync;
    if (sync)
    {
        setLFO2Division(lfo2Division);
    }
    else
    {
        lfo2.setRate(lfo2BaseRate, sampleRate);
    }
}

void DualCoreDSP::setLFO2Division(NoteDivision div)
{
    lfo2Division = div;
    if (lfo2SyncEnabled)
    {
        float quarterNoteHz = static_cast<float>(hostBPM) / 60.0f;
        float rate = quarterNoteHz;

        switch (div)
        {
            case NoteDivision::Div_4_1:  rate = quarterNoteHz / 16.0f; break;
            case NoteDivision::Div_2_1:  rate = quarterNoteHz / 8.0f;  break;
            case NoteDivision::Div_1_1:  rate = quarterNoteHz / 4.0f;  break;
            case NoteDivision::Div_1_2:  rate = quarterNoteHz / 2.0f;  break;
            case NoteDivision::Div_1_4:  rate = quarterNoteHz;         break;
            case NoteDivision::Div_1_8:  rate = quarterNoteHz * 2.0f;  break;
            case NoteDivision::Div_1_16: rate = quarterNoteHz * 4.0f;  break;
            case NoteDivision::Div_1_32: rate = quarterNoteHz * 8.0f;  break;
            case NoteDivision::Div_1_2T: rate = quarterNoteHz / 2.0f * 1.5f;  break;
            case NoteDivision::Div_1_4T: rate = quarterNoteHz * 1.5f;         break;
            case NoteDivision::Div_1_8T: rate = quarterNoteHz * 2.0f * 1.5f;  break;
            case NoteDivision::Div_1_16T: rate = quarterNoteHz * 4.0f * 1.5f; break;
            case NoteDivision::Div_1_2D: rate = quarterNoteHz / 2.0f / 1.5f;  break;
            case NoteDivision::Div_1_4D: rate = quarterNoteHz / 1.5f;         break;
            case NoteDivision::Div_1_8D: rate = quarterNoteHz * 2.0f / 1.5f;  break;
            case NoteDivision::Div_1_16D: rate = quarterNoteHz * 4.0f / 1.5f; break;
        }

        lfo2.setRate(rate, sampleRate);
    }
}

void DualCoreDSP::setHostBPM(double bpm)
{
    hostBPM = juce::jlimit(20.0, 300.0, bpm);
    // Update synced LFOs
    if (lfoSyncEnabled)
        setLFODivision(lfoDivision);
    if (lfo2SyncEnabled)
        setLFO2Division(lfo2Division);
}

void DualCoreDSP::setModSlot(int slotIndex, ModSource source, ModDestination dest, float amount)
{
    if (slotIndex >= 0 && slotIndex < NUM_MOD_SLOTS)
    {
        modSlots[slotIndex].source = source;
        modSlots[slotIndex].destination = dest;
        modSlots[slotIndex].amount = juce::jlimit(-1.0f, 1.0f, amount);
    }
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

void DualCoreDSP::setDriveAmount(float amount)
{
    driveAmount = juce::jlimit(0.0f, 1.0f, amount);
}

void DualCoreDSP::setDriveType(DriveType type)
{
    driveType = type;
}

void DualCoreDSP::setDrivePrePost(bool post)
{
    drivePost = post;
}

// === Saturation/Drive Functions ===

float DualCoreDSP::processDrive(float input)
{
    // Scale drive amount to useful range (1.0 to 20.0)
    float drive = 1.0f + driveAmount * 19.0f;

    switch (driveType)
    {
        case DriveType::Soft:  return saturateSoft(input, drive);
        case DriveType::Tube:  return saturateTube(input, drive);
        case DriveType::Tape:  return saturateTape(input, drive);
        case DriveType::Hard:  return saturateHard(input, drive);
        case DriveType::Fuzz:  return saturateFuzz(input, drive);
        default: return saturateSoft(input, drive);
    }
}

float DualCoreDSP::saturateSoft(float input, float drive)
{
    // Gentle tanh saturation with drive compensation
    float x = input * drive;
    float output = std::tanh(x);
    // Compensate for volume loss at low drive
    return output * (1.0f / std::tanh(drive));
}

float DualCoreDSP::saturateTube(float input, float drive)
{
    // Asymmetric tube-style saturation
    // Positive half gets more compression than negative
    float x = input * drive;

    if (x >= 0.0f)
    {
        // Soft clip positive (tube compression)
        float output = 1.0f - std::exp(-x);
        return output * 0.9f;  // Slight reduction
    }
    else
    {
        // Harder clip on negative (tube asymmetry)
        float output = -1.0f + std::exp(x);
        return output * 1.1f;  // Slight boost for grit
    }
}

float DualCoreDSP::saturateTape(float input, float drive)
{
    // Tape saturation with soft compression and subtle harmonics
    float x = input * drive;

    // Soft saturation curve
    float output = x / (1.0f + std::abs(x));

    // Add subtle odd harmonics (tape characteristic)
    float harmonic = 0.1f * std::sin(x * 3.0f) / (1.0f + std::abs(x * 3.0f));
    output += harmonic * (drive - 1.0f) / 19.0f;

    // Tape compression feel
    return output * 1.2f;
}

float DualCoreDSP::saturateHard(float input, float drive)
{
    // Hard clipping with slight rounding
    float x = input * drive;

    // Hard clip at +-1 with tiny soft knee
    if (x > 0.95f)
        return 0.95f + 0.05f * std::tanh((x - 0.95f) * 10.0f);
    else if (x < -0.95f)
        return -0.95f + 0.05f * std::tanh((x + 0.95f) * 10.0f);
    else
        return x;
}

float DualCoreDSP::saturateFuzz(float input, float drive)
{
    // Aggressive fuzz with octave-up harmonics
    float x = input * drive;

    // Rectification for octave-up effect
    float rectified = std::abs(x) * 0.3f;

    // Aggressive clipping
    float clipped = std::tanh(x * 2.0f);

    // Combine with some of the rectified signal
    float output = clipped * 0.7f + rectified * std::tanh(x);

    // Add grit
    if (std::abs(output) > 0.1f)
        output += 0.05f * std::sin(output * 10.0f);

    return juce::jlimit(-1.0f, 1.0f, output);
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
