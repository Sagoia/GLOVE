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
 *  @file       commandBufferPool.h
 *  @author     Think Silicon
 *  @date       16/10/2018
 *  @version    1.0
 *
 *  @brief      Command Buffer Pool Functionality in Vulkan
 *
 */

#ifndef __VKCOMMANDBUFFERPOOL_H__
#define __VKCOMMANDBUFFERPOOL_H__

#include <vector>
#include "vulkan/vulkan.h"
#include "utils/glLogger.h"

namespace vulkanAPI {

class CommandBufferPool {
private:
    std::vector<VkCommandBuffer *>   mPool;
    uint32_t                         mSize;
    uint32_t                         mLastUsedBuffer;
    bool                             mLocked;

public:
// Constructor
    CommandBufferPool();

// Destructor
    ~CommandBufferPool();

    void                             AddBuffer(VkCommandBuffer *commandBuffer);
    VkCommandBuffer *                RemoveBuffer(void);
    VkCommandBuffer *                BindNextAvailableBuffer(void);
    void                             UnbindAllBuffers(void);

// Get Functions
    inline uint32_t                  GetSize(void)                                 const { FUN_ENTRY(GL_LOG_TRACE); return mSize; } 
};

}
#endif // __VKCOMMANDBUFFERPOOL_H__
