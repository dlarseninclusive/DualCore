# DualCore Development Roadmap

## Current Status: v0.1.0 - Initial Implementation

### Completed Features

- [x] **Core DSP Engine**
  - [x] Dual state-variable filters (Chamberlin SVF)
  - [x] Four filter modes: LP, HP, BP, Notch
  - [x] Resonance with self-oscillation
  - [x] Series/Parallel routing

- [x] **Modulation System**
  - [x] FM modulation (Filter 1 -> Filter 2)
  - [x] ADSR envelope with input triggering
  - [x] LFO with 6 waveforms
  - [x] AM (amplitude modulation)

- [x] **Input Section**
  - [x] Input gain control
  - [x] Hi Boost shelf filter
  - [x] Hi Cut shelf filter
  - [x] Soft limiter

- [x] **User Interface**
  - [x] Dark theme UI
  - [x] Rotary knobs for all parameters
  - [x] Scalable UI (zoom 70%-150%)
  - [x] Input/Output metering

- [x] **Preset System**
  - [x] Save presets
  - [x] Load presets
  - [x] Preset browser dropdown

---

## Phase 2: Refinement (Planned)

### DSP Improvements
- [ ] Oversampling for aliasing reduction
- [ ] Zero-delay feedback (ZDF) filter topology
- [ ] More accurate analog modeling
- [ ] Drive/saturation stage with multiple modes

### UI Enhancements
- [ ] Custom knob graphics
- [ ] Frequency response visualization
- [ ] Envelope visualization
- [ ] Tooltips for parameters

### Modulation Expansion
- [ ] LFO sync to host tempo
- [ ] Additional LFO shapes
- [ ] Modulation matrix
- [ ] Side-chain envelope follower input

---

## Phase 3: Polish (Future)

### Factory Presets
- [ ] Create factory preset bank
- [ ] Categorized preset browser
- [ ] Preset tagging system

### Advanced Features
- [ ] MIDI learn for parameters
- [ ] Undo/redo system
- [ ] A/B comparison
- [ ] Parameter locking

### Performance
- [ ] SIMD optimization
- [ ] CPU usage improvements
- [ ] Multi-threading for heavy processing

---

## Known Issues

- None currently documented

---

## Version History

### v0.1.0 (Current)
- Initial implementation
- Basic dual filter with modulation
- Preset save/load functionality
- Scalable UI
