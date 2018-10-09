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
 *  @file       context.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Context Functionality via Vulkan
 *
 */

#ifndef __VKCONTEXT_H__
#define __VKCONTEXT_H__

#include <map>
#include <vector>
#include "utils/glLogger.h"
#include "vulkan/vulkan.h"
#include "rendering_api_interface.h"

using namespace std;

namespace vulkanAPI {

    typedef struct vkContext_t {
        vkContext_t() {
            vkInstance            = VK_NULL_HANDLE;
            vkQueue               = VK_NULL_HANDLE;
            mInitialized          = false;
            vkGraphicsQueueNodeIndex = 0;
            vkDevice = VK_NULL_HANDLE;
            vkSyncItems             = nullptr;
            mIsMaintenanceExtSupported = false;
            mInitialized            = false;
            memset(static_cast<void*>(&vkDeviceMemoryProperties), 0,
                   sizeof(VkPhysicalDeviceMemoryProperties));
        }

        VkInstance                                          vkInstance;
        vector<VkPhysicalDevice>                            vkGpus;
        VkQueue                                             vkQueue;
        uint32_t                                            vkGraphicsQueueNodeIndex;
        VkDevice                                            vkDevice;
        VkPhysicalDeviceMemoryProperties                    vkDeviceMemoryProperties;
        vkSyncItems_t                                       *vkSyncItems;
        bool                                                mIsMaintenanceExtSupported;
        bool                                                mInitialized;
    } vkContext_t;

    vkContext_t *                     GetContext();
    bool                              InitContext();
    void                              TerminateContext();
    void                              ClearContextResources();

    template<typename T>  inline void SafeDelete(T*& ptr)                       { FUN_ENTRY(GL_LOG_TRACE); delete ptr; ptr = nullptr; }
};

#endif // __VKCONTEXT_H__
