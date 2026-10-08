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

static const char *const LEGACY_KEYS[] = {
    "gemini_key", "canto_url", "canto_token",
    "fish_api_key", "fish_voice_yue", "fish_voice_zh",
};
static bool s_present[sizeof(LEGACY_KEYS) / sizeof(LEGACY_KEYS[0])];
static bool s_erase_pending[sizeof(LEGACY_KEYS) / sizeof(LEGACY_KEYS[0])];
static unsigned s_commit_count;

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
void nvs_close(nvs_handle_t handle) { (void)handle; }
esp_err_t nvs_get_str(nvs_handle_t handle, const char *key, char *out, size_t *length)
{
    (void)handle;
    (void)key;
    (void)out;
    (void)length;
    return ESP_ERR_NVS_NOT_FOUND;
}
esp_err_t nvs_set_str(nvs_handle_t handle, const char *key, const char *value)
{
    (void)handle;
    (void)key;
    (void)value;
    return ESP_OK;
}
esp_err_t nvs_erase_key(nvs_handle_t handle, const char *key)
{
    (void)handle;
    for (size_t i = 0; i < sizeof(LEGACY_KEYS) / sizeof(LEGACY_KEYS[0]); i++) {
        if (!strcmp(key, LEGACY_KEYS[i]) && s_present[i]) {
            s_erase_pending[i] = true;
            return ESP_OK;
        }
    }
    return ESP_ERR_NVS_NOT_FOUND;
}
esp_err_t nvs_commit(nvs_handle_t handle)
{
    (void)handle;
    s_commit_count++;
    for (size_t i = 0; i < sizeof(LEGACY_KEYS) / sizeof(LEGACY_KEYS[0]); i++) {
        if (s_erase_pending[i]) {
            s_present[i] = false;
            s_erase_pending[i] = false;
        }
    }
    return ESP_OK;
}
esp_err_t nvs_get_u8(nvs_handle_t handle, const char *key, unsigned char *out)
{
    (void)handle;
    (void)key;
    (void)out;
    return ESP_ERR_NVS_NOT_FOUND;
}
esp_err_t nvs_set_u8(nvs_handle_t handle, const char *key, unsigned char value)
{
    (void)handle;
    (void)key;
    (void)value;
    return ESP_OK;
}
esp_err_t nvs_get_u16(nvs_handle_t handle, const char *key, unsigned short *out)
{
    (void)handle;
    (void)key;
    (void)out;
    return ESP_ERR_NVS_NOT_FOUND;
}
esp_err_t nvs_set_u16(nvs_handle_t handle, const char *key, unsigned short value)
{
    (void)handle;
    (void)key;
    (void)value;
    return ESP_OK;
}

int main(void)
{
    for (size_t i = 0; i < sizeof(LEGACY_KEYS) / sizeof(LEGACY_KEYS[0]); i++) {
        s_present[i] = true;
    }

    CHECK(muse_settings_init() == ESP_OK);
    for (size_t i = 0; i < sizeof(LEGACY_KEYS) / sizeof(LEGACY_KEYS[0]); i++) {
        CHECK(!s_present[i]);
    }
    CHECK(s_commit_count == 1);
    return 0;
}
