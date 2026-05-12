# Changelog
This document briefly describes changes/additions made to the original miniaudio source. In case of a new version of miniaudio, it'll allow me to more quickly backport the changes.

# Additions in miniaudio.h
```c
#define MA_DATA_SOURCE_IS_DECODER                           0x00000002
#define MA_DATA_SOURCE_IS_PROCEDURAL                        0x00000004

typedef void ma_device_resampling;
typedef void ma_device_playback;
typedef void ma_device_capture;

MA_API ma_device_resampling* ma_device_get_resampling(ma_device* pDevice);
MA_API ma_device_playback* ma_device_get_playback(ma_device* pDevice);
MA_API ma_device_capture* ma_device_get_capture(ma_device* pDevice);

typedef void (*ma_procedural_data_source_proc)(void *pUserData, void* pFramesOut, ma_uint64 frameCount, ma_uint32 channels);

typedef struct ma_procedural_data_source_config ma_procedural_data_source_config;

struct ma_procedural_data_source_config {
    ma_format format;
    ma_uint32 channels;
    ma_uint32 sampleRate;
    ma_procedural_data_source_proc callback;
    void *pUserData;
};

typedef struct ma_procedural_data_source ma_procedural_data_source;

struct ma_procedural_data_source {
    ma_data_source_base ds;
    ma_procedural_data_source_config config;
};

typedef void (*ma_effect_node_process_proc)(ma_node* pNode, const float** ppFramesIn, ma_uint32* pFrameCountIn, float** ppFramesOut, ma_uint32* pFrameCountOut);

typedef struct ma_effect_node_config ma_effect_node_config;

struct ma_effect_node_config {
    ma_uint32 sampleRate;
    ma_uint32 channels;
    ma_effect_node_process_proc onProcess;
    void *pUserData;
};

typedef struct ma_effect_node ma_effect_node;

struct ma_effect_node {
    ma_node_base baseNode;
    ma_effect_node_config config;
}

typedef enum {
    ma_allocation_type_async_notification,
    ma_allocation_type_biquad_coefficient,
    ma_allocation_type_biquad,
    ma_allocation_type_bpf,
    ma_allocation_type_bpf2,
    ma_allocation_type_bpf_node,
    ma_allocation_type_channel,
    ma_allocation_type_context,
    ma_allocation_type_data_source,
    ma_allocation_type_data_source_node,
    ma_allocation_type_data_source_vtable,
    ma_allocation_type_decoder,
    ma_allocation_type_decoding_backend_vtable,
    ma_allocation_type_delay,
    ma_allocation_type_delay_node,
    ma_allocation_type_device,
    ma_allocation_type_device_capture,
    ma_allocation_type_device_descriptor,
    ma_allocation_type_device_id,
    ma_allocation_type_device_info,
    ma_allocation_type_device_notification,
    ma_allocation_type_device_playback,
    ma_allocation_type_device_resampling,
    ma_allocation_type_effect_node,
    ma_allocation_type_encoder,
    ma_allocation_type_engine,
    ma_allocation_type_fader,
    ma_allocation_type_fence,
    ma_allocation_type_gainer,
    ma_allocation_type_hishelf2,
    ma_allocation_type_hishelf_node,
    ma_allocation_type_hpf,
    ma_allocation_type_hpf1,
    ma_allocation_type_hpf2,
    ma_allocation_type_hpf_node,
    ma_allocation_type_log,
    ma_allocation_type_loshelf2,
    ma_allocation_type_loshelf_node,
    ma_allocation_type_lpf,
    ma_allocation_type_lpf1,
    ma_allocation_type_lpf2,
    ma_allocation_type_lpf_node,
    ma_allocation_type_node,
    ma_allocation_type_node_base,
    ma_allocation_type_node_graph,
    ma_allocation_type_node_input_bus,
    ma_allocation_type_node_output_bus,
    ma_allocation_type_node_vtable,
    ma_allocation_type_noise,
    ma_allocation_type_notch2,
    ma_allocation_type_notch_node,
    ma_allocation_type_panner,
    ma_allocation_type_peak2,
    ma_allocation_type_peak_node,
    ma_allocation_type_procedural_data_source,
    ma_allocation_type_pulsewave,
    ma_allocation_type_resampling_backend_vtable,
    ma_allocation_type_resource_manager,
    ma_allocation_type_resource_manager_data_source,
    ma_allocation_type_sound,
    ma_allocation_type_sound_inlined,
    ma_allocation_type_sound_group,
    ma_allocation_type_spatializer,
    ma_allocation_type_spatializer_listener,
    ma_allocation_type_splitter_node,
    ma_allocation_type_stack,
    ma_allocation_type_vfs,
    ma_allocation_type_waveform,
} ma_allocation_type;

MA_API ma_result ma_sound_init_from_memory(ma_engine* pEngine, const void* pData, ma_uint64 dataSize, ma_uint32 flags, ma_sound_group* pGroup, ma_fence* pDoneFence, ma_sound* pSound);
MA_API ma_result ma_sound_init_from_callback(ma_engine* pEngine, const ma_procedural_data_source_config* pConfig, ma_uint32 flags, ma_sound_group* pGroup, ma_fence* pDoneFence, ma_sound* pSound);

MA_API ma_procedural_data_source_config ma_procedural_data_source_config_init(ma_format format, ma_uint32 channels, ma_uint32 sampleRate, ma_procedural_data_source_proc pProceduralSoundProc, void *pUserData);
MA_API ma_result ma_procedural_data_source_init(const ma_procedural_data_source_config* pConfig, ma_procedural_data_source* pProceduralSound);
MA_API void ma_procedural_data_source_uninit(ma_procedural_data_source* pProceduralSound);
MA_API ma_result ma_procedural_data_source_read_pcm_frames(ma_procedural_data_source* pProceduralSound, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead);

MA_API ma_effect_node_config ma_effect_node_config_init(ma_uint32 channels, ma_uint32 sampleRate, ma_effect_node_process_proc onProcess, void *pUserData);
MA_API ma_result ma_effect_node_init(ma_node_graph* pNodeGraph, const ma_effect_node_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, ma_effect_node* pEffectNode);
MA_API void ma_effect_node_uninit(ma_effect_node *pEffectNode, const ma_allocation_callbacks* pAllocationCallbacks);

MA_API void* ma_allocate_type(ma_allocation_type type);
MA_API void* ma_allocate(size_t size);
MA_API void ma_deallocate_type(void *pData);
MA_API size_t ma_get_size_of_type(ma_allocation_type type);
```

# Additions in miniaudio.c
```c
#define MINIAUDIO_IMPLEMENTATION                                                                                                                                                                     
#define MA_DLL

MA_API ma_device_resampling* ma_device_get_resampling(ma_device* pDevice) {
    if (pDevice == NULL) {
        return NULL;
    }
    return (ma_device_resampling*)&pDevice->resampling;
}

MA_API ma_device_playback* ma_device_get_playback(ma_device* pDevice) {
    if (pDevice == NULL) {
        return NULL;
    }
    return (ma_device_playback*)&pDevice->playback;
}

MA_API ma_device_capture* ma_device_get_capture(ma_device* pDevice) {
    if (pDevice == NULL) {
        return NULL;
    }
    return (ma_device_capture*)&pDevice->capture;
}

static ma_result ma_decoder_seek_bytes(ma_decoder* pDecoder, ma_int64 byteOffset, ma_seek_origin origin)
{
    MA_ASSERT(pDecoder != NULL);

    if (pDecoder->onSeek == NULL) { // Only this check has been added
        return MA_NOT_IMPLEMENTED;
    }

    return pDecoder->onSeek(pDecoder, byteOffset, origin);
}

MA_API ma_result ma_sound_init_from_memory(ma_engine* pEngine, const void* pData, ma_uint64 dataSize, ma_uint32 flags, ma_sound_group* pGroup, ma_fence* pDoneFence, ma_sound* pSound)
{
    ma_sound_config config;

    if (pSound == NULL || pData == NULL || dataSize == 0) {
        return MA_INVALID_ARGS;
    }

    config = ma_sound_config_init_2(pEngine);
    config.pDataSource        = (ma_data_source*)malloc(sizeof(ma_decoder));
    config.flags              = flags;
    config.pInitialAttachment = pGroup;
    config.pDoneFence         = pDoneFence;

    if (config.pDataSource == NULL) {
        return MA_ERROR;
    }

    ma_decoder_config decoderConfig = ma_decoder_config_init_default();

    decoderConfig.ppCustomBackendVTables = pEngine->pResourceManager->config.ppCustomDecodingBackendVTables;
    decoderConfig.customBackendCount = pEngine->pResourceManager->config.customDecodingBackendCount;
    decoderConfig.pCustomBackendUserData = pEngine->pResourceManager->config.pCustomDecodingBackendUserData;

    ma_result result = ma_decoder_init_memory(pData, dataSize, &decoderConfig, config.pDataSource);

    if(result != MA_SUCCESS) {
        free(config.pDataSource);
        return result;
    }

    result = ma_sound_init_ex(pEngine, &config, pSound);

    if(result != MA_SUCCESS) {
        free(config.pDataSource);
        return result;
    }

    return result;
}

MA_API ma_result ma_sound_init_from_callback(ma_engine* pEngine, const ma_procedural_data_source_config* pConfig, ma_uint32 flags, ma_sound_group* pGroup, ma_fence* pDoneFence, ma_sound* pSound)
{
    ma_sound_config config;

    if (pSound == NULL) {
        return MA_INVALID_ARGS;
    }

    config = ma_sound_config_init_2(pEngine);
    config.pDataSource        = (ma_data_source*)malloc(sizeof(ma_procedural_data_source));
    config.flags              = flags;
    config.pInitialAttachment = pGroup;
    config.pDoneFence         = pDoneFence;

    if (config.pDataSource == NULL) {
        return MA_ERROR;
    }

    ma_result result = ma_procedural_data_source_init(pConfig, config.pDataSource);

    if(result != MA_SUCCESS) {
        free(config.pDataSource);
        return MA_ERROR;
    }

    return ma_sound_init_ex(pEngine, &config, pSound);
}

MA_API void ma_sound_uninit(ma_sound* pSound)
{
    if (pSound == NULL) {
        return;
    }

    /*
    Always uninitialize the node first. This ensures it's detached from the graph and does not return until it has done
    so which makes thread safety beyond this point trivial.
    */
    ma_engine_node_uninit(&pSound->engineNode, &pSound->engineNode.pEngine->allocationCallbacks);

    if (pSound->pProcessingCache != NULL) {
        ma_free(pSound->pProcessingCache, &pSound->engineNode.pEngine->allocationCallbacks);
        pSound->pProcessingCache = NULL;
    }

    /* Once the sound is detached from the group we can guarantee that it won't be referenced by the mixer thread which means it's safe for us to destroy the data source. */
#ifndef MA_NO_RESOURCE_MANAGER
    if (pSound->ownsDataSource) {
        ma_resource_manager_data_source_uninit(pSound->pResourceManagerDataSource);
        ma_free(pSound->pResourceManagerDataSource, &pSound->engineNode.pEngine->allocationCallbacks);
        pSound->pDataSource = NULL;
    } else {
        if (pSound->pDataSource != NULL) {
            // To do: more robust handling of the custom flags without breaking ABI
            ma_data_source_base* pDataSourceBase = (ma_data_source_base*)pSound->pDataSource;
            if ((pDataSourceBase->vtable->flags & MA_DATA_SOURCE_IS_PROCEDURAL) != 0) {
                ma_procedural_data_source_uninit((ma_procedural_data_source*)pSound->pDataSource);
                free(pSound->pDataSource);
            } else {
                ma_decoder_uninit((ma_decoder*)pSound->pDataSource);
                free(pSound->pDataSource);
            }
            pSound->pDataSource = NULL;
        }
    }
#else
    MA_ASSERT(pSound->ownsDataSource == MA_FALSE);
#endif
}

static ma_result ma_procedural_data_source__data_source_on_read(ma_data_source* pDataSource, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead) {
    return ma_procedural_data_source_read_pcm_frames((ma_procedural_data_source*)pDataSource, pFramesOut, frameCount, pFramesRead);
}

static ma_result ma_procedural_data_source__data_source_on_seek(ma_data_source* pDataSource, ma_uint64 frameIndex) {
    return MA_NOT_IMPLEMENTED;
}

static ma_result ma_procedural_data_source__data_source_on_get_data_format(ma_data_source* pDataSource, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate, ma_channel* pChannelMap, size_t channelMapCap) {
    ma_procedural_data_source* pProceduralSound = (ma_procedural_data_source*)pDataSource;

    *pFormat     = pProceduralSound->config.format;
    *pChannels   = pProceduralSound->config.channels;
    *pSampleRate = pProceduralSound->config.sampleRate;
    ma_channel_map_init_standard(ma_standard_channel_map_default, pChannelMap, channelMapCap, pProceduralSound->config.channels);

    return MA_SUCCESS;
}

static ma_result ma_procedural_data_source__data_source_on_get_cursor(ma_data_source* pDataSource, ma_uint64* pCursor) {
    *pCursor = 0;
    return MA_NOT_IMPLEMENTED;
}

static ma_data_source_vtable g_ma_procedural_data_source_data_source_vtable = {
    ma_procedural_data_source__data_source_on_read,
    ma_procedural_data_source__data_source_on_seek,
    ma_procedural_data_source__data_source_on_get_data_format,
    ma_procedural_data_source__data_source_on_get_cursor,
    NULL,   /* onGetLength. There's no notion of a length in procedural sounds. */
    NULL,   /* onSetLooping */
    0 | MA_DATA_SOURCE_IS_PROCEDURAL
};

MA_API ma_procedural_data_source_config ma_procedural_data_source_config_init(ma_format format, ma_uint32 channels, ma_uint32 sampleRate, ma_procedural_data_source_proc pProceduralSoundProc, void *pUserData) {
    ma_procedural_data_source_config config;
    MA_ASSERT(pProceduralSoundProc != NULL);

    MA_ZERO_OBJECT(&config);
    config.format           = format;
    config.channels         = channels;
    config.sampleRate       = sampleRate;
    config.callback = pProceduralSoundProc;
    config.pUserData        = pUserData;

    return config;
}

MA_API ma_result ma_procedural_data_source_init(const ma_procedural_data_source_config* pConfig, ma_procedural_data_source* pProceduralSound) {
    ma_result result;
    ma_data_source_config dataSourceConfig;

    if (pProceduralSound == NULL) {
        return MA_INVALID_ARGS;
    }

    MA_ZERO_OBJECT(pProceduralSound);

    dataSourceConfig = ma_data_source_config_init();
    dataSourceConfig.vtable = &g_ma_procedural_data_source_data_source_vtable;

    result = ma_data_source_init(&dataSourceConfig, &pProceduralSound->ds);
    if (result != MA_SUCCESS) {
        return result;
    }

    pProceduralSound->config  = *pConfig;

    return MA_SUCCESS;
}

MA_API void ma_procedural_data_source_uninit(ma_procedural_data_source* pProceduralSound) {
    if (pProceduralSound == NULL) {
        return;
    }

    ma_data_source_uninit(&pProceduralSound->ds);
    MA_ZERO_OBJECT(pProceduralSound);
}

MA_API ma_result ma_procedural_data_source_read_pcm_frames(ma_procedural_data_source* pProceduralSound, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead) {
    if (pFramesRead != NULL) {
        *pFramesRead = 0;
    }

    if (frameCount == 0) {
        return MA_INVALID_ARGS;
    }

    if (pProceduralSound == NULL) {
        return MA_INVALID_ARGS;
    }

    if (pFramesOut != NULL) {
        if(pProceduralSound->config.callback != NULL) {
            pProceduralSound->config.callback(pProceduralSound->config.pUserData, pFramesOut, frameCount, pProceduralSound->config.channels);
        }
    }

    if (pFramesRead != NULL) {
        *pFramesRead = frameCount;
    }

    return MA_SUCCESS;
}

static void ma_effect_node_process_pmc_frames(ma_node* pNode, const float** ppFramesIn, ma_uint32* pFrameCountIn, float** ppFramesOut, ma_uint32* pFrameCountOut)
{
    ma_effect_node *pEffectNode = (ma_effect_node*)pNode;

    if(pEffectNode != NULL) {
        if(pEffectNode->config.onProcess != NULL) {
            pEffectNode->config.onProcess(pNode, ppFramesIn, pFrameCountIn, ppFramesOut, pFrameCountOut);
        }
    }
}

static ma_node_vtable g_ma_effect_node_vtable =
{
    ma_effect_node_process_pmc_frames,
    NULL,
    1,  /* 1 input channels. */
    1,  /* 1 output channel. */
    MA_NODE_FLAG_CONTINUOUS_PROCESSING | MA_NODE_FLAG_ALLOW_NULL_INPUT
};

MA_API ma_effect_node_config ma_effect_node_config_init(ma_uint32 channels, ma_uint32 sampleRate, ma_effect_node_process_proc onProcess, void *pUserData) {
    MA_ASSERT(sampleRate > 0);
    MA_ASSERT(channels > 0);
    MA_ASSERT(onProcess != NULL);

    ma_effect_node_config config = {
        .sampleRate = sampleRate,
        .channels = channels,
        .onProcess = onProcess,
        .pUserData = pUserData
    };

    return config;
}

MA_API ma_result ma_effect_node_init(ma_node_graph* pNodeGraph, const ma_effect_node_config* pConfig, const ma_allocation_callbacks* pAllocationCallbacks, ma_effect_node* pEffectNode) {
    ma_node_config baseConfig;

    if(pNodeGraph == NULL) {
        return MA_INVALID_ARGS;
    }

    if(pConfig == NULL) {
        return MA_INVALID_ARGS;
    }

    if (pEffectNode == NULL) {
        return MA_INVALID_ARGS;
    }

    if (pConfig->onProcess == NULL) {
        return MA_INVALID_ARGS;
    }

    if (pConfig->channels < 1) {
        return MA_INVALID_ARGS;
    }

    MA_ZERO_OBJECT(pEffectNode);

    pEffectNode->config = *pConfig;

    baseConfig = ma_node_config_init();
    baseConfig.vtable          = &g_ma_effect_node_vtable;
    baseConfig.pInputChannels  = &pEffectNode->config.channels;
    baseConfig.pOutputChannels = &pEffectNode->config.channels;

    return ma_node_init(pNodeGraph, &baseConfig, pAllocationCallbacks, &pEffectNode->baseNode);
}

MA_API void ma_effect_node_uninit(ma_effect_node *pEffectNode, const ma_allocation_callbacks* pAllocationCallbacks) {
    if (pEffectNode == NULL) {
        return;
    }

    ma_node_uninit(pEffectNode, pAllocationCallbacks);
    MA_ZERO_OBJECT(pEffectNode);
}


MA_API void* ma_allocate_type(ma_allocation_type type) {
    size_t size = ma_get_size_of_type(type);

    if(size == 0)
        return NULL;

    void *pMemory = malloc(size);

    if(pMemory == NULL)
        return NULL;

    memset(pMemory, 0, size);

    return pMemory;
}

MA_API void* ma_allocate(size_t size) {
    if(size == 0)
        return NULL;

    void *pMemory = malloc(size);

    if(pMemory == NULL)
        return NULL;

    memset(pMemory, 0, size);

    return pMemory;
}

MA_API void ma_deallocate_type(void *pData) {
    if(pData)
        free(pData);
}

MA_API size_t ma_get_size_of_type(ma_allocation_type type) {
    switch(type) {
        case ma_allocation_type_async_notification:
            return sizeof(ma_async_notification);
        case ma_allocation_type_biquad_coefficient:
            return sizeof(ma_biquad_coefficient);
        case ma_allocation_type_biquad:
            return sizeof(ma_biquad);
        case ma_allocation_type_bpf:
            return sizeof(ma_bpf);
        case ma_allocation_type_bpf2:
            return sizeof(ma_bpf2);
        case ma_allocation_type_bpf_node:
            return sizeof(ma_bpf_node);
        case ma_allocation_type_channel:
            return sizeof(ma_channel);
        case ma_allocation_type_context:
            return sizeof(ma_context);
        case ma_allocation_type_data_source:
            return sizeof(ma_data_source);
        case ma_allocation_type_data_source_node:
            return sizeof(ma_data_source_node);
        case ma_allocation_type_data_source_vtable:
            return sizeof(ma_data_source_vtable);
        case ma_allocation_type_decoder:
            return sizeof(ma_decoder);
        case ma_allocation_type_decoding_backend_vtable:
            return sizeof(ma_decoding_backend_vtable);
        case ma_allocation_type_delay:
            return sizeof(ma_delay);
        case ma_allocation_type_delay_node:
            return sizeof(ma_delay_node);
        case ma_allocation_type_device:
            return sizeof(ma_device);
        case ma_allocation_type_device_capture:
            return sizeof(((struct ma_device*)0)->capture);
        case ma_allocation_type_device_descriptor:
            return sizeof(ma_device_descriptor);
        case ma_allocation_type_device_id:
            return sizeof(ma_device_id);
        case ma_allocation_type_device_info:
            return sizeof(ma_device_info);
        case ma_allocation_type_device_notification:
            return sizeof(ma_device_notification);
        case ma_allocation_type_device_playback:
            return sizeof(((struct ma_device*)0)->playback);
        case ma_allocation_type_device_resampling:
            return sizeof(((struct ma_device*)0)->resampling);
        case ma_allocation_type_effect_node:
            return sizeof(ma_effect_node);
        case ma_allocation_type_encoder:
            return sizeof(ma_encoder);
        case ma_allocation_type_engine:
            return sizeof(ma_engine);
        case ma_allocation_type_fader:
            return sizeof(ma_fader);
        case ma_allocation_type_fence:
            return sizeof(ma_fence);
        case ma_allocation_type_gainer:
            return sizeof(ma_gainer);
        case ma_allocation_type_hishelf2:
            return sizeof(ma_hishelf2);
        case ma_allocation_type_hishelf_node:
            return sizeof(ma_hishelf_node);
        case ma_allocation_type_hpf:
            return sizeof(ma_hpf);
        case ma_allocation_type_hpf1:
            return sizeof(ma_hpf1);
        case ma_allocation_type_hpf2:
            return sizeof(ma_hpf2);
        case ma_allocation_type_hpf_node:
            return sizeof(ma_hpf_node);
        case ma_allocation_type_log:
            return sizeof(ma_log);
        case ma_allocation_type_loshelf2:
            return sizeof(ma_loshelf2);
        case ma_allocation_type_loshelf_node:
            return sizeof(ma_loshelf_node);
        case ma_allocation_type_lpf:
            return sizeof(ma_lpf);
        case ma_allocation_type_lpf1:
            return sizeof(ma_lpf1);
        case ma_allocation_type_lpf2:
            return sizeof(ma_lpf2);
        case ma_allocation_type_lpf_node:
            return sizeof(ma_lpf_node);
        case ma_allocation_type_node:
            return sizeof(ma_node);
        case ma_allocation_type_node_base:
            return sizeof(ma_node_base);
        case ma_allocation_type_node_graph:
            return sizeof(ma_node_graph);
        case ma_allocation_type_node_input_bus:
            return sizeof(ma_node_input_bus);
        case ma_allocation_type_node_output_bus:
            return sizeof(ma_node_output_bus);
        case ma_allocation_type_node_vtable:
            return sizeof(ma_node_vtable);
        case ma_allocation_type_noise:
            return sizeof(ma_noise);
        case ma_allocation_type_notch2:
            return sizeof(ma_notch2);
        case ma_allocation_type_notch_node:
            return sizeof(ma_notch_node);
        case ma_allocation_type_panner:
            return sizeof(ma_panner);
        case ma_allocation_type_peak2:
            return sizeof(ma_peak2);
        case ma_allocation_type_peak_node:
            return sizeof(ma_peak_node);
        case ma_allocation_type_procedural_data_source:
            return sizeof(ma_procedural_data_source);
        case ma_allocation_type_pulsewave:
            return sizeof(ma_pulsewave);
        case ma_allocation_type_resampling_backend_vtable:
            return sizeof(ma_resampling_backend_vtable);
        case ma_allocation_type_resource_manager:
            return sizeof(ma_resource_manager);
        case ma_allocation_type_resource_manager_data_source:
            return sizeof(ma_resource_manager_data_source);
        case ma_allocation_type_sound:
            return sizeof(ma_sound);
        case ma_allocation_type_sound_inlined:
            return sizeof(ma_sound_inlined);
        case ma_allocation_type_sound_group:
            return sizeof(ma_sound_group);
        case ma_allocation_type_spatializer:
            return sizeof(ma_spatializer);
        case ma_allocation_type_spatializer_listener:
            return sizeof(ma_spatializer_listener);
        case ma_allocation_type_splitter_node:
            return sizeof(ma_splitter_node);
        case ma_allocation_type_stack:
            return sizeof(ma_stack);
        case ma_allocation_type_vfs:
            return sizeof(ma_vfs);
        case ma_allocation_type_waveform:
            return sizeof(ma_waveform);
        default:
            return 0;
    }
}
```