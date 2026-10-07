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

#include "muse_tts_wav.h"

#include <stdlib.h>
#include <string.h>

#define WAV_HEADER_SIZE 12
#define WAV_CHUNK_HEADER_SIZE 8
#define WAV_FMT_SIZE 16
#define WAV_MAX_BYTES (16u * 1024u * 1024u)
#define PCM_BATCH_FRAMES 256

typedef enum {
    PHASE_RIFF_HEADER,
    PHASE_CHUNK_HEADER,
    PHASE_FMT_BODY,
    PHASE_SKIP_BODY,
    PHASE_DATA_BODY,
    PHASE_PADDING,
    PHASE_FAILED,
} phase_t;

struct muse_tts_wav_decoder {
    phase_t phase;
    esp_err_t error;
    muse_tts_pcm_cb_t cb;
    void *context;
    uint8_t riff_header[WAV_HEADER_SIZE];
    size_t riff_header_used;
    uint8_t chunk_header[WAV_CHUNK_HEADER_SIZE];
    size_t chunk_header_used;
    uint8_t fmt[WAV_FMT_SIZE];
    size_t fmt_used;
    uint32_t chunk_remaining;
    uint8_t padding_remaining;
    uint8_t bytes_per_frame;
    uint8_t channels;
    uint8_t sample_bytes;
    bool format_ready;
    bool data_seen;
    bool sample_byte_pending;
    uint8_t sample_byte;
    int16_t channel_sample[2];
    uint8_t channel_used;
    int32_t downsample_sum;
    uint8_t downsample_count;
    int16_t output[PCM_BATCH_FRAMES];
    size_t output_frames;
    size_t decoded_samples;
    size_t received_bytes;
};

static uint16_t read_u16(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static uint32_t read_u32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static esp_err_t fail(muse_tts_wav_decoder_t *decoder, esp_err_t error)
{
    decoder->phase = PHASE_FAILED;
    decoder->error = error;
    return error;
}

static esp_err_t flush_output(muse_tts_wav_decoder_t *decoder)
{
    if (!decoder->output_frames) {
        return ESP_OK;
    }
    if (!decoder->cb(decoder->output, decoder->output_frames, decoder->context)) {
        return fail(decoder, ESP_FAIL);
    }
    decoder->output_frames = 0;
    return ESP_OK;
}

static esp_err_t emit_sample(muse_tts_wav_decoder_t *decoder, int16_t sample)
{
    decoder->downsample_sum += sample;
    decoder->downsample_count++;
    if (decoder->downsample_count == 3) {
        decoder->output[decoder->output_frames++] = (int16_t)(decoder->downsample_sum / 3);
        decoder->downsample_sum = 0;
        decoder->downsample_count = 0;
        if (decoder->output_frames == PCM_BATCH_FRAMES) {
            return flush_output(decoder);
        }
    }
    return ESP_OK;
}

static void end_chunk(muse_tts_wav_decoder_t *decoder, uint32_t chunk_size)
{
    if (chunk_size & 1u) {
        decoder->padding_remaining = 1;
        decoder->phase = PHASE_PADDING;
    } else {
        decoder->phase = PHASE_CHUNK_HEADER;
        decoder->chunk_header_used = 0;
    }
}

static esp_err_t parse_format(muse_tts_wav_decoder_t *decoder)
{
    uint16_t format = read_u16(decoder->fmt);
    uint16_t channels = read_u16(decoder->fmt + 2);
    uint32_t sample_rate = read_u32(decoder->fmt + 4);
    uint32_t byte_rate = read_u32(decoder->fmt + 8);
    uint16_t block_align = read_u16(decoder->fmt + 12);
    uint16_t bits_per_sample = read_u16(decoder->fmt + 14);

    if (format != 1 || (channels != 1 && channels != 2) || sample_rate != 48000 || bits_per_sample != 16 ||
        block_align != channels * sizeof(int16_t) || byte_rate != sample_rate * block_align) {
        return ESP_ERR_INVALID_RESPONSE;
    }
    decoder->channels = (uint8_t)channels;
    decoder->bytes_per_frame = (uint8_t)block_align;
    decoder->sample_bytes = (uint8_t)sizeof(int16_t);
    decoder->format_ready = true;
    return ESP_OK;
}

static esp_err_t decode_pcm_byte(muse_tts_wav_decoder_t *decoder, uint8_t byte)
{
    if (!decoder->sample_byte_pending) {
        decoder->sample_byte = byte;
        decoder->sample_byte_pending = true;
        return ESP_OK;
    }
    int16_t sample = (int16_t)((uint16_t)decoder->sample_byte | ((uint16_t)byte << 8));
    decoder->sample_byte_pending = false;
    decoder->channel_sample[decoder->channel_used++] = sample;
    if (decoder->channel_used == decoder->channels) {
        int32_t mono = decoder->channels == 1
                           ? decoder->channel_sample[0]
                           : ((int32_t)decoder->channel_sample[0] + decoder->channel_sample[1]) / 2;
        decoder->channel_used = 0;
        decoder->decoded_samples++;
        return emit_sample(decoder, (int16_t)mono);
    }
    return ESP_OK;
}

muse_tts_wav_decoder_t *muse_tts_wav_decoder_create(muse_tts_pcm_cb_t cb, void *context)
{
    if (!cb) {
        return NULL;
    }
    muse_tts_wav_decoder_t *decoder = calloc(1, sizeof(*decoder));
    if (!decoder) {
        return NULL;
    }
    decoder->cb = cb;
    decoder->context = context;
    decoder->phase = PHASE_RIFF_HEADER;
    decoder->error = ESP_OK;
    return decoder;
}

esp_err_t muse_tts_wav_decoder_feed(muse_tts_wav_decoder_t *decoder, const uint8_t *data, size_t len)
{
    if (!decoder || (!data && len)) {
        return ESP_ERR_INVALID_ARG;
    }
    if (decoder->phase == PHASE_FAILED) {
        return decoder->error;
    }
    if (len > WAV_MAX_BYTES - decoder->received_bytes) {
        return fail(decoder, ESP_ERR_INVALID_SIZE);
    }
    decoder->received_bytes += len;

    size_t offset = 0;
    while (offset < len) {
        switch (decoder->phase) {
        case PHASE_RIFF_HEADER: {
            size_t need = WAV_HEADER_SIZE - decoder->riff_header_used;
            size_t take = len - offset < need ? len - offset : need;
            memcpy(decoder->riff_header + decoder->riff_header_used, data + offset, take);
            decoder->riff_header_used += take;
            offset += take;
            if (decoder->riff_header_used == WAV_HEADER_SIZE) {
                uint32_t riff_size = read_u32(decoder->riff_header + 4);
                if (memcmp(decoder->riff_header, "RIFF", 4) || memcmp(decoder->riff_header + 8, "WAVE", 4) ||
                    riff_size < 4 || riff_size > WAV_MAX_BYTES - 8) {
                    return fail(decoder, ESP_ERR_INVALID_RESPONSE);
                }
                decoder->phase = PHASE_CHUNK_HEADER;
            }
            break;
        }
        case PHASE_CHUNK_HEADER: {
            size_t need = WAV_CHUNK_HEADER_SIZE - decoder->chunk_header_used;
            size_t take = len - offset < need ? len - offset : need;
            memcpy(decoder->chunk_header + decoder->chunk_header_used, data + offset, take);
            decoder->chunk_header_used += take;
            offset += take;
            if (decoder->chunk_header_used == WAV_CHUNK_HEADER_SIZE) {
                decoder->chunk_remaining = read_u32(decoder->chunk_header + 4);
                if (!memcmp(decoder->chunk_header, "fmt ", 4)) {
                    if (decoder->chunk_remaining < WAV_FMT_SIZE) {
                        return fail(decoder, ESP_ERR_INVALID_RESPONSE);
                    }
                    decoder->fmt_used = 0;
                    decoder->phase = PHASE_FMT_BODY;
                } else if (!memcmp(decoder->chunk_header, "data", 4)) {
                    if (!decoder->format_ready || decoder->chunk_remaining == 0 ||
                        decoder->chunk_remaining % decoder->bytes_per_frame) {
                        return fail(decoder, ESP_ERR_INVALID_RESPONSE);
                    }
                    decoder->data_seen = true;
                    decoder->phase = PHASE_DATA_BODY;
                } else {
                    decoder->phase = PHASE_SKIP_BODY;
                }
                if (decoder->chunk_remaining == 0 && decoder->phase == PHASE_SKIP_BODY) {
                    end_chunk(decoder, 0);
                }
            }
            break;
        }
        case PHASE_FMT_BODY: {
            size_t need = WAV_FMT_SIZE - decoder->fmt_used;
            size_t take = len - offset < need ? len - offset : need;
            memcpy(decoder->fmt + decoder->fmt_used, data + offset, take);
            decoder->fmt_used += take;
            decoder->chunk_remaining -= (uint32_t)take;
            offset += take;
            if (decoder->fmt_used == WAV_FMT_SIZE) {
                esp_err_t err = parse_format(decoder);
                if (err != ESP_OK) {
                    return fail(decoder, err);
                }
                if (decoder->chunk_remaining) {
                    decoder->phase = PHASE_SKIP_BODY;
                } else {
                    end_chunk(decoder, read_u32(decoder->chunk_header + 4));
                }
            }
            break;
        }
        case PHASE_SKIP_BODY: {
            size_t take = len - offset < decoder->chunk_remaining ? len - offset : decoder->chunk_remaining;
            decoder->chunk_remaining -= (uint32_t)take;
            offset += take;
            if (!decoder->chunk_remaining) {
                end_chunk(decoder, read_u32(decoder->chunk_header + 4));
            }
            break;
        }
        case PHASE_DATA_BODY: {
            size_t take = len - offset < decoder->chunk_remaining ? len - offset : decoder->chunk_remaining;
            size_t end = offset + take;
            while (offset < end) {
                esp_err_t err = decode_pcm_byte(decoder, data[offset++]);
                if (err != ESP_OK) {
                    return err;
                }
            }
            decoder->chunk_remaining -= (uint32_t)take;
            if (!decoder->chunk_remaining) {
                end_chunk(decoder, read_u32(decoder->chunk_header + 4));
            }
            break;
        }
        case PHASE_PADDING:
            offset++;
            decoder->padding_remaining--;
            if (!decoder->padding_remaining) {
                decoder->phase = PHASE_CHUNK_HEADER;
                decoder->chunk_header_used = 0;
            }
            break;
        case PHASE_FAILED:
            return decoder->error;
        }
    }
    return ESP_OK;
}

esp_err_t muse_tts_wav_decoder_finish(muse_tts_wav_decoder_t *decoder)
{
    if (!decoder) {
        return ESP_ERR_INVALID_ARG;
    }
    if (decoder->phase == PHASE_FAILED) {
        return decoder->error;
    }
    if (decoder->phase != PHASE_CHUNK_HEADER || decoder->chunk_header_used != 0 || decoder->padding_remaining ||
        !decoder->format_ready || !decoder->data_seen || decoder->sample_byte_pending || decoder->channel_used ||
        !decoder->decoded_samples) {
        return fail(decoder, ESP_ERR_INVALID_RESPONSE);
    }
    if (decoder->downsample_count) {
        decoder->output[decoder->output_frames++] = (int16_t)(decoder->downsample_sum / decoder->downsample_count);
        decoder->downsample_sum = 0;
        decoder->downsample_count = 0;
    }
    return flush_output(decoder);
}

void muse_tts_wav_decoder_destroy(muse_tts_wav_decoder_t *decoder)
{
    free(decoder);
}
