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
// Copyright 2025 W.M.R Jap-A-Joe

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

/* Note that this is not fully compatible with the original miniaudio library
    Notable changes made in miniaudio:
    - added method ma_sound_init_from_memory
    - added method ma_sound_init_from_callback
    - added method ma_sound_notifications_init
    - added method ma_sound_set_notifications_userdata
    - added method ma_sound_set_end_notification_callback
    - added method ma_sound_set_load_notification_callback
    - added method ma_sound_set_process_notification_callback
    - added custom data source: ma_procedural_sound
    - added method ma_procedural_sound_config_init
    - added method ma_procedural_sound_init
    - added method ma_procedural_sound_uninit
    - added method ma_procedural_sound_read_pcm_frames
    - added MA_DATA_SOURCE_IS_DECODER and MA_DATA_SOURCE_IS_PROCEDURAL_SOUND flags (internally used)
    - added notifications to ma_sound (this contains multiple callbacks including endCallback and userData)
    - removed endCallback from ma_sound
    - removed pEndCallbackUserData from ma_sound
    - removed endCallback and pEndCallbackUserData from ma_sound_config
    - added notifications to ma_sound_config
    - modified ma_sound_init_ex so it can initialize notifications passed by ma_sound_config
    - modified ma_sound_set_at_end so it calls the onAtEnd callback set in the sound notifications
    - modified ma_sound_uninit so it can free allocated memory caused by calling ma_sound_init_from_memory and ma_sound_init_from_callback
    - modified ma_engine_node_process_pcm_frames__general so it calls onProcess callback if applicable
    - modified ma_sound_start so it calls onLoaded callback if applicable
*/

#ifndef MINIAUDIOEX_H
#define MINIAUDIOEX_H

#include "miniaudio.h"

typedef struct ma_ex_native_data_format ma_ex_native_data_format;

struct ma_ex_native_data_format {
    ma_format format;       /* Sample format. If set to ma_format_unknown, all sample formats are supported. */
    ma_uint32 channels;     /* If set to 0, all channels are supported. */
    ma_uint32 sampleRate;   /* If set to 0, all sample rates are supported. */
    ma_uint32 flags;        /* A combination of MA_DATA_FORMAT_FLAG_* flags. */
};

typedef struct ma_ex_device_info ma_ex_device_info;

struct ma_ex_device_info {
    char *pName;
    ma_int32 index;
    ma_bool32 isDefault;
    ma_uint32 nativeDataFormatCount;
    ma_ex_native_data_format *nativeDataFormats;
};

typedef struct ma_ex_context_config ma_ex_context_config;

struct ma_ex_context_config {
    ma_ex_device_info deviceInfo;
    ma_uint32 sampleRate;
    ma_uint8 channels;
    ma_uint32 periodSizeInFrames;
    ma_device_data_proc deviceDataProc;
};

typedef struct ma_ex_context ma_ex_context;

struct ma_ex_context {
    ma_context context;
    ma_device device;
    ma_engine engine;
    ma_resource_manager resourceManager;
    ma_uint32 sampleRate;
    ma_uint8 channels;
    ma_format format;
    ma_int32 listeners[MA_ENGINE_MAX_LISTENERS];
};

typedef struct ma_ex_audio_source_callbacks ma_ex_audio_source_callbacks;

struct ma_ex_audio_source_callbacks {
    void *pUserData;
    ma_sound_end_proc endCallback;
    ma_sound_load_proc loadCallback;
    ma_sound_process_proc processCallback;
};

typedef struct ma_ex_audio_source_settings ma_ex_audio_source_settings;

struct ma_ex_audio_source_settings {
    float volume;
    float pitch;
    ma_bool32 loop;
    ma_vec3f position;
    ma_vec3f direction;
    ma_vec3f velocity;
    ma_bool32 spatialization;
    ma_attenuation_model attenuationModel;
    float dopplerFactor;
    float minDistance;
    float maxDistance;
};

typedef struct ma_ex_audio_clip ma_ex_audio_clip;

struct ma_ex_audio_clip {
    ma_sound sound;
    ma_uint64 soundHash;
    ma_uint32 flags;
};

typedef struct ma_ex_audio_source ma_ex_audio_source;

struct ma_ex_audio_source {
    ma_ex_context *context;
    ma_ex_audio_clip clip;
    ma_ex_audio_source_callbacks callbacks;
    ma_ex_audio_source_settings settings;
};

typedef struct ma_ex_audio_listener_settings ma_ex_audio_listener_settings;

struct ma_ex_audio_listener_settings {
    ma_bool32 spatialization;
    ma_vec3f position;
    ma_vec3f direction;
    ma_vec3f velocity;
    ma_vec3f worldUp;
    float coneInnerAngleInRadians;
    float coneOuterAngleInRadians;
    float coneOuterGain;
};

typedef struct ma_ex_audio_listener ma_ex_audio_listener;

struct ma_ex_audio_listener {
    ma_ex_context *context;
    ma_uint32 index;
    ma_ex_audio_listener_settings settings;
};

#if defined(__cplusplus)
extern "C" {
#endif

MA_API ma_ex_device_info *ma_ex_playback_devices_get(ma_uint32 *count);
MA_API void ma_ex_playback_devices_free(ma_ex_device_info *pDeviceInfo, ma_uint32 count);

MA_API ma_ex_context_config ma_ex_context_config_init(ma_uint32 sampleRate, ma_uint8 channels, ma_uint32 periodSizeInFrames, const ma_ex_device_info *pDeviceInfo);
MA_API ma_ex_context *ma_ex_context_init(const ma_ex_context_config *config);
MA_API void ma_ex_context_uninit(ma_ex_context *context);
MA_API void ma_ex_context_set_master_volume(ma_ex_context *context, float volume);
MA_API float ma_ex_context_get_master_volume(ma_ex_context *context);
MA_API ma_engine *ma_ex_context_get_engine(ma_ex_context *context);
MA_API ma_node_graph *ma_ex_context_get_engine_node_graph(ma_ex_context *context);

MA_API void *ma_ex_device_get_user_data(ma_device *pDevice);

MA_API ma_ex_audio_source *ma_ex_audio_source_init(ma_ex_context *context);
MA_API void ma_ex_audio_source_uninit(ma_ex_audio_source *source);
MA_API void ma_ex_audio_source_set_callbacks(ma_ex_audio_source *source, ma_ex_audio_source_callbacks callbacks);
MA_API ma_result ma_ex_audio_source_play_from_file(ma_ex_audio_source *source, const char *filePath, ma_bool8 streamFromDisk);
MA_API ma_result ma_ex_audio_source_play_from_memory(ma_ex_audio_source *source, const void *pData, ma_uint64 dataSize);
MA_API ma_result ma_ex_audio_source_play_from_callback(ma_ex_audio_source *source, ma_procedural_sound_proc callback);
MA_API void ma_ex_audio_source_stop(ma_ex_audio_source *source);
MA_API void ma_ex_audio_source_apply_settings(ma_ex_audio_source *source);
MA_API void ma_ex_audio_source_set_volume(ma_ex_audio_source *source, float value);
MA_API float ma_ex_audio_source_get_volume(ma_ex_audio_source *source);
MA_API void ma_ex_audio_source_set_pitch(ma_ex_audio_source *source, float value);
MA_API float ma_ex_audio_source_get_pitch(ma_ex_audio_source *source);
MA_API void ma_ex_audio_source_set_pcm_position(ma_ex_audio_source *source, ma_uint64 position);
MA_API ma_uint64 ma_ex_audio_source_get_pcm_position(ma_ex_audio_source *source);
MA_API ma_uint64 ma_ex_audio_source_get_pcm_length(ma_ex_audio_source *source);
MA_API void ma_ex_audio_source_set_loop(ma_ex_audio_source *source, ma_bool32 loop);
MA_API ma_bool32 ma_ex_audio_source_get_loop(ma_ex_audio_source *source);
MA_API void ma_ex_audio_source_set_position(ma_ex_audio_source *source, float x, float y, float z);
MA_API void ma_ex_audio_source_get_position(ma_ex_audio_source *source, float *x, float *y, float *z);
MA_API void ma_ex_audio_source_set_direction(ma_ex_audio_source *source, float x, float y, float z);
MA_API void ma_ex_audio_source_get_direction(ma_ex_audio_source *source, float *x, float *y, float *z);
MA_API void ma_ex_audio_source_set_velocity(ma_ex_audio_source *source, float x, float y, float z);
MA_API void ma_ex_audio_source_get_velocity(ma_ex_audio_source *source, float *x, float *y, float *z);
MA_API void ma_ex_audio_source_set_spatialization(ma_ex_audio_source *source, ma_bool32 enabled);
MA_API ma_bool32 ma_ex_audio_source_get_spatialization(ma_ex_audio_source *source);
MA_API void ma_ex_audio_source_set_attenuation_model(ma_ex_audio_source *source, ma_attenuation_model model);
MA_API ma_attenuation_model ma_ex_audio_source_get_attenuation_model(ma_ex_audio_source *source);
MA_API void ma_ex_audio_source_set_doppler_factor(ma_ex_audio_source *source, float factor);
MA_API float ma_ex_audio_source_get_doppler_factor(ma_ex_audio_source *source);
MA_API void ma_ex_audio_source_set_min_distance(ma_ex_audio_source *source, float distance);
MA_API float ma_ex_audio_source_get_min_distance(ma_ex_audio_source *source);
MA_API void ma_ex_audio_source_set_max_distance(ma_ex_audio_source *source, float distance);
MA_API float ma_ex_audio_source_get_max_distance(ma_ex_audio_source *source);
MA_API ma_bool32 ma_ex_audio_source_get_is_playing(ma_ex_audio_source *source);
MA_API ma_ex_audio_clip *ma_ex_audio_source_get_clip(ma_ex_audio_source *source);

MA_API ma_ex_audio_listener *ma_ex_audio_listener_init(ma_ex_context *context);
MA_API void ma_ex_audio_listener_uninit(ma_ex_audio_listener *listener);
MA_API void ma_ex_audio_listener_set_spatialization(ma_ex_audio_listener *listener, ma_bool32 enabled);
MA_API ma_bool32 ma_ex_audio_listener_get_spatialization(ma_ex_audio_listener *listener);
MA_API void ma_ex_audio_listener_set_position(ma_ex_audio_listener *listener, float x, float y, float z);
MA_API void ma_ex_audio_listener_get_position(ma_ex_audio_listener *listener, float *x, float *y, float *z);
MA_API void ma_ex_audio_listener_set_direction(ma_ex_audio_listener *listener, float x, float y, float z);
MA_API void ma_ex_audio_listener_get_direction(ma_ex_audio_listener *listener, float *x, float *y, float *z);
MA_API void ma_ex_audio_listener_set_velocity(ma_ex_audio_listener *listener, float x, float y, float z);
MA_API void ma_ex_audio_listener_get_velocity(ma_ex_audio_listener *listener, float *x, float *y, float *z);
MA_API void ma_ex_audio_listener_set_world_up(ma_ex_audio_listener *listener, float x, float y, float z);
MA_API void ma_ex_audio_listener_get_world_up(ma_ex_audio_listener *listener, float *x, float *y, float *z);
MA_API void ma_ex_audio_listener_set_cone(ma_ex_audio_listener *listener, float innerAngleInRadians, float outerAngleInRadians, float outerGain);
MA_API void ma_ex_audio_listener_get_cone(ma_ex_audio_listener *listener, float *innerAngleInRadians, float *outerAngleInRadians, float *outerGain);

MA_API char *ma_ex_read_bytes_from_file(const char *filepath, size_t *size);
MA_API void ma_ex_free_bytes_from_file(char *pointer);
MA_API void ma_ex_free(void *pointer);

MA_API float *ma_ex_decode_file(const char *pFilePath, ma_uint64 *dataLength, ma_uint32 *channels, ma_uint32 *sampleRate, ma_uint32 desiredChannels, ma_uint32 desiredSampleRate);
MA_API float *ma_ex_decode_memory(const void *pData, ma_uint64 size, ma_uint64 *dataLength, ma_uint32 *channels, ma_uint32 *sampleRate, ma_uint32 desiredChannels, ma_uint32 desiredSampleRate);

#if defined(__cplusplus)
}
#endif

#endif
