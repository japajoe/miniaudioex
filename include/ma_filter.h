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

#ifndef MA_FILTER_H
#define MA_FILTER_H

#include "miniaudio.h"

typedef enum {
    ma_filter_type_lowpass,
    ma_filter_type_highpass,
    ma_filter_type_bandpass,
    ma_filter_type_lowshelf,
    ma_filter_type_highshelf,
    ma_filter_type_peak,
    ma_filter_type_notch
} ma_filter_type;

typedef struct {
    ma_filter_type type;
    ma_int32 sampleRate;
    float frequency;
    float q;
    float gainDB;
} ma_filter_config;

typedef struct ma_biquad_filter ma_biquad_filter;

typedef struct {
    void (* onCalculateBiQuadCoefficients)(ma_biquad_filter* pFilter);
} ma_filter_vtable;

struct ma_biquad_filter{
    ma_filter_type type;
    ma_int32 sampleRate;
    float frequency;
    float q;
    float gainDB;
    float a0;
    float a1;
    float a2;
    float b1;
    float b2;
    float z1;
    float z2;
    ma_filter_vtable vtable;
};

#if defined(__cplusplus)
extern "C" {
#endif

MA_API ma_filter_config ma_filter_config_init(ma_filter_type type, ma_int32 sampleRate, float frequency, float q, float gainDB);
MA_API ma_result ma_filter_init(const ma_filter_config *pConfig, ma_biquad_filter *pFilter);
MA_API void ma_filter_uninit(ma_biquad_filter *pFilter);
MA_API void ma_filter_set_type(ma_biquad_filter *pFilter, ma_filter_type type);
MA_API ma_filter_type ma_filter_get_type(const ma_biquad_filter *pFilter);
MA_API void ma_filter_set_frequency(ma_biquad_filter *pFilter, float frequency);
MA_API float ma_filter_get_frequency(const ma_biquad_filter *pFilter);
MA_API void ma_filter_set_q(ma_biquad_filter *pFilter, float q);
MA_API float ma_filter_get_q(const ma_biquad_filter *pFilter);
MA_API void ma_filter_set_gain_db(ma_biquad_filter *pFilter, float gainDB);
MA_API float ma_filter_get_gain_db(const ma_biquad_filter *pFilter);
MA_API float ma_filter_process_sample(ma_biquad_filter *pFilter, float input);
MA_API void ma_filter_process_samples(ma_biquad_filter *pFilter, const float *pInput, ma_uint64 inputSamples, float *pOutput, ma_uint64 outputSamples);

#if defined(__cplusplus)
}
#endif


#endif