/* Copyright (c) Meta Platforms, Inc. and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "esp_err.h"

typedef struct muse_tts_wav_decoder muse_tts_wav_decoder_t;
typedef bool (*muse_tts_pcm_cb_t)(const int16_t *pcm, size_t frames, void *context);

muse_tts_wav_decoder_t *muse_tts_wav_decoder_create(muse_tts_pcm_cb_t cb, void *context);
esp_err_t muse_tts_wav_decoder_feed(muse_tts_wav_decoder_t *decoder, const uint8_t *data, size_t len);
esp_err_t muse_tts_wav_decoder_finish(muse_tts_wav_decoder_t *decoder);
void muse_tts_wav_decoder_destroy(muse_tts_wav_decoder_t *decoder);

typedef struct {
    int16_t samples[32];
    size_t frames;
    unsigned calls;
    bool accept;
} capture_t;

static bool capture_pcm(const int16_t *pcm, size_t frames, void *context)
{
    capture_t *capture = context;
    assert(capture->frames + frames <= sizeof(capture->samples) / sizeof(capture->samples[0]));
    memcpy(capture->samples + capture->frames, pcm, frames * sizeof(pcm[0]));
    capture->frames += frames;
    capture->calls++;
    return capture->accept;
}

static void put_u16(uint8_t *out, uint16_t value)
{
    out[0] = (uint8_t)value;
    out[1] = (uint8_t)(value >> 8);
}

static void put_u32(uint8_t *out, uint32_t value)
{
    out[0] = (uint8_t)value;
    out[1] = (uint8_t)(value >> 8);
    out[2] = (uint8_t)(value >> 16);
    out[3] = (uint8_t)(value >> 24);
}

static size_t make_wav(uint8_t *out, bool valid, bool complete)
{
    static const int16_t samples[] = {
        300, 600, 600, 900, 900, 1200,
        -300, -600, -600, -900, -900, -1200,
    };
    const size_t data_size = sizeof(samples);
    const size_t total = 44 + data_size;
    memset(out, 0, total);
    memcpy(out, valid ? "RIFF" : "NOPE", 4);
    put_u32(out + 4, (uint32_t)(total - 8));
    memcpy(out + 8, "WAVEfmt ", 8);
    put_u32(out + 16, 16);
    put_u16(out + 20, 1);       /* PCM */
    put_u16(out + 22, 2);       /* stereo */
    put_u32(out + 24, 48000);
    put_u32(out + 28, 192000);
    put_u16(out + 32, 4);
    put_u16(out + 34, 16);
    memcpy(out + 36, "data", 4);
    put_u32(out + 40, (uint32_t)data_size);
    memcpy(out + 44, samples, data_size);
    return complete ? total : total - 1;
}

static esp_err_t decode(const uint8_t *wav, size_t len, capture_t *capture)
{
    muse_tts_wav_decoder_t *decoder = muse_tts_wav_decoder_create(capture_pcm, capture);
    assert(decoder);
    esp_err_t err = ESP_OK;
    /* Deliberately split the RIFF header, chunk headers, and PCM samples. */
    for (size_t offset = 0; offset < len && err == ESP_OK;) {
        size_t chunk = len - offset < 7 ? len - offset : 7;
        err = muse_tts_wav_decoder_feed(decoder, wav + offset, chunk);
        offset += chunk;
    }
    if (err == ESP_OK) {
        err = muse_tts_wav_decoder_finish(decoder);
    }
    muse_tts_wav_decoder_destroy(decoder);
    return err;
}

int main(void)
{
    uint8_t wav[44 + 12 * sizeof(int16_t)];
    capture_t capture = { .accept = true };
    size_t len = make_wav(wav, true, true);
    assert(decode(wav, len, &capture) == ESP_OK);
    assert(capture.frames == 2);
    assert(capture.samples[0] == 750);
    assert(capture.samples[1] == -750);

    capture = (capture_t){ .accept = true };
    len = make_wav(wav, false, true);
    assert(decode(wav, len, &capture) == ESP_ERR_INVALID_RESPONSE);
    assert(capture.frames == 0);

    capture = (capture_t){ .accept = true };
    len = make_wav(wav, true, false);
    assert(decode(wav, len, &capture) == ESP_ERR_INVALID_RESPONSE);

    capture = (capture_t){ .accept = false };
    len = make_wav(wav, true, true);
    assert(decode(wav, len, &capture) == ESP_FAIL);
    return 0;
}
