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
 *  @file       WSIMacOS.cpp
 *  @author     Think Silicon
 *  @date       14/06/2019
 *  @version    1.0
 *
 *  @brief      WSI MacOS module. It gets VkSurface for MacOS Window platform.
 *
 */

#ifdef VK_USE_PLATFORM_MACOS_MVK
#include "WSIMacOS.h"

#import <APPKit/APPKit.h>

#include "api/eglDisplay.h"

EGLBoolean
WSIMacOS::Initialize()
{
    FUN_ENTRY(DEBUG_DEPTH);

    if (VulkanWSI::Initialize() == EGL_FALSE) {
        return EGL_FALSE;
    }

    if (SetPlatformCallbacks() == EGL_FALSE) {
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

EGLBoolean
WSIMacOS::SetPlatformCallbacks(void)
{
    FUN_ENTRY(DEBUG_DEPTH);

    memset(&mWsiMacOSCallbacks, 0, sizeof(mWsiMacOSCallbacks));

    // VK_MVK_macos_surface functions
    GET_WSI_FUNCTION_PTR(mWsiMacOSCallbacks, CreateMacOSSurfaceMVK);

    return EGL_TRUE;
}

VkSurfaceKHR
WSIMacOS::CreateSurface(EGLDisplay_t* dpy, EGLNativeWindowType win, EGLSurface_t *surface)
{
    FUN_ENTRY(DEBUG_DEPTH);

    // if(!surface) {
    //     return VK_NULL_HANDLE;
    // }

    if (!surface->GetWidth() || !surface->GetHeight()) {
        NSView *view = (__bridge NSView *)win;
        CGSize frameSize = view.frame.size;

        surface->SetWidth(frameSize.width );
        surface->SetHeight(frameSize.height );
    }

    VkSurfaceKHR vkSurface;
    VkMacOSSurfaceCreateInfoMVK surfaceCreateInfo;
    memset(static_cast<void *>(&surfaceCreateInfo), 0 ,sizeof(surfaceCreateInfo));
    surfaceCreateInfo.sType      = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
    surfaceCreateInfo.pNext      = nullptr;
    surfaceCreateInfo.pView      = win;

    if(VK_SUCCESS != mWsiMacOSCallbacks.fpCreateMacOSSurfaceMVK(mVkInterface->vkInstance, &surfaceCreateInfo, nullptr, &vkSurface)) {
        return VK_NULL_HANDLE;
    }

    return vkSurface;
}
#endif // VK_USE_PLATFORM_MACOS_MVK
