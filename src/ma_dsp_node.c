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

#include "ma_dsp_node.h"
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

static void dsp_node_process_pcm_frames(ma_node* pNode, const float** ppFramesIn, ma_uint32* pFrameCountIn, float** ppFramesOut, ma_uint32* pFrameCountOut) {
    // Do some processing of ppFramesIn (one stream of audio data per input bus)
    // const float* pFramesIn_0 = ppFramesIn[0]; // Input bus @ index 0.
    // const float* pFramesIn_1 = ppFramesIn[1]; // Input bus @ index 1.
    // float* pFramesOut_0 = ppFramesOut[0];     // Output bus @ index 0.

    ma_dsp_node *node =(ma_dsp_node*)pNode;

    if(pNode != NULL) {
        if(node->config.pCallback != NULL) {
            node->config.pCallback(node->config.pUserData, pNode, ppFramesIn, pFrameCountIn, ppFramesOut, pFrameCountOut);
        }
    }

    // Do some processing. On input, pFrameCountIn will be the number of input frames in each
    // buffer in ppFramesIn and pFrameCountOut will be the capacity of each of the buffers
    // in ppFramesOut. On output, pFrameCountIn should be set to the number of input frames
    // your node consumed and pFrameCountOut should be set the number of output frames that
    // were produced.
    //
    // You should process as many frames as you can. If your effect consumes input frames at the
    // same rate as output frames (always the case, unless you're doing resampling), you need
    // only look at ppFramesOut and process that exact number of frames. If you're doing
    // resampling, you'll need to be sure to set both pFrameCountIn and pFrameCountOut
    // properly.
}

static ma_node_vtable g_ma_dsp_node_vtable = {
    dsp_node_process_pcm_frames,
    NULL,
    1,  /* 1 input channel. */
    1,  /* 1 output channel. */
    MA_NODE_FLAG_CONTINUOUS_PROCESSING  /* Reverb requires continuous processing to ensure the tail get's processed. */
};

MA_API ma_dsp_node_config ma_dsp_node_config_init(ma_uint32 channels, ma_uint32 sampleRate, ma_dsp_node_proc pCallback, void *pUserData) {
    MA_ASSERT(pCallback != NULL);
    
    ma_dsp_node_config config;

    MA_ZERO_OBJECT(&config);
    config.nodeConfig = ma_node_config_init();  /* Input and output channels will be set in ma_reverb_node_init(). */
    config.channels   = channels;
    config.sampleRate = sampleRate;
    config.pCallback  = pCallback;
    config.pUserData  = pUserData;

    return config;
}

MA_API ma_result ma_dsp_node_init(ma_node_graph* pNodeGraph, const ma_dsp_node_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, ma_dsp_node* pDspNode) {
    ma_result result;
    ma_node_config baseConfig;

    if (pDspNode == NULL) {
        return MA_INVALID_ARGS;
    }

    MA_ZERO_OBJECT(pDspNode);

    if (pConfig == NULL) {
        return MA_INVALID_ARGS;
    }

    baseConfig = pConfig->nodeConfig;
    baseConfig.vtable          = &g_ma_dsp_node_vtable;
    baseConfig.pInputChannels  = &pConfig->channels;
    baseConfig.pOutputChannels = &pConfig->channels;

    result = ma_node_init(pNodeGraph, &baseConfig, pAllocationCallbacks, &pDspNode->baseNode);
    if (result != MA_SUCCESS) {
        return result;
    }

    pDspNode->config = *pConfig;

    return MA_SUCCESS;
}

MA_API void ma_dsp_node_uninit(ma_dsp_node_config* pDspNode, const ma_allocation_callbacks* pAllocationCallbacks) {
    if(pDspNode != NULL)
        ma_node_uninit(pDspNode, pAllocationCallbacks);
}