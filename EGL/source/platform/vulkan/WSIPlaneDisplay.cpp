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
 *  @file       WSIPlaneDisplay.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      WSI Plane Display module. It gets Plane Display VkSurface.
 *
 */

#include "WSIPlaneDisplay.h"
#include "vulkanResources.h"

EGLBoolean
WSIPlaneDisplay::Initialize()
{
    FUN_ENTRY(DEBUG_DEPTH);

    if (VulkanWSI::Initialize() == EGL_FALSE) {
        return EGL_FALSE;
    }

    if (SetPlatformCallbacks() == EGL_FALSE) {
        return EGL_FALSE;
    }

    SetPhysicalDeviceDisplayProperties();

    return EGL_TRUE;
}

EGLBoolean
WSIPlaneDisplay::SetPlatformCallbacks(void)
{
    FUN_ENTRY(DEBUG_DEPTH);

    memset(&mWsiPlaneDisplayCallbacks, 0, sizeof(mWsiPlaneDisplayCallbacks));

    // VK_KHR_display functions
    GET_WSI_FUNCTION_PTR(mWsiPlaneDisplayCallbacks, CreateDisplayPlaneSurfaceKHR);
    GET_WSI_FUNCTION_PTR(mWsiPlaneDisplayCallbacks, GetPhysicalDeviceDisplayPropertiesKHR);

    return EGL_TRUE;
}

VkSurfaceKHR
WSIPlaneDisplay::CreateSurface(EGLDisplay_t* dpy, EGLNativeWindowType win, EGLSurface_t *surface)
{
    FUN_ENTRY(DEBUG_DEPTH);

    if(!surface) {
        return VK_NULL_HANDLE;
    }

    assert(mDisplayPropertiesList.size() > 0);

    surface->SetWidth(mDisplayPropertiesList[0].physicalResolution.width);
    surface->SetHeight(mDisplayPropertiesList[0].physicalResolution.height);

    /// Create a vk surface
    VkSurfaceKHR vkSurface;
    VkDisplaySurfaceCreateInfoKHR surfaceCreateInfo;
    memset(static_cast<void *>(&surfaceCreateInfo), 0 ,sizeof(surfaceCreateInfo));
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.pNext = nullptr;
    surfaceCreateInfo.imageExtent.width = static_cast<uint32_t>(surface->GetWidth());
    surfaceCreateInfo.imageExtent.height = static_cast<uint32_t>(surface->GetHeight());

    if(VK_SUCCESS != mWsiPlaneDisplayCallbacks.fpCreateDisplayPlaneSurfaceKHR(mVkInterface->vkInstance, &surfaceCreateInfo, nullptr, &vkSurface)) {
        return VK_NULL_HANDLE;
    }

    return vkSurface;
}

void
WSIPlaneDisplay::SetPhysicalDeviceDisplayProperties()
{
    FUN_ENTRY(DEBUG_DEPTH);

    EGLBoolean ASSERT_ONLY displaySuccess;

    VkResult ASSERT_ONLY res;
    uint32_t physicalDeviceDisplayPropertiesCount = 0;
    res = mWsiPlaneDisplayCallbacks.fpGetPhysicalDeviceDisplayPropertiesKHR(mVkInterface->vkGpus[0], &physicalDeviceDisplayPropertiesCount, nullptr);
    assert(!res);

    mDisplayPropertiesList.resize(physicalDeviceDisplayPropertiesCount);
    res = mWsiPlaneDisplayCallbacks.fpGetPhysicalDeviceDisplayPropertiesKHR(mVkInterface->vkGpus[0], &physicalDeviceDisplayPropertiesCount, mDisplayPropertiesList.data());
    assert(!res);
}
