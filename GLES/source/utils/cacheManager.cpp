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
#include "resources/bufferObject.h"
#include "resources/uniformBufferObject.h"
#include "resources/texture.h"

void
CacheManager::UncacheUBOs()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if (!mUBOCache.empty()) {
        for (auto ubo : mUBOCache) {
            VkDeviceSize size = ubo->GetSize();
            mUBOs[size].push_back(ubo);
        }

        mUBOCache.clear();
    }
}

void
CacheManager::CleanUpUBOs()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if (!mUBOs.empty()) {
        for (auto uboListPair : mUBOs) {
            for (auto ubo : uboListPair.second) {
                if (ubo != nullptr) {
                    delete ubo;
                }
            }
            uboListPair.second.clear();
        }
        mUBOs.clear();
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
CacheManager::CleanUpImageViewCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if (!mVkImageViewCache.empty()) {
        for (auto imageView : mVkImageViewCache) {
            vkDestroyImageView(mVkContext->vkDevice, imageView, nullptr);
        }

        mVkImageViewCache.clear();
    }
}

void
CacheManager::CleanUpImageCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if (!mVkImageCache.empty()) {
        for (auto image : mVkImageCache) {
            vkDestroyImage(mVkContext->vkDevice, image, nullptr);
        }

        mVkImageCache.clear();
    }
}

void
CacheManager::CleanUpBufferCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if (!mVkBufferCache.empty()) {
        for (auto buffer : mVkBufferCache) {
            vkDestroyBuffer(mVkContext->vkDevice, buffer, nullptr);
        }

        mVkBufferCache.clear();
    }
}

void
CacheManager::CleanUpDeviceMemoryCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if (!mVkDeviceMemoryCache.empty()) {
        for (auto deviceMemory : mVkDeviceMemoryCache) {
            vkFreeMemory(mVkContext->vkDevice, deviceMemory, nullptr);
        }

        mVkDeviceMemoryCache.clear();
    }
}

void
CacheManager::CleanUpSampleCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if (!mVkSamplerCache.empty()) {
        for (auto sampler : mVkSamplerCache) {
            vkDestroySampler(mVkContext->vkDevice, sampler.second, nullptr);
        }

        mVkSamplerCache.clear();
    }
}

void
CacheManager::CleanUpRenderPassCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if (!mVkRenderPassCache.empty()) {
        for (auto renderPass : mVkRenderPassCache) {
            vkDestroyRenderPass(mVkContext->vkDevice, renderPass.second, nullptr);
        }

        mVkRenderPassCache.clear();
    }
}

void
CacheManager::CleanUpPipelineCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if (!mVkPipelineCache.empty()) {
        for (auto pipelineMap : mVkPipelineCache) {
            for (auto pipeline : pipelineMap.second) {
                vkDestroyPipeline(mVkContext->vkDevice, pipeline.second, nullptr);
            }
            pipelineMap.second.clear();
        }

        mVkRenderPassCache.clear();
    }
}

void
CacheManager::CacheUBO(UniformBufferObject *uniformBufferObject)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mUBOCache.push_back(uniformBufferObject);
}

UniformBufferObject * 
CacheManager::GetUBO(VkDeviceSize size)
{
    FUN_ENTRY(GL_LOG_TRACE);

    UniformBufferObject *ubo = nullptr;

    UBOMap::iterator it = mUBOs.find(size);
    if (it != mUBOs.end()) {
        if (it->second.size() > 0) {
            ubo = it->second.back();
            it->second.pop_back();
        }
    }

    return ubo;
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
CacheManager::CacheVkBuffer(VkBuffer buffer)
{
    FUN_ENTRY(GL_LOG_TRACE);
    
    mVkBufferCache.push_back(buffer);
}

void
CacheManager::CacheDeviceMemory(VkDeviceMemory deviceMemory)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mVkDeviceMemoryCache.push_back(deviceMemory);
}

void
CacheManager::CacheSampler(uint64_t hash, VkSampler sampler)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mVkSamplerCache[hash] = sampler;
}

VkSampler
CacheManager::GetSampler(uint64_t hash)
{
    FUN_ENTRY(GL_LOG_TRACE);

    std::map<uint64_t, VkSampler>::iterator it = mVkSamplerCache.find(hash);
    if (it != mVkSamplerCache.end()) {
        return it->second;
    }

    return VK_NULL_HANDLE;
}

void
CacheManager::CacheRenderPass(uint64_t hash, VkRenderPass renderPass)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mVkRenderPassCache[hash] = renderPass;
}

VkRenderPass
CacheManager::GetRenderPass(uint64_t hash)
{
    FUN_ENTRY(GL_LOG_TRACE);

    std::map<uint64_t, VkRenderPass>::iterator it = mVkRenderPassCache.find(hash);
    if (it != mVkRenderPassCache.end()) {
        return it->second;
    }

    return VK_NULL_HANDLE;
}

void
CacheManager::CachePipeline(VkPipelineCache pipelineCache, uint64_t hash, VkPipeline pipeline)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mVkPipelineCache[pipelineCache][hash] = pipeline;
}

VkPipeline
CacheManager::GetPipeline(VkPipelineCache pipelineCache, uint64_t hash)
{
    FUN_ENTRY(GL_LOG_TRACE);

    PipelineMap::iterator it = mVkPipelineCache.find(pipelineCache);
    if (it != mVkPipelineCache.end()) {
        PipelineHashMap::iterator it2 = it->second.find(hash);
        if (it2 != it->second.end()) {
            return it2->second;
        }
    }

    return VK_NULL_HANDLE;
}

void
CacheManager::CleanUpFrameCaches()
{
    FUN_ENTRY(GL_LOG_TRACE);

    UncacheUBOs();

    CleanUpVBOCache();
    CleanUpImageViewCache();
    CleanUpImageCache();
    CleanUpBufferCache();
    CleanUpDeviceMemoryCache();
    CleanUpTextureCache();
}

void
CacheManager::CleanUpCaches()
{
    FUN_ENTRY(GL_LOG_TRACE);

    CleanUpFrameCaches();
    CleanUpUBOs();
    CleanUpSampleCache();
    CleanUpRenderPassCache();
    CleanUpPipelineCache();
}

