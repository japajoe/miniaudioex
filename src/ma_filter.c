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

// // based on http://www.earlevel.com/main/2011/01/02/biquad-formulas/

#include "ma_filter.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

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

#ifndef MA_PI
#define MA_PI 3.14159265359
#endif

#define ma_abs(x)                       (((x) > 0) ? (x) : -(x))

static void ma_filter_lowpass_calculate_biquad_coefficients(ma_biquad_filter *pFilter) {
    if(pFilter == NULL) 
        return;
    float k = tanf(MA_PI * pFilter->frequency / pFilter->sampleRate);
    float norm = 1.0f / (1.0f + k / pFilter->q + k * k);
    pFilter->a0 = k * k * norm;
    pFilter->a1 = 2.0f * pFilter->a0;
    pFilter->a2 = pFilter->a0;
    pFilter->b1 = 2.0f * (k * k - 1.0f) * norm;
    pFilter->b2 = (1.0f - k / pFilter->q + k * k) * norm;
}

static void ma_filter_highpass_calculate_biquad_coefficients(ma_biquad_filter *pFilter) {
    if(pFilter == NULL) 
        return;
    float k = tanf(MA_PI * pFilter->frequency / pFilter->sampleRate);
    float norm = 1.0f / (1.0f + k / pFilter->q + k * k);
    pFilter->a0 = 1.0f * norm;
    pFilter->a1 = -2.0f * pFilter->a0;
    pFilter->a2 = pFilter->a0;
    pFilter->b1 = 2.0f * (k * k - 1.0f) * norm;
    pFilter->b2 = (1.0f - k / pFilter->q + k * k) * norm;
}

static void ma_filter_bandpass_calculate_biquad_coefficients(ma_biquad_filter *pFilter) {
    if(pFilter == NULL) 
        return;
    float k = tanf(MA_PI * pFilter->frequency / pFilter->sampleRate);
    float norm = 1.0f / (1.0f + k / pFilter->q + k * k);
    pFilter->a0 = k / pFilter->q * norm;
    pFilter->a1 = 0.0f;
    pFilter->a2 = -pFilter->a0;
    pFilter->b1 = 2.0f * (k * k - 1.0f) * norm;
    pFilter->b2 = (1.0f - k / pFilter->q + k * k) * norm;
}

static void ma_filter_lowshelf_calculate_biquad_coefficients(ma_biquad_filter *pFilter) {
    if(pFilter == NULL) 
        return;
    const double sqrt2 = 1.4142135623730951;
    float k = tanf(MA_PI * pFilter->frequency / pFilter->sampleRate);
    float v = powf(10.0f, ma_abs(pFilter->gainDB) / 20.0f);
    float norm;
    if (pFilter->gainDB >= 0.0f) {
        // boost
        norm = 1.0f / (1.0f + sqrt2 * k + k * k);
        pFilter->a0 = (1.0f + sqrtf(2.0f * v) * k + v * k * k) * norm;
        pFilter->a1 = 2.0f * (v * k * k - 1.0f) * norm;
        pFilter->a2 = (1.0f - sqrtf(2.0f * v) * k + v * k * k) * norm;
        pFilter->b1 = 2.0f * (k * k - 1.0f) * norm;
        pFilter->b2 = (1.0f - sqrt2 * k + k * k) * norm;
    } else {
        // cut
        norm = 1.0f / (1.0f + sqrtf(2.0f * v) * k + v * k * k);
        pFilter->a0 = (1.0f + sqrt2 * k + k * k) * norm;
        pFilter->a1 = 2.0f * (k * k - 1.0f) * norm;
        pFilter->a2 = (1.0f - sqrt2 * k + k * k) * norm;
        pFilter->b1 = 2.0f * (v * k * k - 1.0f) * norm;
        pFilter->b2 = (1.0f - sqrtf(2.0f * v) * k + v * k * k) * norm;
    }
}

static void ma_filter_highshelf_calculate_biquad_coefficients(ma_biquad_filter *pFilter) {
    if(pFilter == NULL) 
        return;
    const double sqrt2 = 1.4142135623730951;
    float k = tanf(MA_PI * pFilter->frequency / pFilter->sampleRate);
    float v = powf(10.0f, ma_abs(pFilter->gainDB) / 20.0f);
    float norm = 0.0f;
    if (pFilter->gainDB >= 0) {
        // boost
        norm = 1.0f / (1.0f + sqrt2 * k + k * k);
        pFilter->a0 = (v + sqrtf(2.0f * v) * k + k * k) * norm;
        pFilter->a1 = 2.0f * (k * k - v) * norm;
        pFilter->a2 = (v - sqrtf(2.0f * v) * k + k * k) * norm;
        pFilter->b1 = 2.0f * (k * k - 1.0f) * norm;
        pFilter->b2 = (1.0f - sqrt2 * k + k * k) * norm;
    } else {
        // cut
        norm = 1.0f / (v + sqrtf(2.0f * v) * k + k * k);
        pFilter->a0 = (1.0f + sqrt2 * k + k * k) * norm;
        pFilter->a1 = 2.0f * (k * k - 1.0f) * norm;
        pFilter->a2 = (1.0f - sqrt2 * k + k * k) * norm;
        pFilter->b1 = 2.0f * (k * k - v) * norm;
        pFilter->b2 = (v - sqrtf(2.0f * v) * k + k * k) * norm;
    }
}

static void ma_filter_peak_calculate_biquad_coefficients(ma_biquad_filter *pFilter) {
    if(pFilter == NULL) 
        return;
    float v = powf(10, ma_abs(pFilter->gainDB) / 20.0f);
    float k = tanf(MA_PI * pFilter->frequency / pFilter->sampleRate);
    float q = pFilter->q;
    float norm = 0;

    if (pFilter->gainDB >= 0.0f) {
        //boost 
        norm = 1.0f / (1.0f + 1.0f / q * k + k * k);
        pFilter->a0 = (1.0f + v / q * k + k * k) * norm;
        pFilter->a1 = 2.0f * (k * k - 1.0f) * norm;
        pFilter->a2 = (1.0f - v / q * k + k * k) * norm;
        pFilter->b1 = pFilter->a1;
        pFilter->b2 = (1.0f - 1.0f / q * k + k * k) * norm;
    }  else {
        //cut
        norm = 1.0f / (1.0f + v / q * k + k * k);
        pFilter->a0 = (1.0f + 1.0f / q * k + k * k) * norm;
        pFilter->a1 = 2.0f * (k * k - 1.0f) * norm;
        pFilter->a2 = (1.0f - 1.0f / q * k + k * k) * norm;
        pFilter->b1 = pFilter->a1;
        pFilter->b2 = (1.0f - v / q * k + k * k) * norm;
    }
}

static void ma_filter_notch_calculate_biquad_coefficients(ma_biquad_filter *pFilter) {
    if(pFilter == NULL) 
        return;
    float k = tanf(MA_PI * pFilter->frequency / pFilter->sampleRate);
    float norm = 1.0f / (1.0f + k / pFilter->q + k * k);
    pFilter->a0 = (1.0f + k * k) * norm;
    pFilter->a1 = 2.0f * (k * k - 1.0f) * norm;
    pFilter->a2 = pFilter->a0;
    pFilter->b1 = pFilter->a1;
    pFilter->b2 = (1.0f - k / pFilter->q + k * k) * norm;
}

static ma_filter_vtable g_ma_filter_lowpass_vtable = {
    ma_filter_lowpass_calculate_biquad_coefficients
};

static ma_filter_vtable g_ma_filter_highpass_vtable = {
    ma_filter_highpass_calculate_biquad_coefficients
};

static ma_filter_vtable g_ma_filter_bandpass_vtable = {
    ma_filter_bandpass_calculate_biquad_coefficients
};

static ma_filter_vtable g_ma_filter_lowshelf_vtable = {
    ma_filter_lowshelf_calculate_biquad_coefficients
};

static ma_filter_vtable g_ma_filter_highshelf_vtable = {
    ma_filter_highshelf_calculate_biquad_coefficients
};

static ma_filter_vtable g_ma_filter_peak_vtable = {
    ma_filter_peak_calculate_biquad_coefficients
};

static ma_filter_vtable g_ma_filter_notch_vtable = {
    ma_filter_notch_calculate_biquad_coefficients
};

MA_API ma_filter_config ma_filter_config_init(ma_filter_type type, ma_int32 sampleRate, float frequency, float q, float gainDB) {
    MA_ASSERT(sampleRate > 0);
    MA_ASSERT(frequency > 0.0f);
    MA_ASSERT(sampleRate >= (frequency * 2));
    MA_ASSERT(q > 0.0f);

    ma_filter_config config;

    config.type = type;
    config.sampleRate = sampleRate;
    config.frequency = frequency;
    config.q = q;
    config.gainDB = gainDB;

    return config;
}

MA_API ma_result ma_filter_init(const ma_filter_config *pConfig, ma_biquad_filter *pFilter) {
    MA_ASSERT(pConfig != NULL);
    MA_ASSERT(pFilter != NULL);

    MA_ZERO_OBJECT(pFilter);

    pFilter->type = pConfig->type;
    pFilter->sampleRate = pConfig->sampleRate;
    pFilter->frequency = pConfig->frequency;
    pFilter->q = pConfig->q;
    pFilter->gainDB = pConfig->gainDB > 0.0f ? pConfig->gainDB : 6.0f;

    switch(pFilter->type) {
        case ma_filter_type_lowpass:
            pFilter->vtable = g_ma_filter_lowpass_vtable;
            break;
        case ma_filter_type_highpass:
            pFilter->vtable = g_ma_filter_highpass_vtable;
            break;
        case ma_filter_type_bandpass:
            pFilter->vtable = g_ma_filter_bandpass_vtable;
            break;
        case ma_filter_type_lowshelf:
            pFilter->vtable = g_ma_filter_lowshelf_vtable;
            break;
        case ma_filter_type_highshelf:
            pFilter->vtable = g_ma_filter_highshelf_vtable;
            break;
        case ma_filter_type_peak:
            pFilter->vtable = g_ma_filter_peak_vtable;
            break;
        case ma_filter_type_notch:
            pFilter->vtable = g_ma_filter_notch_vtable;
            break;
    }

    pFilter->vtable.onCalculateBiQuadCoefficients(pFilter);

    return MA_SUCCESS;
}

MA_API void ma_filter_uninit(ma_biquad_filter *pFilter) {
    if(pFilter == NULL)
        return;
    MA_ZERO_OBJECT(pFilter);
}

MA_API void ma_filter_set_type(ma_biquad_filter *pFilter, ma_filter_type type) {
    if(pFilter == NULL)
        return;
    
    pFilter->type = type;
    
    switch(pFilter->type) {
        case ma_filter_type_lowpass:
            pFilter->vtable = g_ma_filter_lowpass_vtable;
            break;
        case ma_filter_type_highpass:
            pFilter->vtable = g_ma_filter_highpass_vtable;
            break;
        case ma_filter_type_bandpass:
            pFilter->vtable = g_ma_filter_bandpass_vtable;
            break;
        case ma_filter_type_lowshelf:
            pFilter->vtable = g_ma_filter_lowshelf_vtable;
            break;
        case ma_filter_type_highshelf:
            pFilter->vtable = g_ma_filter_highshelf_vtable;
            break;
        case ma_filter_type_peak:
            pFilter->vtable = g_ma_filter_peak_vtable;
            break;
        case ma_filter_type_notch:
            pFilter->vtable = g_ma_filter_notch_vtable;
            break;
    }

    pFilter->vtable.onCalculateBiQuadCoefficients(pFilter);
}

MA_API ma_filter_type ma_filter_get_type(const ma_biquad_filter *pFilter) {
    if(pFilter == NULL)
        return ma_filter_type_lowpass;
    return pFilter->type;
}

MA_API void ma_filter_set_frequency(ma_biquad_filter *pFilter, float frequency) {
    if(pFilter == NULL)
        return;
    if(pFilter->vtable.onCalculateBiQuadCoefficients == NULL)
        return;
    if (pFilter->sampleRate < (frequency * 2)) {
        fprintf(stderr, "The samplerate has to be bigger than 2 * frequency\n");
        return;
    }
    pFilter->frequency = frequency;
    pFilter->vtable.onCalculateBiQuadCoefficients(pFilter);
}

MA_API float ma_filter_get_frequency(const ma_biquad_filter *pFilter) {
    if(pFilter == NULL)
        return 0.0f;
    return pFilter->frequency;
}

MA_API void ma_filter_set_q(ma_biquad_filter *pFilter, float q) {
    if(pFilter == NULL)
        return;
    if(pFilter->vtable.onCalculateBiQuadCoefficients == NULL)
        return;
    if (q <= 0) {
        fprintf(stderr, "q can not be less than 0");
        return;
    }
    pFilter->q = q;
    pFilter->vtable.onCalculateBiQuadCoefficients(pFilter);
}

MA_API float ma_filter_get_q(const ma_biquad_filter *pFilter) {
    if(pFilter == NULL)
        return 0.0f;
    return pFilter->q;
}

MA_API void ma_filter_set_gain_db(ma_biquad_filter *pFilter, float gainDB) {
    if(pFilter == NULL)
        return;
    if(pFilter->vtable.onCalculateBiQuadCoefficients == NULL)
        return;
    pFilter->gainDB = gainDB;
    pFilter->vtable.onCalculateBiQuadCoefficients(pFilter);
}

MA_API float ma_filter_get_gain_db(const ma_biquad_filter *pFilter) {
    if(pFilter == NULL)
        return 0.0f;
    return pFilter->gainDB;
}

MA_API float ma_filter_process_sample(ma_biquad_filter *pFilter, float input) {
    if(pFilter == NULL)
        return 0.0f;
    float o = input * pFilter->a0 + pFilter->z1;
    pFilter->z1 = input * pFilter->a1 + pFilter->z2 - pFilter->b1 * o;
    pFilter->z2 = input * pFilter->a2 - pFilter->b2 * o;
    return o;
}

MA_API void ma_filter_process_samples(ma_biquad_filter *pFilter, const float *pInput, ma_uint64 inputSamples, float *pOutput, ma_uint64 outputSamples) {
    if(pFilter == NULL)
        return;

    float o = 0.0f;

    for(ma_uint64 i = 0; i < inputSamples; i++) {
        o = pInput[i] * pFilter->a0 + pFilter->z1;
        pFilter->z1 = pInput[i] * pFilter->a1 + pFilter->z2 - pFilter->b1 * o;
        pFilter->z2 = pInput[i] * pFilter->a2 - pFilter->b2 * o;
        if(i < outputSamples) {
            pOutput[i] = o;
        }
    }
}