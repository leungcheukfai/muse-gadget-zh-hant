# Fish Audio TTS integration

The community firmware uses Fish Audio for both Cantonese and Mandarin speech. Each builder supplies a Fish Audio API key and one voice model ID for each language through the board's local ESP-IDF `menuconfig`. The ESP32 sends generated reply text directly to Fish Audio; no self-hosted proxy or always-on computer is required.

## Firmware path

- `muse_chat_session.cpp` chooses the configured voice ID based on the saved reply language.
- `muse_fish_tts.c` sends a JSON `text`, `reference_id`, and `format: wav` request to `https://api.fish.audio/v1/tts` over HTTPS. The request includes the `s2.1-pro` model header and a Bearer API key.
- `muse_tts_wav.c` parses the streaming PCM16 48 kHz WAV response, converts stereo to mono when necessary, downsamples to the 16 kHz format used by the device, and queues audio for playback.
- If the API key or selected voice ID is absent, or the service request fails before audio arrives, the device displays captions without speech.

Fish Audio's [TTS API guide](https://beta.fish.audio/text-to-speech-api/) documents the endpoint, Bearer authorization, model header, voice reference IDs, and WAV output. Fish Audio handles synthesis for both configured languages; Muse itself continues to generate Traditional Chinese captions and language-specific text replies.

## Credential handling

The Fish Audio API key and voice IDs come from local Kconfig options. The key is saved in the local build's `sdkconfig` and compiled into the firmware image, so both files must be kept private. Anyone who obtains the image may recover the key. At runtime it is sent to Fish Audio only in the HTTPS `Authorization` header, is not placed in the JSON body, and is not included in logs or BLE responses. The device removes Fish Audio, Gemini, and Canto-TTS credentials saved by older firmware from NVS during initialization.

Voice IDs are model references, not API keys. They are compiled into the firmware and the selected ID is sent with each Fish Audio text request. The BLE setup page is retained for Wi-Fi and Muse provisioning, but no longer handles Fish Audio credentials.
