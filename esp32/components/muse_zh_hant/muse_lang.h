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

typedef enum {
    MUSE_REPLY_CANTONESE,
    MUSE_REPLY_MANDARIN,
} muse_reply_language_t;

#ifdef __cplusplus
extern "C" {
#endif

/* Traditional Chinese UI copy. Unknown strings are returned unchanged. */
const char *muse_lang_get(const char *english);
/* Formats the localized version of an English printf-style template. */
int muse_lang_snprintf(char *out, size_t out_size, const char *english_format, ...)
    __attribute__((format(printf, 3, 4)));

/* Instructions sent to Muse to keep replies in the language chosen by the user. */
const char *muse_lang_chat_instruction(muse_reply_language_t language);
/* Returns the required length, like snprintf; -1 for invalid arguments. */
int muse_lang_build_chat_prompt(muse_reply_language_t language, const char *text, char *out, size_t out_size);

#ifdef __cplusplus
}
#endif
