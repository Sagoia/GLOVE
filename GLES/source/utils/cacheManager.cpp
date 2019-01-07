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
 *  @file       cacheManager.cpp
 *  @author     Think Silicon
 *  @date       13/09/2018
 *  @version    1.0
 *
 *  @brief      Vulkan objects cache manager. These caches are needed to keep in memory Vulkan objects referred to by secondary command buffers.
 *
 */

#include "cacheManager.h"

void
CacheManager::CleanUpUBOCache(void)
{
    FUN_ENTRY(GL_LOG_TRACE);

    if(!mUBOCache.empty()) {
        for(uint32_t i = 0; i < mUBOCache.size(); ++i) {
            if(mUBOCache[i] != nullptr) {
                delete mUBOCache[i];
                mUBOCache[i] = nullptr;
            }
        }

        mUBOCache.clear();
    }
}

void
CacheManager::CleanUpVBOCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if(!mVBOCache.empty()) {
        for(uint32_t i = 0; i < mVBOCache.size(); ++i) {
            if(mVBOCache[i] != nullptr) {
                delete mVBOCache[i];
                mVBOCache[i] = nullptr;
            }
        }

        mVBOCache.clear();
    }
}

void
CacheManager::CleanUpTextureCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if(!mTextureCache.empty()) {
        for(uint32_t i = 0; i < mTextureCache.size(); ++i) {
            if(mTextureCache[i] != nullptr) {
                delete mTextureCache[i];
                mTextureCache[i] = nullptr;
            }
        }

        mTextureCache.clear();
    }
}

void
CacheManager::CleanUpVkPipelineObjectCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if(!mVkPipelineObjectCache.empty()) {
        for(uint32_t i = 0; i < mVkPipelineObjectCache.size(); ++i) {
            if(mVkPipelineObjectCache[i] != VK_NULL_HANDLE){
                vkDestroyPipeline(mVkContext->vkDevice, mVkPipelineObjectCache[i], nullptr);
                mVkPipelineObjectCache[i] = VK_NULL_HANDLE;
            }
        }

        mVkPipelineObjectCache.clear();
    }
}

void
CacheManager::CleanUpImageViewCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if (!mVkImageViewCache.empty()) {
        for (auto imageView : mVkImageViewCache) {
            vkDestroyImageView(mVkContext->vkDevice, imageView, nullptr);
        }
    }

    mVkImageViewCache.clear();
}

void
CacheManager::CleanUpImageCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if (!mVkImageCache.empty()) {
        for (auto image : mVkImageCache) {
            vkDestroyImage(mVkContext->vkDevice, image, nullptr);
        }
    }

    mVkImageCache.clear();
}

void
CacheManager::CleanUpDeviceMemoryCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if (!mVkDeviceMemoryCache.empty()) {
        for (auto deviceMemory : mVkDeviceMemoryCache) {
            vkFreeMemory(mVkContext->vkDevice, deviceMemory, nullptr);
        }
    }

    mVkDeviceMemoryCache.clear();
}

void
CacheManager::CacheUBO(UniformBufferObject *uniformBufferObject)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mUBOCache.push_back(uniformBufferObject);
}

void
CacheManager::CacheVBO(BufferObject *vbo)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mVBOCache.push_back(vbo);
}

void
CacheManager::CacheTexture(Texture *tex)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mTextureCache.push_back(tex);
}

void
CacheManager::CacheVkPipelineObject(VkPipeline pipeline)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mVkPipelineObjectCache.push_back(pipeline);
}

void
CacheManager::CacheVkImageView(VkImageView imageView)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mVkImageViewCache.push_back(imageView);
}

void
CacheManager::CacheVkImage(VkImage image)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mVkImageCache.push_back(image);
}

void
CacheManager::CacheDeviceMemory(VkDeviceMemory deviceMemory)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mVkDeviceMemoryCache.push_back(deviceMemory);
}

void
CacheManager::CleanUpCaches()
{
    FUN_ENTRY(GL_LOG_TRACE);

    CleanUpUBOCache();
    CleanUpVBOCache();
    CleanUpImageViewCache();
    CleanUpImageCache();
    CleanUpDeviceMemoryCache();
    CleanUpTextureCache();
    CleanUpVkPipelineObjectCache();
}
