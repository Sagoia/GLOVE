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
 *  @file       vulkanWSI.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Base class connecting EGL to WSI callbacks.
 *
 */

#include "vulkanWSI.h"

VulkanWSI::VulkanWSI():
    mVkInterface(nullptr)
{

}

EGLBoolean
VulkanWSI::Initialize()
{
    FUN_ENTRY(DEBUG_DEPTH);

    memset(&mWsiCallbacks, 0, sizeof(mWsiCallbacks));

    // VK_KHR_surface functions
    GET_WSI_FUNCTION_PTR(mWsiCallbacks, GetPhysicalDeviceSurfaceSupportKHR);
    GET_WSI_FUNCTION_PTR(mWsiCallbacks, GetPhysicalDeviceSurfaceCapabilitiesKHR);
    GET_WSI_FUNCTION_PTR(mWsiCallbacks, GetPhysicalDeviceSurfaceFormatsKHR);
    GET_WSI_FUNCTION_PTR(mWsiCallbacks, GetPhysicalDeviceSurfacePresentModesKHR);

    // VK_KHR_swapchain functions
    GET_WSI_FUNCTION_PTR(mWsiCallbacks, CreateSwapchainKHR);
    GET_WSI_FUNCTION_PTR(mWsiCallbacks, DestroySwapchainKHR);
    GET_WSI_FUNCTION_PTR(mWsiCallbacks, GetSwapchainImagesKHR);
    GET_WSI_FUNCTION_PTR(mWsiCallbacks, AcquireNextImageKHR);
    GET_WSI_FUNCTION_PTR(mWsiCallbacks, QueuePresentKHR);

    return EGL_TRUE;
}
