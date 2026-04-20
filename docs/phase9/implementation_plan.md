# Phase 9: Dynamic Effect Chain & Missing Effects

## Status: APPROVED — Phase 9A In Progress

---

## Approved Decisions

| Decision | Answer |
|----------|--------|
| Max chain length | **16 slots** |
| Multiple instances | **Allowed** (e.g., two compressors) |
| External libraries | **Git submodules** (not FetchContent) |
| Priority | **Phase 9A first** (dynamic chain before new effects) |

---

## Part A: Dynamic Effect Chain Architecture (Phase 9A)

### Architectural Requirements

1. **Slot System**: Each of the 16 slots has a `slotX.type` parameter identifying which effect is active. DSP and UI only read parameters relevant to the active type.

2. **Real-Time Safety**: No allocation or deletion in the audio thread. Module creation/removal on UI thread only. Double-buffered chain swap for thread safety.

3. **EffectRegistry**: Central system containing effect type ID, display name, parameter descriptors, and factory function. Drives DSP creation, UI generation, and preset reconstruction.

4. **Preset System**: Must serialize chain structure (which effect in which slot) alongside APVTS parameter state.

5. **Reordering**: Operates at the slot level — swap slot indices and associated module instances. Do NOT move parameters.

6. **UI Requirements**: Add effect ("+"), remove effect, reorder (up/down buttons), bypass toggle per slot, display effect type clearly.

### Slot Parameter Design

Each slot pre-registers parameters for ALL effect types:
```
slotX.type    = choice (none, gate, compressor, eq, ...)
slotX.bypass  = bool
slotX.mix     = float [0, 100]

// Per-type parameters (only active type is read by DSP)
slotX.gate.threshold, slotX.gate.ratio, slotX.gate.attack, slotX.gate.release
slotX.compressor.threshold, slotX.compressor.ratio, ...
slotX.eq.band0.gain, slotX.eq.band0.freq, slotX.eq.band0.q, ...
... (all effect types)
```

Total: ~16 slots × ~95 params = ~1520 parameters. Negligible memory cost (~30KB).

### New/Modified Files

| File | Action | Purpose |
|------|--------|---------|
| `src/dsp/EffectRegistry.h` | NEW | Factory + descriptors for all effect types |
| `src/dsp/EffectChain.h/.cpp` | MODIFY | Thread-safe insert/remove/reorder/swap |
| `src/PluginProcessor.h/.cpp` | MODIFY | Slot-based parameter registration, chain management |
| `src/PluginEditor.h/.cpp` | MODIFY | "+ Add Effect" button, dynamic sidebar |
| `src/ui/SidebarRowCustomComponent.h` | MODIFY | Remove button, reorder arrows |
| `src/ui/ModuleDescriptors.h` | MODIFY | Adapt to slot-based parameter IDs |
| `src/ui/GenericModuleEditor.h` | MODIFY | Read from slot-prefixed parameters |

---

## Part B: Missing Effects (Phase 9B, 9C, 9D)

### Inventory: 13/28 done, 15 remaining

**Pure DSP (Phase 9B):** Stereo Tools, Expander, Crusher, Crystalizer, Crossfeed, Maximizer, Bass Loudness, Auto Gain/Loudness

**External Libraries (Phase 9C):** RNNoise (Noise Reduction), SoundTouch (Pitch Shift), Multiband Compressor, Multiband Gate — via git submodules

**Deferred (Phase 9D):** Deep Noise Remover, Echo Canceller, Speech Processor

---

## Execution Plan

### Phase 9A — Dynamic Chain (MANDATORY FIRST)
1. EffectRegistry factory system
2. Slot-based parameter pre-registration (16 slots)
3. EffectChain insert/remove/reorder with thread safety
4. UI: "+ Add Effect" popup menu
5. UI: Remove and reorder controls per sidebar row
6. Preset chain serialization
7. **Stop and stabilize**

### Phase 9B — Pure DSP Effects (one at a time, validate each)
### Phase 9C — External Libraries (after system stability confirmed)
### Phase 9D — Deferred

---

## Verification Plan

### Phase 9A
- Add 3 effects via UI, verify DSP chain processes in order
- Remove middle effect, verify chain re-links correctly
- Reorder effects, verify processing order changes
- Save/load preset with custom chain, verify it restores
- Verify no audio glitches during chain modifications
