# EasyEffects Windows Native Architecture

## 1. System Overview
To avoid the pitfalls of the previous port attempt, the v2 project will employ a strict separation of concerns:
- **Audio Host Layer:** A pure Windows WASAPI module responsible for I/O.
- **DSP Engine Layer:** A collection of OS-agnostic math filters.
- **Controller/UI Layer:** A Qt6 frontend managing state and user intent.

## 2. Chosen Technologies
* **Audio API:** **WASAPI** 
  * *Why:* It's the native Windows API, supporting loopback capture and low-latency rendering. (PortAudio could be used for simplicity, but WASAPI loopback is explicitly needed for system-wide capture).
* **UI Framework:** **Qt 6 (QML + C++)**
  * *Why:* Reusable cross-platform, similar to the recent Linux EasyEffects migration, allowing maximal UI reuse.
* **Build System:** **CMake**
  * *Why:* Standardized, supports Qt easily via `find_package(Qt6)`.
* **Compiler Base:** **Visual Studio 2022 (MSVC)** or **MinGW-w64** 
  * *Why:* Native windows targeting. MSVC is drastically easier to integrate with WASAPI COM interfaces.

## 3. Data Flow
```text
[System Default Audio Endpoint]
       ↓ (WASAPI Loopback Capture Thread)
  [Ring Buffer (Lock-Free)]
       ↓
  [Audio Processing Thread]
       → EffectNode (EQ) → EffectNode (Compressor) → ...
       ↓
  [Ring Buffer (Lock-Free)]
       ↓ (WASAPI Render Thread)
[Virtual Audio Cable / Output Endpoint]
```

## 4. Components
### A. The DSP Interface (`EffectNode`)
No `std::span` overloads mixed with LV2 proxies. Simply:
```cpp
class EffectNode {
public:
    virtual ~EffectNode() = default;
    virtual void process(float* buffer, size_t num_frames, int channels) = 0;
    virtual void set_parameter(const std::string& name, float value) = 0;
};
```

### B. The Host (`WasapiEngine`)
Initializes COM (`CoInitializeEx`), finds the endpoint devices using `IMMDeviceEnumerator`, starts `IAudioClient` in loopback mode, processes the event handle, and triggers the `EffectChain::process()` before pushing to the output `IAudioRenderClient`.

### C. UI Synchronization
Real-time audio cannot block. The UI (Main Thread) must push parameter updates into a lock-free queue that the Audio Thread drains at the beginning of each processing block.

## 5. Tradeoffs
* **WASAPI Direct vs APO:** 
  * *Tradeoff:* Developing a true Windows Audio Processing Object (APO) is exceptionally complex and requires driver signing.
  * *Decision:* We will use WASAPI Loopback -> Render to a Virtual Audio Cable (VAC). This is typical for Windows system-audio equalizers (like EqualizerAPO using APOs, or Voicemeeter using Loopback).
* **MSVC vs MinGW:**
  * *Tradeoff:* MinGW requires strict PATH handling and COM wrapper translation. MSVC has perfect zero-config WASAPI support.
  * *Decision:* Keep CMake versatile, but heavily lean towards MSVC on Windows if permitted.
