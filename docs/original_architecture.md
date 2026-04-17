# Original Linux Project Architecture Analysis (EasyEffects)

## 1. Overview
EasyEffects (formerly PulseEffects) was designed to provide simple system-wide audio effects for Linux. It achieves this by integrating tightly with the native Linux audio backend.

## 2. Audio Pipeline
* **Backend Integration:** Originally built for PulseAudio and GStreamer, it has evolved into a native **PipeWire** client.
* **Routing:** Uses PipeWire's graph routing to insert its DSP nodes between source applications and hardware sink devices.
* **SPA (Simple Plugin API):** Uses PipeWire's SPA to handle format negotiation and buffer management at low latency.

## 3. Architecture & Threading
* **Real-time Audio Thread:** Maintained by PipeWire. Nodes process audio buffers sequentially.
* **DSP Node Design:** Effects are chained. The order of the nodes determines the signal path. Each effect holds its DSP state and logic.
* **UI Thread:** Uses GTK4 (and currently transitioning to Qt/QML/Kirigami) to render the configuration GUI independently from the audio fast-path.
* **Communication:** Lock-free parameter updates or atomic variables negotiate between the UI and audio threads.

## 4. Dependencies
* **Core DSP Libraries:** Highly dependent on external Linux-centric audio plugins like:
  * LSP (Linux Studio Plugins)
  * Calf Studio Plugins
  * Zita-convolver, MDA, SpeexDSP, SoundTouch, RNNoise
* **Serialization:** `nlohmann_json` is used extensively for saving, loading, and passing preset configurations.
* **LV2 Wrappers:** Much of the effect code involves plumbing parameters into LV2 plugin hosts since it wraps many pre-existing standard Linux LV2 plugins.

## 5. Summary
The Linux version is not a monolithic application containing all DSP libraries, but rather a host/wrapper that builds a graph of third-party plugins (mostly LV2) combined with PipeWire routing to achieve system-wide effects.
