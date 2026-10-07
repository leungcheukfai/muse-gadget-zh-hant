# Hey Muse model

`hey_muse.tflite` is the 137,984-byte microWakeWord model for “Hey Muse” from
[`wobsoriano/hey-muse`](https://github.com/wobsoriano/hey-muse), copied without
modification from `tools/wake/hey_muse.tflite`. The adjacent JSON file is its
upstream manifest. The model uses a 16 kHz mono stream, 10 ms feature steps,
40 frontend features, a six-output probability window, a 0.99 cutoff, and a
30,000-byte tensor arena.

The upstream repository is MIT licensed; see [HEY-MUSE-LICENSE](HEY-MUSE-LICENSE).
The firmware uses Espressif's TFLite Micro component and ESPHome's Apache-2.0
micro-speech feature frontend, pinned in `idf_component.yml`.

The upstream model's real-room validation is limited. Detection quality on the
1.75C is experimental until tested with the intended microphone gain, voice,
room, and speaker volume.
