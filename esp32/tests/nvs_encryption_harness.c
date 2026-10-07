/* Copyright (c) Meta Platforms, Inc. and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <assert.h>
#include <stdbool.h>
#include <string.h>

#include "config_store.h"
#include "esp_efuse.h"
#include "nvs.h"

esp_efuse_purpose_t fake_hmac_purpose;
int fake_read_disable_locked;
static int s_encrypted_init_calls;
static int s_plaintext_init_calls;

esp_efuse_purpose_t esp_efuse_get_key_purpose(esp_efuse_block_t block)
{
    assert(block == EFUSE_BLK_KEY0);
    return fake_hmac_purpose;
}

int esp_efuse_read_field_bit(const void *field[])
{
    (void)field;
    return fake_read_disable_locked;
}

esp_err_t nvs_flash_init(void)
{
    s_encrypted_init_calls++;
    return ESP_OK;
}

esp_err_t nvs_flash_init_partition(const char *partition)
{
    (void)partition;
    s_plaintext_init_calls++;
    return ESP_OK;
}

esp_err_t nvs_open(const char *name, nvs_open_mode_t mode, nvs_handle_t *handle)
{
    (void)name;
    (void)mode;
    *handle = 1;
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
    (void)key;
    return ESP_OK;
}
esp_err_t nvs_commit(nvs_handle_t handle)
{
    (void)handle;
    return ESP_OK;
}
const char *esp_err_to_name(esp_err_t error)
{
    (void)error;
    return "fake error";
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    if (!strcmp(argv[1], "provisioned")) {
        fake_hmac_purpose = ESP_EFUSE_KEY_PURPOSE_HMAC_UP;
        config_store_init();
        assert(s_encrypted_init_calls == 1);
        assert(s_plaintext_init_calls == 0);
        return 0;
    }
    assert(!strcmp(argv[1], "unprovisioned"));
    fake_hmac_purpose = 0;
    config_store_init(); /* must abort before NVS can auto-generate an eFuse key */
    return 1;
}
