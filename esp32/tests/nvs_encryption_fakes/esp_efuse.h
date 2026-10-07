#pragma once

typedef int esp_efuse_block_t;
typedef int esp_efuse_purpose_t;

#define EFUSE_BLK_KEY0 4
#define ESP_EFUSE_KEY_PURPOSE_HMAC_UP 8

extern esp_efuse_purpose_t fake_hmac_purpose;
extern int fake_read_disable_locked;

esp_efuse_purpose_t esp_efuse_get_key_purpose(esp_efuse_block_t block);
int esp_efuse_read_field_bit(const void *field[]);
