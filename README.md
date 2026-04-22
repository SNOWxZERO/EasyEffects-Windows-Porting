# EasyEffects - Windows Port (Development Version)

This is a Windows-native port of the EasyEffects audio processing engine, rebuilt using JUCE for cross-platform compatibility and real-time stability on Windows/MSVC.

## Core Architecture
- **Engine**: JUCE-based audio processor.
- **DSP Hierarchy**: Modular `EffectModule` system with support for dynamic slot assignment, dry/wet mixing, and bypass.
- **Latency Management**: Automatic reporting of module-specific latencies to the host.

## Supported Effects

### Dynamics & Gain
- **Gain**: Standard output level control.
- **Compressor**: Soft/Hard knee dynamic range compression.
- **Gate**: Noise gate with hysteresis.
- **Expander**: Downward/Upward expansion.
- **Auto Gain**: Real-time LUFS estimation and normalization.
- **Limiter**: Brick-wall peak limiting.
- **Multiband Compressor**: 4th-order Linkwitz-Riley 3-band dynamics.
- **Multiband Gate**: Frequency-selective gating.

### EQ & Filters
- **Equalizer**: Multi-band parametric EQ.
- **Filter**: Standard LP/HP/BP/Shelf filters.
- **Convolver**: Impulse Response (IR) processing with zero-latency FFT.
- **Deesser**: Sibilance reduction.

### Enhancement & Spatial
- **Bass Enhancer**: Psychoacoustic bass reinforcement.
- **Exciter**: High-frequency harmonic enhancement.
- **Stereo Tools**: Width, pan, and phase control.
- **Reverb**: Algorithmic spatial simulation.
- **Delay**: Tempo-synced or millisecond delay.
- **Crystalizer**: Multiband transient enhancement.
- **Crossfeed**: Headphone spatialization.
- **Bass Loudness**: Frequency-dependent loudness correction.

### Modulation & Special
- **Chorus**: Multi-voice modulation.
- **Phaser**: All-pass modulation.
- **Noise Reduction (RNNoise)**: RNN-based neural noise suppression (Optimized for Windows).
- **Pitch Shift (SoundTouch)**: High-quality pitch transposition.

## Build Requirements
- **Compiler**: MSVC 2022 (v143).
- **Framework**: JUCE 7.x.
- **CMake**: 3.20+.

## Technical Notes (Windows Specific)
- **RNNoise**: Patched for MSVC compatibility (Replacing VLAs with stack allocation).
- **SoundTouch**: Integrated via submodule with custom CMake wrapper.
