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

// based on http://www.earlevel.com/main/2011/01/02/biquad-formulas/

#include "ma_oscillator.h"
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

#ifndef MA_TAU
#define MA_TAU (3.14159265359 * 2)
#endif

#define ma_abs(x)                       (((x) > 0) ? (x) : -(x))

static MA_INLINE int ma_sign(float num) {
    if (num > 0)
        return 1;
    else if (num < 0)
        return -1;
    else
        return 0;
}

static float ma_oscillator_get_saw_sample(float phase) {
    phase = phase / MA_TAU;
    return 2.0f * phase - 1.0f;
}

static float ma_oscillator_get_sine_sample(float phase) {
    return sinf(phase);
}

static float ma_oscillator_get_square_sample(float phase) {
    return ma_sign(sinf(phase));
}

static float ma_oscillator_get_triangle_sample(float phase) {
    phase = phase / MA_TAU;
    return (2.0 * ma_abs(2 * (phase - 0.5f)) - 1.0f);
}

MA_API ma_oscillator_config ma_oscillator_config_init(ma_waveform_type type, float frequency, float amplitude, float sampleRate) {
    ma_oscillator_config config;

    MA_ZERO_OBJECT(&config);
    MA_ASSERT(frequency > 0);
    MA_ASSERT(sampleRate > 0);
    MA_ASSERT((int)type >= 0 && (int)type <= (int)ma_waveform_type_sawtooth);

    config.type = type;
    config.frequency = frequency;
    config.amplitude = amplitude;
    config.sampleRate = sampleRate;

    return config;
}

MA_API ma_result ma_oscillator_init(const ma_oscillator_config *pConfig, ma_oscillator *pOscillator) {
    if(pConfig == NULL) {
        fprintf(stderr, "pConfig can not be NULL\n");
        return MA_INVALID_ARGS;
    }

    if(pOscillator == NULL) {
        fprintf(stderr, "pOscillator can not be NULL\n");
        return MA_INVALID_ARGS;
    }

    MA_ZERO_OBJECT(pOscillator);

    pOscillator->type = pConfig->type;
    pOscillator->amplitude = pConfig->amplitude;
    pOscillator->frequency = pConfig->frequency;
    pOscillator->sampleRate = pConfig->sampleRate;
    pOscillator->phase = 0.0f;
    pOscillator->phaseIncrement = MA_TAU * pOscillator->frequency / pOscillator->sampleRate;
    pOscillator->pWaveFunction = &ma_oscillator_get_sine_sample;

    switch(pOscillator->type)
    {
        case ma_waveform_type_sawtooth:
            pOscillator->pWaveFunction = &ma_oscillator_get_saw_sample;
            break;
        case ma_waveform_type_sine:
            pOscillator->pWaveFunction = &ma_oscillator_get_sine_sample;
            break;
        case ma_waveform_type_square:
            pOscillator->pWaveFunction = &ma_oscillator_get_square_sample;
            break;
        case ma_waveform_type_triangle:
            pOscillator->pWaveFunction = &ma_oscillator_get_triangle_sample;
            break;
    }

    return MA_SUCCESS;
}

MA_API void ma_oscillator_set_type(ma_oscillator *pOscillator, ma_waveform_type type) {
    if(pOscillator == NULL)
        return;
    pOscillator->type = type;

    switch(pOscillator->type)
    {
        case ma_waveform_type_sawtooth:
            pOscillator->pWaveFunction = &ma_oscillator_get_saw_sample;
            break;
        case ma_waveform_type_sine:
            pOscillator->pWaveFunction = &ma_oscillator_get_sine_sample;
            break;
        case ma_waveform_type_square:
            pOscillator->pWaveFunction = &ma_oscillator_get_square_sample;
            break;
        case ma_waveform_type_triangle:
            pOscillator->pWaveFunction = &ma_oscillator_get_triangle_sample;
            break;
    }
}

MA_API ma_waveform_type ma_oscillator_get_type(const ma_oscillator *pOscillator) {
    if(pOscillator == NULL)
        return ma_waveform_type_sine;
    return pOscillator->type;
}

MA_API void ma_oscillator_set_frequency(ma_oscillator *pOscillator, float frequency) {
    if(pOscillator == NULL)
        return;
    pOscillator->frequency = frequency;
    pOscillator->phaseIncrement = MA_TAU * pOscillator->frequency / pOscillator->sampleRate;
}

MA_API float ma_oscillator_get_frequency(const ma_oscillator *pOscillator) {
    if(pOscillator == NULL)
        return 0.0f;
    return pOscillator->frequency;
}

MA_API void ma_oscillator_set_amplitude(ma_oscillator *pOscillator, float amplitude) {
    if(pOscillator == NULL)
        return;
    pOscillator->amplitude = amplitude;
}

MA_API float ma_oscillator_get_amplitude(const ma_oscillator *pOscillator) {
    if(pOscillator == NULL)
        return 0.0f;
    return pOscillator->amplitude;
}

MA_API void ma_oscillator_set_phase(ma_oscillator *pOscillator, float phase) {
    if(pOscillator == NULL)
        return;
    pOscillator->phase = phase;
}

MA_API float ma_oscillator_get_phase(const ma_oscillator *pOscillator) {
    if(pOscillator == NULL)
        return 0.0f;
    return pOscillator->phase;
}

MA_API float ma_oscillator_get_value(ma_oscillator *pOscillator) {
    if(pOscillator == NULL)
        return 0.0f;

    if(pOscillator->pWaveFunction == NULL)
        return 0.0f;

    float result = pOscillator->pWaveFunction(pOscillator->phase);
    pOscillator->phase += pOscillator->phaseIncrement;
    pOscillator->phase = fmodf(pOscillator->phase, MA_TAU);
    return result;
}

MA_API float ma_oscillator_get_value_at_phase(ma_oscillator *pOscillator, float phase) {
    if(pOscillator == NULL)
        return 0.0f;

    if(pOscillator->pWaveFunction == NULL)
        return 0.0f;

    return pOscillator->pWaveFunction(phase);
}

MA_API float ma_oscillator_get_modulated_value(ma_oscillator *pOscillator, float phase) {
    if(pOscillator == NULL)
        return 0.0f;

    if(pOscillator->pWaveFunction == NULL)
        return 0.0f;

    float result = pOscillator->pWaveFunction(pOscillator->phase + phase);
    pOscillator->phase += pOscillator->phaseIncrement;
    pOscillator->phase = fmodf(pOscillator->phase, MA_TAU);
    return result;
}

MA_API void ma_oscillator_uninit(ma_oscillator *pOscillator) {
    if(pOscillator == NULL)
        return;
    MA_ZERO_OBJECT(pOscillator);
}