# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Source-level Traditional Chinese extension for the upstream Muse component.
# Muse calls its language/font helpers, and its Hatch chat path calls Fish Audio TTS.
set(srcs "muse_lang.c")
set(requires)
# ESP-IDF resolves component dependencies before Kconfig-controlled source
# selection, so declare the TTS dependencies unconditionally. The TTS source is
# still only compiled for Hatch and Voice builds below.
set(priv_requires esp_http_client esp-tls mbedtls espressif__cjson)

if(CONFIG_MUSE_ENABLED)
    list(APPEND srcs "muse_font.c")
    list(APPEND requires lvgl)
    if(CONFIG_MUSE_CJK_FONT)
        list(APPEND srcs "fonts/muse_font_cjk_16.c")
    endif()
endif()

if(CONFIG_MUSE_HATCH OR CONFIG_HOMEHUB_VOICE)
    list(APPEND srcs "muse_fish_tts.c" "muse_tts_wav.c")
endif()

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "."
    REQUIRES ${requires}
    PRIV_REQUIRES ${priv_requires})
