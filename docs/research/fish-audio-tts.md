# Fish Audio TTS integration

The community firmware uses Fish Audio for both Cantonese and Mandarin speech. Each device owner supplies their own Fish Audio API key and a voice model ID for each language. The ESP32 sends generated reply text directly to Fish Audio; no self-hosted proxy or always-on computer is required.

## Firmware path

- `muse_chat_session.cpp` chooses the voice ID based on the saved reply language.
- `muse_fish_tts.c` sends a JSON `text`, `reference_id`, and `format: wav` request to `https://api.fish.audio/v1/tts` over HTTPS. The request includes the `s2.1-pro` model header and a Bearer API key.
- `muse_tts_wav.c` parses the streaming PCM16 48 kHz WAV response, converts stereo to mono when necessary, downsamples to the 16 kHz format used by the device, and queues audio for playback.
- If the API key or selected voice ID is absent, or the service request fails before audio arrives, the device displays captions without speech.

Fish Audio's [TTS API guide](https://beta.fish.audio/text-to-speech-api/) documents the endpoint, Bearer authorization, model header, voice reference IDs, and WAV output. Fish Audio handles synthesis for both configured languages; Muse itself continues to generate Traditional Chinese captions and language-specific text replies.

## Credential handling

The Fish Audio API key arrives over authenticated BLE and is saved to encrypted NVS. It is sent to Fish Audio only in the HTTPS `Authorization` header, is not placed in the JSON body, and is not included in logs or BLE responses. The device deletes the legacy Gemini key and Canto-TTS URL/token NVS entries once when this firmware initializes, so an upgrade does not retain obsolete speech-provider credentials.

Voice IDs are model references, not API keys. The setup page keeps them in its input fields only until the user clears or closes the page; they are sent to the device over BLE and stored in NVS. Fish Audio receives the selected voice ID with each text request.
