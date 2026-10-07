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

#include "muse_font.h"

#include <stddef.h>

#include "sdkconfig.h"

#if CONFIG_MUSE_CJK_FONT
LV_FONT_DECLARE(muse_font_cjk_16)

const lv_font_t *muse_font_for_ui(const lv_font_t *base)
{
    enum { FONT_CACHE_SIZE = 12 };
    static const lv_font_t *originals[FONT_CACHE_SIZE];
    static lv_font_t fonts[FONT_CACHE_SIZE];

    if (!base || base == &muse_font_cjk_16) {
        return base;
    }
    for (size_t i = 0; i < FONT_CACHE_SIZE; i++) {
        if (originals[i] == base) {
            return &fonts[i];
        }
    }
    for (size_t i = 0; i < FONT_CACHE_SIZE; i++) {
        if (!originals[i]) {
            originals[i] = base;
            fonts[i] = *base;
            fonts[i].fallback = &muse_font_cjk_16;
            return &fonts[i];
        }
    }
    return base;
}
#else
const lv_font_t *muse_font_for_ui(const lv_font_t *base)
{
    return base;
}
#endif
