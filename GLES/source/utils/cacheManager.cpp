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
#include "vulkan/caches.h"

CacheManager::CacheManager(const vulkanAPI::vkContext_t *vkContext) 
: mVkContext(vkContext) 
{ 
    FUN_ENTRY(GL_LOG_TRACE);

    mSubCaches = new vulkanAPI::Caches(vkContext);

    mUBOCache.Reserve(DEFAULT_COUNT);
    for (uint32_t i = 0; i < UBO_ARRAY_COUNT; ++i) {
        mUBOLists[i].Reserve(DEFAULT_COUNT);
    }
    mVBOCache.Reserve(DEFAULT_COUNT);
    mTextureCache.Reserve(DEFAULT_COUNT);
}

CacheManager::~CacheManager() 
{
    FUN_ENTRY(GL_LOG_TRACE);

    if (mSubCaches) {
        delete mSubCaches;
    }
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
CacheManager::CleanUpFrameCaches()
{
    FUN_ENTRY(GL_LOG_TRACE);

    UncacheUBOs();

    mSubCaches->CleanUpImageViewCache();
    mSubCaches->CleanUpImageCache();
    mSubCaches->CleanUpBufferCache();
    mSubCaches->CleanUpDeviceMemoryCache();

    CleanUpVBOCache();
    CleanUpTextureCache();
}

void
CacheManager::CleanUpCaches()
{
    FUN_ENTRY(GL_LOG_TRACE);

    CleanUpFrameCaches();
    CleanUpUBOs();
    mSubCaches->CleanUpSampleCache();
    mSubCaches->CleanUpRenderPassCache();
    mSubCaches->CleanUpPipelineCache();
}

