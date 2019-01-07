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
#include <map>
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
    const
    vulkanAPI::vkContext_t *            mVkContext;

    std::vector<UniformBufferObject *>  mUBOCache;
    std::vector<BufferObject *>         mVBOCache;
    std::vector<Texture *>              mTextureCache;
    std::vector<VkPipeline>             mVkPipelineObjectCache;
    std::vector<VkImageView>            mVkImageViewCache;
    std::vector<VkImage>                mVkImageCache;
    std::vector<VkDeviceMemory>         mVkDeviceMemoryCache;

    std::map<uint64_t, VkSampler>       mVkSamplerCache;

    void                                CleanUpUBOCache();
    void                                CleanUpVBOCache();
    void                                CleanUpTextureCache();
    void                                CleanUpVkPipelineObjectCache();
    void                                CleanUpImageViewCache();
    void                                CleanUpImageCache();
    void                                CleanUpDeviceMemoryCache();

public:
     CacheManager(const vulkanAPI::vkContext_t *vkContext) : mVkContext(vkContext) { }
    ~CacheManager() { }

    void                                CacheUBO(UniformBufferObject *uniformBufferObject);
    void                                CacheVBO(BufferObject *vbo);
    void                                CacheTexture(Texture *tex);
    void                                CacheVkPipelineObject(VkPipeline pipeline);
    void                                CacheVkImageView(VkImageView imageView);
    void                                CacheVkImage(VkImage image);
    void                                CacheDeviceMemory(VkDeviceMemory deviceMemory);

    void                                CacheSampler(uint64_t hash, VkSampler sampler);
    VkSampler                           GetSampler(uint64_t hash);

    void                                CleanUpCaches();
};

#endif //__CACHEMANAGER_H__
