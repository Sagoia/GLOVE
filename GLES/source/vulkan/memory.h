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
 *  @file       memory.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Device Memory Allocation Functionality in Vulkan
 *
 */

#ifndef __VKMEMORY_H__
#define __VKMEMORY_H__

#include "utils.h"
#include "context.h"

class CacheManager;

namespace vulkanAPI {

class Memory {

private:

    const
    XContext_t *                   mVkContext;

    VkDeviceMemory                  mVkMemory;
    const
    VkMemoryMapFlags                mVkMemoryFlags;
    VkFlags                         mVkFlags;
    VkMemoryRequirements            mVkRequirements;

    CacheManager *                  mCacheManager;

public:
// Constructor
    Memory(const XContext_t *vkContext = nullptr, VkFlags flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

// Destructor
    ~Memory();

// Allocate Functions
    bool                            Create(void);

// Release Functions
    void                            Release(void);

// Bind Functions
    bool                            BindBufferMemory(VkBuffer &buffer);
    bool                            BindImageMemory(VkImage &image);

// Get Functions
    inline VkFlags                  GetFlags(void)                            { FUN_ENTRY(GL_LOG_DEBUG); return mVkFlags; }
    void                            GetImageMemoryRequirements(VkImage &image);
    bool                            GetBufferMemoryRequirements(VkBuffer &buffer);
    VkResult                        GetMemoryTypeIndexFromProperties(uint32_t *typeIndex);
    bool                            GetData(VkDeviceSize size, VkDeviceSize offset, void *data) const;

// Set/Update Functions
    bool                            SetData(VkDeviceSize size, VkDeviceSize offset, const void *data);
    void                            UpdateData(VkDeviceSize size, VkDeviceSize offset, const void *data);

    inline void                     SetContext(const XContext_t *vkContext)  { FUN_ENTRY(GL_LOG_TRACE); mVkContext = vkContext; }
    inline void                     SetCacheManager(CacheManager *manager)    { FUN_ENTRY(GL_LOG_TRACE); mCacheManager = manager; }
};

}

#endif // __VKMEMORY_H__
