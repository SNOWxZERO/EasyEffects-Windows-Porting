#pragma once

#include <string>

namespace eeval {
namespace DefaultPresets {

const char* broadcastHighPitch = R"({
  "version": 1,
  "slots": [
    {
      "typeId": "rnnoise",
      "params": {
        "mix": 1.0
      }
    },
    {
      "typeId": "compressor",
      "params": {
        "threshold": -20.0,
        "ratio": 4.0,
        "attack": 2.0,
        "release": 100.0,
        "makeup": 4.0,
        "mix": 1.0
      }
    },
    {
      "typeId": "equalizer",
      "params": {
        "band0_active": 1.0,
        "band0_type": 1.0,
        "band0_freq": 80.0,
        "band0_gain": 0.0,
        "band0_q": 0.707,
        "band1_active": 1.0,
        "band1_type": 0.0,
        "band1_freq": 150.0,
        "band1_gain": 3.0,
        "band1_q": 1.0,
        "band2_active": 1.0,
        "band2_type": 0.0,
        "band2_freq": 4000.0,
        "band2_gain": 4.0,
        "band2_q": 1.0,
        "band3_active": 1.0,
        "band3_type": 4.0,
        "band3_freq": 10000.0,
        "band3_gain": 0.0,
        "band3_q": 0.707,
        "mix": 1.0
      }
    }
  ]
})";

const char* deepVoice = R"({
  "version": 1,
  "slots": [
    {
      "typeId": "rnnoise",
      "params": { "mix": 1.0 }
    },
    {
      "typeId": "equalizer",
      "params": {
        "band0_active": 1.0,
        "band0_type": 1.0,
        "band0_freq": 50.0,
        "band0_gain": 0.0,
        "band0_q": 0.707,
        "band1_active": 1.0,
        "band1_type": 0.0,
        "band1_freq": 100.0,
        "band1_gain": 6.0,
        "band1_q": 1.0,
        "band2_active": 1.0,
        "band2_type": 0.0,
        "band2_freq": 250.0,
        "band2_gain": 3.0,
        "band2_q": 1.0,
        "mix": 1.0
      }
    }
  ]
})";

const char* walkieTalkie = R"({
  "version": 1,
  "slots": [
    {
      "typeId": "equalizer",
      "params": {
        "band0_active": 1.0,
        "band0_type": 1.0,
        "band0_freq": 400.0,
        "band0_gain": 0.0,
        "band0_q": 1.0,
        "band1_active": 1.0,
        "band1_type": 0.0,
        "band1_freq": 1000.0,
        "band1_gain": 8.0,
        "band1_q": 2.0,
        "band2_active": 1.0,
        "band2_type": 2.0,
        "band2_freq": 3000.0,
        "band2_gain": 0.0,
        "band2_q": 1.0,
        "mix": 1.0
      }
    },
    {
      "typeId": "crusher",
      "params": {
        "bit_depth": 8.0,
        "reduction": 4.0,
        "mix": 0.8
      }
    }
  ]
})";

const char* flat = R"({
  "version": 1,
  "slots": [
    {
      "typeId": "rnnoise",
      "params": { "mix": 1.0 }
    },
    {
      "typeId": "compressor",
      "params": {
        "threshold": -15.0,
        "ratio": 3.0,
        "attack": 5.0,
        "release": 50.0,
        "makeup": 2.0,
        "mix": 1.0
      }
    }
  ]
})";

} // namespace DefaultPresets
} // namespace eeval
