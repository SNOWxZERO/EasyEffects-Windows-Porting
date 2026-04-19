# Failure Analysis of the Windows Port Attempt

## 1. What Was Attempted
* The porting effort tried to directly compile the existing `easyeffects` source code on Windows using MinGW-w64.
* It sought to bypass Linux-specific audio backend (PipeWire/SPA) and third-party library errors (LV2 plugins, FFTW3, GSL) by providing dummy headers ("stubs") for them.
* It attempted to retain the exact class hierarchies (`Effect`, `PluginBase`, `EffectFactory`) while trying to swap out internal code to use WASAPI instead.

## 2. What Failed
* **Build System Inconsistencies:** Continuous issues finding the correct CMake generators (`MinGW Makefiles` vs `Unix Makefiles` vs Visual Studio). The environment was misconfigured often.
* **Abstract Class Hell:** Because the `Effect` base class had pure virtual functions linked to C++ `std::span` methods that weren't implemented by the new stubs, the factory failed compilation. The fixes were hacking default behavior into the base class, breaking its contract.
* **Header / Dependency Nightmare:** Files kept failing because macros (`BIND_LV2_PORT`) and Linux dependencies (`json/json.h`) were scattered deeply inside the actual files. 
* **Incorrect Assumption of Portability:** Assumed that Linux LV2 wrapper classes could be quickly "stripped" of LV2 and reused as raw DSP blocks.

## 3. Repeated Errors & Circular Fixes
* **Missing Headers:** `json/json.h`, `M_PI`, `dlfcn.h`. The AI repeatedly tried to fix include paths instead of fixing the root cause (using the wrong compilation strategy).
* **Linker Errors:** Stubbed out methods compiled but obviously couldn't link because the implementations didn't exist, leading to unresolved symbols like `effects::EffectFactory::create`.
* **Toolchain Cycling:** The attempt was constantly jumping between CMake commands (`rm -rf build`, switching generators, fixing PATHs) rather than looking at code structure.

## 4. Why This Failed (The Root Cause)
The approach was **bottom-up compilation-driven**, aggressively attempting to "fix the next compiler error" of a fundamentally Linux-native, LV2-hosting application. Rather than architecting a new application that implements the *features* of EasyEffects, it tried to force a Square Peg (PipeWire Node) into a Round Hole (Windows WASAPI) by shaving the corners via stubs.
