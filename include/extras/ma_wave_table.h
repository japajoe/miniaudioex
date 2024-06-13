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

#ifndef MA_WAVE_TABLE_H
#define MA_WAVE_TABLE_H

#include "miniaudio.h"

typedef enum {
    ma_wave_table_type_sine,
    ma_wave_table_type_saw,
    ma_wave_table_type_square,
    ma_wave_table_type_triangle,
    ma_wave_table_type_custom
} ma_wave_table_type;

typedef struct {
    ma_wave_table_type type;
    float *pData;
    ma_uint64 dataSampleCount;
} ma_wave_table_config;

typedef struct {
    float *pData;
    ma_uint64 dataSize;
    ma_uint64 dataSampleCount;
    ma_uint64 dataIndex;
    ma_wave_table_type type;
    float phase;
    float phaseIncrement;
} ma_wave_table;

#if defined(__cplusplus)
extern "C" {
#endif

MA_API ma_wave_table_config ma_wave_table_config_init(ma_wave_table_type type, float *pData, ma_uint64 dataSampleCount);
MA_API ma_result ma_wave_table_init(const ma_wave_table_config* pConfig, ma_wave_table* pWavetable);
MA_API void ma_wave_table_uninit(ma_wave_table* pWavetable);
MA_API void ma_wave_table_reset(ma_wave_table* pWavetable);
MA_API float ma_wave_table_get_sample(ma_wave_table* pWavetable, float frequency, float sampleRate);
MA_API float ma_wave_table_get_sample_at_phase(ma_wave_table* pWavetable, float phase);

#if defined(__cplusplus)
}
#endif

#endif