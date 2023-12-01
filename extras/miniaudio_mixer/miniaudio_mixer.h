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

#ifndef MINIAUDIO_MIXER_H
#define MINIAUDIO_MIXER_H

#include "miniaudio.h"

#define MA_MAX_AUDIO_SOURCES 1024

typedef void (*ma_sound_loaded_proc)(ma_sound *pSound, void *pUserData);

typedef struct {
    ma_engine *engine;
} ma_audio_listener;

typedef struct {
    ma_sound sound;
    char *filePath;
    ma_sound_loaded_proc soundLoadedProc;
    void *soundLoadedProcUserData;
    ma_sound_end_proc soundEndedProc;
    void *soundEndedProcUserData;
    ma_engine *engine;
    ma_dsp_proc dspProc;
} ma_audio_source;

typedef struct {
    ma_device *device;
    ma_engine *engine;
    ma_audio_source *sources[MA_MAX_AUDIO_SOURCES];
    ma_uint32 sampleRate;
    ma_uint8 channels;
    ma_format format;
    ma_device_data_proc dataProc;
}  ma_audio_mixer;

#if defined(__cplusplus)
extern "C" {
#endif
    MA_API ma_audio_mixer *ma_audio_mixer_create(ma_uint32 sampleRate, ma_uint32 channels, ma_format format, ma_device_data_proc dataProc);
    MA_API void ma_audio_mixer_destroy(ma_audio_mixer *mixer);
    MA_API ma_result ma_audio_mixer_add_audio_source(ma_audio_mixer *mixer, ma_audio_source *source);
    MA_API ma_result ma_audio_mixer_remove_audio_source(ma_audio_mixer *mixer, ma_audio_source *source);

    MA_API ma_audio_source *ma_audio_source_create(ma_engine *engine, ma_dsp_proc dspProc);
    MA_API void ma_audio_source_destroy(ma_audio_source *source);
    MA_API void ma_audio_source_set_sound_loaded_proc(ma_audio_source *source, ma_sound_loaded_proc proc, void *userData);
    MA_API void ma_audio_source_set_sound_ended_proc(ma_audio_source *source, ma_sound_end_proc proc, void *userData);
    MA_API ma_result ma_audio_source_play(ma_audio_source *source, const char *filePath, ma_bool8 streamFromDisk);
    MA_API void ma_audio_source_stop(ma_audio_source *source);
    MA_API void ma_audio_source_set_pcm_position(ma_audio_source *source, ma_uint64 frameIndex);
    MA_API void ma_audio_source_set_pcm_start_position(ma_audio_source *source, ma_uint64 frameIndex);
    MA_API void ma_audio_source_set_loop(ma_audio_source *source, ma_bool32 loop);
    MA_API void ma_audio_source_set_volume(ma_audio_source *source, float volume);
    MA_API void ma_audio_source_set_pitch(ma_audio_source *source, float pitch);
    MA_API void ma_audio_source_set_position(ma_audio_source *source, float x, float y, float z);
    MA_API void ma_audio_source_set_direction(ma_audio_source *source, float x, float y, float z);
    MA_API void ma_audio_source_set_velocity(ma_audio_source *source, float x, float y, float z);
    MA_API void ma_audio_source_set_spatialization(ma_audio_source *source, ma_bool32 enabled);
    MA_API void ma_audio_source_set_attenuation_model(ma_audio_source *source, ma_attenuation_model model);
    MA_API void ma_audio_source_set_doppler_factor(ma_audio_source *source, float factor);
    MA_API void ma_audio_source_set_min_distance(ma_audio_source *source, float distance);
    MA_API void ma_audio_source_set_max_distance(ma_audio_source *source, float distance);
    MA_API ma_bool32 ma_audio_source_get_is_playing(ma_audio_source *source);

    MA_API ma_audio_listener *ma_audio_listener_create(ma_engine *engine);
    MA_API void ma_audio_listener_destroy(ma_audio_listener *listener);
    MA_API void ma_audio_listener_set_spatialization(ma_audio_listener *listener, ma_bool32 enabled);
    MA_API void ma_audio_listener_set_position(ma_audio_listener *listener, float x, float y, float z);
    MA_API void ma_audio_listener_set_direction(ma_audio_listener *listener, float x, float y, float z);
    MA_API void ma_audio_listener_set_velocity(ma_audio_listener *listener, float x, float y, float z);
    MA_API void ma_audio_listener_set_world_up(ma_audio_listener *listener, float x, float y, float z);
    MA_API void ma_audio_listener_set_cone(ma_audio_listener *listener, float innerAngleInRadians, float outerAngleInRadians, float outerGain);
#if defined(__cplusplus)
}
#endif

#endif