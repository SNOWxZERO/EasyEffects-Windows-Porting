# Phase 9B: Pure DSP Effects Implementation Plan

This phase focuses on implementing 8 additional audio effects that rely on standard DSP algorithms without external library dependencies.

## Proposed Effects

### 1. Stereo Tools
A utility for manipulating stereo image and channel properties.
- **DSP Logic**: Custom math for M/S processing, balance, and width. Sample-based delay for Haas effect.
- **Parameters**: 
    - `balance_in` [-100, 100] %
    - `balance_out` [-100, 100] %
    - `width` [0.0, 2.0] (0 = mono, 1 = normal, 2 = extra wide)
    - `delay` [0.0, 20.0] ms
    - `mute_l`, `mute_r` [Bool]
    - `invert_l`, `invert_r` [Bool]
    - `side_level` [-24.0, 24.0] dB
    - `mid_level` [-24.0, 24.0] dB
    - `mode` [Stereo, LR->LL, LR->RR, LR->RL, LR->L+R, Mid-Side]

### 2. Expander
Dynamic range processor that attenuates signals below a threshold.
- **DSP Logic**: Custom envelope follower with ratio/threshold logic.
- **Parameters**: Threshold, Ratio, Attack, Release, Knee, Makeup Gain.

### 3. Crusher (Bitcrusher)
Digital distortion effect.
- **DSP Logic**: Sample rate reduction and bit depth reduction.
- **Parameters**: Bit Depth, Downsampling, Jitter.

### 4. Maximizer
A look-ahead limiter optimized for "loudness".
- **DSP Logic**: `juce::dsp::Limiter` with auto-makeup and soft-clipping.
- **Parameters**: Threshold, Release, Ceiling, Soft Clip.

### 5. Crossfeed
Simulates speaker listening on headphones.
- **DSP Logic**: Linkwitz or Bauer crossfeed filter (Low-shelf cross-mixing).
- **Parameters**: Strength, Cutoff Frequency.

### 6. Crystalizer
Granular pitch-shifting delay.
- **DSP Logic**: Rotating buffers with varying playback speeds.
- **Parameters**: Pitch, Size, Density, Feedback.

### 7. Bass Loudness
Psychoacoustic bass enhancement.
- **DSP Logic**: Harmonic generation of low frequencies.
- **Parameters**: Amount, Frequency, Harmonics.

### 8. Auto Gain / Loudness
Automatic volume leveler.
- **DSP Logic**: LUFS-based gain tracking.
- **Parameters**: Target Loudness, Range, Speed.

## Implementation Steps

### Step 1: Update EffectRegistry
Add the descriptors for these new effects so they are selectable in the UI.

### Step 2: Implement DSP Modules
Create new `EffectModule` subclasses for each effect in `src/dsp/`.

### Step 3: UI Integration
Ensure `GenericModuleEditor` correctly renders the new parameters.

## Verification Plan
- **Manual Audio Test**: Verify each effect works and parameters respond as expected.
- **UI Test**: Ensure "+ Add Effect" menu shows all new items.
- **Persistence**: Save/Load presets with new effects.
