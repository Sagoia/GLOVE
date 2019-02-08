/**
 * Copyright (C) 2015-2018 Think Silicon S.A. (https://think-silicon.com/)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public v3
 * License as published by the Free Software Foundation;
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 */

/**
 *  @file       cacheManager.h
 *  @author     Think Silicon
 *  @date       13/09/2018
 *  @version    1.0
 *
 *  @brief      Vulkan objects cache manager. These caches are needed to keep in memory Vulkan objects referred to by secondary command buffers.
 *
 */
#ifndef __CACHEMANAGER_H__
#define __CACHEMANAGER_H__

#include <vector>
#include <unordered_map>
#include "utils/arrays.hpp"
#include "vulkan/vulkan.h"
#include "utils/glLogger.h"

namespace vulkanAPI {
    struct vkContext_t;
}

class UniformBufferObject;
class BufferObject;
class Texture;

class CacheManager {
private:
    const static uint32_t DEFAULT_CACHE_SIZE = 256;
    const static uint32_t UBO_ARRAY_COUNT = 16;

    const
    vulkanAPI::vkContext_t *            mVkContext;

    typedef Array<UniformBufferObject*, DEFAULT_CACHE_SIZE> UBOList;
    UBOList                             mUBOCache;
    UBOList                             mUBOLists[UBO_ARRAY_COUNT];

    std::vector<BufferObject *>         mVBOCache;
    std::vector<Texture *>              mTextureCache;
    std::vector<VkImageView>            mVkImageViewCache;
    std::vector<VkImage>                mVkImageCache;
    std::vector<VkBuffer>               mVkBufferCache;
    std::vector<VkDeviceMemory>         mVkDeviceMemoryCache;

    typedef std::unordered_map<uint64_t, VkSampler> SamplerMap;
    SamplerMap                          mVkSamplerCache;
    typedef std::unordered_map<uint64_t, VkRenderPass> RenderPassMap;
    RenderPassMap                       mVkRenderPassCache;

    typedef std::unordered_map<uint64_t, VkPipeline> PipelineHashMap;
    typedef std::unordered_map<VkPipelineCache, PipelineHashMap> PipelineMap;
    PipelineMap                         mVkPipelineCache;

    void                                UncacheUBOs();
    void                                CleanUpUBOs();

    void                                CleanUpVBOCache();
    void                                CleanUpTextureCache();
    void                                CleanUpImageViewCache();
    void                                CleanUpImageCache();
    void                                CleanUpBufferCache();
    void                                CleanUpDeviceMemoryCache();

    void                                CleanUpSampleCache();
    void                                CleanUpRenderPassCache();
    void                                CleanUpPipelineCache();

public:
     CacheManager(const vulkanAPI::vkContext_t *vkContext);
    ~CacheManager();

    void                                CacheUBO(UniformBufferObject *uniformBufferObject);
    UniformBufferObject *               GetUBO(uint32_t index);

    void                                CacheVBO(BufferObject *vbo);
    void                                CacheTexture(Texture *tex);
    void                                CacheVkImageView(VkImageView imageView);
    void                                CacheVkImage(VkImage image);
    void                                CacheVkBuffer(VkBuffer buffer);
    void                                CacheDeviceMemory(VkDeviceMemory deviceMemory);

    void                                CacheSampler(uint64_t hash, VkSampler sampler);
    VkSampler                           GetSampler(uint64_t hash);

    void                                CacheRenderPass(uint64_t hash, VkRenderPass renderPass);
    VkRenderPass                        GetRenderPass(uint64_t hash);

    void                                CachePipeline(VkPipelineCache pipelineCache, uint64_t hash, VkPipeline pipeline);
    VkPipeline                          GetPipeline(VkPipelineCache pipelineCache, uint64_t hash);

    void                                CleanUpFrameCaches();
    void                                CleanUpCaches();
};

#endif //__CACHEMANAGER_H__
