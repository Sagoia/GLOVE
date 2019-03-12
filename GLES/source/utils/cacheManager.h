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

#include "arrays.hpp"
#include "utils/glLogger.h"

namespace vulkanAPI {
    struct XContext_t;
    class Caches;
}

class UniformBufferObject;
class BufferObject;
class Texture;

class CacheManager {
private:
    const static uint32_t DEFAULT_COUNT     = 256;
    const static uint32_t UBO_ARRAY_COUNT   = 16;

    const
    vulkanAPI::XContext_t *            mVkContext;

    Array<UniformBufferObject *>        mUBOCache;
    Array<UniformBufferObject *>        mUBOLists[UBO_ARRAY_COUNT];

    Array<BufferObject *>               mVBOCache;
    Array<Texture *>                    mTextureCache;

    vulkanAPI::Caches *                 mSubCaches;

    void                                UncacheUBOs();
    void                                CleanUpUBOs();

    void                                CleanUpVBOCache();
    void                                CleanUpTextureCache();

public:
     CacheManager(const vulkanAPI::XContext_t *vkContext);
    ~CacheManager();

    inline vulkanAPI::Caches *          GetSubCaches(void) { FUN_ENTRY(GL_LOG_TRACE); return mSubCaches; }

    void                                CacheUBO(UniformBufferObject *uniformBufferObject);
    UniformBufferObject *               GetUBO(uint32_t index);

    void                                CacheVBO(BufferObject *vbo);
    void                                CacheTexture(Texture *tex);

    void                                CleanUpFrameCaches();
    void                                CleanUpCaches();
};

#endif //__CACHEMANAGER_H__
