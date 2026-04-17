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

## Phase 3 Extended: Complex Modules
* Ported user-request to ensure the modular chain could handle complex parameter sets.
* Added `eeval::CompressorModule` leveraging `juce::dsp::Compressor`.
* Mapped `comp_threshold`, `comp_ratio`, `comp_attack`, `comp_release` to the APVTS.
* Re-arranged the UI in `PluginEditor.cpp` to place compressor knobs at the top.
* Inserted `CompressorModule` upstream of `GainModule` inside the `EffectChain`.
* Compiled successfully.

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
* Compiled successfully.
