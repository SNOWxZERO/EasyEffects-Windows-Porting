# EasyEffects Windows Quick Start

Welcome to EasyEffects Windows! This is a native Windows audio processor inspired by the original EasyEffects for Linux.

## 🚀 Getting Started

### 1. Launch the App
Run `EasyEffects.exe` from this folder.

### 2. Configure Devices
1. In the top-left menu, click **Options → Audio Settings...**
2. Set your **Input Device** (e.g., your microphone)
3. Set your **Output Device** (e.g., your headphones)
4. Audio should now pass through the processor!

### 3. Add Effects
Use the left sidebar to add effects like EQ, Compressor, or Noise Reduction.

---

## 🎙️ Microphone Audio Setup (Voice Processing)

To apply effects to your microphone before sending it to Discord, Zoom, or OBS:

1. **Install a Virtual Audio Cable:** We recommend [VB-Cable (Free)](https://vb-audio.com/Cable/).
2. **In EasyEffects Audio Settings:**
   - Set **Input** to your physical microphone.
   - Set **Output** to `CABLE Input`.
3. **In your chat application:**
   - Set your input/microphone device to `CABLE Output`.

---

## 🎧 System-Wide Audio Routing (Optional)

By default, EasyEffects Windows only processes the direct input you select. It **does not** automatically capture all system audio (like games or Spotify) out-of-the-box.

To apply effects to your entire system's output:

1. **Install a Virtual Audio Cable:** We recommend [VB-Cable (Free)](https://vb-audio.com/Cable/).
2. **Set Windows Default Output** to `CABLE Input`.
3. **In EasyEffects Audio Settings:**
   - Set **Input** to `CABLE Output`
   - Set **Output** to your physical speakers/headphones.

---

## 🔌 Using as a VST3 Plugin

You can also use EasyEffects inside OBS Studio or DAWs!

1. Copy the `EasyEffects.vst3` folder included in this zip.
2. Paste it into: `C:\Program Files\Common Files\VST3\`
3. It will now appear as an effect in OBS and standard DAWs.

---

## ⚖️ Legal & Licensing

- **License:** GNU General Public License v3.0 (GPLv3)
- **External Libraries:** This software uses RNNoise (BSD) and SoundTouch (LGPL, dynamically linked or otherwise documented in source).
- See the `LICENSE` file for full details.
