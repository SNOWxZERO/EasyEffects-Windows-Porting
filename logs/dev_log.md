# Development Log

## Initialization (Pre-Phase 0)
* Project structure and initial technical analysis requested.
* Created architecture designs and roadmap documents.
* *Update:* User requested a pivot to **JUCE** and **MSVC**.
* Drafted `/docs/juce_architecture.md` outlining the `juce::AudioProcessor` and APVTS paradigm.
* Re-drafted `/docs/roadmap.md` into 6 distinct stages focusing on building a reliable standalone audio host before worrying about system-wide routing.
* Logged decision in `logs/decisions.md`.
* Awaiting explicit approval on the implementation plan before Phase 0 coding begins.

## Phase 0: Environment Setup
* Configured `CMakeLists.txt` using `juce_add_plugin`.
* Disabled VST2 compatibility flags to allow clean MSVC build.
* Output `EasyEffects_Standalone.exe` successfully.
* Git repository initialized.

## Phase 1 & 2: Basic Audio Pipeline and Minimal DSP
* Implemented `juce::AudioProcessorValueTreeState` to manage parameters across thread boundaries.
* Added standard `-24dB` to `24dB` Gain slider parameter.
* Added `juce::Slider` in the UI thread mapped to the APVTS parameter layout.
* Linked `juce::dsp::Gain` processor into the `processBlock` and parameterized via the APVTS tree.
* Compiled successfully.

## Phase 3: Modular DSP System
* Abstracted the DSP code into an `eeval::EffectModule` base interface (`prepare`, `process`, `reset`, `updateParameters`).
* Implemented `eeval::GainModule` as the first polymorphic effect block.
* Implemented `eeval::EffectChain` which internally manages an ordered vector of effect modules.
* Replaced the hardcoded Gain hook in the audio processor with the dynamic `EffectChain`.
* Compiled successfully without performance regressions.

## Phase 4 & 5: UI Overhaul and Preset System
* Restructured the PluginEditor from a flat slider dump into a sidebar-navigated layout.
* Implemented `juce::ListBoxModel` for a sidebar listing all loaded DSP modules dynamically.
* Added swappable editor views — clicking "Compressor" in sidebar shows compressor knobs, "Gain" shows gain slider.
* Centralized UI state via `selectedEditorIndex` stored in the AudioProcessor (survives editor recreation).
* Implemented `getStateInformation` / `setStateInformation` for full APVTS XML serialization.
* Added "Save Preset" and "Load Preset" buttons in the top bar using async `juce::FileChooser`.
* Presets store full APVTS state + `selectedEditorIndex` for future module chain metadata.
* Added auto-save on exit: `~Editor()` writes to `%APPDATA%/EasyEffectsWindows/autosave.xml`.
* Added auto-load on startup: constructor reads `autosave.xml` if present and restores full state.

## Phase 3B: Core Effects Implementation (Batch 1 & 2)
* Refactored `EffectModule` to internalize dry/wet Mix and Bypass logic completely, hiding it from subclasses.
* Subclasses now only implement `processInternal()`, enforcing real-time safety.
* Enforced `<module>.<param>` naming standard for APVTS mappings.
* Implemented `GateModule`, `EqualizerModule`, `LimiterModule`, `FilterModule`, `DelayModule`, `ReverbModule`.
* Implemented `ExciterModule`, `BassEnhancerModule`, `DeesserModule` with `LinkwitzRileyFilter` routing.
* Implemented `ConvolverModule`.
* Implemented `LevelMeterModule` with lock-free atomics and exponential smoothing curves.
* Fixed chain execution order established.

## Phase 4B: Full Custom UI Creation
* **Theme System:** Developed `Theme.h` passing a Catppuccin-esque modern dark layout into `juce::LookAndFeel_V4` globally. Replaces old default grey knobs with neon-blue active flat rotary dials and sleek layouts.
* **Module Descriptors:** Replaced ugly string prefix scanning with a hardcoded struct (`ModuleDescriptors.h`) supplying Label, Unit, and Parameter IDs safely.
* **Dynamic Content Engine:** Developed `GenericModuleEditor.h` which accepts a DSP module ID, calls the descriptor, and spawns, binds, and places sliders automatically into a dynamic row-based responsive grid layout. 
* **Lifecycle State Validation:** Fixed `juce::SliderAttachment` memory leaks by wrapping the arrays directly in `juce::OwnedArray` tied to the lifecycle of the GenericModuleEditor.
* **LevelMeter Custom UI:** Developed specialized `LevelMeterEditor.h` drawing accurate graphical volume bars running via an asynchronous decoupled `juce::Timer` pulling the lock-free atomics at 30Hz natively.
* **PluginEditor Core Refactor:** Re-skinned the core shell, adding deep dark colors, proper Header spacing, async `FileChooser` operations for preset management, and a `juce::Viewport` wrapper explicitly preventing components from overflowing the fixed screen layout.
