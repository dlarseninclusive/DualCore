# DualCore

A dual multimode filter VST3 plugin featuring 6 classic analog filter emulations with extensive modulation capabilities. Built with JUCE.

Inspired by legendary hardware including the Sherman Filterbank, Moog ladder filters, Roland TB-303, Korg MS-20, Steiner-Parker Synthacon, and classic 80s polysynth OTA designs.

![DualCore Screenshot](docs/screenshot.png)

## Features

### Dual Filters with Classic Analog Types
- **Filter 1 & Filter 2**: Independent filters with selectable types
- **6 Filter Types**:
  - **SVF** - Clean state-variable filter (default) - precise and articulate
  - **Ladder** - Moog-style transistor ladder - warm, fat, classic bass loss at resonance
  - **Diode** - TB-303 style diode ladder - acidic, sharp, buzzy
  - **MS-20** - Korg MS-20 style - aggressive, screaming at high resonance
  - **Steiner** - Steiner-Parker style - vocal, formant-like, rubbery
  - **OTA** - 80s polysynth style (CEM/SSM) - punchy, snappy
- **4 Filter Modes**: Low Pass, High Pass, Band Pass, Notch
- **Resonance**: Full range with self-oscillation capability
- **Routing**: Series or Parallel configuration

### Modulation
- **6-Slot Modulation Matrix**: Flexible routing with bipolar amounts
- **LFO1 & LFO2**: Dual LFOs with 6 waveforms each (Sine, Triangle, Square, Saw Up, Saw Down, Random)
- **Tempo Sync**: Both LFOs can sync to host tempo with note divisions
- **ADSR Envelope**: Input-triggered envelope with Attack, Decay, Sustain, Release
- **Input Follower**: Envelope follower for dynamic modulation
- **FM**: Filter 1 output modulates Filter 2 frequency
- **AM**: Amplitude modulation from filter output

### Drive
- 5 saturation modes: Soft, Tube, Tape, Hard, Fuzz
- Pre or post-filter positioning
- 2x oversampling to reduce aliasing

### Input Section
- Input gain control (-12dB to +24dB)
- Hi Boost (high shelf)
- Hi Cut (low shelf)
- Soft limiter

### Additional Features
- **Preset System**: Save/load presets with factory presets included
- **Scalable UI**: 70% to 200% scaling
- **Real-time Filter Display**: Visual frequency response showing both filters
- **Input/Output Metering**: Level meters for signal monitoring

## Factory Presets

DualCore includes factory presets showcasing the different filter types:

| Preset | Filter Types | Description |
|--------|--------------|-------------|
| Moog Ladder Bass | Ladder + Ladder | Classic warm Moog-style bass |
| Acid 303 | Diode + Diode | TB-303 inspired acid sounds |
| MS20 Screamer | MS-20 + MS-20 | Aggressive screaming filter |
| Steiner Vowel | Steiner + Steiner | Formant-like vocal sounds |
| OTA Punch | OTA + OTA | Snappy 80s polysynth character |
| Hybrid Acid MS20 | Diode + MS-20 | Mixed acid/aggressive combo |

## Building

### Requirements
- CMake 3.15+
- C++17 compiler
- JUCE (fetched automatically)

### Build Steps

```bash
cd DualCore
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

The VST3 plugin will be installed to your system's VST3 folder.

## Project Structure

```
DualCore/
├── CMakeLists.txt          # Build configuration
├── README.md               # This file
├── ROADMAP.md              # Development roadmap
├── docs/                   # Additional documentation
│   ├── DSP.md              # DSP implementation details
│   └── HOWTO.md            # User guide
├── Presets/                # Factory presets
└── Source/
    ├── PluginProcessor.h/cpp   # Audio processing & parameters
    ├── PluginEditor.h/cpp      # User interface
    ├── DualCoreDSP.h/cpp       # Core DSP engine
    ├── CustomLookAndFeel.h/cpp # UI styling
    └── FilterResponseDisplay.h/cpp # Filter visualization
```

## Documentation

- [How To Use DualCore](docs/HOWTO.md) - User guide and tips
- [Roadmap](ROADMAP.md) - Development status and planned features
- [DSP Details](docs/DSP.md) - Technical DSP implementation

## Changelog

### v1.1.0
- Added 6 analog filter type emulations (SVF, Ladder, Diode, MS-20, Steiner, OTA)
- New factory presets showcasing filter types
- Improved UI readability with larger fonts
- Filter response display now shows type-specific curves
- DSP stability improvements

### v1.0.0
- Initial release with dual SVF filters
- 6-slot modulation matrix
- Dual LFOs with tempo sync
- 5 drive types
- Preset system

## License

Copyright (c) 2024 Dave Larsen. All rights reserved.
