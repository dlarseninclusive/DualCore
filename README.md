# DualCore

A dual multimode filter VST3 plugin with extensive modulation capabilities, built with JUCE.

## Features

### Dual Filters
- **Filter 1 & Filter 2**: Independent state-variable filters
- **Modes**: Low Pass, High Pass, Band Pass, Notch
- **Resonance**: Full range with self-oscillation capability
- **Routing**: Series or Parallel configuration

### Modulation
- **FM**: Filter 1 output modulates Filter 2 frequency
- **ADSR Envelope**: Input-triggered envelope with Attack, Decay, Sustain, Release
- **LFO**: 6 waveforms (Sine, Triangle, Square, Saw Up, Saw Down, Random)
- **AM**: Amplitude modulation from filter output

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
