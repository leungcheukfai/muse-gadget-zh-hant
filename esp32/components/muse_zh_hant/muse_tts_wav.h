/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"

typedef struct muse_tts_wav_decoder muse_tts_wav_decoder_t;
typedef bool (*muse_tts_pcm_cb_t)(const int16_t *pcm, size_t frames, void *context);

/* Decode a streaming PCM16 48 kHz WAV into 16 kHz mono PCM. */
muse_tts_wav_decoder_t *muse_tts_wav_decoder_create(muse_tts_pcm_cb_t cb, void *context);
esp_err_t muse_tts_wav_decoder_feed(muse_tts_wav_decoder_t *decoder, const uint8_t *data, size_t len);
esp_err_t muse_tts_wav_decoder_finish(muse_tts_wav_decoder_t *decoder);
void muse_tts_wav_decoder_destroy(muse_tts_wav_decoder_t *decoder);
