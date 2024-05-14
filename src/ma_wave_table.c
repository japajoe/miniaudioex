#include "ma_wave_table.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#define MA_ASSERT(condition) assert(condition)

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

static MA_INLINE int ma_sign(float num) {
    if (num > 0)
        return 1;
    else if (num < 0)
        return -1;
    else
        return 0;
}

static float ma_wave_table_get_saw_sample(float phase) {
    return 2 * (phase - 0.5f);
}

static float ma_wave_table_get_sine_sample(float phase) {
    return sinf(2 * MA_PI * phase);
}

static float ma_wave_table_get_square_sample(float phase) {
    return (float)ma_sign(sinf(2 * MA_PI * phase));
}

static float ma_wave_table_get_triangle_sample(float phase) {
    return 2 * ma_abs(2 * (phase - 0.5)) - 1;
}

MA_API ma_wave_table_config ma_wave_table_config_init(ma_wave_table_type type, float *pData, ma_uint64 dataSampleCount) {
    ma_wave_table_config config;
    MA_ZERO_OBJECT(&config);
    MA_ASSERT(dataSampleCount > 0);

    if(type == ma_wave_table_type_custom) {
        MA_ASSERT(pData != NULL);
        config.pData = pData;
    } else {
        config.pData = NULL;
    }

    config.dataSampleCount = dataSampleCount;
    config.type = type;
    return config;
}

MA_API ma_result ma_wave_table_init(const ma_wave_table_config* pConfig, ma_wave_table* pWavetable) {
    if(pConfig == NULL) {
        fprintf(stderr, "pConfig can not be NULL\n");
        return MA_INVALID_ARGS;
    }
    if(pWavetable == NULL) {
        fprintf(stderr, "pWavetable can not be NULL\n");
        return MA_INVALID_ARGS;
    }
    if(pConfig->dataSampleCount == 0) {
        fprintf(stderr, "dataSampleCount can not be 0\n");
        return MA_INVALID_ARGS;
    }

    MA_ZERO_OBJECT(pWavetable);

    if(pConfig->type == ma_wave_table_type_custom) {
        if(pConfig->pData == NULL) {
            fprintf(stderr, "pData can not be NULL\n");
            return MA_INVALID_ARGS;
        }
        pWavetable->pData = pConfig->pData;
    } else {
        pWavetable->pData = NULL;
    }
    
    pWavetable->type = pConfig->type;
    pWavetable->dataSampleCount = pConfig->dataSampleCount;
    pWavetable->dataSize = pWavetable->dataSampleCount * sizeof(ma_float);
    pWavetable->dataIndex = 0;

    if(pWavetable->type != ma_wave_table_type_custom) {
        pWavetable->pData = (ma_float*)MA_MALLOC(pWavetable->dataSize);

        if(pWavetable->pData == NULL) {
            fprintf(stderr, "Failed to allocate memory for wave table\n");
            return MA_ERROR;
        }

        float (*wave_fn)(float);
        wave_fn = ma_wave_table_get_sine_sample;

        switch(pWavetable->type) {
            case ma_wave_table_type_sine:
                wave_fn = ma_wave_table_get_sine_sample;
                break;
            case ma_wave_table_type_saw:
                wave_fn = ma_wave_table_get_saw_sample;
                break;
            case ma_wave_table_type_square:
                wave_fn = ma_wave_table_get_square_sample;
                break;
            case ma_wave_table_type_triangle:
                wave_fn = ma_wave_table_get_triangle_sample;
                break;
            default:
                break;
        }

        for(ma_uint32 i = 0; i < pWavetable->dataSampleCount; i++) {
            pWavetable->pData[i] = wave_fn((float)i / pWavetable->dataSampleCount);
        }
    }

    return MA_SUCCESS;
}

MA_API void ma_wave_table_uninit(ma_wave_table* pWavetable) {
    if(pWavetable != NULL) {
        //Only frees memory if memory was not allocated by the user
        if(pWavetable->pData != NULL && pWavetable->dataSize > 0 && pWavetable->type != ma_wave_table_type_custom) {
            MA_FREE(pWavetable->pData);
        }
        MA_ZERO_OBJECT(pWavetable);
    }
}

static MA_INLINE float ma_wave_table_interpolate_sample(float value1, float value2, float t)
{
    return value1 + (value2 - value1) * t;
}

MA_API float ma_wave_table_get_sample(ma_wave_table* pWavetable, ma_uint64 time, float frequency, float sampleRate) {
    if(pWavetable == NULL)
        return 0.0f;
    if(pWavetable->pData == NULL)
        return 0.0f;
    float phase = time * frequency / sampleRate;
    int length = pWavetable->dataSampleCount;
    int index = (int)(phase * length);
    pWavetable->dataIndex = index;
    float t = phase * length - index;
    float value1 = pWavetable->pData[index % length];
    float value2 = pWavetable->pData[(index + 1) % length];
    return ma_wave_table_interpolate_sample(value1, value2, t);
}