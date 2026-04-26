# Architectural Decisions — Phases 10-12

## 6. Decision: Interactive Draggable EQ Nodes over Fixed Slider UI
* **Why:** Professional EQs (FabFilter, SteelSeries) use draggable nodes for intuitive frequency/gain manipulation. Fixed sliders limit spatial understanding of the frequency response.
* **Tradeoff:** More complex UI code (custom Component, drag math, coordinate mapping).
* **Status:** Accepted (2026-04-23)

## 7. Decision: Dynamic Band Count (max 10, display only active)
* **Why:** User feedback — showing 10 bands at once clutters the UI. Users add bands as needed via "+ Add Band" button; only enabled bands have visible nodes.
* **Status:** Accepted (2026-04-23)

## 8. Decision: Lock-Free FIFO for FFT Data Transfer
* **Why:** Audio thread MUST NOT allocate memory or block. `juce::AbstractFifo` + pre-allocated `AudioBuffer` gives zero-allocation audio→UI transfer. UI polls at 60Hz via Timer.
* **Alternative:** Shared atomic buffer — rejected because FFT needs 2048 contiguous samples.
* **Status:** Accepted (2026-04-25)

## 9. Decision: JSON for Presets over JUCE XML
* **Why:** JSON is human-readable/editable. JUCE's APVTS XML is tightly coupled to the tree structure and harder to share across versions. Custom JSON gives us full control over schema.
* **Tradeoff:** We manually serialize/deserialize instead of using APVTS `copyState()`.
* **Status:** Accepted (2026-04-25)

## 10. Decision: Global Bypass Skips DSP Only, Not Analysis
* **Why:** When A/B testing, users still want to see the spectrum analyzer. Bypassing only the `dspChain.process()` call while keeping FFT `pushSamples()` active gives accurate visual feedback during bypass.
* **Status:** Accepted (2026-04-26)

## 11. Decision: Monitor Device Switching via JUCE DeviceManager
* **Why:** User tests effects by switching output from Virtual Cable to Speakers. Instead of building a file player, we programmatically switch `AudioDeviceManager::setAudioDeviceSetup()` — zero new DSP code needed.
* **Tradeoff:** Only works in Standalone build (not VST3). Guarded by `#if JucePlugin_Build_Standalone`.
* **Status:** Accepted (2026-04-26)

## 12. Decision: Active Preset Tracking with Dirty State
* **Why:** Without tracking, "Save" always creates a new file. Users expect "Save" to overwrite the current preset. `currentPresetName` + `dirty` flag enables Save vs Save-As workflow.
* **Status:** Accepted (2026-04-26)
