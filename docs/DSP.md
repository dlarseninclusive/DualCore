# DualCore DSP Implementation

## Overview

DualCore uses a dual state-variable filter architecture with extensive modulation capabilities. The signal flow is designed for both musical and experimental use.

## Signal Flow

```
Input -> Input Gain -> [Hi Boost] -> [Hi Cut] -> [Limiter]
                                                     |
                                                     v
                                          [Drive PRE-FILTER]
                                                     |
                                                     v
                              +--------------------+
                              |   Filter Routing   |
                              |  (Series/Parallel) |
                              +--------------------+
                                        |
           +----------------------------+----------------------------+
           |                                                         |
           v (Series)                                    v (Parallel)
    +-------------+                               +-------------+
    |  Filter 1   |                               |  Filter 1   |
    +-------------+                               +-------------+
           |                                              |
           | (+ FM mod)                                   |
           v                                              |
    +-------------+                               +-------------+
    |  Filter 2   |                               |  Filter 2   |
    +-------------+                               +-------------+
           |                                              |
           v                                              v
         Output                                    Mix(F1 + F2)
           |                                              |
           +---------------------+------------------------+
                                 |
                                 v
                         [Drive POST-FILTER]
                                 |
                                 v
                          [AM Modulation]
                                 |
                                 v
                            Dry/Wet Mix
                                 |
                                 v
                              Output
```

Note: Drive stage position (pre/post filter) is selectable via the POST toggle.

## Filter Implementation

### State Variable Filter (Chamberlin)

The filters use the Chamberlin state variable filter algorithm, which provides simultaneous LP, HP, BP, and Notch outputs:

```cpp
lowpass += f * bandpass;
highpass = input - lowpass - q * bandpass;
bandpass += f * highpass;
notch = highpass + lowpass;
```

Where:
- `f = 2 * sin(π * frequency / sampleRate)` (frequency coefficient)
- `q = 1 - resonance * 0.99` (damping coefficient)

### Resonance

Resonance is mapped from 0.0-1.0 to control the Q factor. At maximum resonance, the filter approaches self-oscillation.

## Modulation Sources

### ADSR Envelope

Input-triggered ADSR envelope for filter frequency modulation:

1. **Trigger Detection**: Input level crosses sensitivity threshold
2. **Attack**: Linear ramp to peak (0 to 1)
3. **Decay**: Linear decay to sustain level
4. **Sustain**: Held while input above threshold
5. **Release**: Triggered when input falls below threshold

### LFO1 & LFO2

Two independent LFOs with six waveform options each:
- **Sine**: `sin(phase * 2π)`
- **Triangle**: `4 * |phase - 0.5| - 1`
- **Square**: `phase < 0.5 ? 1 : -1`
- **Saw Up**: `2 * phase - 1`
- **Saw Down**: `1 - 2 * phase`
- **Random**: Smoothly interpolated random values

LFO1 has a legacy "Target" selector for quick routing to Filter 1 Freq, Filter 2 Freq, or Both. Both LFOs are available as sources in the modulation matrix.

### Input Follower

Envelope follower tracking the input signal level. Uses attack/release smoothing (shared with AM section) to provide a dynamic modulation source.

## Modulation Matrix

A 6-slot modulation matrix allows flexible routing of modulation sources to destinations.

### Sources

| Source | Description |
|--------|-------------|
| None | No modulation |
| LFO1 | Low frequency oscillator 1 output (-1 to +1) |
| LFO2 | Low frequency oscillator 2 output (-1 to +1) |
| Envelope | ADSR envelope output (0 to 1) |
| Input | Input follower / envelope follower (0 to 1) |

### Destinations

| Destination | Mod Range | Description |
|-------------|-----------|-------------|
| F1 Freq | ±8000 Hz | Filter 1 cutoff frequency |
| F1 Reso | ±0.5 | Filter 1 resonance |
| F2 Freq | ±8000 Hz | Filter 2 cutoff frequency |
| F2 Reso | ±0.5 | Filter 2 resonance |
| FM | ±1.0 | FM modulation amount |
| Drive | ±1.0 | Drive/saturation amount |
| LFO1 Rate | ±10 Hz | LFO1 frequency |
| LFO2 Rate | ±10 Hz | LFO2 frequency |
| Mix | ±1.0 | Dry/wet mix |
| AM | ±1.0 | Amplitude modulation amount |

### Slot Configuration

Each slot has:
- **Source**: Modulation source selector
- **Dest**: Destination parameter selector
- **Amount**: Bipolar modulation amount (-100% to +100%)

Multiple slots can target the same destination - modulation values are summed before being applied. All modulation is computed per-sample for smooth results.

### FM Modulation

Filter 1 output is used to modulate Filter 2's frequency:
```
Filter2_freq = base_freq + (Filter1_output * FM_amount * 4000Hz)
```

## Input Section

### Shelf Filters

High and low shelf filters for input conditioning:
- **Hi Boost**: +6dB shelf at 3kHz
- **Hi Cut**: -12dB shelf at 8kHz

### Soft Limiter

Soft-knee limiter to prevent clipping:
```cpp
if (|input| > threshold)
    output = threshold + knee * tanh((|input| - threshold) / knee)
```

## Drive/Saturation

The drive stage provides five saturation algorithms, selectable for pre- or post-filter positioning. All processing uses 2x oversampling to reduce aliasing artifacts.

### Drive Types

| Type | Character | Algorithm |
|------|-----------|-----------|
| **Soft** | Gentle warmth | `tanh(input * drive)` - smooth symmetric clipping |
| **Tube** | Asymmetric warmth | Asymmetric soft clipping with even harmonics |
| **Tape** | Vintage compression | Tape-style saturation with soft compression |
| **Hard** | Aggressive edge | Hard clipping at threshold |
| **Fuzz** | Extreme distortion | Waveshaping with aggressive harmonic content |

### Oversampling

2x polyphase IIR oversampling is applied during drive processing to reduce aliasing:
- Upsample input signal
- Apply saturation at higher sample rate
- Downsample with anti-aliasing filter

## Parameters

### Input & Filters

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| Input Gain | -12 to +24 dB | 0 dB | Pre-filter gain |
| Filter 1/2 Freq | 20 to 20000 Hz | 1000/2000 Hz | Cutoff frequency |
| Filter 1/2 Reso | 0 to 1 | 0.5 | Resonance/Q |
| FM Amount | 0 to 1 | 0 | FM modulation depth |

### Envelope

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| Env Attack | 0.1 to 1000 ms | 10 ms | Envelope attack time |
| Env Decay | 1 to 2000 ms | 100 ms | Envelope decay time |
| Env Sustain | 0 to 1 | 0.7 | Envelope sustain level |
| Env Release | 1 to 3000 ms | 200 ms | Envelope release time |
| Env Amount | -1 to +1 | 0 | Legacy envelope mod depth |
| Env Sens | 0 to 1 | 0.5 | Trigger sensitivity |

### LFO1

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| LFO1 Rate | 0.01 to 20 Hz | 1 Hz | LFO1 frequency |
| LFO1 Depth | 0 to 1 | 0 | Legacy LFO1 mod depth |
| LFO1 Wave | 6 options | Sine | LFO1 waveform |
| LFO1 Target | Off/F1/F2/Both | Off | Legacy quick routing |

### LFO2

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| LFO2 Rate | 0.01 to 20 Hz | 1 Hz | LFO2 frequency |
| LFO2 Depth | 0 to 1 | 0 | LFO2 depth (for matrix) |
| LFO2 Wave | 6 options | Sine | LFO2 waveform |

### Modulation Matrix (x6 slots)

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| Mod Source | None/LFO1/LFO2/Env/Input | None | Modulation source |
| Mod Dest | None + 10 destinations | None | Modulation destination |
| Mod Amount | -100% to +100% | 0% | Bipolar mod amount |

### AM & Drive

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| AM Amount | 0 to 1 | 0 | Amplitude modulation depth |
| AM Attack | 0.1 to 100 ms | 5 ms | Input follower attack |
| AM Release | 1 to 500 ms | 50 ms | Input follower release |
| Drive Amount | 0 to 1 | 0 | Saturation intensity |
| Drive Type | Soft/Tube/Tape/Hard/Fuzz | Soft | Saturation algorithm |
| Drive Post | On/Off | On | Pre or post-filter position |

### Routing

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| Routing | Series/Parallel | Series | Filter routing mode |
| Mix | 0 to 100% | 100% | Dry/wet mix |
