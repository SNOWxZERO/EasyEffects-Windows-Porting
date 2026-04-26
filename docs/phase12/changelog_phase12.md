# Phase 12 — UX Polish, Audio Playback & Preset Workflow

## [Phase 12] - UX Polish, Bypass & Preset Overhaul

### Added
- **Global Chain Bypass**: Toggle in header to bypass the entire effect chain for A/B comparison.
- **Preset Workflow**: Active preset tracking, Save (overwrite), Save As (new name), Delete with confirmation.
- **Active Preset Label**: Header shows current preset name with `*` dirty indicator.
- **EQ Presets**: Parametric Equalizer now has its own Presets button with Save/Load/Delete.
- **Module Preset Delete**: All per-module preset menus now support deleting presets.
- **Audio Device Hint**: Footer shows reminder to use Options button for I/O device selection.

### Removed
- **Broken Footer Meter**: Removed non-functional `-100 dB` level meter from the footer.
- **Redundant FFT Bar**: Removed the global spectrum analyzer bar (FFT is integrated in EQ plot).

### Fixed
- **Preset Overwrite**: Saving now properly overwrites existing files instead of appending.
- **Async Dialogs**: All confirmation dialogs use async patterns (no modal loop blocking).
