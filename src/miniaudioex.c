// This software is available as a choice of the following licenses. Choose
// whichever you prefer.

// ===============================================================================
// ALTERNATIVE 1 - Public Domain (www.unlicense.org)
// ===============================================================================
// This is free and unencumbered software released into the public domain.

// Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
// software, either in source code form or as a compiled binary, for any purpose,
// commercial or non-commercial, and by any means.

// In jurisdictions that recognize copyright laws, the author or authors of this
// software dedicate any and all copyright interest in the software to the public
// domain. We make this dedication for the benefit of the public at large and to
// the detriment of our heirs and successors. We intend this dedication to be an
// overt act of relinquishment in perpetuity of all present and future rights to
// this software under copyright law.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// For more information, please refer to <http://unlicense.org/>

// ===============================================================================
// ALTERNATIVE 2 - MIT No Attribution
// ===============================================================================
// Copyright 2024 W.M.R Jap-A-Joe

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "miniaudioex.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifndef MA_ASSERT
#define MA_ASSERT(condition) assert(condition)
#endif

static MA_INLINE void ma_zero_memory_default(void* p, size_t sz)
{
    if (p == NULL) {
        MA_ASSERT(sz == 0); /* If this is triggered there's an error with the calling code. */
        return;
    }

    if (sz > 0) {
        memset(p, 0, sz);
    }
}

#ifndef MA_ZERO_MEMORY
#define MA_ZERO_MEMORY(p, sz)           ma_zero_memory_default((p), (sz))
#endif

#ifndef MA_ZERO_OBJECT
#define MA_ZERO_OBJECT(p)               MA_ZERO_MEMORY((p), sizeof(*(p)))
#endif

#ifndef MA_MALLOC
#define MA_MALLOC(sz)                   malloc((sz))
#endif

#ifndef MA_FREE
#define MA_FREE(p)                      free((p))
#endif

#ifndef MA_FLT_MAX
    #ifdef FLT_MAX
        #define MA_FLT_MAX FLT_MAX
    #else
        #define MA_FLT_MAX 3.402823466e+38F
    #endif
#endif

static MA_INLINE ma_uint64 ma_ex_create_hashcode(const void *data, size_t size) {
    ma_uint8 *d = (ma_uint8*)data;
    ma_uint64 hash = 0;
    int c;

    for(size_t i = 0; i < size; i++) {
        c = d[i];
        hash = c + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}

static MA_INLINE ma_uintptr ma_ex_pointer_to_hashcode(const void *data) {
    ma_uintptr d = (ma_uintptr)data;
    return ma_ex_create_hashcode(&d, sizeof(ma_uintptr));
}

static MA_INLINE ma_bool32 ma_ex_hashcode_is_same(ma_uint64 a, ma_uint64 b) {
    return a == b ? MA_TRUE : MA_FALSE;
}

static MA_INLINE void ma_ex_vec3f_set(ma_vec3f *v, float x, float y, float z) {
    v->x = x;
    v->y = y;
    v->z = z;
}

static MA_INLINE void ma_ex_vec3f_get(const ma_vec3f *v, float *x, float *y, float *z) {
    *x = v->x;
    *y = v->y;
    *z = v->z;
}

static ma_uint32 ma_next_power_of_two(ma_uint32 value) {
    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value++;
    return value;
}

static void ma_ex_on_data_proc(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    ma_engine_read_pcm_frames((ma_engine *)pDevice->pUserData, pOutput, frameCount, NULL);
    (void)pInput;
}

MA_API ma_ex_device_info *ma_ex_playback_devices_get(ma_uint32 *count) {
    *count = 0;

    ma_context context;
    ma_result result = ma_context_init(NULL, 0, NULL, &context);
    
    if (result != MA_SUCCESS) {
        return NULL;
    }

    ma_device_info *pPlaybackInfos;
    ma_uint32 playbackCount;
    ma_device_info* pCaptureInfos;
    ma_uint32 captureCount;
    result = ma_context_get_devices(&context, &pPlaybackInfos, &playbackCount, &pCaptureInfos, &captureCount);

    if (result != MA_SUCCESS) {
        return NULL;
    }

    ma_ex_device_info *pDeviceInfo = NULL;

    if(playbackCount == 0)
        return NULL;

    size_t allocationSize = sizeof(ma_ex_device_info) * playbackCount;
    pDeviceInfo = MA_MALLOC(allocationSize);
    
    if(pDeviceInfo == NULL)
        return NULL;

    MA_ZERO_MEMORY(pDeviceInfo, allocationSize);

    ma_bool32 allocationError = MA_FALSE;

    for (ma_uint32 iDevice = 0; iDevice < playbackCount; iDevice++) {
        pDeviceInfo[iDevice].index = iDevice;
        size_t len = strlen(pPlaybackInfos[iDevice].name) + 1;
        pDeviceInfo[iDevice].pName = MA_MALLOC(len);
        if(pDeviceInfo[iDevice].pName == NULL) {
            allocationError = MA_TRUE;
            break;
        }
        memset(pDeviceInfo[iDevice].pName, 0, len);
        memcpy(pDeviceInfo[iDevice].pName, pPlaybackInfos[iDevice].name, len);
    }

    if(allocationError == MA_TRUE) {
        for(ma_uint32 i = 0; i < playbackCount; i++) {
            if(pDeviceInfo[i].pName != NULL) {
                MA_FREE(pDeviceInfo[i].pName);
            }
        }        
        MA_FREE(pDeviceInfo);
        return NULL;
    }

    ma_context_uninit(&context);
    *count = playbackCount;
    return pDeviceInfo;
}

MA_API void *ma_ex_playback_devices_free(ma_ex_device_info *pDeviceInfo, ma_uint32 count) {
    if(pDeviceInfo != NULL) {
        for(ma_uint32 i = 0; i < count; i++) {
            if(pDeviceInfo[i].pName != NULL)
                MA_FREE(pDeviceInfo[i].pName);
        }
        MA_FREE(pDeviceInfo);
    }
}

MA_API ma_ex_context_config ma_ex_context_config_init(ma_uint32 sampleRate, ma_uint8 channels, ma_uint32 periodSizeInFrames, const ma_ex_device_info *pDeviceInfo) {
    MA_ASSERT(sampleRate > 0);
    MA_ASSERT(channels > 0);

    ma_ex_context_config config;
    config.sampleRate = sampleRate;
    config.channels = channels;
    config.periodSizeInFrames = periodSizeInFrames == 0 ? 0 : ma_next_power_of_two(periodSizeInFrames);

    if(pDeviceInfo == NULL) {
        config.deviceInfo.index = 0;
    } else {
        config.deviceInfo = *pDeviceInfo;
    }

    return config;
}

MA_API ma_ex_context *ma_ex_context_init(const ma_ex_context_config *config) {
    MA_ASSERT(config != NULL);
    MA_ASSERT(config->sampleRate > 0);
    MA_ASSERT(config->channels > 0);

    ma_ex_context *context = MA_MALLOC(sizeof(ma_ex_context));
    MA_ZERO_OBJECT(context);
    MA_ZERO_OBJECT(&context->context);
    MA_ZERO_OBJECT(&context->engine);
    MA_ZERO_OBJECT(&context->device);

    context->sampleRate = config->sampleRate;
    context->channels = config->channels;
    context->format = ma_format_f32;

    if (ma_context_init(NULL, 0, NULL, &context->context) != MA_SUCCESS) {
        fprintf(stderr, "Failed to initialize ma_context\n");
        MA_FREE(context);
        return NULL;
    }

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = context->format;
    deviceConfig.playback.channels = context->channels;
    deviceConfig.sampleRate = context->sampleRate;
    deviceConfig.dataCallback = &ma_ex_on_data_proc;
    deviceConfig.periodSizeInFrames = config->periodSizeInFrames;

    ma_device_info* pPlaybackInfos;
    ma_uint32 playbackCount;
    ma_device_info* pCaptureInfos;
    ma_uint32 captureCount;

    if (ma_context_get_devices(&context->context, &pPlaybackInfos, &playbackCount, &pCaptureInfos, &captureCount) != MA_SUCCESS) {
        fprintf(stderr, "Failed to get playback devices\n");
        ma_context_uninit(&context->context);
        MA_FREE(context);
        return NULL;
    }

    if(config->deviceInfo.index >= playbackCount) {
        fprintf(stderr, "Device index is greater than or equal to the number of playback devices\n");
        ma_context_uninit(&context->context);
        MA_FREE(context);
        return NULL;
    }
    
    deviceConfig.playback.pDeviceID = &pPlaybackInfos[config->deviceInfo.index].id;

    if(ma_device_init(&context->context, &deviceConfig, &context->device) != MA_SUCCESS) {
        fprintf(stderr, "Failed to initialize ma_device\n");
        ma_context_uninit(&context->context);
        MA_FREE(context);
        return NULL;
    }

    ma_engine_config engineConfig = ma_engine_config_init();
    engineConfig.listenerCount = MA_ENGINE_MAX_LISTENERS;
    engineConfig.pDevice = &context->device;

    if(ma_engine_init(&engineConfig, &context->engine) != MA_SUCCESS) {
        fprintf(stderr, "Failed to initialize ma_engine\n");
        ma_device_uninit(&context->device);
        ma_context_uninit(&context->context);
        MA_FREE(context);
        return NULL;
    }

    context->device.pUserData = &context->engine;

    if (ma_device_start(&context->device) != MA_SUCCESS) {
        fprintf(stderr, "Failed to start ma_device\n");
        ma_engine_uninit(&context->engine);
        ma_device_uninit(&context->device);
        ma_context_uninit(&context->context);
        MA_FREE(context);
        return NULL;
    }

    for(size_t i = 0; i < MA_ENGINE_MAX_LISTENERS; i++) {
        context->listeners[i] = -1;
    }

    return context;
}

MA_API void ma_ex_context_uninit(ma_ex_context *context) {
    if(context != NULL) {
        ma_engine_uninit(&context->engine);
        ma_device_uninit(&context->device);
        ma_context_uninit(&context->context);
        MA_FREE(context);
    }
}

MA_API void ma_ex_context_set_master_volume(ma_ex_context *context, float volume) {
    if(context != NULL)
        ma_engine_set_volume(&context->engine, volume);
}

MA_API float ma_ex_context_get_master_volume(ma_ex_context *context) {
    if(context != NULL)
        return ma_engine_get_volume(&context->engine);
    return 0.0f;
}

MA_API ma_ex_audio_source *ma_ex_audio_source_init(ma_ex_context *context) {
    MA_ASSERT(context != NULL);
    
    ma_ex_audio_source *source = MA_MALLOC(sizeof(ma_ex_audio_source));
    source->context = context;
    MA_ZERO_OBJECT(&source->sound);
    MA_ZERO_OBJECT(&source->callbacks);
    MA_ZERO_OBJECT(&source->settings);
    source->callbacks.endCallback = NULL;
    source->callbacks.loadCallback = NULL;
    source->callbacks.processCallback = NULL;
    source->callbacks.waveformCallback = NULL;
    source->callbacks.pUserData = NULL;
    source->soundHash = 0;

    source->settings.attenuationModel = ma_attenuation_model_linear;
    ma_ex_vec3f_set(&source->settings.direction, 0.0f, 0.0f, -1.0f);
    ma_ex_vec3f_set(&source->settings.position, 0.0f, 0.0f, 0.0f);
    ma_ex_vec3f_set(&source->settings.velocity, 0.0f, 0.0f, 0.0f);
    source->settings.dopplerFactor = 1.0f;
    source->settings.loop = MA_FALSE;
    source->settings.maxDistance = MA_FLT_MAX;
    source->settings.minDistance = 1.0f;
    source->settings.pitch = 1.0f;
    source->settings.spatialization = MA_FALSE;
    source->settings.volume = 1.0f;

    return source;
}

MA_API void ma_ex_audio_source_uninit(ma_ex_audio_source *source) {
    if(source != NULL) {
        ma_sound_uninit(&source->sound);
        MA_FREE(source);
    }
}

MA_API void ma_ex_audio_source_set_callbacks(ma_ex_audio_source *source, ma_ex_audio_source_callbacks callbacks) {
    if(source != NULL) {
        if(callbacks.pUserData == NULL)
            callbacks.pUserData = source;
        source->callbacks = callbacks;
    }
}

static MA_INLINE void ma_ex_sound_set_callbacks(ma_sound *pSound, const ma_ex_audio_source_callbacks *pCallbacks) {
    if(pSound != NULL) {
        ma_sound_set_notifications_userdata(pSound, pCallbacks->pUserData);
        ma_sound_set_end_notification_callback(pSound, pCallbacks->endCallback);
        ma_sound_set_load_notification_callback(pSound, pCallbacks->loadCallback);
        ma_sound_set_process_notification_callback(pSound, pCallbacks->processCallback);
    }
}

MA_API ma_result ma_ex_audio_source_play(ma_ex_audio_source *source) {
    if(source == NULL)
        return MA_ERROR;

    ma_uint64 hashcode = ma_ex_pointer_to_hashcode(&source->waveform);

    if(ma_ex_hashcode_is_same(hashcode, source->soundHash) != MA_TRUE) {
        ma_data_source *dataSource = ma_sound_get_data_source(&source->sound);

        if(dataSource != NULL) {
            ma_sound_uninit(&source->sound);
            ma_data_source_uninit(dataSource);
        } else {
            MA_ZERO_OBJECT(&source->waveform);
        }

        ma_procedural_wave_config config = ma_procedural_wave_config_init(ma_format_f32, 2, source->context->engine.sampleRate, source->callbacks.waveformCallback, source->callbacks.pUserData);
        ma_result result = ma_procedural_wave_init(&config, &source->waveform);

        if(result != MA_SUCCESS)
            return result;

        result = ma_sound_init_from_data_source(&source->context->engine, &source->waveform, 0, NULL, &source->sound);

        if(result != MA_SUCCESS)
            return result;
    }

    source->soundHash = hashcode;

    ma_ex_audio_source_apply_settings(source);
    ma_ex_sound_set_callbacks(&source->sound, &source->callbacks);    
    return ma_sound_start(&source->sound);
}

MA_API ma_result ma_ex_audio_source_play_from_file(ma_ex_audio_source *source, const char *filePath, ma_bool32 streamFromDisk) {
    if(source == NULL)
        return MA_ERROR;

    if(filePath == NULL)
        return MA_INVALID_FILE;
    
    ma_uint32 flags = MA_SOUND_FLAG_DECODE;
    
    if(streamFromDisk)
        flags |= MA_SOUND_FLAG_STREAM;

    ma_uint64 hashcode = ma_ex_create_hashcode(filePath, strlen(filePath));

    if(ma_ex_hashcode_is_same(hashcode, source->soundHash) != MA_TRUE) {
        ma_data_source *dataSource = ma_sound_get_data_source(&source->sound);

        if(dataSource != NULL) {
            ma_sound_uninit(&source->sound);
            ma_data_source_uninit(dataSource);
        }

        ma_result result = ma_sound_init_from_file(&source->context->engine, filePath, flags, NULL, NULL, &source->sound);

        if(result != MA_SUCCESS)
            return result;
    }

    source->soundHash = hashcode;

    ma_ex_audio_source_apply_settings(source);
    ma_ex_sound_set_callbacks(&source->sound, &source->callbacks);
    return ma_sound_start(&source->sound);
}

MA_API ma_result ma_ex_audio_source_play_from_memory(ma_ex_audio_source *source, const void *data, ma_uint64 dataSize) {
    if(source == NULL)
        return MA_ERROR;

    if(data == NULL)
        return MA_INVALID_DATA;

    ma_uint64 hashcode = ma_ex_pointer_to_hashcode(data);

    if(ma_ex_hashcode_is_same(hashcode, source->soundHash) != MA_TRUE) {
        ma_data_source *dataSource = ma_sound_get_data_source(&source->sound);

        if(dataSource != NULL) {
            ma_sound_uninit(&source->sound);
            ma_data_source_uninit(dataSource);
        } else {
            MA_ZERO_OBJECT(&source->decoder);
        }

        ma_result result = ma_decoder_init_memory(data, dataSize, NULL, &source->decoder);

        if(result != MA_SUCCESS)
            return result;

        ma_uint32 flags = MA_SOUND_FLAG_DECODE;

        result = ma_sound_init_from_data_source(&source->context->engine, &source->decoder, flags, NULL, &source->sound);

        if(result != MA_SUCCESS)
            return result;
    }

    source->soundHash = hashcode;

    ma_ex_audio_source_apply_settings(source);
    ma_ex_sound_set_callbacks(&source->sound, &source->callbacks);
    return ma_sound_start(&source->sound);
}

MA_API void ma_ex_audio_source_stop(ma_ex_audio_source *source) {
    if(source != NULL)
        ma_sound_stop(&source->sound);
}

MA_API void ma_ex_audio_source_apply_settings(ma_ex_audio_source *source) {
    if(source != NULL) {
        ma_sound_set_attenuation_model(&source->sound, source->settings.attenuationModel);
        ma_sound_set_direction(&source->sound, source->settings.direction.x, source->settings.direction.y, source->settings.direction.z);
        ma_sound_set_doppler_factor(&source->sound, source->settings.dopplerFactor);
        ma_sound_set_looping(&source->sound, source->settings.loop);
        ma_sound_set_min_distance(&source->sound, source->settings.minDistance);
        ma_sound_set_max_distance(&source->sound, source->settings.maxDistance);
        ma_sound_set_pitch(&source->sound, source->settings.pitch);
        ma_sound_set_position(&source->sound, source->settings.position.x, source->settings.position.y, source->settings.position.z);
        ma_sound_set_spatialization_enabled(&source->sound, source->settings.spatialization);
        ma_sound_set_velocity(&source->sound, source->settings.velocity.x, source->settings.velocity.y, source->settings.velocity.z);
        ma_sound_set_volume(&source->sound, source->settings.volume);
    }
}

MA_API void ma_ex_audio_source_set_volume(ma_ex_audio_source *source, float value) {
    if(source != NULL) {
        source->settings.volume = value;
        ma_sound_set_volume(&source->sound, value);
    }
}

MA_API float ma_ex_audio_source_get_volume(ma_ex_audio_source *source) {
    if(source != NULL)
        return source->settings.volume;
    return 1.0f;
}

MA_API void ma_ex_audio_source_set_pitch(ma_ex_audio_source *source, float value) {
    if(source != NULL) {
        source->settings.pitch = value;
        ma_sound_set_pitch(&source->sound, value);
    }
}

MA_API float ma_ex_audio_source_get_pitch(ma_ex_audio_source *source) {
    if(source != NULL)
        return source->settings.pitch;
    return 1.0f;
}

MA_API void ma_ex_audio_source_set_pcm_position(ma_ex_audio_source *source, ma_uint64 position) {
    if(source != NULL)
        ma_sound_seek_to_pcm_frame(&source->sound, position);
}

MA_API ma_uint64 ma_ex_audio_source_get_pcm_position(ma_ex_audio_source *source) {
    if(source != NULL)
        return ma_sound_get_time_in_pcm_frames(&source->sound);
    return 0;
}

MA_API ma_uint64 ma_ex_audio_source_get_pcm_length(ma_ex_audio_source *source) {
    if(source != NULL) {
        ma_data_source *dataSource = ma_sound_get_data_source(&source->sound);
        if(dataSource != NULL) {
            ma_uint64 length = 0;
            ma_data_source_get_length_in_pcm_frames(dataSource, &length);
            return length;
        }
    }
    return 0;
}

MA_API void ma_ex_audio_source_set_loop(ma_ex_audio_source *source, ma_bool32 loop) {
    if(source != NULL) {
        source->settings.loop = loop;
        ma_sound_set_looping(&source->sound, loop);
    }
}

MA_API ma_bool32 ma_ex_audio_source_get_loop(ma_ex_audio_source *source) {
    if(source != NULL)
        return source->settings.loop;
    return MA_FALSE;
}

MA_API void ma_ex_audio_source_set_position(ma_ex_audio_source *source, float x, float y, float z) {
    if(source != NULL) {
        ma_ex_vec3f_set(&source->settings.position, x, y, z);
        ma_sound_set_position(&source->sound, x, y, z);
    }
}

MA_API void ma_ex_audio_source_get_position(ma_ex_audio_source *source, float *x, float *y, float *z) {
    if(source != NULL) {
        ma_ex_vec3f_get(&source->settings.position, x, y, z);
    } else {
        *x = 0.0f;
        *y = 0.0f;
        *z = 0.0f;
    }
}

MA_API void ma_ex_audio_source_set_direction(ma_ex_audio_source *source, float x, float y, float z) {
    if(source != NULL) {
        ma_ex_vec3f_set(&source->settings.direction, x, y, z);
        ma_sound_set_direction(&source->sound, x, y, z);
    }
}

MA_API void ma_ex_audio_source_get_direction(ma_ex_audio_source *source, float *x, float *y, float *z) {
    if(source != NULL) {
        ma_ex_vec3f_get(&source->settings.direction, x, y, z);
    } else {
        *x = 0.0f;
        *y = 0.0f;
        *z = 0.0f;
    }
}

MA_API void ma_ex_audio_source_set_velocity(ma_ex_audio_source *source, float x, float y, float z) {
    if(source != NULL) {
        ma_ex_vec3f_set(&source->settings.velocity, x, y, z);
        ma_sound_set_velocity(&source->sound, x, y, z);
    }
}

MA_API void ma_ex_audio_source_get_velocity(ma_ex_audio_source *source, float *x, float *y, float *z) {
    if(source != NULL) {
        ma_ex_vec3f_get(&source->settings.velocity, x, y, z);
    } else {
        *x = 0.0f;
        *y = 0.0f;
        *z = 0.0f;
    }
}

MA_API void ma_ex_audio_source_set_spatialization(ma_ex_audio_source *source, ma_bool32 enabled) {
    if(source != NULL) {
        source->settings.spatialization = enabled;
        ma_sound_set_spatialization_enabled(&source->sound, enabled);
    }
}

MA_API ma_bool32 ma_ex_audio_source_get_spatialization(ma_ex_audio_source *source) {
    if(source != NULL)
        return source->settings.spatialization;
    return MA_FALSE;
}

MA_API void ma_ex_audio_source_set_attenuation_model(ma_ex_audio_source *source, ma_attenuation_model model) {
    if(source != NULL) {
        source->settings.attenuationModel = model;
        ma_sound_set_attenuation_model(&source->sound, model);
    }
}

MA_API ma_attenuation_model ma_ex_audio_source_get_attenuation_model(ma_ex_audio_source *source) {
    if(source != NULL)
        return source->settings.attenuationModel;
    return ma_attenuation_model_none;
}

MA_API void ma_ex_audio_source_set_doppler_factor(ma_ex_audio_source *source, float factor) {
    if(source != NULL) {
        source->settings.dopplerFactor = factor;
        ma_sound_set_doppler_factor(&source->sound, factor);
    }
}

MA_API float ma_ex_audio_source_get_doppler_factor(ma_ex_audio_source *source) {
    if(source != NULL)
        return source->settings.dopplerFactor;
    return 0.0f;
}

MA_API void ma_ex_audio_source_set_min_distance(ma_ex_audio_source *source, float distance) {
    if(source != NULL) {
        source->settings.minDistance = distance;
        ma_sound_set_min_distance(&source->sound, distance);
    }
}

MA_API float ma_ex_audio_source_get_min_distance(ma_ex_audio_source *source) {
    if(source != NULL)
        return source->settings.minDistance;
    return 1.0f;
}

MA_API void ma_ex_audio_source_set_max_distance(ma_ex_audio_source *source, float distance) {
    if(source != NULL) {
        source->settings.maxDistance = distance;
        ma_sound_set_max_distance(&source->sound, distance);
    }
}

MA_API float ma_ex_audio_source_get_max_distance(ma_ex_audio_source *source) {
    if(source != NULL)
        return source->settings.maxDistance;
    return MA_FLT_MAX;
}

MA_API ma_bool32 ma_ex_audio_source_get_is_playing(ma_ex_audio_source *source) {
    if(source != NULL)
        return ma_sound_is_playing(&source->sound);
    return MA_FALSE;
}

MA_API ma_ex_audio_listener *ma_ex_audio_listener_init(ma_ex_context *context) {
    MA_ASSERT(context != NULL);

    ma_int32 listenerIndex = -1;

    for(ma_int32 i = 0; i < MA_ENGINE_MAX_LISTENERS; i++) {
        if(context->listeners[i] == -1) {
            context->listeners[i] = i;
            listenerIndex = i;
            break;
        }
    }

    if(listenerIndex == -1) {
        fprintf(stderr, "Can't create any more audio listeners, the max of %d is already achieved\n", MA_ENGINE_MAX_LISTENERS);
        return NULL;
    }

    ma_ex_audio_listener *listener = (ma_ex_audio_listener*)MA_MALLOC(sizeof(ma_ex_audio_listener));
    MA_ZERO_OBJECT(listener);

    listener->context = context;
    listener->index = listenerIndex;
    ma_ex_vec3f_set(&listener->settings.position, 0.0f, 0.0f, 0.0f);
    ma_ex_vec3f_set(&listener->settings.direction, 0.0f, 0.0f, -1.0f);
    ma_ex_vec3f_set(&listener->settings.velocity, 0.0f, 0.0f, 0.0f);
    ma_ex_vec3f_set(&listener->settings.worldUp, 0.0f, 1.0f, 0.0f);
    listener->settings.spatialization = MA_TRUE;
    listener->settings.coneInnerAngleInRadians = 6.283185f; /* 360 degrees. */
    listener->settings.coneOuterAngleInRadians = 6.283185f; /* 360 degrees. */
    listener->settings.coneOuterGain = 0;

    ma_engine_listener_set_position(&listener->context->engine, listener->index, listener->settings.position.x, listener->settings.position.y, listener->settings.position.z);
    ma_engine_listener_set_direction(&listener->context->engine, listener->index, listener->settings.direction.x, listener->settings.direction.y, listener->settings.direction.z);
    ma_engine_listener_set_velocity(&listener->context->engine, listener->index, listener->settings.velocity.x, listener->settings.velocity.y, listener->settings.velocity.z);
    ma_engine_listener_set_world_up(&listener->context->engine, listener->index, listener->settings.worldUp.x, listener->settings.worldUp.y, listener->settings.worldUp.z);
    ma_engine_listener_set_cone(&listener->context->engine, listener->index, listener->settings.coneInnerAngleInRadians, listener->settings.coneOuterAngleInRadians, listener->settings.coneOuterGain);
    ma_engine_listener_set_enabled(&listener->context->engine, listener->index, listener->settings.spatialization);
    
    return listener;
}

MA_API void ma_ex_audio_listener_uninit(ma_ex_audio_listener *listener) {
    if(listener != NULL) {
        if(listener->context != NULL && listener->index < MA_ENGINE_MAX_LISTENERS) {
            listener->context->listeners[listener->index] = -1;
        }
        MA_FREE(listener);
    }
}

MA_API void ma_ex_audio_listener_set_spatialization(ma_ex_audio_listener *listener, ma_bool32 enabled) {
    if(listener != NULL)
        ma_engine_listener_set_enabled(&listener->context->engine, listener->index, enabled);
}

MA_API ma_bool32 ma_ex_audio_listener_get_spatialization(ma_ex_audio_listener *listener) {
    if(listener != NULL)
        return listener->settings.spatialization;
    return MA_FALSE;
}

MA_API void ma_ex_audio_listener_set_position(ma_ex_audio_listener *listener, float x, float y, float z) {
    if(listener != NULL) {
        listener->settings.position.x = x;
        listener->settings.position.y = y;
        listener->settings.position.z = z;
        ma_engine_listener_set_position(&listener->context->engine, listener->index, x, y, z);
    }
}

MA_API void ma_ex_audio_listener_get_position(ma_ex_audio_listener *listener, float *x, float *y, float *z) {
    if(listener != NULL) {
        ma_ex_vec3f_get(&listener->settings.position, x, y, z);
    } else {
        *x = 0.0f;
        *y = 0.0f;
        *z = 0.0f;
    }
}

MA_API void ma_ex_audio_listener_set_direction(ma_ex_audio_listener *listener, float x, float y, float z) {
    if(listener != NULL) {
        listener->settings.direction.x = x;
        listener->settings.direction.y = y;
        listener->settings.direction.z = z;
        ma_engine_listener_set_direction(&listener->context->engine, listener->index, x, y, z);
    }
}

MA_API void ma_ex_audio_listener_get_direction(ma_ex_audio_listener *listener, float *x, float *y, float *z) {
    if(listener != NULL) {
        ma_ex_vec3f_get(&listener->settings.direction, x, y, z);
    } else {
        *x = 0.0f;
        *y = 0.0f;
        *z = 0.0f;
    }
}

MA_API void ma_ex_audio_listener_set_velocity(ma_ex_audio_listener *listener, float x, float y, float z) {
    if(listener != NULL) {
        listener->settings.velocity.x = x;
        listener->settings.velocity.y = y;
        listener->settings.velocity.z = z;
        ma_engine_listener_set_velocity(&listener->context->engine, listener->index, x, y, z);
    }
}

MA_API void ma_ex_audio_listener_get_velocity(ma_ex_audio_listener *listener, float *x, float *y, float *z) {
    if(listener != NULL) {
        ma_ex_vec3f_get(&listener->settings.velocity, x, y, z);
    } else {
        *x = 0.0f;
        *y = 0.0f;
        *z = 0.0f;
    }
}

MA_API void ma_ex_audio_listener_set_world_up(ma_ex_audio_listener *listener, float x, float y, float z) {
    if(listener != NULL) {
        listener->settings.worldUp.x = x;
        listener->settings.worldUp.y = y;
        listener->settings.worldUp.z = z;
        ma_engine_listener_set_world_up(&listener->context->engine, listener->index, x, y, z);
    }
}

MA_API void ma_ex_audio_listener_get_world_up(ma_ex_audio_listener *listener, float *x, float *y, float *z) {
    if(listener != NULL) {
        ma_ex_vec3f_get(&listener->settings.worldUp, x, y, z);
    } else {
        *x = 0.0f;
        *y = 0.0f;
        *z = 0.0f;
    }
}

MA_API void ma_ex_audio_listener_set_cone(ma_ex_audio_listener *listener, float innerAngleInRadians, float outerAngleInRadians, float outerGain) {
    if(listener != NULL) {
        listener->settings.coneInnerAngleInRadians = innerAngleInRadians;
        listener->settings.coneOuterAngleInRadians = outerAngleInRadians;
        listener->settings.coneOuterGain = outerGain;
        ma_engine_listener_set_cone(&listener->context->engine, listener->index, innerAngleInRadians, outerAngleInRadians, outerGain);
    }
}

MA_API void ma_ex_audio_listener_get_cone(ma_ex_audio_listener *listener, float *innerAngleInRadians, float *outerAngleInRadians, float *outerGain) {
    if(listener != NULL) {
        *innerAngleInRadians = listener->settings.coneInnerAngleInRadians;
        *outerAngleInRadians = listener->settings.coneOuterAngleInRadians;
        *outerGain = listener->settings.coneOuterGain;
    } else {
        *innerAngleInRadians = 0.0f;
        *outerAngleInRadians = 0.0f;
        *outerGain = 0.0f;
    }
}

MA_API char *ma_ex_read_bytes_from_file(const char *filepath, size_t *size) {
    FILE *file = fopen(filepath, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error opening file\n");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char *buffer = (char *)MA_MALLOC(fileSize * sizeof(char));
    if (buffer == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

    if (fread(buffer, sizeof(char), fileSize, file) != fileSize) {
        fprintf(stderr, "Error reading file\n");
        MA_FREE(buffer);
        fclose(file);
        return NULL;
    }

    fclose(file);

    *size = fileSize;
    return buffer;
}

MA_API void ma_ex_free_bytes_from_file(char *pointer) {
    if(pointer != NULL)
        MA_FREE(pointer);
}

MA_API void ma_ex_free(void *pointer) {
    if(pointer != NULL)
        MA_FREE(pointer);
}

MA_API float *ma_ex_decode_file(const char *pFilePath, ma_uint64 *dataLength, ma_uint32 *channels, ma_uint32 *sampleRate, ma_uint32 desiredChannels, ma_uint32 desiredSampleRate) {
    ma_decoder_config config = ma_decoder_config_init_default();
    config.format = ma_format_f32;
    
    if(desiredChannels > 0)
        config.channels = desiredChannels;

    if(desiredSampleRate > 0)
        config.sampleRate = desiredSampleRate;

    void* pPCMFrames;
    ma_uint64 frameCount;

    if(ma_decode_file(pFilePath, &config, &frameCount, &pPCMFrames) == MA_SUCCESS) {
        *channels = config.channels;
        *sampleRate = config.sampleRate;
        *dataLength = config.channels * frameCount;
        return (float*)pPCMFrames;
    }
    return NULL;
}