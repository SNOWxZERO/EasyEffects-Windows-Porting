# Lessons Learned

## What Must NEVER Be Repeated
1. **Never "Stub" Core Paradigms:** Do not stub PipeWire, SPA, or LV2. If the OS doesn't support the target framework, the dependent code must be completely stripped, abstracted, or rewritten for the new OS.
2. **Never Fix Linker/Compiler Errors Blindly:** Do not hack default implementations into `virtual` base classes (`Effect`) just to silence factory compilation errors.
3. **Never Reuse the Old Build Configuration:** The old CMake and Qt integration was deeply intertwined with Linux packaging. Start with a fresh, simple `CMakeLists.txt` for Windows.
4. **Never Cling to Toolchain Fights:** If MinGW struggles with a CMake generator, verify the environment once, and stick to a consistent, working approach (like MSVC/Visual Studio for native Windows, unless specifically constrained).

## What Parts Are Reusable
1. **Pure C++ DSP Logic (If Isolated):** Filters, algorithms, and purely mathematical DSP conversions from the source `.cpp` files. To reuse this, it must be untangled from `BIND_LV2_PORT` macros.
2. **JSON Interfaces:** The serialization logic for presets (`nlohmann_json`), if decoupled from PipeWire state.
3. **General GUI Architecture:** The Qt/QML views and the intent of the UI (sliders, parameter management).

## Core Strategy Shift
The audio engine (WASAPI) must be fully isolated from DSP nodes. We will not embed WASAPI in the effect classes. Instead, we need a Host wrapper that routes audio, and pure C++ classes that process a given `float*` buffer.
