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

#include "muse_wake.h"

#include <cstring>
#include <new>

#include "esp_heap_caps.h"
#include "esp_log.h"
#include "frontend_util.h"
#include "muse_audio.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/micro_resource_variable.h"
#include "tensorflow/lite/schema/schema_generated.h"

extern "C" const uint8_t hey_muse_model_start[] asm("_binary_hey_muse_tflite_start");

namespace {

constexpr char TAG[] = "muse_wake";
constexpr size_t FEATURE_COUNT = 40;
constexpr size_t FEATURE_DURATION_MS = 30;
constexpr size_t FEATURE_STEP_MS = 10;
constexpr size_t TENSOR_ARENA_SIZE = 30000;
constexpr size_t VARIABLE_ARENA_SIZE = 1024;
constexpr size_t PROBABILITY_WINDOW = 6;
constexpr size_t WARMUP_WINDOWS = 100;
constexpr uint8_t PROBABILITY_CUTOFF = static_cast<uint8_t>(0.99f * 255.0f);

struct FrontendState s_frontend{};
bool s_frontend_ready;
bool s_enabled;
bool s_resolver_ready;

uint8_t *s_tensor_arena;
uint8_t *s_variable_arena;
tflite::MicroAllocator *s_variable_allocator;
tflite::MicroResourceVariables *s_resource_variables;
tflite::MicroInterpreter *s_interpreter;
tflite::MicroMutableOpResolver<20> s_resolver;
TfLiteTensor *s_input;
TfLiteTensor *s_output;

size_t s_stride;
size_t s_stride_step;
uint8_t s_probabilities[PROBABILITY_WINDOW];
size_t s_probability_next;
size_t s_probability_fill;
int16_t s_warmup_remaining;

void *arena_alloc(size_t size)
{
    void *memory = heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    return memory ? memory : heap_caps_malloc(size, MALLOC_CAP_8BIT);
}

void release_model(void)
{
    delete s_interpreter;
    s_interpreter = nullptr;
    s_input = nullptr;
    s_output = nullptr;
    s_variable_allocator = nullptr;
    s_resource_variables = nullptr;
    if (s_tensor_arena) {
        heap_caps_free(s_tensor_arena);
        s_tensor_arena = nullptr;
    }
    if (s_variable_arena) {
        heap_caps_free(s_variable_arena);
        s_variable_arena = nullptr;
    }
    s_stride = s_stride_step = 0;
}

void reset_probabilities(void)
{
    std::memset(s_probabilities, 0, sizeof(s_probabilities));
    s_probability_next = 0;
    s_probability_fill = 0;
    s_warmup_remaining = -static_cast<int16_t>(WARMUP_WINDOWS);
}

bool register_ops(void)
{
    if (s_resolver_ready) {
        return true;
    }
#define ADD_WAKE_OP(name) \
    do { \
        if (s_resolver.Add##name() != kTfLiteOk) { \
            ESP_LOGE(TAG, "could not register TFLite op %s", #name); \
            return false; \
        } \
    } while (0)
    ADD_WAKE_OP(CallOnce);
    ADD_WAKE_OP(VarHandle);
    ADD_WAKE_OP(Reshape);
    ADD_WAKE_OP(ReadVariable);
    ADD_WAKE_OP(StridedSlice);
    ADD_WAKE_OP(Concatenation);
    ADD_WAKE_OP(AssignVariable);
    ADD_WAKE_OP(Conv2D);
    ADD_WAKE_OP(Mul);
    ADD_WAKE_OP(Add);
    ADD_WAKE_OP(Mean);
    ADD_WAKE_OP(FullyConnected);
    ADD_WAKE_OP(Logistic);
    ADD_WAKE_OP(Quantize);
    ADD_WAKE_OP(DepthwiseConv2D);
    ADD_WAKE_OP(AveragePool2D);
    ADD_WAKE_OP(MaxPool2D);
    ADD_WAKE_OP(Pad);
    ADD_WAKE_OP(Pack);
    ADD_WAKE_OP(SplitV);
#undef ADD_WAKE_OP
    s_resolver_ready = true;
    return true;
}

bool init_model(void)
{
    if (s_interpreter) {
        return true;
    }
    if (!register_ops()) {
        return false;
    }

    const tflite::Model *model = tflite::GetModel(hey_muse_model_start);
    if (!model || model->version() != TFLITE_SCHEMA_VERSION) {
        ESP_LOGE(TAG, "Hey Muse model schema is not supported");
        return false;
    }

    s_tensor_arena = static_cast<uint8_t *>(arena_alloc(TENSOR_ARENA_SIZE));
    s_variable_arena = static_cast<uint8_t *>(arena_alloc(VARIABLE_ARENA_SIZE));
    if (!s_tensor_arena || !s_variable_arena) {
        ESP_LOGE(TAG, "could not allocate TFLite arenas");
        release_model();
        return false;
    }

    s_variable_allocator = tflite::MicroAllocator::Create(s_variable_arena, VARIABLE_ARENA_SIZE);
    if (!s_variable_allocator) {
        ESP_LOGE(TAG, "could not create TFLite variable allocator");
        release_model();
        return false;
    }
    s_resource_variables = tflite::MicroResourceVariables::Create(s_variable_allocator, 20);
    if (!s_resource_variables) {
        ESP_LOGE(TAG, "could not create TFLite resource variables");
        release_model();
        return false;
    }

    s_interpreter = new (std::nothrow) tflite::MicroInterpreter(
        model, s_resolver, s_tensor_arena, TENSOR_ARENA_SIZE, s_resource_variables);
    if (!s_interpreter || s_interpreter->AllocateTensors() != kTfLiteOk) {
        ESP_LOGE(TAG, "could not allocate Hey Muse model tensors");
        release_model();
        return false;
    }

    s_input = s_interpreter->input(0);
    s_output = s_interpreter->output(0);
    if (!s_input || s_input->type != kTfLiteInt8 || s_input->dims->size != 3 ||
        s_input->dims->data[0] != 1 || s_input->dims->data[2] != FEATURE_COUNT ||
        !s_input->data.int8 || s_input->dims->data[1] <= 0 ||
        !s_output || s_output->type != kTfLiteUInt8 || s_output->dims->size != 2 ||
        s_output->dims->data[0] != 1 || s_output->dims->data[1] != 1 || !s_output->data.uint8) {
        ESP_LOGE(TAG, "Hey Muse model tensor dimensions or types do not match microWakeWord");
        release_model();
        return false;
    }

    s_stride = static_cast<size_t>(s_input->dims->data[1]);
    s_stride_step = 0;
    reset_probabilities();
    ESP_LOGI(TAG, "Hey Muse model ready (arena %u bytes, stride %u)",
             static_cast<unsigned>(TENSOR_ARENA_SIZE), static_cast<unsigned>(s_stride));
    return true;
}

bool init_frontend(void)
{
    struct FrontendConfig config{};
    FrontendFillConfigWithDefaults(&config);
    config.window.size_ms = FEATURE_DURATION_MS;
    config.window.step_size_ms = FEATURE_STEP_MS;
    config.filterbank.num_channels = FEATURE_COUNT;
    config.filterbank.lower_band_limit = 125.0f;
    config.filterbank.upper_band_limit = 7500.0f;
    config.noise_reduction.smoothing_bits = 10;
    config.noise_reduction.even_smoothing = 0.025f;
    config.noise_reduction.odd_smoothing = 0.06f;
    config.noise_reduction.min_signal_remaining = 0.05f;
    config.pcan_gain_control.enable_pcan = true;
    config.pcan_gain_control.strength = 0.95f;
    config.pcan_gain_control.offset = 80.0f;
    config.pcan_gain_control.gain_bits = 21;
    config.log_scale.enable_log = true;
    config.log_scale.scale_shift = 6;
    if (!FrontendPopulateState(&config, &s_frontend, MUSE_AUDIO_RATE)) {
        ESP_LOGE(TAG, "could not initialize microWakeWord audio frontend");
        FrontendFreeStateContents(&s_frontend);
        return false;
    }
    s_frontend_ready = true;
    return true;
}

bool invoke_feature(const struct FrontendOutput *feature)
{
    if (!s_interpreter || !s_input || !s_output || feature->size != FEATURE_COUNT) {
        return false;
    }

    int8_t quantized[FEATURE_COUNT];
    for (size_t i = 0; i < FEATURE_COUNT; i++) {
        int32_t value = (static_cast<int32_t>(feature->values[i]) * 256 + 333) / 666;
        value += INT8_MIN;
        if (value < INT8_MIN) {
            value = INT8_MIN;
        } else if (value > INT8_MAX) {
            value = INT8_MAX;
        }
        quantized[i] = static_cast<int8_t>(value);
    }

    int8_t *input = s_input->data.int8 + FEATURE_COUNT * s_stride_step;
    std::memcpy(input, quantized, sizeof(quantized));
    if (++s_stride_step < s_stride) {
        return false;
    }
    s_stride_step = 0;

    if (s_interpreter->Invoke() != kTfLiteOk) {
        ESP_LOGE(TAG, "Hey Muse inference failed; disabling wake detection");
        s_enabled = false;
        return false;
    }

    uint8_t probability = s_output->data.uint8[0];
    s_probabilities[s_probability_next] = probability;
    s_probability_next = (s_probability_next + 1) % PROBABILITY_WINDOW;
    if (s_probability_fill < PROBABILITY_WINDOW) {
        s_probability_fill++;
    }
    if (s_warmup_remaining < 0) {
        if (probability < PROBABILITY_CUTOFF) {
            s_warmup_remaining++;
        }
        return false;
    }
    if (s_probability_fill < PROBABILITY_WINDOW) {
        return false;
    }
    uint32_t sum = 0;
    for (uint8_t p : s_probabilities) {
        sum += p;
    }
    return sum > static_cast<uint32_t>(PROBABILITY_CUTOFF) * PROBABILITY_WINDOW;
}

} // namespace

extern "C" esp_err_t muse_wake_init(void)
{
    if (s_enabled) {
        return ESP_OK;
    }
    if (!init_frontend()) {
        return ESP_FAIL;
    }
    if (!init_model()) {
        FrontendFreeStateContents(&s_frontend);
        s_frontend_ready = false;
        return ESP_FAIL;
    }
    s_enabled = true;
    ESP_LOGI(TAG, "local Hey Muse detection enabled");
    return ESP_OK;
}

extern "C" bool muse_wake_process(const int16_t *pcm, size_t frames)
{
    if (!s_enabled || !s_frontend_ready || !pcm || !frames) {
        return false;
    }
    if (!init_model()) {
        ESP_LOGE(TAG, "could not reload Hey Muse model; disabling wake detection");
        s_enabled = false;
        return false;
    }

    size_t offset = 0;
    while (offset < frames && s_enabled) {
        size_t consumed = 0;
        struct FrontendOutput feature = FrontendProcessSamples(
            &s_frontend, pcm + offset, frames - offset, &consumed);
        offset += consumed;
        if (!feature.size) {
            if (!consumed) {
                break;
            }
            continue;
        }
        if (invoke_feature(&feature)) {
            ESP_LOGI(TAG, "detected Hey Muse");
            return true;
        }
    }
    return false;
}

extern "C" void muse_wake_reset(void)
{
    if (s_frontend_ready) {
        FrontendReset(&s_frontend);
    }
    /* Recreate the interpreter so its streaming resource variables cannot
     * carry room audio or reply playback into the next listening period. */
    release_model();
    reset_probabilities();
}

extern "C" bool muse_wake_enabled(void)
{
    return s_enabled;
}
