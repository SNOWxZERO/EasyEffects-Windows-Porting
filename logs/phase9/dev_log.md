# Phase 9 Development Log

## 2026-04-19 — Planning & Architecture Design
- Analyzed original EasyEffects README: 28 effects total, we have 13/28.
- Categorized 15 missing effects: 8 pure DSP, 4 external libs, 3 deferred.
- Identified JUCE APVTS constraint: cannot add/remove parameters post-construction.
- Proposed pre-registration with 16 effect slots.
- Reorganized docs: `docs/phase1-8/` (archive), `docs/phase9/` (current).

## 2026-04-19 — User Approval
Decisions: 16 slots max, multiple instances allowed, git submodules for libs, Phase 9A first.

## 2026-04-20 — Phase 9A Implementation

### New Files
| File | Purpose |
|------|---------|
| `src/dsp/EffectRegistry.h` | Central factory: typeId → DSP module + param descriptors |
| `src/dsp/EffectRegistry.cpp` | 12 effect type descriptors, `registerSlotParameters()`, `createModule()` |

### Rewritten Files
| File | Key Changes |
|------|-------------|
| `src/dsp/EffectModule.h` | Added `slotParamPrefix` support for slot-based parameter lookup |
| `src/dsp/EffectChain.h/.cpp` | Fixed 16-slot array with mutex, `setSlotModule()`, `clearSlot()`, `swapSlots()` |
| `src/PluginProcessor.h/.cpp` | Slot-based APVTS, `addEffect()`, `removeEffect()`, `moveEffect()`, `getActiveSlots()`, preset chain serialization |
| `src/PluginEditor.h/.cpp` | "+ Add Effect" popup menu, dynamic sidebar, `refreshSidebar()`, always-recreate row components |
| `src/ui/SidebarRowCustomComponent.h` | Takes slotIndex + callbacks, draws ▲/▼/× buttons |
| `src/ui/GenericModuleEditor.h` | Reads params via `EffectRegistry::findType()`, builds `slotX.typeId.suffix` IDs |
| `CMakeLists.txt` | Added `EffectRegistry.cpp` to sources |

### Bugs Fixed
1. **Sidebar not clickable (Phase 8):** Custom components intercepted ListBox clicks → added `findParentComponentOfClass<ListBox>()->selectRow()`
2. **moveEffect parameter swap:** Modules swapped but APVTS values stayed in old slots → save/restore via `std::map<string, float>` temp storage
3. **UI stale after reorder:** `refreshComponentForRow` reused old components with stale slot indices → always delete and recreate

### Commits
```
18f748b [FIX] Sidebar rows always recreated after reorder/remove
4b7ce9e [FIX] moveEffect now properly swaps parameter values between slots
abf4d8f [DOCS] Updated Phase 9 dev log
127a14e [PHASE 9A] Dynamic effect chain architecture
fdeec31 [PHASE 9] Reorganized docs/logs, created Phase 9 plan
d2d8fc5 [DOCS] Discord voice routing guide
bd98dcf [PHASE 8] Complete UI overhaul
```

### Architecture Summary
```
EffectRegistry (central factory)
  ├── getEffectTypes() → 12 type descriptors
  ├── registerSlotParameters(layout, slotIndex) → APVTS params for 1 slot
  ├── createModule(typeId, slotPrefix) → DSP module with correct param prefix
  └── findType(typeId) → param descriptors for UI

PluginProcessor
  ├── createParameterLayout() → 16 slots × all types = ~1500 params
  ├── addEffect(typeId) → find empty slot, create module, set type param
  ├── removeEffect(slotIndex) → clear + compact chain
  ├── moveEffect(slotIndex, dir) → save params → swap → restore params
  └── getActiveSlots() → SlotInfo[] for sidebar

EffectChain (16-slot array)
  ├── setSlotModule(idx, module) → mutex-protected, auto-prepare
  ├── clearSlot(idx) / swapSlots(a, b)
  └── process(buffer) → iterate non-null slots sequentially
```

### Current Status
- ✅ Dynamic chain fully working: add, remove, reorder effects at runtime
- ✅ Parameter values follow effects during reorder
- ✅ UI updates instantly on all operations
- ✅ Presets save/restore chain structure via CHAIN_ORDER property
- ✅ Default chain: Gate → Compressor → Limiter → Gain

### Next Steps
- **Phase 9B:** Implement 8 pure DSP effects (Stereo Tools, Expander, Crusher, etc.)
- **Phase 9C:** External library effects via git submodules (RNNoise, SoundTouch)
