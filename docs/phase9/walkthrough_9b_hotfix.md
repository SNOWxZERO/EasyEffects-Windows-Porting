# Walkthrough: Phase 9B Hotfix (Parameter ID System & Stability)

This document logs the critical architectural fix applied during Phase 9B to resolve runtime crashes and parameter binding inconsistencies.

## 🔴 The Issue
The application crashed when selecting any of the 8 new DSP modules. 
- **Cause**: Null-pointer dereference in `updateParameters`.
- **Root Cause**: Mismatch between registered IDs (`slotX.mix`) and accessed IDs (`slotX.typeId.mix`).
- **Legacy Impact**: Mix and Bypass controls were silently broken for all existing modules.

## 🟢 The Solution
I implemented a core architectural change to how parameter IDs are managed in the DSP layer.

### 1. Infrastructure Update
- **`EffectModule`**: Now separates `slotPrefix` ("slot0") and `slotParamPrefix` ("slot0.gate").
- **ID Helpers**: Added `paramId(name)` for effect-specific params and `slotParamId(name)` for slot-level params (mix/bypass).

### 2. Global Safety Refactor
Refactored **all 21 DSP modules** with the following strict stability rules:
- **No Raw Dereferencing**: Replaced direct `->load()` calls with a safe `loadFloat` helper.
- **Null Checks**: Added explicit checks for every parameter pointer.
- **Safe Defaults**: If a parameter is missing, the module falls back to a neutral default value instead of crashing.

### 3. UI Alignment
- **`GenericModuleEditor`**: Now manually includes the **Dry/Wet Mix** slider for all modules.
- **`SidebarRow`**: Verified to work with the standardized `slotX.bypass` ID.

## 🧪 Verification Results
- [x] **Stable Selection**: Switching between any of the 21 effects no longer causes crashes.
- [x] **Mix/Bypass Functional**: Verified that the mix slider and bypass toggle correctly affect the audio processing for all modules.
- [x] **Build Standalone**: Successfully compiled `EasyEffects.exe` in Release mode.

---
**Status**: RESOLVED
**Commit**: `[HOTFIX] Fix slot parameter ID system and crash`
**Date**: 2026-04-21
