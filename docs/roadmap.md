# Implementation Roadmap

## Phase 0 — Environment Setup
* **Goals:** Verify build system and JUCE framework integration.
* **Deliverables:** A compiling JUCE standalone application using CMake and MSVC.
* **Test Criteria:** App launches, audio device manager opens, and basic input/output configuration works.

## Phase 1 — Basic Audio Pipeline
* **Goals:** Validate the I/O flow.
* **Deliverables:** A clean `AudioProcessor` passing audio buffers straight through without modification.
* **Test Criteria:** User can route mic/audio into the app and hear it unaltered in the output.

## Phase 2 — DSP Chain (Minimal)
* **Goals:** Prove the DSP processing overhead.
* **Deliverables:** Add a simple `juce::dsp::Gain` or simple EQ block to the `processBlock()`.
* **Test Criteria:** Gain modifier correctly alters the volume of the passthrough audio. Real-time processing is confirmed with no xruns.

## Phase 3 — Modular DSP System
* **Goals:** Establish the full extensible chain framework.
* **Deliverables:** An architecture to chain multiple effects dynamically, enabling and disabling modules in sequence.
* **Test Criteria:** Multiple effects run in series.

## Phase 4 — UI Integration
* **Goals:** Build the user interface.
* **Deliverables:** Custom JUCE editors (sliders, knobs, meters).
* **Test Criteria:** UI parameters bind correctly to the `AudioProcessorValueTreeState` and alter audio in real time.

## Phase 5 — Preset System
* **Goals:** Serialization.
* **Deliverables:** Save/load functionality for the APVTS data.
* **Test Criteria:** User can save a preset to disk, restart the app, and load the preset identically.

## Phase 6 — System-wide Routing (LATER)
* **Goals:** Provide the final EasyEffects experience.
* **Deliverables:** Documentation instructing the user how to configure a Virtual Audio Cable (VAC) with the standalone application.
* **Test Criteria:** System audio is routed through the program.
