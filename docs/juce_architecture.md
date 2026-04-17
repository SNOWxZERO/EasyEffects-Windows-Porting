# JUCE Architecture Design

## 1. Requirements
* **Modular DSP Chain:** A structure allowing the chaining of multiple audio filters and effects.
* **Real-time Safe Processing:** Audio thread must not be blocked by allocations, locks, or UI updates.
* **Separation of Concerns:** Clean boundary between the `juce::AudioProcessor` (audio thread) and `juce::AudioProcessorEditor` (UI thread).
* **Extensible Plugin-like Structure:** Individual effects are self-contained modules that can be toggled or reordered.

## 2. Audio Processing Flow
```text
[Audio Input Device (e.g. VAC)]
       ↓ (juce::AudioIODeviceCallback)
  [Main AudioProcessor::processBlock()]
       ↓
  [DSP Chain (juce::dsp::ProcessorChain / Custom Chain)]
       → Module: juce::dsp::Gain (Output Level, etc)
       → Module: juce::dsp::IIR::Filter (EQ)
       → Module: juce::dsp::Compressor
       ↓
  [Main AudioProcessor::processBlock()]
       ↓
[Audio Output Device (Physical Speakers)]
```

*Note: For system-wide audio (like original EasyEffects), users will map their Windows Default Output to a Virtual Audio Cable (VAC) Input, and tell the JUCE Standalone App to listen to the VAC Output and play to their Physical Speakers.*

## 3. Class Structure
1. `EasyEffectsAudioProcessor` (Inherits `juce::AudioProcessor`)
   * Hosts the Audio Processor Value Tree State (APVTS) for thread-safe parameter handling.
   * Manages the active sequence of DSP modules.
2. `EasyEffectsAudioProcessorEditor` (Inherits `juce::AudioProcessorEditor`)
   * The JUCE-based UI (sliders, graphs).
   * Attaches to the APVTS to reflect live parameters.
3. `EffectModule` (Base Interface)
   * A wrapper for `juce::dsp` objects or pure C++ math.
   * Exposes `prepare()`, `process()`, and `reset()`.

## 4. Threading Model
* **Audio Thread:** Managed completely by JUCE. Executing `prepareToPlay()` and `processBlock()`.
* **Message Thread (UI):** Managed completely by JUCE.
* **Synchronization:** Utilizing `juce::AudioProcessorValueTreeState` which uses atomic operations/lock-free queues to pass slider values to the audio thread safely. No mutexes or dynamic allocation on the audio thread.
