# Phase 9: Dynamic Effect Chain & Missing Effects

## Overview

This phase transforms EasyEffects Windows from a **fixed effect chain** into a **dynamic, user-controlled chain** matching the original Linux app's behavior, and implements all missing audio effects.

---

## Part A: Dynamic Effect Chain Architecture

### Current Problem
All 13 effects are hardcoded in `PluginProcessor.cpp` constructor. All parameters for all effects are registered at startup. Users cannot add, remove, or reorder effects.

### Proposed Architecture

#### A1. Effect Registry (Factory Pattern)
Create an `EffectRegistry` that maps effect type IDs to factory lambdas:

**[NEW] `src/dsp/EffectRegistry.h`**
- Static registry mapping string IDs → factory functions
- Each factory returns `std::unique_ptr<EffectModule>`
- Each factory also returns parameter descriptors for dynamic APVTS registration
- Provides `getAvailableEffects()` for the "Add Effect" UI

#### A2. Dynamic EffectChain
**[MODIFY] `src/dsp/EffectChain.h/.cpp`**
- Add `insertModule(int position, std::unique_ptr<EffectModule>)` 
- Add `removeModule(int position)`
- Add `moveModule(int fromIndex, int toIndex)` for reordering
- Add `getModule(int index)` accessor
- Thread-safe swapping using double-buffering or lock-free queue

#### A3. Dynamic Parameter Management
**[MODIFY] `src/PluginProcessor.h/.cpp`**

> [!WARNING]
> **Major architectural constraint**: JUCE's `AudioProcessorValueTreeState` (APVTS) does **not** support adding/removing parameters after construction. This is a fundamental limitation.

**Solution options:**
1. **Pre-register all possible parameters** (current approach, extended) — Register parameters for N instances of each effect type (e.g., up to 2 compressors, 2 EQs). Simple but wastes memory.
2. **Use raw ValueTree** instead of APVTS for dynamic modules — More flexible but loses slider attachment convenience.  
3. **Rebuild processor on chain change** — Destroys and recreates the processor. Not viable for real-time audio.

**Recommended: Option 1 (Pre-register with instance slots)**
- Register parameters for up to 16 effect slots: `slot0.compressor.threshold`, `slot1.eq.band0.gain`, etc.
- Each slot maps to a position in the chain
- When user adds "Compressor" to slot 3, the UI reads from `slot3.compressor.*`

#### A4. UI Changes
**[MODIFY] `src/PluginEditor.h/.cpp`**
- Add "+ Add Effect" button above the sidebar (like original app)
- Add "Remove" option per sidebar row (right-click or button)
- Add up/down arrows for reordering
- Sidebar now shows only active effects, not all possible ones

**[MODIFY] `src/ui/SidebarRowCustomComponent.h`**
- Add reorder handle (drag or up/down arrows)
- Add remove/close button

---

## Part B: Missing Effects Inventory

### Comparison: Original vs. Our Implementation

| # | Effect | Original Dependency | Our Status | Implementation Strategy |
|---|--------|-------------------|------------|------------------------|
| 1 | Auto Gain | libebur128 | ❌ Missing | Use JUCE loudness measurement + gain adjustment |
| 2 | Bass Enhancer | Built-in | ✅ Done | — |
| 3 | Bass Loudness | MDA plugin | ❌ Missing | Port MDA bass loudness algorithm (simple equal-loudness curve) |
| 4 | Compressor | Built-in | ✅ Done | — |
| 5 | Convolver | Zita-convolver | ✅ Done | — |
| 6 | Crossfeed | libbs2b | ❌ Missing | Implement Bauer crossfeed filter (simple IIR) |
| 7 | Crusher | Built-in | ❌ Missing | Bit-depth reduction + sample rate decimation (pure DSP, no deps) |
| 8 | Crystalizer | Built-in | ❌ Missing | Harmonic enhancement via difference amplification (pure DSP) |
| 9 | De-esser | Built-in | ✅ Done | — |
| 10 | Delay | Built-in | ✅ Done | — |
| 11 | Deep Noise Remover | DeepFilterNet | ❌ Missing | **Defer** — requires ML inference runtime (ONNX/TensorRT) |
| 12 | Echo Canceller | SpeexDSP | ❌ Missing | **Defer** — requires speexdsp library, complex AEC algorithm |
| 13 | Equalizer | Built-in | ✅ Done | — |
| 14 | Exciter | Built-in | ✅ Done | — |
| 15 | Expander | Built-in | ❌ Missing | Inverse compressor using JUCE `dsp::Compressor` with ratio < 1 |
| 16 | Filter | Built-in | ✅ Done | — |
| 17 | Gate | Built-in | ✅ Done | — |
| 18 | Level Meter | libebur128 | ✅ Done | — |
| 19 | Limiter | Built-in | ✅ Done | — |
| 20 | Loudness | libebur128 | ❌ Missing | JUCE-based LUFS measurement + gain compensation |
| 21 | Maximizer | ZamAudio | ❌ Missing | Look-ahead limiter + makeup gain (pure DSP) |
| 22 | Multiband Compressor | LSP plugins | ❌ Missing | Linkwitz-Riley crossover + per-band JUCE compressors |
| 23 | Multiband Gate | LSP plugins | ❌ Missing | Linkwitz-Riley crossover + per-band gates |
| 24 | Noise Reduction | RNNoise | ❌ Missing | Bundle RNNoise C library (MIT license, ~100KB) |
| 25 | Pitch Shift | SoundTouch | ❌ Missing | Bundle SoundTouch library (LGPL, well-supported on Windows) |
| 26 | Reverberation | Built-in | ✅ Done | — |
| 27 | Speech Processor | SpeexDSP | ❌ Missing | **Defer** — AGC + noise suppression from speexdsp |
| 28 | Stereo Tools | Built-in | ❌ Missing | Mid/Side encoding, stereo width, balance (pure DSP) |

### Summary
- **Already implemented:** 13/28
- **Pure DSP (no dependencies):** 8 more (Crusher, Crystalizer, Crossfeed, Expander, Stereo Tools, Maximizer, Bass Loudness, Loudness/Auto Gain)
- **Need external libraries:** 3 (RNNoise, SoundTouch, Multiband needs Linkwitz-Riley)
- **Deferred (complex ML/AEC):** 3 (Deep Noise Remover, Echo Canceller, Speech Processor)

---

## Part C: Implementation Order

### Phase 9A: Dynamic Chain Infrastructure
1. Create `EffectRegistry` with factory pattern
2. Pre-register parameter slots (16 slots)
3. Modify `EffectChain` for insert/remove/reorder
4. Add "+ Add Effect" UI and remove buttons
5. Serialize active chain to presets

### Phase 9B: Pure DSP Effects (No Dependencies)
Priority order (most useful for voice/music first):
1. **Stereo Tools** — Simple and universally useful
2. **Expander** — Inverse compressor, almost trivial
3. **Crusher** — Bit crush + decimation, fun effect
4. **Crystalizer** — Harmonic enhancement
5. **Crossfeed** — Bauer stereophonic filter for headphones
6. **Maximizer** — Look-ahead limiter with makeup gain
7. **Bass Loudness** — Equal-loudness contour filter
8. **Loudness (Auto Gain)** — LUFS-based automatic gain

### Phase 9C: External Library Effects
1. **RNNoise** (Noise Reduction) — Bundle the C library directly
2. **SoundTouch** (Pitch Shift) — Link via CMake `FetchContent`
3. **Multiband Compressor** — Build Linkwitz-Riley crossover from JUCE IIR filters
4. **Multiband Gate** — Same crossover infrastructure

### Phase 9D: Deferred (Future)
- Deep Noise Remover (needs ONNX inference)
- Echo Canceller (needs full AEC pipeline)
- Speech Processor (AGC + VAD + noise suppression)

---

## Open Questions

> [!IMPORTANT]
> **Dynamic parameters vs. pre-registration**: Should we pre-register 16 effect slots (simpler but registers ~500 unused parameters), or move to raw `ValueTree` management (more complex but cleaner)?

> [!IMPORTANT]  
> **Maximum chain length**: How many simultaneous effects should we support? The original app allows unlimited. Suggest capping at 16 for the pre-registration approach.

> [!IMPORTANT]
> **External libraries**: RNNoise and SoundTouch need to be bundled. Should we use CMake `FetchContent` to auto-download them, or vendor them as git submodules?

---

## Verification Plan

### Phase 9A Verification
- Add 3 effects via UI, verify DSP chain processes in order
- Remove middle effect, verify chain re-links correctly
- Reorder effects, verify processing order changes
- Save/load preset with custom chain, verify it restores

### Phase 9B Verification  
- Each new effect: bypass on/off produces audible difference
- Stereo Tools: verify mono collapse and width expansion
- Crusher: verify audible bit reduction artifacts
- Compare output against reference (original EasyEffects with same settings)
