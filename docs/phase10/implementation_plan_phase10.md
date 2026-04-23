# Implementation Plan - Phase 10: UI/UX Master Class & Visual EQ

This phase transforms EasyEffects from a slider-based utility into a professional-grade visual audio suite. The centerpiece is the **Visual Parametric Equalizer**, inspired by high-end apps like SteelSeries Sonar, featuring an interactive magnitude response graph and multi-type filters.

## User Review Required

> [!IMPORTANT]
> **Performance**: The Visual EQ requires calculating the frequency response of 10+ filters in real-time on the UI thread to draw the curve. I will optimize this using a lookup table to ensure the UI remains snappy.
>
> **Bands**: I propose a fixed **10-band layout** where each band can be individually enabled/disabled and assigned any filter type (High Pass, Low Pass, Peaking, etc.). This is more stable than dynamic band creation.

## Open Questions

- **Presets**: Should the "Save Preset" button save the state of *all* active slots, or just the currently selected effect? (Standard behavior is usually global).
- **VAD Metering**: For RNNoise, should the Voice Activity Detection meter be integrated directly into the generic control list?

---

## Proposed Changes

### 1. DSP: Parametric EQ Core
Refactor the current basic EQ into a flexible 10-band parametric engine.

#### [MODIFY] [EqualizerModule.h](file:///d:/Pc%20BackUp/Muhammad/Coding/EasyEffects/easyeffects-windows-v2/src/dsp/EqualizerModule.h)
- Increase `NUM_BANDS` to 10.
- Add support for per-band filter types (Enum: Peak, LowPass, HighPass, HighShelf, LowShelf, Notch).

#### [MODIFY] [EqualizerModule.cpp](file:///d:/Pc%20BackUp/Muhammad/Coding/EasyEffects/easyeffects-windows-v2/src/dsp/EqualizerModule.cpp)
- Update coefficient calculation to switch based on the `type` parameter.
- Add per-band bypass logic.

#### [MODIFY] [EffectRegistry.cpp](file:///d:/Pc%20BackUp/Muhammad/Coding/EasyEffects/easyeffects-windows-v2/src/dsp/EffectRegistry.cpp)
- Expand the EQ registration to include `bandX.type` (Choice) and `bandX.enabled` (Toggle).

### 2. UI: Visual EQ Editor (SteelSeries Style)
Create a rich, interactive canvas for EQ control.

#### [NEW] [EQPlotComponent.h/cpp](file:///d:/Pc%20BackUp/Muhammad/Coding/EasyEffects/easyeffects-windows-v2/src/ui/EQPlotComponent.h)
- Draw a logarithmic frequency grid (20Hz - 20kHz).
- Calculate the combined magnitude response of all enabled bands.
- Render the curve using a JUCE `Path`.

#### [NEW] [EQNodeComponent.h/cpp](file:///d:/Pc%20BackUp/Muhammad/Coding/EasyEffects/easyeffects-windows-v2/src/ui/EQNodeComponent.h)
- Draggable handles for each EQ band.
- Context-aware coloring and tooltips.

#### [NEW] [VisualEqualizerEditor.h/cpp](file:///d:/Pc%20BackUp/Muhammad/Coding/EasyEffects/easyeffects-windows-v2/src/ui/VisualEqualizerEditor.h)
- The container for the plot and the node controls.
- Dynamically inject this into the main window when the EQ slot is selected.

### 3. UI: Metering & Feedback
#### [MODIFY] [GenericModuleEditor.cpp](file:///d:/Pc%20BackUp/Muhammad/Coding/EasyEffects/easyeffects-windows-v2/src/ui/GenericModuleEditor.cpp)
- Detect if the effect is a Compressor or Gate.
- Add a vertical **Gain Reduction (GR) Meter** next to the controls.

### 4. System: Global Preset Management
#### [NEW] [PresetManager.h/cpp](file:///d:/Pc%20BackUp/Muhammad/Coding/EasyEffects/easyeffects-windows-v2/src/PresetManager.h)
- Logic to serialize/deserialize the APVTS and Slot List to JSON.
- Save files to `%APPDATA%/EasyEffects/Presets`.

---

## Verification Plan

### Automated Tests
- Verify that saving/loading a preset restores all 20+ effects correctly.
- Verify that 10-band EQ doesn't exceed 5% UI CPU usage.

### Manual Verification
- **EQ Interaction**: Drag nodes on the graph; verify that the DSP sound changes accordingly.
- **Node Type Switch**: Change a band from "Peaking" to "High Pass" and verify the visual curve update.
- **Dynamics Feedback**: Apply heavy compression and verify that the GR meter shows the reduction level.
