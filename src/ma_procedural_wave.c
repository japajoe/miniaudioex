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

#include "ma_procedural_wave.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static ma_result ma_procedural_wave__data_source_on_read(ma_data_source* pDataSource, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead) {
    return ma_procedural_wave_read_pcm_frames((ma_procedural_wave*)pDataSource, pFramesOut, frameCount, pFramesRead);
}

static ma_result ma_procedural_wave__data_source_on_seek(ma_data_source* pDataSource, ma_uint64 frameIndex) {
    return MA_NOT_IMPLEMENTED;
}

static ma_result ma_procedural_wave__data_source_on_get_data_format(ma_data_source* pDataSource, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate, ma_channel* pChannelMap, size_t channelMapCap) {
    ma_procedural_wave* pWaveform = (ma_procedural_wave*)pDataSource;

    *pFormat     = pWaveform->config.format;
    *pChannels   = pWaveform->config.channels;
    *pSampleRate = pWaveform->config.sampleRate;
    ma_channel_map_init_standard(ma_standard_channel_map_default, pChannelMap, channelMapCap, pWaveform->config.channels);

    return MA_SUCCESS;
}

static ma_result ma_procedural_wave__data_source_on_get_cursor(ma_data_source* pDataSource, ma_uint64* pCursor) {
    *pCursor = 0;
    return MA_NOT_IMPLEMENTED;
}

static ma_data_source_vtable g_ma_procedural_wave_data_source_vtable = {
    ma_procedural_wave__data_source_on_read,
    ma_procedural_wave__data_source_on_seek,
    ma_procedural_wave__data_source_on_get_data_format,
    ma_procedural_wave__data_source_on_get_cursor,
    NULL,   /* onGetLength. There's no notion of a length in waveforms. */
    NULL,   /* onSetLooping */
    0
};


MA_API ma_procedural_wave_config ma_procedural_wave_config_init(ma_format format, ma_uint32 channels, ma_uint32 sampleRate, ma_procedural_wave_proc pWaveformProc, void *pUserData) {
    MA_ASSERT(pWaveformProc != NULL);

    ma_procedural_wave_config config;

    MA_ZERO_OBJECT(&config);
    config.format           = format;
    config.channels         = channels;
    config.sampleRate       = sampleRate;
    config.waveformCallback = pWaveformProc;
    config.pUserData        = pUserData;

    return config;
}

MA_API ma_result ma_procedural_wave_init(const ma_procedural_wave_config* pConfig, ma_procedural_wave* pWaveform) {
    ma_result result;
    ma_data_source_config dataSourceConfig;

    if (pWaveform == NULL) {
        return MA_INVALID_ARGS;
    }

    MA_ZERO_OBJECT(pWaveform);

    dataSourceConfig = ma_data_source_config_init();
    dataSourceConfig.vtable = &g_ma_procedural_wave_data_source_vtable;

    result = ma_data_source_init(&dataSourceConfig, &pWaveform->ds);
    if (result != MA_SUCCESS) {
        return result;
    }

    pWaveform->config  = *pConfig;

    return MA_SUCCESS;
}

MA_API void ma_procedural_wave_uninit(ma_procedural_wave* pWaveform) {
    if (pWaveform == NULL) {
        return;
    }

    ma_data_source_uninit(&pWaveform->ds);
    MA_ZERO_OBJECT(pWaveform);
}

MA_API ma_result ma_procedural_wave_read_pcm_frames(ma_procedural_wave* pWaveform, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead) {
    if (pFramesRead != NULL) {
        *pFramesRead = 0;
    }

    if (frameCount == 0) {
        return MA_INVALID_ARGS;
    }

    if (pWaveform == NULL) {
        return MA_INVALID_ARGS;
    }

    if (pFramesOut != NULL) {
        if(pWaveform->config.waveformCallback != NULL) {
            pWaveform->config.waveformCallback(pWaveform->config.pUserData, pFramesOut, frameCount, pWaveform->config.channels);
        }
    }

    if (pFramesRead != NULL) {
        *pFramesRead = frameCount;
    }

    return MA_SUCCESS;
}