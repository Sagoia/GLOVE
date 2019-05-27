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
 *  @file       pipelineCache.h
 *  @author     Think Silicon
 *  @date       06/09/2018
 *  @version    1.0
 *
 *  @brief      Pipeline cache objects allow the result of pipeline construction
 *              to be reused between pipelines and between runs of an application.
 *
 */

#ifndef __VKPIPELINECACHE_H__
#define __VKPIPELINECACHE_H__

#include "context.h"

namespace vulkanAPI {

class PipelineCache {

private:

    const
    vkContext_t *                     mVkContext;

    VkPipelineCache                   mVkPipelineCache;

public:
// Constructor
    PipelineCache(const vkContext_t *vkContext = nullptr);

// Destructor
    ~PipelineCache();

// Create Functions
    bool                              Create(const void *data, size_t size);

// Release Functions
    void                              Release(void);

// Release Functions

// Get Functions
           bool                       GetData(void* data, size_t* size)   const;
    inline VkPipelineCache            GetPipelineCache(void)              const { FUN_ENTRY(GL_LOG_TRACE); return mVkPipelineCache; }

// Set Functions
    inline void                       SetContext(const vkContext_t *vkContext)  { FUN_ENTRY(GL_LOG_TRACE); mVkContext = vkContext; }
};

}

#endif // __VKPIPELINECACHE_H__
