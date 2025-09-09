#include "miniaudioex.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SAMPLE_RATE 44100
#define NUM_CHANNELS 2

void wave_proc(void *pUserData, void* pFramesOut, ma_uint64 frameCount, ma_uint32 channels);

int main(int argc, char **argv) {
    ma_ex_context_config contextConfig = ma_ex_context_config_init(SAMPLE_RATE, NUM_CHANNELS, 0, NULL); 
    ma_ex_context *context = ma_ex_context_init(&contextConfig);

    ma_ex_audio_source *source = ma_ex_audio_source_init(context);
    ma_ex_audio_source_set_loop(source, MA_TRUE);

    ma_procedural_wave_config waveConfig = ma_procedural_wave_config_init(ma_format_f32, NUM_CHANNELS, SAMPLE_RATE, wave_proc, NULL);

    ma_ex_audio_clip *clip = ma_ex_audio_clip_init_from_procedural_wave(context, &waveConfig);

    ma_ex_audio_source_play(source, clip);

    printf("Press enter to stop ");
    
    getchar();

    ma_ex_audio_source_stop(source);
    ma_ex_audio_source_uninit(source);
    ma_ex_audio_clip_uninit(clip);
    ma_ex_context_uninit(context);

    return 0;
}

#ifndef M_PI
#define M_PI 3.14159265358979323846264f
#endif

ma_uint64 waveTimer = 0;

void wave_proc(void *pUserData, void* pFramesOut, ma_uint64 frameCount, ma_uint32 channels) {
    float *pData = (float*)pFramesOut;
    const ma_uint64 sampleCount = frameCount * channels;

    for(ma_uint64 i = 0; i < sampleCount; i+=channels) {
        float sample = sinf(2 * M_PI * 440 * waveTimer / SAMPLE_RATE) * 0.1f;
        pData[i] = sample;
        if(channels == 2) {
            pData[i+1] = sample;
        }
        waveTimer++;
    }
}