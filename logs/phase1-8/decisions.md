# Architectural Decisions

## 1. Decision: Use JUCE framework over Native WASAPI + Qt6
* **Why:** JUCE provides industry-standard abstractions for audio I/O, threads, and complex DSP algorithms. It eliminates the need to manually write low-level `CoInitializeEx` and `IAudioClient` bindings, which was a massive risk factor in early porting attempts. It also removes the need to cross threading boundaries forcefully between Qt6 and the audio fast path.
* **Tradeoff:** We lose native system-wide loopback capability. We will need to instruct users to run a Virtual Audio Cable (VAC).
* **Status:** Accepted (2026-04-16)

## 2. Decision: Use MSVC Toolchain over MinGW
* **Why:** MSVC integrates significantly better with JUCE and standard Windows C++ ecosystem builds. MinGW frequently struggled with CMake finding the correct build programs without extreme PATH management.
* **Status:** Accepted (2026-04-16)

## 3. Decision: XML-based Preset System over JSON
* **Why:** JUCE's `AudioProcessorValueTreeState` natively exports to XML via `copyState().createXml()`. Using XML avoids format conversion overhead and leverages JUCE's built-in, tested serialization. The original Linux EasyEffects used `nlohmann_json`, but since our parameter schema is different (JUCE APVTS vs LV2 ports), there is no backwards-compatibility benefit.
* **Tradeoff:** Presets from the Linux version cannot be directly imported. A future converter could bridge this.
* **Status:** Accepted (2026-04-17)

## 4. Decision: Centralized EditorState in AudioProcessor
* **Why:** The `selectedEditorIndex` and future module chain ordering must survive editor recreation (JUCE destroys and recreates editors). Storing UI state in the processor ensures persistence across editor lifecycle events.
* **Status:** Accepted (2026-04-17)

## 5. Decision: Auto-save to %APPDATA% on Exit
* **Why:** Mirrors the original EasyEffects behavior where the last-used configuration is restored on startup. Using `%APPDATA%/EasyEffectsWindows/autosave.xml` follows Windows conventions.
* **Status:** Accepted (2026-04-17)
