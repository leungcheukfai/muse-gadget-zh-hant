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

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"
#include "muse_tts_wav.h"

/* Sends reply text to Fish Audio's HTTPS API and streams its WAV response as
 * 16 kHz mono PCM to cb. The API key is never logged or included in the body. */
esp_err_t muse_fish_tts_generate(const char *api_key, const char *text, const char *voice_id,
                                 muse_tts_pcm_cb_t cb, void *context);
