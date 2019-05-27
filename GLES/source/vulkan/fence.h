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
 *  @file       fence.h
 *  @author     Think Silicon
 *  @date       03/09/2018
 *  @version    1.0
 *
 *  @brief      Determine completion of execution of queue operations via Fences in Vulkan
 *
 */

#ifndef __VKFENCE_H__
#define __VKFENCE_H__

#include "context.h"

namespace vulkanAPI {

class Fence {

private:

    const
    XContext_t *                     mVkContext;

    VkFence                           mVkFence;

public:
// Constructor
    Fence(const XContext_t *vkContext = nullptr);

// Destructor
    ~Fence();

// Create Functions
    bool                              Create(bool signaled);

// Release Functions
    void                              Release(void);

// Reset Functions
    bool                              Reset(void);

// Wait Functions
    bool                              Wait(VkBool32  waitAll, uint64_t timeout);

// Get Functions
    inline VkFence                    GetFence(void)                      const { FUN_ENTRY(GL_LOG_TRACE); return mVkFence; }

// Set Functions
    inline void                       SetContext(const XContext_t *vkContext)  { FUN_ENTRY(GL_LOG_TRACE); mVkContext = vkContext; }
};

}

#endif // __VKFENCE_H__
