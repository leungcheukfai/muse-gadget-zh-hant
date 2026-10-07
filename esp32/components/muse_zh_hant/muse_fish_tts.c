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

#include "muse_fish_tts.h"

#include <stdio.h>
#include <string.h>

#include "cJSON.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_log.h"

static const char *TAG = "muse_fish_tts";

#define FISH_TTS_URL "https://api.fish.audio/v1/tts"
#define FISH_TTS_MODEL "s2.1-pro"
#define FISH_TEXT_MAX 2048
#define FISH_API_KEY_MAX 256
#define FISH_VOICE_ID_MAX 128
#define FISH_TIMEOUT_MS 120000

typedef struct {
    muse_tts_wav_decoder_t *decoder;
} request_t;

static bool valid_voice_id(const char *voice_id)
{
    if (!voice_id || !voice_id[0]) {
        return false;
    }
    for (const unsigned char *p = (const unsigned char *)voice_id; *p; p++) {
        if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') ||
              (*p >= '0' && *p <= '9') || *p == '-' || *p == '_')) {
            return false;
        }
    }
    return true;
}

static esp_err_t on_http_event(esp_http_client_event_t *event)
{
    request_t *request = event ? event->user_data : NULL;
    if (!request || !request->decoder || event->event_id != HTTP_EVENT_ON_DATA || event->data_len <= 0) {
        return ESP_OK;
    }
    return muse_tts_wav_decoder_feed(request->decoder, event->data, (size_t)event->data_len);
}

static void wipe(char *value, size_t len)
{
    volatile char *p = value;
    while (len--) {
        *p++ = '\0';
    }
}

esp_err_t muse_fish_tts_generate(const char *api_key, const char *text, const char *voice_id,
                                 muse_tts_pcm_cb_t cb, void *context)
{
    if (!api_key || !api_key[0] || !text || !text[0] || !valid_voice_id(voice_id) || !cb ||
        strchr(api_key, '\r') || strchr(api_key, '\n')) {
        return ESP_ERR_INVALID_ARG;
    }
    size_t key_len = strlen(api_key);
    size_t text_len = strlen(text);
    size_t voice_len = strlen(voice_id);
    if (key_len > FISH_API_KEY_MAX || text_len > FISH_TEXT_MAX || voice_len > FISH_VOICE_ID_MAX) {
        return ESP_ERR_INVALID_SIZE;
    }

    cJSON *body = cJSON_CreateObject();
    if (!body) {
        return ESP_ERR_NO_MEM;
    }
    if (!cJSON_AddStringToObject(body, "text", text) ||
        !cJSON_AddStringToObject(body, "reference_id", voice_id) ||
        !cJSON_AddStringToObject(body, "format", "wav")) {
        cJSON_Delete(body);
        return ESP_ERR_NO_MEM;
    }
    char *json = cJSON_PrintUnformatted(body);
    cJSON_Delete(body);
    if (!json) {
        return ESP_ERR_NO_MEM;
    }
    size_t json_len = strlen(json);
    if (json_len > INT32_MAX) {
        cJSON_free(json);
        return ESP_ERR_INVALID_SIZE;
    }

    request_t request = {0};
    request.decoder = muse_tts_wav_decoder_create(cb, context);
    if (!request.decoder) {
        cJSON_free(json);
        return ESP_ERR_NO_MEM;
    }

    esp_http_client_config_t config = {
        .url = FISH_TTS_URL,
        .timeout_ms = FISH_TIMEOUT_MS,
        .buffer_size = 4096,
        .buffer_size_tx = 2048,
        .event_handler = on_http_event,
        .user_data = &request,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .disable_auto_redirect = true,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client) {
        cJSON_free(json);
        muse_tts_wav_decoder_destroy(request.decoder);
        return ESP_ERR_NO_MEM;
    }

    char authorization[sizeof("Bearer ") + FISH_API_KEY_MAX] = "Bearer ";
    memcpy(authorization + sizeof("Bearer ") - 1, api_key, key_len);
    authorization[sizeof("Bearer ") - 1 + key_len] = '\0';

    esp_err_t err = esp_http_client_set_method(client, HTTP_METHOD_POST);
    if (err == ESP_OK) {
        err = esp_http_client_set_header(client, "Authorization", authorization);
    }
    if (err == ESP_OK) {
        err = esp_http_client_set_header(client, "Content-Type", "application/json");
    }
    if (err == ESP_OK) {
        err = esp_http_client_set_header(client, "model", FISH_TTS_MODEL);
    }
    if (err == ESP_OK) {
        err = esp_http_client_set_post_field(client, json, (int)json_len);
    }
    if (err == ESP_OK) {
        err = esp_http_client_perform(client);
    }
    /* Remove the credential from the client header map before destroying it. */
    esp_http_client_set_header(client, "Authorization", "");
    wipe(authorization, sizeof(authorization));

    if (err == ESP_OK) {
        int status = esp_http_client_get_status_code(client);
        if (status < 200 || status >= 300) {
            ESP_LOGW(TAG, "Fish Audio TTS HTTP status %d", status);
            err = ESP_FAIL;
        }
    }
    if (err == ESP_OK) {
        err = muse_tts_wav_decoder_finish(request.decoder);
    }
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Fish Audio TTS request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    cJSON_free(json);
    muse_tts_wav_decoder_destroy(request.decoder);
    return err;
}
