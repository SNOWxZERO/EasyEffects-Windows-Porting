# AI Context — Compacted Conversation Summary

> Last updated: 2026-04-26 (Phase 12 complete)
> This file replaces raw chat dumps. Dense, structured, zero fluff.

---

## Project: EasyEffects Windows Port (V2)
**Goal:** Windows-native audio processor inspired by EasyEffects (Linux/PipeWire).
**Stack:** C++20 · JUCE 8 · MSVC 2026 · CMake · JSON presets

---

## Architecture (Final State)

```
[Microphone/VAC Input]
       ↓
  PluginProcessor (juce::AudioProcessor)
       ↓
  EffectChain (dynamic slots, max 8)
    ├─ Slot 0: [Effect Module] ← bypass/enable per slot
    ├─ Slot 1: [Effect Module]
    └─ ...
       ↓
  SpectrumAnalyzer (lock-free FIFO → UI)
       ↓
  [Speakers/VAC Output]
```

**Threading rule:** Audio thread = zero allocation. UI polls atomics + FIFO at 60Hz.

---

## Key Components

| Component | File | Purpose |
|-----------|------|---------|
| `PluginProcessor` | `PluginProcessor.h/cpp` | DSP host, APVTS, chain management |
| `EffectChain` | `dsp/EffectChain.h/cpp` | Slot-based module chain |
| `EffectRegistry` | `dsp/EffectRegistry.h/cpp` | Static registry of all 25+ effect types + parameter layouts |
| `PresetManager` | `PresetManager.h/cpp` | Global + module presets, active tracking, dirty state |
| `SpectrumAnalyzer` | `dsp/SpectrumAnalyzer.h` | 2048-pt FFT, AbstractFifo, display smoothing |
| `PluginEditor` | `PluginEditor.h/cpp` | Sidebar list, viewport, header controls |
| `VisualEqualizerEditor` | `ui/VisualEqualizerEditor.h/cpp` | 10-band parametric EQ with drag nodes |
| `GenericModuleEditor` | `ui/GenericModuleEditor.h/cpp` | Auto-generated slider UI for any effect |
| `EQPlotComponent` | `ui/EQPlotComponent.h/cpp` | Frequency response curve + spectrum overlay |

---

## All 25+ Effects (Implemented)

Gain · Compressor · Gate · EQ (10-band parametric) · Limiter · Filter · Delay · Reverb · Exciter · Bass Enhancer · Deesser · Convolver · Level Meter · Stereo Tools · Expander · Crusher · Maximizer · Crossfeed · Bass Loudness · Crystalizer · Auto Gain · Multiband Compressor · Multiband Gate · Chorus · Phaser · RNNoise · Pitch Shift

---

## Phase History (Compact)

| Phase | What | Key Commits |
|-------|------|-------------|
| 0 | Environment: JUCE + MSVC + CMake | `git init` |
| 1-2 | Basic audio pipeline + Gain slider | `[PHASE 1/2]` |
| 3 | Modular EffectChain + Compressor | `[PHASE 3]` |
| 4-5 | UI sidebar + preset system + APVTS | `[PHASE 4B]` |
| 6 | VAC routing docs | `[PHASE 6]` |
| 7-8 | GTK-style theme, FFT, bypass buttons | `[PHASE 7/8]` |
| 9 | Dynamic chain: add/remove/reorder + 15 new effects | `[PHASE 9A/9B]` |
| 9C | RNNoise + PitchShift (external libs) | `b3522f4` |
| 10 | Visual parametric EQ + drag nodes + inspector | `328ad07` |
| 11 | FFT spectrum analyzer + dynamics viz + module presets | `0245467` |
| 12 | Global bypass, preset overhaul, monitor toggle, EQ presets | `be2f231..2899281` |

---

## Solved Bugs (Reference)

1. **Slot index shifting** — `removeEffect` compacts array → use while-loop for bulk clear
2. **RNNoise scaling** — VAD was 0/1 binary → applied soft-gating + smoothing
3. **Spectrum void <50Hz** — FFT bins sparse at low freq on log scale → extend path to left edge
4. **Preset overwrite** — `FileOutputStream` appends → delete file before writing
5. **Choice params missing from presets** — Added `desc->choices` loop in serialization
6. **Mix level missing from presets** — Added `prefix + ".mix"` to save/load

---

## File Locations

- **Presets:** `%APPDATA%/EasyEffects/Presets/*.json`
- **Module Presets:** `%APPDATA%/EasyEffects/ModulePresets/<typeId>/*.json`
- **Build Output:** `build/EasyEffects_artefacts/Release/Standalone/EasyEffects.exe`
- **VST3:** `build/EasyEffects_artefacts/Release/VST3/EasyEffects.vst3`

---

## External Dependencies

| Lib | Location | Purpose |
|-----|----------|---------|
| JUCE 8 | `../JUCE` | Audio framework |
| rnnoise | `external/rnnoise` | Noise reduction |
| SoundTouch | `external/soundtouch` | Pitch shifting |
