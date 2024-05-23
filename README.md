# miniaudioex
This is a modified version of the [MiniAudio](https://github.com/mackron/miniaudio) library. My aim was to extend the library so I can more easily use it in applications, and write simpler bindings for other languages. Most notable feature of this extension is the concept of an audio source. By design, MiniAudio works on a 'per sound basis', which might be desirable for some people, but for me it's not. Instead I like to create one or multiple audio sources which can all play the same sound simultaneously, while still being able to control the properties of the sound (such as spatial settings) through the audio source.

# Building
There's no need to install any dependencies. On Windows and macOS there's no need to link to  anything. On Linux just link to `-lpthread`, `-lm` and `-ldl`. On BSD just link to `-lpthread` and `-lm`. On iOS you need to compile as Objective-C.

If you get errors about undefined references to `__sync_val_compare_and_swap_8`, `__atomic_load_8`, etc. you need to link with `-latomic`.

# Example 1
This shows how to play audio from a given file path.
```c
#include "miniaudioex.h"

#define SAMPLE_RATE 44100
#define NUM_CHANNELS 2

int main(int argc, char **argv) {
    const char *file = "some_audio.mp3";

    ma_ex_context_config contextConfig = ma_ex_context_config_init(SAMPLE_RATE, NUM_CHANNELS, 0, NULL);
    ma_ex_context *context = ma_ex_context_init(&contextConfig);

    ma_ex_audio_source *source = ma_ex_audio_source_init(context);

    ma_ex_audio_source_play_from_file(source, file, MA_TRUE);

    printf("Press enter to stop ");
    getchar();

    ma_ex_audio_source_stop(source);
    ma_ex_audio_source_uninit(source);
    ma_ex_context_uninit(context);

    return 0;
}
```
# Example 2
You can also play audio directly from memory.
```c
#include "miniaudioex.h"

#define SAMPLE_RATE 44100
#define NUM_CHANNELS 2

int main(int argc, char **argv) {
    size_t fileSize = 0;
    char *file = ma_ex_read_bytes_from_file("some_audio.mp3", &fileSize);

    if(file == NULL)
        return 1;

    ma_ex_context_config contextConfig = ma_ex_context_config_init(SAMPLE_RATE, NUM_CHANNELS, 0, NULL);
    ma_ex_context *context = ma_ex_context_init(&contextConfig);

    ma_ex_audio_source *source = ma_ex_audio_source_init(context);

    ma_ex_audio_source_play_from_memory(source, file, fileSize);

    printf("Press enter to stop ");
    getchar();

    ma_ex_audio_source_stop(source);
    ma_ex_audio_source_uninit(source);
    ma_ex_context_uninit(context);

    ma_ex_free_bytes_from_file(file);

    return 0;
}
```
# Example 3
Generating audio with a callback function.
```c
#include "miniaudioex.h"
#include <math.h>

#define SAMPLE_RATE 44100
#define NUM_CHANNELS 2

void on_waveform(void *pUserData, void* pFramesOut, ma_uint64 frameCount, ma_uint32 channels, NULL);

int main(int argc, char **argv) {
    ma_ex_context_config contextConfig = ma_ex_context_config_init(SAMPLE_RATE, NUM_CHANNELS, 0, NULL);
    ma_ex_context *context = ma_ex_context_init(&contextConfig);

    ma_ex_audio_source *source = ma_ex_audio_source_init(context);

    ma_ex_audio_source_callbacks callbacks = {
        .pUserData = NULL,
        .loadCallback = NULL,
        .endCallback = NULL,
        .processCallback = NULL,
        .waveformCallback = &on_waveform
    };

    ma_ex_audio_source_set_callbacks(source, callbacks);
    ma_ex_audio_source_play(source);

    printf("Press enter to stop ");
    getchar();

    ma_ex_audio_source_stop(source);
    ma_ex_audio_source_uninit(source);
    ma_ex_context_uninit(context);

    return 0;
}

#ifndef M_PI
#define M_PI 3.14159265359
#endif
ma_uint64 timeCounter = 0;

void on_waveform(void *pUserData, void* pFramesOut, ma_uint64 frameCount, ma_uint32 channels) {
    float *dataOut = (float*)pFramesOut;
    size_t numSamples = frameCount * channels;
    float sample = 0.0f;

    for(size_t i = 0; i < numSamples; i+=channels) {
        sample = sinf(2 * M_PI * 440 * timeCounter / SAMPLE_RATE);
        dataOut[i] = sample;
        if(channels == 2)
            dataOut[i+1] = sample;
        timeCounter++;
    }
}
```