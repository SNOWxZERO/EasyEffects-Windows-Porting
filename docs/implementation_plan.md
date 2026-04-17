# EasyEffects Windows Port (v2) - Implementation Plan

## Goal Description
The objective is to do a clean, top-down native Windows implementation of EasyEffects, rather than trying to hack out compiler errors from the Linux source. 

The previous attempt failed because it tried to adapt an inherently PipeWire/LV2-based codebase to Windows by "stubbing" missing Linux components natively, resulting in circular dependencies, abstract base class conflicts, and unscalable toolchain errors. We will NOT repeat this.

The new project will provide a pure Windows backend (WASAPI loopback + render) and port **only** the pure C++ DSP math from the original codebase, isolating the effects from device routing via lock-free buffers.

## User Review Required
> [!IMPORTANT]
> - Do you agree with the Native WASAPI approach for audio isolation, recognizing we will implement our own simple Loopback Capture system rather than battling 3rd party host wrappers like in the past?
> - Let me know if you strongly prefer sticking to MinGW or are willing to use MSVC as your compiler. I recommend MSVC specifically for easier WASAPI COM interface integration, but can use MinGW if strictly needed.
> - Please approve the Phase 0 start: Setting up the CMake file and environment checks before any audio coding begins.

## Proposed Changes

### [Architecture & Setup Workspace]
```text
easyeffects-windows-v2/
├── docs/ (Already generated inside workspace)
├── logs/ (Already generated inside workspace)
├── src/
│   ├── backend/   (WASAPI, Audio Buffers)
│   ├── dsp/       (Isolated Effect Nodes)
│   └── gui/       (Qt6 QML Interface)
```

The execution will follow the Roadmap defined in `docs/roadmap.md`, starting with Phase 0. No coding will begin on Phase 1 until Phase 0 passes. No code from the failed Windows attempt will be blindly reused without a validation and cleanup first.

### [Phase Breakdown]
1. **Phase 0:** Environment validation (CMake, Compiler, Qt)
2. **Phase 1:** Audio capture (WASAPI loopback working)
3. **Phase 2:** Audio playback pipeline (Render loop working)
4. **Phase 3:** DSP chain (basic EQ integration into loop)
5. **Phase 4:** Plugin system (Dynamic loading of DSP classes)
6. **Phase 5:** UI (Qt) (Binding QML UI thread safely to DSP thread)

## Open Questions

- Should I begin executing Phase 0 directly after your approval?

## Verification Plan

### Automated Tests
- Integration tests over a standard floating-point audio buffer checking basic DSP response (e.g. 0.0 in -> 0.0 out).

### Manual Verification
- A custom compilation endpoint for Phase 1 will log out actual sample values pulled from WASAPI loopback from Windows to ensure device binding succeeded.
