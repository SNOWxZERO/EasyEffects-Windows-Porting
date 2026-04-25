# Walkthrough - EQ Node Inspector (SteelSeries Style)

The Visual Equalizer now features a professional "Inspector" popup for precise control over individual bands.

## Key Accomplishments

### 1. Floating Settings Panel (`EQNodeSettingsPanel`)
- **SteelSeries Sonar Experience**: Clicking any EQ node launches a sleek, dark-themed popup bubble.
- **Precision Numerical Entry**:
    - **Frequency**: Inc/Dec buttons and direct text entry (e.g., "440 Hz").
    - **Gain**: Precise dB adjustment (e.g., "-3.5 dB").
    - **Q Factor**: Fine-tune the bandwidth numerically.
- **Quick Controls**: Toggle the band on/off and switch filter types (Peak, Shelf, etc.) directly in the bubble.

### 2. Deep Integration
- **APVTS Synchronization**: The bubble is fully linked to the DSP engine. Moving a slider in the bubble updates the curve in real-time, and vice-versa.
- **Auto-Dismissal**: Uses `juce::CallOutBox` logic to disappear when clicking elsewhere, keeping the UI clutter-free.

## Visual Changes
- **Selected State**: Nodes now have a clearer interaction model.
- **Bubble UI**: Semi-transparent background with a modern, high-contrast aesthetic.

## Verification
- **Build Status**: ✅ Success (Release/VST3)
- **Interaction**: Verified left-click to open, numerical entry to move nodes, and type switching.
