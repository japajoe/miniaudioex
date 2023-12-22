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
// Copyright 2023 W.M.R Jap-A-Joe

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

#define MA_EX_ASSERT(condition) assert(condition)

#ifdef MA_EX_DEBUG_LOGGING
#define ma_ex_debug_log(...) printf(__VA_ARGS__)
#else
#define ma_ex_debug_log(...)
#endif

static int strcmp_null_safe(const char *a, const char *b) {
    if(a == NULL || b == NULL) {
        return 1;
    }
    return strcmp(a, b);
}

ma_ex_context_config ma_ex_context_config_init(ma_uint32 sampleRate, ma_uint32 channels, ma_format format, ma_device_data_proc dataProc) {   
    ma_ex_context_config config;
    memset(&config, 0, sizeof(ma_ex_context_config));
    config.sampleRate = sampleRate;
    config.channels = channels;
    config.format = format;
    config.dataProc = dataProc;
    return config;
}

ma_ex_context *ma_ex_context_init(const ma_ex_context_config *config) {
    MA_EX_ASSERT(config != NULL);
    MA_EX_ASSERT(config->sampleRate > 0);
    MA_EX_ASSERT(config->channels > 0);
    MA_EX_ASSERT(config->dataProc != NULL);

    ma_ex_context *context = malloc(sizeof(ma_ex_context));
    memset(context, 0, sizeof(ma_ex_context));

    context->engine = malloc(sizeof(ma_engine));
    memset(context->engine, 0, sizeof(ma_engine));
    
    context->device = malloc(sizeof(ma_device));
    memset(context->device, 0, sizeof(ma_device));

    context->channels = config->channels;
    context->dataProc = config->dataProc;
    context->format = config->format;
    context->sampleRate = config->sampleRate;

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = context->format;
    deviceConfig.playback.channels = context->channels;
    deviceConfig.sampleRate        = context->sampleRate;
    deviceConfig.dataCallback      = context->dataProc;

    if(ma_device_init(NULL, &deviceConfig, context->device) != MA_SUCCESS) {
        free(context->device);
        free(context->engine);
        free(context);
        return NULL;
    }

    ma_engine_config engineConfig = ma_engine_config_init();
    engineConfig.listenerCount = 1;
    engineConfig.pDevice = context->device;

    if(ma_engine_init(&engineConfig, context->engine) != MA_SUCCESS) {
        ma_device_uninit(context->device);
        free(context->device);
        free(context->engine);
        free(context);
        return NULL;
    }

    context->device->pUserData = context->engine;

    if (ma_device_start(context->device) != MA_SUCCESS) {
        ma_engine_uninit(context->engine);
        ma_device_uninit(context->device);
        free(context->device);
        free(context->engine);
        free(context);
        return NULL;
    }

    return context;
}

void ma_ex_context_uninit(ma_ex_context *context) {
    if(context != NULL) {
        ma_engine_uninit(context->engine);
        ma_device_uninit(context->device);
        free(context->engine);
        free(context->device);
        free(context);
    }
}

ma_ex_audio_source_config ma_ex_audio_source_config_init(ma_ex_context *context, ma_ex_audio_source_callbacks callbacks) {
    ma_ex_audio_source_config config;
    memset(&config, 0, sizeof(ma_ex_audio_source_config));
    config.context = context;
    config.callbacks = callbacks;
    return config;
}

ma_ex_audio_source *ma_ex_audio_source_init(const ma_ex_audio_source_config *config) {
    MA_EX_ASSERT(config != NULL);
    MA_EX_ASSERT(config->context != NULL);
    
    ma_ex_audio_source *source = malloc(sizeof(ma_ex_audio_source));
    source->engine = config->context->engine;
    source->filePath = NULL;
    source->callbacks = config->callbacks;
    memset(&source->sound, 0, sizeof(ma_sound));

    return source;
}

void ma_ex_audio_source_uninit(ma_ex_audio_source *source) {
    if(source != NULL) {
        ma_sound_uninit(&source->sound);
        if(source->filePath != NULL) {
            free(source->filePath);
        }
        free(source);
    }
}

void ma_ex_audio_source_set_callbacks_user_data(ma_ex_audio_source *source, void *userData) {
    if(source != NULL) {
        source->callbacks.pUserData = userData;
    }
}

ma_result ma_ex_audio_source_play(ma_ex_audio_source *source, const char *filePath, ma_bool8 streamFromDisk) {
    if(source == NULL) {
        return MA_ERROR;
    }

    if(filePath == NULL) {
        return MA_ERROR;
    }

    if(strcmp_null_safe(source->filePath, filePath) != 0) {
        ma_sound_uninit(&source->sound);
        //ma_uint32 flags = MA_SOUND_FLAG_ASYNC | MA_SOUND_FLAG_DECODE;
        ma_uint32 flags = MA_SOUND_FLAG_DECODE;
        
        if(streamFromDisk) {
            flags |= MA_SOUND_FLAG_STREAM;
        }

        ma_result result = ma_sound_init_from_file(source->engine, filePath, flags, NULL, NULL, &source->sound);

        if(result != MA_SUCCESS) {
            return result;
        } else {
            source->sound.engineNode.dspConfig.proc = source->callbacks.dspProc;
            source->sound.engineNode.dspConfig.pUserData = source->callbacks.pUserData;

            if(source->callbacks.soundEndedProc != NULL) {
                ma_sound_set_end_callback(&source->sound, source->callbacks.soundEndedProc, source->callbacks.pUserData);
            }
            if(source->callbacks.soundLoadedProc != NULL) {
                source->callbacks.soundLoadedProc(&source->sound, source->callbacks.pUserData);
            }
        }

        if(source->filePath != NULL) {
            free(source->filePath);
        }

        size_t pathSize = strlen(filePath);

        if(pathSize == 0) {
            return MA_INVALID_FILE;
        }

        source->filePath = malloc(pathSize + 1);
        memset(source->filePath, 0, pathSize + 1);
        strcpy(source->filePath, filePath);
    }

    return ma_sound_start(&source->sound);
}

ma_result ma_ex_audio_source_play_from_waveform_proc(ma_ex_audio_source *source) {
    if(source != NULL) {
        ma_data_source *dataSource = ma_sound_get_data_source(&source->sound);

        if(dataSource != NULL) {
            ma_sound_uninit(&source->sound);
            ma_data_source_uninit(dataSource);
        }

        ma_waveform_config config = ma_waveform_config_init(ma_format_f32, 2, source->engine->sampleRate, ma_waveform_type_custom, 1.0f, 1.0f);
        config.customConfig.proc = source->callbacks.waveformProc;
        config.customConfig.pUserData = source->callbacks.pUserData;

        ma_result result = MA_ERROR;

        result = ma_waveform_init(&config, &source->waveform);

        if(result != MA_SUCCESS) {
            return result;
        }

        result = ma_sound_init_from_data_source(source->engine, &source->waveform, 0, NULL, &source->sound);
        //result = ma_sound_init_from_data_source(source->engine, &source->waveform, MA_SOUND_FLAG_ASYNC, NULL, &source->sound);

        if(result != MA_SUCCESS) {
            return result;
        }

        return ma_sound_start(&source->sound);
    }

    return MA_ERROR;
}

void ma_ex_audio_source_stop(ma_ex_audio_source *source) {
    if(source != NULL) {
        ma_sound_stop(&source->sound);
    }
}

ma_result ma_ex_audio_source_get_pcm_position(ma_ex_audio_source *source, ma_uint64 *position) {
    if(source != NULL) {
        *position = ma_sound_get_time_in_pcm_frames(&source->sound);
        return MA_SUCCESS;
    }

    return MA_ERROR;
}

void ma_ex_audio_source_set_pcm_position(ma_ex_audio_source *source, ma_uint64 frameIndex) {
    if(source != NULL) {
        ma_sound_seek_to_pcm_frame(&source->sound, frameIndex);
    }
}

void ma_ex_audio_source_set_pcm_start_position(ma_ex_audio_source *source, ma_uint64 frameIndex) {
    if(source != NULL) {
        ma_sound_set_start_time_in_pcm_frames(&source->sound, frameIndex);
    }
}

ma_result ma_ex_audio_source_get_pcm_length(ma_ex_audio_source *source, ma_uint64 *length) {
    if(source != NULL) {
        ma_data_source *dataSource = ma_sound_get_data_source(&source->sound);
        if(dataSource != NULL) {
            return ma_data_source_get_length_in_pcm_frames(dataSource, length);
        }
    }
    return MA_ERROR;
}

void ma_ex_audio_source_set_loop(ma_ex_audio_source *source, ma_bool32 loop) {
    if(source != NULL) {
        ma_sound_set_looping(&source->sound, loop);
    }
}

void ma_ex_audio_source_set_volume(ma_ex_audio_source *source, float volume) {
    if(source != NULL) {
        ma_sound_set_volume(&source->sound, volume);
    }
}

void ma_ex_audio_source_set_pitch(ma_ex_audio_source *source, float pitch) {
    if(source != NULL) {
        ma_sound_set_pitch(&source->sound, pitch);
    }
}

void ma_ex_audio_source_set_position(ma_ex_audio_source *source, float x, float y, float z) {
    if(source != NULL) {
        ma_sound_set_position(&source->sound, x, y, z);
    }
}

void ma_ex_audio_source_set_direction(ma_ex_audio_source *source, float x, float y, float z) {
    if(source != NULL) {
        ma_sound_set_direction(&source->sound, x, y, z);
    }
}

void ma_ex_audio_source_set_velocity(ma_ex_audio_source *source, float x, float y, float z) {
    if(source != NULL) {
        ma_sound_set_velocity(&source->sound, x, y, z);
    }
}

void ma_ex_audio_source_set_spatialization(ma_ex_audio_source *source, ma_bool32 enabled) {
    if(source != NULL) {
        ma_sound_set_spatialization_enabled(&source->sound, enabled);
    }
}

void ma_ex_audio_source_set_attenuation_model(ma_ex_audio_source *source, ma_attenuation_model model) {
    if(source != NULL) {
        ma_sound_set_attenuation_model(&source->sound, model);
    }
}

void ma_ex_audio_source_set_doppler_factor(ma_ex_audio_source *source, float factor) {
    if(source != NULL) {
        ma_sound_set_doppler_factor(&source->sound, factor);
    }
}

void ma_ex_audio_source_set_min_distance(ma_ex_audio_source *source, float distance) {
    if(source != NULL) {
        ma_sound_set_min_distance(&source->sound, distance);
    }
}

void ma_ex_audio_source_set_max_distance(ma_ex_audio_source *source, float distance) {
    if(source != NULL) {
        ma_sound_set_max_distance(&source->sound, distance);
    }
}

ma_bool32 ma_ex_audio_source_get_is_playing(ma_ex_audio_source *source) {
    if(source != NULL) {
        ma_sound_is_playing(&source->sound);
    }
}

ma_ex_audio_listener *ma_ex_audio_listener_init(const ma_ex_context *context) {
    if(context == NULL) {
        return NULL;
    }
    ma_ex_audio_listener *listener = malloc(sizeof(ma_ex_audio_listener));
    memset(listener, 0, sizeof(ma_ex_audio_listener));
    listener->engine = context->engine;
    return listener;
}

void ma_ex_audio_listener_uninit(ma_ex_audio_listener *listener) {
    if(listener != NULL) {
        free(listener);
    }
}

void ma_ex_audio_listener_set_spatialization(ma_ex_audio_listener *listener, ma_bool32 enabled) {
    if(listener != NULL) {
        if(listener->engine != NULL) {
            ma_engine_listener_set_enabled(listener->engine, 0, enabled);
        }
    }
}

void ma_ex_audio_listener_set_position(ma_ex_audio_listener *listener, float x, float y, float z) {
    if(listener != NULL) {
        if(listener->engine != NULL) {
            ma_engine_listener_set_position(listener->engine, 0, x, y, z);
        }
    }
}

void ma_ex_audio_listener_set_direction(ma_ex_audio_listener *listener, float x, float y, float z) {
    if(listener != NULL) {
        if(listener->engine != NULL) {
            ma_engine_listener_set_direction(listener->engine, 0, x, y, z);
        }
    }
}

void ma_ex_audio_listener_set_velocity(ma_ex_audio_listener *listener, float x, float y, float z) {
    if(listener != NULL) {
        if(listener->engine != NULL) {
            ma_engine_listener_set_velocity(listener->engine, 0, x, y, z);
        }
    }
}

void ma_ex_audio_listener_set_world_up(ma_ex_audio_listener *listener, float x, float y, float z) {
    if(listener != NULL) {
        if(listener->engine != NULL) {
            ma_engine_listener_set_world_up(listener->engine, 0, x, y, z);
        }
    }
}

void ma_ex_audio_listener_set_cone(ma_ex_audio_listener *listener, float innerAngleInRadians, float outerAngleInRadians, float outerGain) {
    if(listener != NULL) {
        if(listener->engine != NULL) {
            ma_engine_listener_set_cone(listener->engine, 0, innerAngleInRadians, outerAngleInRadians, outerGain);
        }
    }
}
