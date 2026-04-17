# Architectural Decisions

## 1. Decision: Use JUCE framework over Native WASAPI + Qt6
* **Why:** JUCE provides industry-standard abstractions for audio I/O, threads, and complex DSP algorithms. It eliminates the need to manually write low-level `CoInitializeEx` and `IAudioClient` bindings, which was a massive risk factor in early porting attempts. It also removes the need to cross threading boundaries forcefully between Qt6 and the audio fast path.
* **Tradeoff:** We lose native system-wide loopback capability. We will need to instruct users to run a Virtual Audio Cable (VAC).
* **Status:** Accepted (2026-04-16)

## 2. Decision: Use MSVC Toolchain over MinGW
* **Why:** MSVC integrates significantly better with JUCE and standard Windows C++ ecosystem builds. MinGW frequently struggled with CMake finding the correct build programs without extreme PATH management.
* **Status:** Accepted (2026-04-16)
