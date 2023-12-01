#include "miniaudio_mixer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef MA_MIXER_DEBUG_LOGGING
#define ma_mixer_debug_log(...) printf(__VA_ARGS__)
#else
#define ma_mixer_debug_log(...)
#endif

static int strcmp_null_safe(const char *a, const char *b) {
    if(a == NULL || b == NULL) {
        return 1;
    }
    return strcmp(a, b);
}

ma_audio_mixer *ma_audio_mixer_create(ma_uint32 sampleRate, ma_uint32 channels, ma_format format, ma_device_data_proc dataProc) {
    ma_audio_mixer *mixer = malloc(sizeof(ma_audio_mixer));
    memset(mixer, 0, sizeof(ma_audio_mixer));

    for(size_t i = 0; i < MA_MAX_AUDIO_SOURCES; i++) {
        mixer->sources[i] = NULL;
    }

    mixer->engine = malloc(sizeof(ma_engine));
    memset(mixer->engine, 0, sizeof(ma_engine));
    
    mixer->device = malloc(sizeof(ma_device));
    memset(mixer->device, 0, sizeof(ma_device));

    mixer->channels = channels;
    mixer->dataProc = dataProc;
    mixer->format = format;
    mixer->sampleRate = sampleRate;

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = mixer->format;
    deviceConfig.playback.channels = mixer->channels;
    deviceConfig.sampleRate        = mixer->sampleRate;
    deviceConfig.dataCallback      = mixer->dataProc;

    if(ma_device_init(NULL, &deviceConfig, mixer->device) != MA_SUCCESS) {
        free(mixer->device);
        free(mixer->engine);
        free(mixer);
        ma_mixer_debug_log("ma_audio_mixer_create: failed to initialize device.\n");
        return NULL;
    }

    ma_engine_config engineConfig = ma_engine_config_init();
    engineConfig.listenerCount = 1;

    if(ma_engine_init(&engineConfig, mixer->engine) != MA_SUCCESS) {
        ma_device_uninit(mixer->device);
        free(mixer->device);
        free(mixer->engine);
        free(mixer);
        ma_mixer_debug_log("ma_audio_mixer_create: failed to initialize engine.");
        return NULL;
    }

    if (ma_device_start(mixer->device) != MA_SUCCESS) 
    {
        ma_device_uninit(mixer->device);
        ma_engine_uninit(mixer->engine);
        free(mixer->device);
        free(mixer->engine);
        free(mixer);
        ma_mixer_debug_log("ma_audio_mixer_create: failed to start device.\n");
        return NULL;
    }

    ma_mixer_debug_log("ma_audio_mixer_create: audio mixer created\n");

    return mixer;
}

void ma_audio_mixer_destroy(ma_audio_mixer *mixer) {
    if(mixer != NULL) {
        ma_engine_uninit(mixer->engine);
        ma_device_uninit(mixer->device);
        free(mixer->engine);
        free(mixer->device);
        free(mixer);
        ma_mixer_debug_log("ma_audio_mixer_destroy: audio mixer destroyed\n");
    }
}

ma_result ma_audio_mixer_add_audio_source(ma_audio_mixer *mixer, ma_audio_source *source) {
    if(mixer != NULL && source != NULL) {
        for(size_t i = 0; i < MA_MAX_AUDIO_SOURCES; i++) {
            if(mixer->sources[i] == NULL) {
                mixer->sources[i] = source;
                ma_mixer_debug_log("ma_audio_mixer_add_audio_source: audio source added\n");
                return MA_SUCCESS;
            } else {
                if(mixer->sources[i] == source) {
                    ma_mixer_debug_log("ma_audio_mixer_add_audio_source: audio source already exists\n");
                    return MA_ALREADY_EXISTS;
                }
            }
        }
    }

    ma_mixer_debug_log("ma_audio_mixer_add_audio_source: no slots available for new audio source\n");
    return MA_OUT_OF_RANGE;
}

ma_result ma_audio_mixer_remove_audio_source(ma_audio_mixer *mixer, ma_audio_source *source) {
    if(mixer != NULL && source != NULL) {
        for(size_t i = 0; i < MA_MAX_AUDIO_SOURCES; i++) {
            if(mixer->sources[i] == source) {
                mixer->sources[i] = NULL;
                ma_mixer_debug_log("ma_audio_mixer_remove_audio_source: audio source removed\n");
                return MA_SUCCESS;
            }
        }
    }

    ma_mixer_debug_log("ma_audio_mixer_remove_audio_source: the given audio source does not exist\n");
    return MA_DOES_NOT_EXIST;
}

ma_audio_source *ma_audio_source_create(ma_engine *engine, ma_dsp_proc dspProc) {
    if(engine == NULL) {
        ma_mixer_debug_log("ma_audio_source_create: engine can not be null\n");
        return NULL;
    }

    ma_audio_source *source = malloc(sizeof(ma_audio_source));
    source->engine = engine;
    source->dspProc = dspProc;
    source->filePath = NULL;
    source->soundLoadedProc = NULL;
    source->soundLoadedProcUserData = NULL;
    source->soundEndedProc = NULL;
    source->soundEndedProcUserData = NULL;
    memset(&source->sound, 0, sizeof(ma_sound));

    ma_mixer_debug_log("ma_audio_source_create: audio source created\n");
    return source;
}

void ma_audio_source_destroy(ma_audio_source *source) {
    if(source != NULL) {
        ma_sound_uninit(&source->sound);
        if(source->filePath != NULL) {
            free(source->filePath);
        }
        free(source);
        ma_mixer_debug_log("ma_audio_source_destroy: audio source destroyed\n");
    }
}

void ma_audio_source_set_sound_loaded_proc(ma_audio_source *source, ma_sound_loaded_proc proc, void *userData) {
    if(source != NULL) {
        source->soundLoadedProc = proc;
        source->soundLoadedProcUserData = userData;
    }
}

void ma_audio_source_set_sound_ended_proc(ma_audio_source *source, ma_sound_end_proc proc, void *userData) {
    if(source != NULL) {
        source->soundEndedProc = proc;
        source->soundEndedProcUserData = userData;
    }
}

ma_result ma_audio_source_play(ma_audio_source *source, const char *filePath, ma_bool8 streamFromDisk) {
    if(filePath == NULL) {
        ma_mixer_debug_log("ma_audio_source_play: the given file path is null\n");
        return MA_ERROR;
    }

    if(source != NULL) {
        if(strcmp_null_safe(source->filePath, filePath) != 0) {
            ma_sound_uninit(&source->sound);
            ma_uint32 flags = MA_SOUND_FLAG_ASYNC | MA_SOUND_FLAG_DECODE;
            
            if(streamFromDisk) {
                flags |= MA_SOUND_FLAG_STREAM;            
            }

            ma_result result = ma_sound_init_from_file(source->engine, filePath, flags, NULL, NULL, &source->sound);

            if(result != MA_SUCCESS) {
                ma_mixer_debug_log("ma_audio_source_play: could not initialize file\n");
                return result;
            } else {
                source->sound.engineNode.dspProc = source->dspProc;

                if(source->soundEndedProc != NULL) {
                    ma_sound_set_end_callback(&source->sound, source->soundEndedProc, source->soundEndedProcUserData);
                }
                if(source->soundLoadedProc != NULL) {
                    source->soundLoadedProc(&source->sound, source->soundLoadedProcUserData);
                }
            }

            if(source->filePath != NULL) {
                free(source->filePath);
            }

            size_t pathSize = strlen(filePath);

            if(pathSize == 0) {
                ma_mixer_debug_log("ma_audio_source_play: the length of given file path is 0\n");
                return MA_INVALID_FILE;
            }

            source->filePath = malloc(pathSize + 1);
            memset(source->filePath, 0, pathSize + 1);
            strcpy(source->filePath, filePath);
        }

        ma_result result = ma_sound_start(&source->sound);

        if(result != MA_SUCCESS) {
            ma_mixer_debug_log("ma_audio_source_play: could not start the sound\n");
        } else {
            ma_mixer_debug_log("ma_audio_source_play: success\n");
        }

        return result;
    }

    ma_mixer_debug_log("ma_audio_source_play: source/clip can not be null\n");
    return MA_ERROR;
}

void ma_audio_source_stop(ma_audio_source *source) {
    if(source != NULL) {
        ma_mixer_debug_log("ma_audio_source_stop: stopping sound\n");
        ma_sound_stop(&source->sound);
    }
}

void ma_audio_source_set_pcm_position(ma_audio_source *source, ma_uint64 frameIndex) {
    if(source != NULL) {
        ma_sound_seek_to_pcm_frame(&source->sound, frameIndex);
    }
}

void ma_audio_source_set_pcm_start_position(ma_audio_source *source, ma_uint64 frameIndex) {
    if(source != NULL) {
        ma_sound_set_start_time_in_pcm_frames(&source->sound, frameIndex);
    }
}

void ma_audio_source_set_loop(ma_audio_source *source, ma_bool32 loop) {
    if(source != NULL) {
        ma_sound_set_looping(&source->sound, loop);
    }
}

void ma_audio_source_set_volume(ma_audio_source *source, float volume) {
    if(source != NULL) {
        ma_sound_set_volume(&source->sound, volume);
    }
}

void ma_audio_source_set_pitch(ma_audio_source *source, float pitch) {
    if(source != NULL) {
        ma_sound_set_pitch(&source->sound, pitch);
    }
}

void ma_audio_source_set_position(ma_audio_source *source, float x, float y, float z) {
    if(source != NULL) {
        ma_sound_set_position(&source->sound, x, y, z);
    }
}

void ma_audio_source_set_direction(ma_audio_source *source, float x, float y, float z) {
    if(source != NULL) {
        ma_sound_set_direction(&source->sound, x, y, z);
    }
}

void ma_audio_source_set_velocity(ma_audio_source *source, float x, float y, float z) {
    if(source != NULL) {
        ma_sound_set_velocity(&source->sound, x, y, z);
    }
}

void ma_audio_source_set_spatialization(ma_audio_source *source, ma_bool32 enabled) {
    if(source != NULL) {
        ma_sound_set_spatialization_enabled(&source->sound, enabled);
    }
}

void ma_audio_source_set_attenuation_model(ma_audio_source *source, ma_attenuation_model model) {
    if(source != NULL) {
        ma_sound_set_attenuation_model(&source->sound, model);
    }
}

void ma_audio_source_set_doppler_factor(ma_audio_source *source, float factor) {
    if(source != NULL) {
        ma_sound_set_doppler_factor(&source->sound, factor);
    }
}

void ma_audio_source_set_min_distance(ma_audio_source *source, float distance) {
    if(source != NULL) {
        ma_sound_set_min_distance(&source->sound, distance);
    }
}

void ma_audio_source_set_max_distance(ma_audio_source *source, float distance) {
    if(source != NULL) {
        ma_sound_set_max_distance(&source->sound, distance);
    }
}

ma_bool32 ma_audio_source_get_is_playing(ma_audio_source *source) {
    if(source != NULL) {
        ma_sound_is_playing(&source->sound);
    }
}

ma_audio_listener *ma_audio_listener_create(ma_engine *engine) {
    if(engine == NULL) {
        ma_mixer_debug_log("ma_audio_listener_create: engine can not be null\n");
        return NULL;
    }
    ma_audio_listener *listener = malloc(sizeof(ma_audio_listener));
    memset(listener, 0, sizeof(ma_audio_listener));
    listener->engine = engine;
    ma_mixer_debug_log("ma_audio_listener_create: audio listener created\n");
    return listener;
}

void ma_audio_listener_destroy(ma_audio_listener *listener) {
    if(listener != NULL) {
        ma_mixer_debug_log("ma_audio_listener_destroy: audio listener destroyed\n");
        free(listener);
    }
}

void ma_audio_listener_set_spatialization(ma_audio_listener *listener, ma_bool32 enabled) {
    if(listener != NULL) {
        if(listener->engine != NULL) {
            ma_engine_listener_set_enabled(listener->engine, 0, enabled);
        }
    }
}

void ma_audio_listener_set_position(ma_audio_listener *listener, float x, float y, float z) {
    if(listener != NULL) {
        if(listener->engine != NULL) {
            ma_engine_listener_set_position(listener->engine, 0, x, y, z);
        }
    }
}

void ma_audio_listener_set_direction(ma_audio_listener *listener, float x, float y, float z) {
    if(listener != NULL) {
        if(listener->engine != NULL) {
            ma_engine_listener_set_direction(listener->engine, 0, x, y, z);
        }
    }
}

void ma_audio_listener_set_velocity(ma_audio_listener *listener, float x, float y, float z) {
    if(listener != NULL) {
        if(listener->engine != NULL) {
            ma_engine_listener_set_velocity(listener->engine, 0, x, y, z);
        }
    }
}

void ma_audio_listener_set_world_up(ma_audio_listener *listener, float x, float y, float z) {
    if(listener != NULL) {
        if(listener->engine != NULL) {
            ma_engine_listener_set_world_up(listener->engine, 0, x, y, z);
        }
    }
}

void ma_audio_listener_set_cone(ma_audio_listener *listener, float innerAngleInRadians, float outerAngleInRadians, float outerGain) {
    if(listener != NULL) {
        if(listener->engine != NULL) {
            ma_engine_listener_set_cone(listener->engine, 0, innerAngleInRadians, outerAngleInRadians, outerGain);
        }
    }
}