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

CacheManager::CacheManager(const vulkanAPI::vkContext_t *vkContext) 
: mVkContext(vkContext) 
{ 
    FUN_ENTRY(GL_LOG_TRACE);

    mUBOCache.Reserve(DEFAULT_COUNT);
    for (uint32_t i = 0; i < UBO_ARRAY_COUNT; ++i) {
        mUBOLists[i].Reserve(DEFAULT_COUNT);
    }
    mVBOCache.Reserve(DEFAULT_COUNT);
    mTextureCache.Reserve(DEFAULT_COUNT);
    mVkImageViewCache.Reserve(DEFAULT_COUNT);
    mVkImageCache.Reserve(DEFAULT_COUNT);
    mVkBufferCache.Reserve(DEFAULT_COUNT);
    mVkDeviceMemoryCache.Reserve(DEFAULT_COUNT);
}

CacheManager::~CacheManager() 
{
    FUN_ENTRY(GL_LOG_TRACE);
}

void
CacheManager::UncacheUBOs()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if (!mUBOCache.Empty()) {
        for (uint32_t i = 0; i < mUBOCache.Size(); ++i) {
            auto &ubo = mUBOCache[i];
            uint32_t index = ubo->GetCacheIndex();
            if (index < UBO_ARRAY_COUNT) {
                mUBOLists[index].PushBack(ubo);
            } else {
                delete ubo;
            }
        }

        mUBOCache.Clear();
    }
}

void
CacheManager::CleanUpUBOs()
{
    FUN_ENTRY(GL_LOG_TRACE);

    for (uint32_t i = 0; i < UBO_ARRAY_COUNT; ++i) {
        auto &ubos = mUBOLists[i];
        for (uint32_t j = 0; j < ubos.Size(); ++j) {
            auto &ubo = ubos[i];
            if (ubo != nullptr) {
                    delete ubo;
            }
        }
        ubos.Clear();
    }
}

void
CacheManager::CleanUpVBOCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if(!mVBOCache.Empty()) {
        for(uint32_t i = 0; i < mVBOCache.Size(); ++i) {
            if(mVBOCache[i] != nullptr) {
                delete mVBOCache[i];
                mVBOCache[i] = nullptr;
            }
        }

        mVBOCache.Clear();
    }
}

void
CacheManager::CleanUpTextureCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if(!mTextureCache.Empty()) {
        for(uint32_t i = 0; i < mTextureCache.Size(); ++i) {
            if(mTextureCache[i] != nullptr) {
                delete mTextureCache[i];
                mTextureCache[i] = nullptr;
            }
        }

        mTextureCache.Clear();
    }
}

void
CacheManager::CleanUpImageViewCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if (!mVkImageViewCache.Empty()) {
        for (uint32_t i = 0; i < mVkImageViewCache.Size(); ++i) {
            vkDestroyImageView(mVkContext->vkDevice, mVkImageViewCache[i], nullptr);
        }

        mVkImageViewCache.Clear();
    }
}

void
CacheManager::CleanUpImageCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if (!mVkImageCache.Empty()) {
        for (uint32_t i = 0; i < mVkImageCache.Size(); ++i) {
            vkDestroyImage(mVkContext->vkDevice, mVkImageCache[i], nullptr);
        }

        mVkImageCache.Clear();
    }
}

void
CacheManager::CleanUpBufferCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if (!mVkBufferCache.Empty()) {
        for (uint32_t i = 0; i < mVkBufferCache.Size(); ++i) {
            vkDestroyBuffer(mVkContext->vkDevice, mVkBufferCache[i], nullptr);
        }

        mVkBufferCache.Clear();
    }
}

void
CacheManager::CleanUpDeviceMemoryCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    if (!mVkDeviceMemoryCache.Empty()) {
        for (uint32_t i = 0; i < mVkDeviceMemoryCache.Size(); ++i) {
            vkFreeMemory(mVkContext->vkDevice, mVkDeviceMemoryCache[i], nullptr);
        }

        mVkDeviceMemoryCache.Clear();
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

    mUBOCache.PushBack(uniformBufferObject);
}

UniformBufferObject * 
CacheManager::GetUBO(uint32_t index)
{
    FUN_ENTRY(GL_LOG_TRACE);

    UniformBufferObject *ubo = nullptr;
    
    if (index < UBO_ARRAY_COUNT) {
        auto &ubos = mUBOLists[index];
        if (!ubos.Empty()) {
            ubo = ubos.Back();
            ubos.PopBack();
        }
    }

    return ubo;
}

void
CacheManager::CacheVBO(BufferObject *vbo)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mVBOCache.PushBack(vbo);
}

void
CacheManager::CacheTexture(Texture *tex)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mTextureCache.PushBack(tex);
}

void
CacheManager::CacheVkImageView(VkImageView imageView)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mVkImageViewCache.PushBack(imageView);
}

void
CacheManager::CacheVkImage(VkImage image)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mVkImageCache.PushBack(image);
}

void
CacheManager::CacheVkBuffer(VkBuffer buffer)
{
    FUN_ENTRY(GL_LOG_TRACE);
    
    mVkBufferCache.PushBack(buffer);
}

void
CacheManager::CacheDeviceMemory(VkDeviceMemory deviceMemory)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mVkDeviceMemoryCache.PushBack(deviceMemory);
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

    SamplerMap::iterator it = mVkSamplerCache.find(hash);
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

    RenderPassMap::iterator it = mVkRenderPassCache.find(hash);
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

