# EasyEffects Windows Port (v2) - Implementation Plan (JUCE Pivot)

## Goal Description
The objective is to pivot the architecture to **JUCE**, the industry standard for C++ audio, utilizing the **MSVC toolchain**. 

By utilizing JUCE, we avoid the fragile, low-level WASAPI / Qt6 bindings, gaining access to solid thread-safe parameters (`AudioProcessorValueTreeState`), out-of-the-box GUI elements for audio, and pre-built robust DSP modules. To achieve the "system-wide" effect loop, we will explicitly rely on Virtual Audio Cable (VAC) routing down the line, significantly speeding up development and removing driver-level bugs.

## User Review Required
> [!IMPORTANT]
> - All Markdown files have been rewritten for JUCE (`juce_architecture.md`, `roadmap.md`, `decisions.md`).
> - The prompt correctly mentions you downloaded the JUCE framework from GitHub. I will need the literal path to this JUCE folder so I can include it properly via CMake `add_subdirectory()`.
> - Do you approve initiating **Phase 0**? In Phase 0, I will set up the CMake file pointing to your JUCE location to compile a blank JUCE application with MSVC.

## Proposed Changes

### [Architecture & Files]
The codebase will adhere to classic JUCE structuring:
```text
easyeffects-windows-v2/
├── CMakeLists.txt (Consuming JUCE)
├── docs/ (Updated to juce_architecture.md)
├── logs/ (Added decisions.md)
├── src/
│   ├── PluginProcessor.cpp/h (Audio Thread)
│   ├── PluginEditor.cpp/h    (UI Thread)
│   └── dsp/                  (Custom math/chained modules)
```

### [Phase Breakdown]
1. **Phase 0:** Environment Setup (CMake + JUCE MSVC build successful).
2. **Phase 1:** Basic Audio Pipeline (Passthrough audio working).
3. **Phase 2:** DSP Chain (Minimal - basic EQ working).
4. **Phase 3:** Modular DSP System (Chaining infrastructure).
5. **Phase 4:** UI Integration (JUCE Sliders mapped to APVTS).
6. **Phase 5:** Preset System (Save/Load).
7. **Phase 6:** System-wide routing (Documentation on VAC routing).

## Open Questions

1. **Where is the downloaded JUCE repository located on your system?** (e.g., `D:\JUCE`) I need this to write the CMake file.
2. Are you ready for me to start coding Phase 0 immediately upon receiving the path?

## Verification Plan

### Automated Tests
- JUCE natively handles unit tests via the `juce_core` module if needed, but integration tests will focus on building cleanly first.

### Manual Verification
- Compiling Phase 0 will immediately spring up a standalone Windows application with default JUCE text "Hello World!" and an Audio Device configuration menu.
