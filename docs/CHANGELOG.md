# Changelog - EasyEffects Windows

## [Phase 12] - UX Polish, Bypass & Preset Overhaul
### Added
- **Global Chain Bypass**: Toggle button to bypass the entire effect chain for A/B comparison.
- **Preset Workflow Overhaul**: Active preset tracking, Save (overwrite), Save As (new name), Delete.
- **Active Preset Label**: Header shows current preset name with `*` dirty indicator.
- **EQ Presets**: Parametric Equalizer now has its own Presets button with Save/Load/Delete.
- **Module Preset Delete**: All per-module preset menus now support deleting presets.
- **Audio Device Hint**: Footer shows reminder for I/O device selection.

### Removed
- **Broken Footer Meter**: Non-functional `-100 dB` level meter.
- **Redundant FFT Bar**: Global spectrum analyzer bar (FFT is in the EQ plot).

### Fixed
- **Preset Overwrite**: Saving now properly overwrites existing files.
- **Async Dialogs**: All confirmation dialogs use async patterns.

## [Phase 11] - Professional Analytics & Modular Presets
### Added
- **Real-Time FFT Spectrum Analyzer**: Integrated into the Visual EQ and global dashboard.
- **Dynamics Visualizers**: Interactive transfer function graphs for Compressor, Gate, and Expander.
- **Modular Presets**: Save/load individual effect settings independently of the global chain.
- **SpectrumAnalyzer Engine**: Centralized, lock-free FFT engine for high-performance visualization.

### Fixed
- **RNNoise Metering**: Corrected VAD scaling and reliability.
- **FFT Legacy Cleanup**: Removed redundant/messy FFT code from the processor.

## [Phase 10] - Modernized EQ & Metering
### Added
- **10-Band Parametric EQ**: Modern draggable nodes with selectable filter types.
- **Floating EQ Inspector**: Precision parameter entry popup.
- **Interactive Gain Reduction (GR) Meters**: Real-time feedback for all dynamics modules.
- **RNNoise VAD Meter**: Visual feedback for voice activity detection.

## [Phase 9] - Plugin Architecture & Preset System
### Added
- **Dynamic Slot System**: Add/remove/reorder effects in the DSP chain.
- **Global Preset Manager**: JSON-based saving and loading of the full chain.
- **Generic Module UI**: Auto-generated sliders for all 30+ effect types.
