# Windows System-wide Audio Routing Guide

Because JUCE does not directly interact with the low-level Windows Audio Processing Object (APO) architecture, EasyEffects Windows operates as a Standalone Audio Host. 

To apply these effects to your system-wide audio (YouTube, Spotify, Games) gracefully, we use a **Virtual Audio Cable (VAC)** workflow.

## Overview of the Pipeline

1. **System Output** is sent to a *Virtual Cable Input*.
2. **EasyEffects App** listens to the *Virtual Cable Output*.
3. **EasyEffects App** processes the audio through the DSP chain.
4. **EasyEffects App** pushes the processed audio to your physical *Headphones/Speakers*.

---

## Step-by-Step Setup

### Step 1: Install a Virtual Audio Cable
You need a kernel-level driver that creates a virtual loopback device. The two best options are:
- [VB-Audio Virtual Cable (Free)](https://vb-audio.com/Cable/)
- [Virtual Audio Cable (Paid/Trial)](https://vac.muzychenko.net/en/)

Download and install your chosen cable, and then explicitly **Restart your PC**.

### Step 2: Configure Windows Audio
1. Open the Windows **Settings** app -> **System** -> **Sound**.
2. Under **Output**, set your default playback device to **CABLE Input (VB-Audio Virtual Cable)**.
   *Note: At this point, you will not hear any audio on your system because it is being piped into the void.*

### Step 3: Configure EasyEffects Windows
1. Open the **EasyEffects Standalone** application.
2. In the top OS menubar of the app, click **Options -> Audio/MIDI Settings**.
3. Configure the settings exactly as follows:
   * **Audio Device Type:** `Windows Audio (Exclusive)` or `WASAPI`
   * **Input Device:** `CABLE Output (VB-Audio Virtual Cable)`
   * **Output Device:** `Your Real Physical Headphones / Speakers (e.g., Realtek High Definition Audio)`
   * **Sample Rate:** `48000 Hz` (Recommended to match standard Windows format)
   * **Audio Buffer Size:** `256` or `512` samples (Lower means less latency, but higher CPU usage).

### Step 4: Verify the DSP Chain
1. Play some audio from a web browser or Spotify.
2. Open EasyEffects and click the **Level Meter**. You should see the graphical meters moving in real-time.
3. Turn on the **Equalizer** or **Limiter** and make extreme changes to verify that the core audio pipeline is successfully routing into your headphones.

---

## Latency Optimization Tips
- **Buffer Size:** If you experience "crackling" or "pops", increase the buffer size in EasyEffects (e.g. 512 -> 1024). If you experience visual lag while watching videos, *decrease* the buffer size (e.g. 512 -> 128).
- **Match Sample Rates:** Ensure that the Windows Sound Control Panel properties for both your *Physical Headphones* and the *Virtual Cable* are perfectly matched (e.g., exactly `24-bit, 48000 Hz`). Mixed sample rates force Windows into heavy resampling, destroying latency.
