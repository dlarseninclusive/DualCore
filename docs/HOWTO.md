# DualCore User Guide

## Overview

DualCore is a dual multimode filter plugin inspired by the Sherman Filterbank. It features two independent filters that can be routed in series or parallel, with extensive modulation capabilities and 6 classic analog filter type emulations.

## Interface Layout

The interface is divided into several sections:

### Top Bar
- **Preset selector** - Load saved presets
- **Save button** - Save current settings as a preset
- **+/-** buttons - Zoom the interface (70% to 200%)

### Row 1: Signal Path

#### INPUT Section
- **INPUT knob** - Input gain (-12dB to +24dB)
- **HI BOOST** - Activates a high-frequency shelf boost
- **HI CUT** - Activates a high-frequency cut
- **LIMIT** - Enables soft limiter to prevent clipping

#### FILTER 1 & FILTER 2 Sections
- **FREQ** - Filter cutoff frequency (20Hz - 20kHz)
- **RESO** - Resonance/Q (0-100%)
- **Mode dropdown** - LP (Low Pass), HP (High Pass), BP (Band Pass), NOTCH
- **Type dropdown** - Filter algorithm (see Filter Types below)

#### FM Section
- **FM knob** - Amount of Filter 1 output modulating Filter 2 frequency

#### DRIVE Section
- **DRIVE knob** - Saturation amount
- **Type dropdown** - Soft, Tube, Tape, Hard, Fuzz
- **POST toggle** - When on, drive is post-filter; when off, pre-filter

#### ROUTING Section
- **PARALLEL toggle** - When on, filters are parallel; when off, series
- **MIX knob** - Dry/wet mix

### Row 2: Modulation Sources

#### ENVELOPE Section
- **A** - Attack time
- **D** - Decay time
- **S** - Sustain level
- **R** - Release time
- **AMT** - Envelope amount (bipolar, affects filter frequency)
- **SENS** - Input sensitivity for triggering the envelope

#### LFO1 & LFO2 Sections
- **RATE** - LFO speed (free-running)
- **DEPTH** - Modulation depth
- **Waveform dropdown** - Sine, Tri, Sqr, Saw+, Saw-, Rnd
- **Target dropdown** (LFO1 only) - F1, F2, or Both
- **SYNC toggle** - Sync to host tempo
- **Division dropdown** - Note value when synced (1/4, 1/8, etc.)

#### AM Section
- **AM knob** - Amplitude modulation amount
- **A** - AM envelope attack
- **R** - AM envelope release

### Row 3: Modulation Matrix

6 modulation slots, each with:
- **Source dropdown** - None, LFO1, LFO2, Env, Input
- **Destination dropdown** - None, F1 Freq, F1 Reso, F2 Freq, F2 Reso, FM, Drive, LFO1 Rate, LFO2 Rate, Mix, AM
- **Amount slider** - Bipolar modulation amount (-100% to +100%)

### Row 4: Output

- **IN meter** - Input level
- **OUT meter** - Output level
- **Filter response display** - Real-time visualization of both filters

---

## Filter Types Explained

DualCore offers 6 filter type emulations, each with its own character:

### SVF (State Variable Filter)
- **Character**: Clean, precise, neutral
- **Best for**: Transparent filtering, surgical cuts, clean sweeps
- **12dB/octave** slope
- The default and most versatile option

### Ladder (Moog-style)
- **Character**: Warm, fat, round
- **Best for**: Bass, pads, classic synth sounds
- **24dB/octave** slope
- Classic "bass dip" at high resonance - the filter loses low end as resonance increases
- Soft saturation in the feedback path

### Diode (TB-303 style)
- **Character**: Acidic, sharp, buzzy
- **Best for**: Acid bass lines, aggressive leads, techno
- **24dB/octave** slope
- Sharper resonance peak than Ladder
- Less bass loss, more bite
- Asymmetric clipping for that classic acid sound

### MS-20 (Korg style)
- **Character**: Aggressive, screaming, industrial
- **Best for**: Aggressive leads, industrial sounds, experimental
- At high resonance, the filter can "scream" and fold
- Asymmetric feedback distortion
- Can become quite chaotic at extreme settings

### Steiner (Steiner-Parker style)
- **Character**: Vocal, rubbery, formant-like
- **Best for**: Vocal sounds, talking bass, unique textures
- Asymmetric resonance creates formant-like peaks
- Intermodulation between stages adds harmonic richness
- Works great with parallel routing

### OTA (80s Polysynth style)
- **Character**: Punchy, snappy, slightly gritty
- **Best for**: Poly synths, punchy basses, 80s sounds
- Fast attack response
- Subtle graininess at high resonance
- Classic CEM/SSM chip sound

---

## Routing Modes

### Series Mode (PARALLEL off)
Signal flows: Input → Filter 1 → Filter 2 → Output

- More dramatic filtering effect
- FM modulation creates complex frequency interactions
- Good for extreme filtering and complex textures

### Parallel Mode (PARALLEL on)
Signal flows: Input → Filter 1 ─┬─→ Mix → Output
                    Input → Filter 2 ─┘

- Filters operate independently on the same input
- Output is mixed 50/50
- Good for creating formant-like sounds
- Useful for subtle coloration

---

## Tips & Techniques

### Classic Acid Bass
1. Set both filters to **Diode** type
2. Filter 1: LP mode, low frequency (300-600 Hz), high resonance (70-85%)
3. Enable envelope with fast attack, short decay
4. High envelope amount (70%+)
5. Add some **Tube** or **Fuzz** drive

### Moog Bass Sweep
1. Set Filter 1 to **Ladder** type, LP mode
2. Low frequency, moderate resonance (50-65%)
3. Slow LFO (0.5 Hz) modulating filter frequency
4. Add gentle **Soft** drive for warmth

### Screaming Lead
1. Set Filter 1 to **MS-20** type
2. High resonance (85%+)
3. Enable HI BOOST
4. Fast envelope with high amount
5. Add **Hard** or **Fuzz** drive post-filter

### Vocal/Talking Effect
1. Use **Steiner** type on both filters
2. Set to BP (bandpass) mode
3. Parallel routing
4. Offset filter frequencies (600 Hz and 1800 Hz)
5. Modulate both with slow LFOs at different rates

### Dynamic Filtering
1. Use the **Input Follower** as modulation source
2. Route to filter frequency
3. The filter will respond to your playing dynamics
4. Combine with envelope for complex dynamic response

### Tempo-Synced Wobble
1. Enable SYNC on LFO1
2. Set division to 1/4 or 1/8
3. Square or Triangle wave
4. High depth modulating both filters
5. Works great with **Ladder** or **OTA** types

---

## Modulation Matrix Quick Reference

### Sources
| Source | Description |
|--------|-------------|
| None | No modulation |
| LFO1 | Low frequency oscillator 1 |
| LFO2 | Low frequency oscillator 2 |
| Env | ADSR envelope |
| Input | Input level follower |

### Destinations
| Destination | Description |
|-------------|-------------|
| None | No destination |
| F1 Freq | Filter 1 cutoff frequency |
| F1 Reso | Filter 1 resonance |
| F2 Freq | Filter 2 cutoff frequency |
| F2 Reso | Filter 2 resonance |
| FM | FM amount |
| Drive | Drive amount |
| LFO1 Rate | LFO1 speed |
| LFO2 Rate | LFO2 speed |
| Mix | Dry/wet mix |
| AM | Amplitude modulation |

---

## Preset Management

### Saving Presets
1. Dial in your sound
2. Click **Save**
3. Enter a name
4. Preset is saved to your presets folder

### Loading Presets
1. Click the preset dropdown
2. Select a preset
3. Settings are loaded immediately

### Preset Location
- **Linux**: `~/.config/DualCore/Presets/`
- **macOS**: `~/Library/Application Support/DualCore/Presets/`
- **Windows**: `%APPDATA%\DualCore\Presets\`

---

## Troubleshooting

### No Sound
- Check the MIX knob is not at 0%
- Ensure input gain is adequate
- Check filter frequencies aren't cutting all content

### Distorted Output
- Lower input gain
- Enable the LIMIT function
- Reduce drive amount
- Check resonance levels

### Filter Self-Oscillating
- This is normal at high resonance
- Reduce resonance if unwanted
- Use the limiter to tame extreme peaks

### CPU Usage High
- The oversampling in the drive section uses extra CPU
- Reduce drive amount if not needed
- MS-20 and Steiner types are slightly more CPU intensive
