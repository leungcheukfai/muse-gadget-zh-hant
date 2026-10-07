# Hey Muse Wake Word on Waveshare ESP32-S3 1.75C

## Goal

Add hands-free “Hey Muse” voice turns to the Waveshare ESP32-S3-Touch-AMOLED-1.75C firmware while retaining its existing push-to-talk behavior.

## Context

The firmware already captures 16 kHz mono audio, pairs with Muse, streams voice turns, and plays replies on the 1.75C. The board has an ES7210 microphone ADC, ES8311 speaker codec, and 8 MB octal PSRAM. The `hey-muse` project supplies a 136 KB microWakeWord TFLite model and manifest for “Hey Muse”; the model expects 16 kHz audio and uses a 30 KB tensor arena.

The model’s reported evaluation is preliminary: the project reports evaluation on generated speech and ambient audio and a single Echo Show 5, not on the Waveshare board. Treat detection quality on this board as experimental until it is exercised in the intended room.

## Options considered

1. **Port the supplied microWakeWord model to ESP-IDF.** This retains the requested phrase and works with the existing 16 kHz capture path. It requires a TFLite Micro runtime and compatible streaming feature extraction.
2. **Use ESP-SR WakeNet.** This fits Espressif’s platform, but the supplied “Hey Muse” model is not a WakeNet model. Using a different built-in phrase or commissioning a custom WakeNet model would change the requested experience or add a separate model-training dependency.

Choose option 1. Keep the wake-word component isolated so its runtime can be changed without rewriting the Muse voice flow.

## Design

- Scope the first implementation to the Waveshare 1.75C board profile.
- Add a focused wake-word component that embeds the supplied TFLite model and runs streaming inference on 16 kHz mono microphone frames using Espressif’s TFLite Micro component and compatible microWakeWord feature processing.
- Feed the existing microphone frames to the detector from the voice task; do not open a second audio capture path.
- On detection, retain enough recent audio to carry the user’s first words into the request, then start the existing Muse voice turn automatically. End the recording after trailing silence, with the existing 15-second maximum as a cap.
- Keep push-to-talk available as a manual fallback. Suspend detection during recording and reply playback, then reset the detector before listening again so the speaker does not trigger a new turn.
- Listen while the device is awake or powered by USB. Preserve current battery sleep behavior: when the device sleeps on battery, power down the codecs and pause wake detection.
- If the detector or model fails to initialize, log the failure and leave push-to-talk working.
- Show a concise idle hint that the device is listening for “Hey Muse” while detection is active.

## Data flow

`ES7210 mic → existing 16 kHz capture → Hey Muse feature extraction/model → existing Muse voice turn → existing reply playback`

The wake detector operates locally. Audio is sent to Muse only after a wake word starts a voice turn, using the firmware’s existing transport and account pairing.

## Validation

No automated tests or build are part of this design approval. Once implemented, review should include a firmware build and hardware checks on the 1.75C: wake-word detection, capture through the first words after the phrase, trailing-silence endpointing, reply playback without self-trigger, push-to-talk fallback, and codec shutdown during battery sleep. Detection reliability and false activations need real-room evaluation because the upstream model has not been validated on this board.

## References

- [hey-muse project](https://github.com/wobsoriano/hey-muse)
- [hey-muse wake-word model notes](https://github.com/wobsoriano/hey-muse/blob/muse/tools/wake/README.md)
- [Espressif TFLite Micro](https://github.com/espressif/esp-tflite-micro)
- [ESP-SR WakeNet customization](https://github.com/espressif/esp-sr/blob/master/docs/en/wake_word_engine/ESP_Wake_Words_Customization.rst)
