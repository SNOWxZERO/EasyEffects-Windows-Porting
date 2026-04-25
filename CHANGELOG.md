# Changelog - EasyEffects Windows

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
