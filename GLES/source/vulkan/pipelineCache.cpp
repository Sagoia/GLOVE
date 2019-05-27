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
 *  @file       pipelineCache.cpp
 *  @author     Think Silicon
 *  @date       06/09/2018
 *  @version    1.0
 *
 *  @brief      Pipeline cache objects allow the result of pipeline construction
 *              to be reused between pipelines and between runs of an application.
 *
 */

#include "pipelineCache.h"

namespace vulkanAPI {

PipelineCache::PipelineCache(const XContext_t *vkContext)
: mVkContext(vkContext), mVkPipelineCache(VK_NULL_HANDLE)
{
    FUN_ENTRY(GL_LOG_TRACE);
}

PipelineCache::~PipelineCache()
{
    FUN_ENTRY(GL_LOG_TRACE);

    Release();
}

void
PipelineCache::Release(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mVkPipelineCache != VK_NULL_HANDLE) {
        vkDestroyPipelineCache(mVkContext->vkDevice, mVkPipelineCache, nullptr);
        mVkPipelineCache = VK_NULL_HANDLE;
    }
}

bool
PipelineCache::GetData(void* data, size_t* size) const
{
    VkResult err = vkGetPipelineCacheData(mVkContext->vkDevice, mVkPipelineCache, size, data);
    assert(!err);

    return (err != VK_ERROR_OUT_OF_HOST_MEMORY && err != VK_ERROR_OUT_OF_DEVICE_MEMORY);
}

bool
PipelineCache::Create(const void *data, size_t size)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    Release();

    VkPipelineCacheCreateInfo info;
    info.sType           = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    info.pNext           = nullptr;
    info.flags           = 0;
    info.pInitialData    = data;
    info.initialDataSize = size;

    VkResult err = vkCreatePipelineCache(mVkContext->vkDevice, &info, nullptr, &mVkPipelineCache);
    assert(!err);

    return (err != VK_ERROR_OUT_OF_HOST_MEMORY && err != VK_ERROR_OUT_OF_DEVICE_MEMORY);
}

}
