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
 *  @file       WSIXcb.cpp
 *  @author     Think Silicon
 *  @date       25/05/2019
 *  @version    1.0
 *
 *  @brief      WSI WAYLAND module. It gets VkSurface for WAYLAND Window platform.
 *
 */

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
#include "WSIWayland.h"
#include "wayland-egl-backend.h"
#include "api/eglDisplay.h"

EGLBoolean
WSIWayland::Initialize()
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
WSIWayland::SetPlatformCallbacks(void)
{
    FUN_ENTRY(DEBUG_DEPTH);

    memset(&mWsiWaylandCallbacks, 0, sizeof(mWsiWaylandCallbacks));

    // VK_KHR_wayland_surface functions
    GET_WSI_FUNCTION_PTR(mWsiWaylandCallbacks, CreateWaylandSurfaceKHR);

    return EGL_TRUE;
}

VkSurfaceKHR
WSIWayland::CreateSurface(EGLDisplay_t* dpy, EGLNativeWindowType win, EGLSurface_t *surface)
{
    FUN_ENTRY(DEBUG_DEPTH);

    if(!surface) {
        return VK_NULL_HANDLE;
    }

    if(!surface->GetWidth() || !surface->GetHeight()) {
        surface->SetWidth(win->width);
        surface->SetHeight(win->height);
    }

    VkSurfaceKHR vkSurface;
    VkWaylandSurfaceCreateInfoKHR surfaceCreateInfo;
    memset(static_cast<void *>(&surfaceCreateInfo), 0 ,sizeof(surfaceCreateInfo));
    surfaceCreateInfo.sType      = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.pNext      = nullptr;
    surfaceCreateInfo.display    = dpy->display_id;
    surfaceCreateInfo.surface    = win->surface;

    if(VK_SUCCESS != mWsiWaylandCallbacks.fpCreateWaylandSurfaceKHR(mVkInterface->vkInstance, &surfaceCreateInfo, nullptr, &vkSurface)) {
        return VK_NULL_HANDLE;
    }

    return vkSurface;
}
#endif // VK_USE_PLATFORM_WAYLAND_KHR
