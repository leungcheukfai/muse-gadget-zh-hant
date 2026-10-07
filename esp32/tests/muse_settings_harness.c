/* Copyright (c) Meta Platforms, Inc. and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "muse_settings.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        exit(1); \
    } \
} while (0)

static char s_persisted_key[MUSE_FISH_API_KEY_MAX + 1];
static char s_pending_key[MUSE_FISH_API_KEY_MAX + 1];
static char s_persisted_yue[MUSE_FISH_VOICE_ID_MAX + 1];
static char s_pending_yue[MUSE_FISH_VOICE_ID_MAX + 1];
static char s_persisted_zh[MUSE_FISH_VOICE_ID_MAX + 1];
static char s_pending_zh[MUSE_FISH_VOICE_ID_MAX + 1];
static char s_legacy_gemini[129];
static char s_legacy_canto_url[256];
static char s_legacy_canto_token[129];
static bool s_pending_key_dirty, s_pending_yue_dirty, s_pending_zh_dirty;
static bool s_erase_gemini, s_erase_canto_url, s_erase_canto_token;
static bool s_fail_set, s_fail_commit;
static unsigned s_notifications;

#ifndef __APPLE__
size_t strlcpy(char *dst, const char *src, size_t dst_size)
{
    size_t src_len = strlen(src);
    if (dst_size) {
        size_t copy_len = src_len < dst_size - 1 ? src_len : dst_size - 1;
        memcpy(dst, src, copy_len);
        dst[copy_len] = '\0';
    }
    return src_len;
}
#endif

SemaphoreHandle_t xSemaphoreCreateMutex(void) { return (SemaphoreHandle_t)1; }
BaseType_t xSemaphoreTake(SemaphoreHandle_t semaphore, TickType_t wait_ticks)
{
    (void)semaphore;
    (void)wait_ticks;
    return pdTRUE;
}
BaseType_t xSemaphoreGive(SemaphoreHandle_t semaphore)
{
    (void)semaphore;
    return pdTRUE;
}

bool muse_link_wifi_get(char *ssid, char *pass)
{
    (void)ssid;
    (void)pass;
    return false;
}
bool muse_link_wifi_set(const char *ssid, const char *pass)
{
    (void)ssid;
    (void)pass;
    return false;
}

esp_err_t nvs_flash_init(void) { return ESP_OK; }
esp_err_t nvs_open(const char *name, nvs_open_mode_t mode, nvs_handle_t *out)
{
    (void)name;
    (void)mode;
    *out = 1;
    return ESP_OK;
}
void nvs_close(nvs_handle_t handle)
{
    (void)handle;
    s_pending_key[0] = s_pending_yue[0] = s_pending_zh[0] = '\0';
    s_pending_key_dirty = s_pending_yue_dirty = s_pending_zh_dirty = false;
    s_erase_gemini = s_erase_canto_url = s_erase_canto_token = false;
}
static const char *persisted_value(const char *key)
{
    if (!strcmp(key, "fish_api_key")) return s_persisted_key;
    if (!strcmp(key, "fish_voice_yue")) return s_persisted_yue;
    if (!strcmp(key, "fish_voice_zh")) return s_persisted_zh;
    return NULL;
}
esp_err_t nvs_get_str(nvs_handle_t handle, const char *key, char *out, size_t *length)
{
    (void)handle;
    const char *stored = persisted_value(key);
    if (!stored || !stored[0]) return ESP_ERR_NVS_NOT_FOUND;
    size_t need = strlen(stored) + 1;
    if (!out) {
        *length = need;
        return ESP_OK;
    }
    if (*length < need) {
        *length = need;
        return ESP_ERR_NVS_INVALID_LENGTH;
    }
    memcpy(out, stored, need);
    *length = need;
    return ESP_OK;
}
esp_err_t nvs_set_str(nvs_handle_t handle, const char *key, const char *value)
{
    (void)handle;
    if (s_fail_set) return ESP_FAIL;
    if (!strcmp(key, "fish_api_key")) {
        strlcpy(s_pending_key, value, sizeof(s_pending_key));
        s_pending_key_dirty = true;
    } else if (!strcmp(key, "fish_voice_yue")) {
        strlcpy(s_pending_yue, value, sizeof(s_pending_yue));
        s_pending_yue_dirty = true;
    } else if (!strcmp(key, "fish_voice_zh")) {
        strlcpy(s_pending_zh, value, sizeof(s_pending_zh));
        s_pending_zh_dirty = true;
    } else {
        return ESP_ERR_INVALID_ARG;
    }
    return ESP_OK;
}
esp_err_t nvs_erase_key(nvs_handle_t handle, const char *key)
{
    (void)handle;
    if (!strcmp(key, "gemini_key") && s_legacy_gemini[0]) s_erase_gemini = true;
    else if (!strcmp(key, "canto_url") && s_legacy_canto_url[0]) s_erase_canto_url = true;
    else if (!strcmp(key, "canto_token") && s_legacy_canto_token[0]) s_erase_canto_token = true;
    else return ESP_ERR_NVS_NOT_FOUND;
    return ESP_OK;
}
esp_err_t nvs_commit(nvs_handle_t handle)
{
    (void)handle;
    if (s_fail_commit) return ESP_FAIL;
    if (s_pending_key_dirty) strlcpy(s_persisted_key, s_pending_key, sizeof(s_persisted_key));
    if (s_pending_yue_dirty) strlcpy(s_persisted_yue, s_pending_yue, sizeof(s_persisted_yue));
    if (s_pending_zh_dirty) strlcpy(s_persisted_zh, s_pending_zh, sizeof(s_persisted_zh));
    if (s_erase_gemini) s_legacy_gemini[0] = '\0';
    if (s_erase_canto_url) s_legacy_canto_url[0] = '\0';
    if (s_erase_canto_token) s_legacy_canto_token[0] = '\0';
    s_pending_key_dirty = s_pending_yue_dirty = s_pending_zh_dirty = false;
    s_erase_gemini = s_erase_canto_url = s_erase_canto_token = false;
    return ESP_OK;
}
esp_err_t nvs_get_u8(nvs_handle_t handle, const char *key, unsigned char *out)
{
    (void)handle; (void)key; (void)out;
    return ESP_ERR_NVS_NOT_FOUND;
}
esp_err_t nvs_set_u8(nvs_handle_t handle, const char *key, unsigned char value)
{
    (void)handle; (void)key; (void)value;
    return ESP_OK;
}
esp_err_t nvs_get_u16(nvs_handle_t handle, const char *key, unsigned short *out)
{
    (void)handle; (void)key; (void)out;
    return ESP_ERR_NVS_NOT_FOUND;
}
esp_err_t nvs_set_u16(nvs_handle_t handle, const char *key, unsigned short value)
{
    (void)handle; (void)key; (void)value;
    return ESP_OK;
}

static void on_setting(muse_setting_t setting)
{
    CHECK(setting == MUSE_SETTING_FISH);
    s_notifications++;
}

int main(void)
{
    strlcpy(s_persisted_key, "preexisting-key", sizeof(s_persisted_key));
    strlcpy(s_legacy_gemini, "old-secret", sizeof(s_legacy_gemini));
    strlcpy(s_legacy_canto_url, "http://old-host", sizeof(s_legacy_canto_url));
    strlcpy(s_legacy_canto_token, "old-token", sizeof(s_legacy_canto_token));
    CHECK(muse_settings_init() == ESP_OK);
    CHECK(!s_legacy_gemini[0] && !s_legacy_canto_url[0] && !s_legacy_canto_token[0]);
    char key[MUSE_FISH_API_KEY_MAX + 1] = {};
    char voice[MUSE_FISH_VOICE_ID_MAX + 1] = {};
    muse_settings_set_listener(on_setting);

#if CONFIG_HOMEHUB_NVS_ENCRYPTION
    muse_settings_fish_api_key(key);
    CHECK(strcmp(key, "preexisting-key") == 0);
    CHECK(muse_settings_fish_api_key_len() == strlen("preexisting-key"));
    CHECK(muse_settings_set_fish_api_key("new-", false) == ESP_OK);
    CHECK(muse_settings_set_fish_api_key("key", true) == ESP_OK);
    CHECK(strcmp(s_persisted_key, "new-key") == 0);

    s_fail_commit = true;
    CHECK(muse_settings_set_fish_api_key("rejected", false) == ESP_FAIL);
    CHECK(strcmp(s_persisted_key, "new-key") == 0);
    muse_settings_fish_api_key(key);
    CHECK(strcmp(key, "new-key") == 0);
    s_fail_commit = false;

    muse_settings_set_listener(NULL);
    muse_settings_set_reply_language(MUSE_REPLY_MANDARIN);
    CHECK(strcmp(s_persisted_key, "new-key") == 0);
    muse_settings_set_listener(on_setting);

    char too_long[MUSE_FISH_API_KEY_MAX + 2];
    memset(too_long, 'x', sizeof(too_long) - 1);
    too_long[sizeof(too_long) - 1] = '\0';
    CHECK(muse_settings_set_fish_api_key(too_long, false) == ESP_ERR_INVALID_SIZE);
    CHECK(strcmp(s_persisted_key, "new-key") == 0);
    CHECK(muse_settings_set_fish_api_key("", false) == ESP_OK);
    CHECK(s_persisted_key[0] == '\0');
    CHECK(muse_settings_fish_api_key_len() == 0);
#else
    muse_settings_fish_api_key(key);
    CHECK(key[0] == '\0');
    CHECK(muse_settings_fish_api_key_len() == 0);
    CHECK(muse_settings_set_fish_api_key("rejected", false) == ESP_ERR_NOT_SUPPORTED);
    CHECK(strcmp(s_persisted_key, "preexisting-key") == 0);
#endif

    CHECK(muse_settings_set_fish_voice_id(MUSE_REPLY_CANTONESE, "yue_voice_123") == ESP_OK);
    CHECK(muse_settings_set_fish_voice_id(MUSE_REPLY_MANDARIN, "mandarin-voice") == ESP_OK);
    muse_settings_fish_voice_id(MUSE_REPLY_CANTONESE, voice);
    CHECK(strcmp(voice, "yue_voice_123") == 0);
    muse_settings_fish_voice_id(MUSE_REPLY_MANDARIN, voice);
    CHECK(strcmp(voice, "mandarin-voice") == 0);
    CHECK(muse_settings_set_fish_voice_id(MUSE_REPLY_MANDARIN, "../invalid") == ESP_ERR_INVALID_ARG);
    CHECK(muse_settings_set_fish_voice_id(MUSE_REPLY_MANDARIN, "") == ESP_OK);
    muse_settings_fish_voice_id(MUSE_REPLY_MANDARIN, voice);
    CHECK(voice[0] == '\0');
    CHECK(s_notifications >= 2);
    return 0;
}
