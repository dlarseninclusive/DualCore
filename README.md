# DualCore

A dual multimode filter VST3 plugin with extensive modulation capabilities, built with JUCE. Inspired by the Sherman Filterbank.

![DualCore Screenshot](docs/screenshot.png)

## Features

### Dual Filters
- **Filter 1 & Filter 2**: Independent state-variable filters
- **Modes**: Low Pass, High Pass, Band Pass, Notch
- **Resonance**: Full range with self-oscillation capability
- **Routing**: Series or Parallel configuration

### Modulation
- **6-Slot Modulation Matrix**: Flexible routing with bipolar amounts
- **LFO1 & LFO2**: Dual LFOs with 6 waveforms each (Sine, Triangle, Square, Saw Up, Saw Down, Random)
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
- Preset save/load system
- Scalable UI (70% to 150%)
- Input and output level metering

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
│   └── DSP.md              # DSP implementation details
└── Source/
    ├── PluginProcessor.h/cpp   # Audio processing & parameters
    ├── PluginEditor.h/cpp      # User interface
    └── DualCoreDSP.h/cpp       # Core DSP engine
```

## Documentation

- [Roadmap](ROADMAP.md) - Development status and planned features
- [DSP Details](docs/DSP.md) - Technical DSP implementation

## License

Copyright (c) 2024 Dave Larsen. All rights reserved.
