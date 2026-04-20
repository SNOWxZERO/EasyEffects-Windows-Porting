# Phase 9 Development Log

## 2026-04-19 — Planning & Architecture Design
- Analyzed original EasyEffects README for complete effect list (28 effects total).
- Compared against our implementation: 13/28 effects done, 15 remaining.
- Categorized missing effects:
  - 8 pure DSP (no external deps): Stereo Tools, Expander, Crusher, Crystalizer, Crossfeed, Maximizer, Bass Loudness, Loudness/Auto Gain
  - 4 need external libraries: RNNoise, SoundTouch, Multiband Compressor/Gate (Linkwitz-Riley)
  - 3 deferred (complex ML/AEC): Deep Noise Remover, Echo Canceller, Speech Processor
- Identified JUCE APVTS constraint: cannot add/remove parameters after construction.
- Proposed pre-registration with 16 effect slots as the dynamic chain solution.
- Reorganized project docs into `docs/phase1-8/` (archive) and `docs/phase9/` (current).
- Created Phase 9 implementation plan.

## 2026-04-19 — User Approval with Refinements
User approved Phase 9 plan with these key decisions:
- **16 slot max** — sufficient for any realistic use case
- **Multiple instances allowed** — same effect can appear in chain twice
- **Git submodules** for external libraries (not FetchContent)
- **Phase 9A first** — dynamic chain before new effects
- Each slot must have `slotX.type` parameter to identify active effect
- Real-time safety: no allocation in audio thread, UI-thread only module creation
- EffectRegistry must be the central system driving DSP, UI, and preset reconstruction
- Presets must serialize chain structure alongside APVTS state
- Reordering at slot level (swap slot indices and module instances)

## 2026-04-20 — Phase 9A Implementation (Dynamic Chain)

### Files Created
- **`src/dsp/EffectRegistry.h`** — Central factory system:
  - `EffectTypeDescriptor` struct: typeId, displayName, params[], choices[], factory
  - `SlotParamDescriptor` struct: suffix, label, unit, min/max/step/default
  - `registerSlotParameters()` — pre-registers all params for one slot (type choice + bypass + mix + all per-type params)
  - `createModule()` — instantiates DSP module with correct slot prefix
  - `getTypeChoices()` — returns StringArray for "Add Effect" menu

- **`src/dsp/EffectRegistry.cpp`** — Defines all 12 current effect type descriptors:
  - Gate (4 params), Compressor (4), Equalizer (12), Deesser (3), Exciter (3)
  - Bass Enhancer (3), Filter (2 + 1 choice), Convolver (0), Delay (3)
  - Reverb (5), Limiter (2), Gain (1)
  - Total: ~42 unique params × 16 slots = ~1500+ APVTS parameters

### Files Rewritten
- **`src/dsp/EffectModule.h`** — Added `slotParamPrefix` support:
  - `setParamPrefix("slot3.compressor")` → `paramId("threshold")` = `"slot3.compressor.threshold"`
  - Backward compatible: empty prefix falls back to `getModuleId()`

- **`src/dsp/EffectChain.h/.cpp`** — Slot-based architecture:
  - Fixed `std::array<Slot, 16>` instead of dynamic `std::vector`
  - `setSlotModule()`, `clearSlot()`, `swapSlots()` with mutex protection
  - `setSlotTypeId()` / `getSlotTypeId()` for slot metadata
  - Auto-prepares new modules from cached ProcessSpec
  - Legacy `addModule()` / `clearModules()` preserved for compatibility

- **`src/PluginProcessor.h/.cpp`** — Dynamic chain management:
  - `createParameterLayout()` now calls `EffectRegistry::registerSlotParameters()` for 16 slots
  - `addEffect(typeId)` → finds first empty slot, creates module, sets type param
  - `removeEffect(slotIndex)` → clears slot then compacts chain (shifts modules down)
  - `moveEffect(slotIndex, direction)` → swaps two adjacent slots
  - `getActiveSlots()` → returns vector of SlotInfo for UI
  - `getStateInformation()` → serializes `CHAIN_ORDER` as comma-separated typeIds
  - `setStateInformation()` → rebuilds chain from saved `CHAIN_ORDER`
  - Default chain: Gate → Compressor → Limiter → Gain

- **`src/PluginEditor.h/.cpp`** — Dynamic UI:
  - `activeSlots` vector refreshed from `getActiveSlots()`
  - `showAddEffectMenu()` → PopupMenu with all registry effect types
  - `refreshSidebar()` → rebuilds sidebar and adjusts selection
  - `rebuildEditorView()` → creates `GenericModuleEditor(slotIndex, typeId)`
  - `"+ Add Effect"` button in sidebar header area

- **`src/ui/SidebarRowCustomComponent.h`** — Enhanced sidebar rows:
  - Constructor takes slotIndex, typeId, displayName + callbacks
  - Draws ▲/▼ arrows and × close button on right side
  - mouseDown routes to: bypass toggle / move up / move down / remove / select row
  - Bypass reads from `slotX.bypass` parameter

- **`src/ui/GenericModuleEditor.h`** — Slot-aware parameter editor:
  - Takes (apvts, slotIndex, typeId, displayName)
  - Reads parameter descriptors from `EffectRegistry::findType()`
  - Builds param IDs as `"slotX.typeId.suffix"`
  - Creates SliderAttachments and ComboBoxAttachments dynamically

### Build Status
- ✅ CMake configured successfully
- ✅ Full Release build passed (exit code 0)
- ✅ Committed as `127a14e` — 13 files changed, 1077 insertions, 524 deletions

### Current State
- Phase 9A core architecture is **complete and compiling**
- Default chain starts with: Gate → Compressor → Limiter → Gain
- User can add any of 12 effect types via popup menu
- User can remove effects and reorder with up/down arrows
- Sidebar shows only active effects (not all 16 slots)
- Presets save/restore chain structure
- **Needs user testing** to verify runtime behavior

### Known Items to Verify
- [ ] Adding effects via popup menu works at runtime
- [ ] Removing effects compacts chain correctly
- [ ] Reordering preserves parameter values
- [ ] Preset save/load restores chain structure
- [ ] No audio glitches during chain modification
- [ ] LevelMeter module still works (not in default chain anymore)
