# Dev Log — Phases 10-12

## Phase 10: Modernized EQ & Metering (2026-04-23)

### Implemented
- **10-Band Parametric EQ**: Interactive draggable nodes on a logarithmic frequency plot (20Hz–20kHz)
- **Filter Types**: Peaking, Low Pass, High Pass, Low Shelf, High Shelf, Notch — per band
- **EQ Node Inspector**: Floating popup for precision parameter entry (freq, gain, Q, type)
- **GR Meters**: Real-time gain reduction indicators for Compressor, Gate, Expander
- **VAD Meter**: Voice Activity Detection visualization for RNNoise

### Architecture
- `EQPlotComponent`: Renders frequency response curve + FFT spectrum overlay
- `EQNodeComponent`: Draggable node with freq/gain/Q mapped to APVTS params
- `EQNodeSettingsPanel`: Floating inspector with text fields for precise editing
- `DynamicsVisualizer`: Transfer function graph showing knee/ratio curves
- All DSP computation done via `juce::dsp::IIR::Coefficients::getMagnitudeForFrequency()` on UI thread (cached, throttled)

### Commits
- `328ad07` Phase 10: UI/UX Master Class
- `4ec6a70` Phase 10E: EQ Node Inspector

---

## Phase 11: Professional Analytics & Modular Presets (2026-04-25)

### Implemented
- **FFT Spectrum Analyzer**: 2048-point FFT engine with lock-free FIFO (zero audio thread allocation)
- **Dynamics Visualizers**: Bouncing-ball input/output meters for all dynamics modules
- **Modular Preset System**: Save/load individual effect settings (per-slot JSON files)

### Architecture
- `SpectrumAnalyzer`: Central engine — audio thread pushes via `pushSamples()`, UI calls `performAnalysis()`
- Lock-free `AbstractFifo` + pre-allocated `AudioBuffer` for thread-safe data transfer
- FFT output averaged with 0.8 smoothing factor for visual stability
- Module presets stored in `%APPDATA%/EasyEffects/ModulePresets/<typeId>/`

### Bug Fixes
- Fixed RNNoise VAD scaling (was showing 0/1, now shows proper 0–100% range)
- Fixed preset index-shifting during bulk chain clearing (while-loop instead of for-loop)
- Added serialization for Choice parameters and Mix levels in module presets

### Commits
- `0245467` Phase 11: Professional Analytics & Modular Presets
- `6ad6c9e` Docs: Update Phase 11 logs
- `d71931f` Fix: Global preset clearing bug

---

## Phase 12: UX Polish, Bypass & Preset Overhaul (2026-04-26)

### Implemented
- **Global Chain Bypass**: "Bypass All" header toggle — skips DSP chain but keeps FFT alive for A/B testing
- **Preset Workflow Overhaul**:
  - Active preset tracking (`currentPresetName` + dirty flag)
  - Save (overwrite active) / Save As (new name) / Delete (with async confirmation)
  - Preset name + `*` dirty indicator shown in header
- **EQ Presets**: Added "Presets" button to VisualEqualizerEditor with Save/Load/Delete
- **Module Preset Delete**: All per-module preset menus now support deletion
- **Monitor Toggle**: Button to switch audio output device for live testing (Speakers ↔ Virtual Cable)
- **FFT Bar Restored**: Global spectrum analyzer bar at top of editor

### Removed
- Broken footer meter (`LevelMeterEditor` showing -100 dB)
- Previously redundant global FFT bar (then restored by user request)

### Architecture
- `globalBypass`: `std::atomic<bool>` in processor; bypass in `processBlock` before `dspChain.process()`
- Monitor device switching via `StandalonePluginHolder::getInstance()->deviceManager`
- Monitor device name persisted in JUCE `PropertySet` settings
- All confirmation dialogs use `AlertWindow::showAsync` (no modal loops)

### Bug Fixes
- Fixed spectrum analyzer void below 50Hz (path now starts from left edge)
- Fixed preset overwrite (delete file before re-creating output stream)

### Commits
- `be2f231` Phase 12: UX Polish
- `7b35be3` Phase 12b: Restore FFT bar, Monitor toggle
- `2899281` Fix: Spectrum analyzer left edge fill
