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
 *  @file       WSIAndroid.cpp
 *  @author     Think Silicon
 *  @date       
 *  @version    1.0
 *
 *  @brief      
 *
 */

#include "WSIAndroid.h"

EGLBoolean
WSIAndroid::Initialize()
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
WSIAndroid::SetPlatformCallbacks(void)
{
    FUN_ENTRY(DEBUG_DEPTH);

    // VK_KHR_xcb_surface functions
    GET_WSI_FUNCTION_PTR(mWsiAndroidCallbacks, CreateAndroidSurfaceKHR);

    return EGL_TRUE;
}

VkSurfaceKHR
WSIAndroid::CreateSurface(EGLDisplay dpy, EGLNativeWindowType win, EGLSurface_t *surface)
{
    FUN_ENTRY(DEBUG_DEPTH);

    if(!surface) {
        return VK_NULL_HANDLE;
    }

    if(native_window_api_connect(win, NATIVE_WINDOW_API_EGL)) {
        //EGL_LOG(EGL_LOG_DEBUG, "native_window_connect failed.\n");
        return VK_NULL_HANDLE;
    }

    int format = HAL_PIXEL_FORMAT_RGBA_8888;
    format = ANativeWindow_getFormat(win);

    int err = native_window_set_buffers_format(win, format);
    if (err != 0) {
        //EGL_LOG(EGL_LOG_DEBUG, "error setting native window pixel format: %s (%d)\n", strerror(-err), err);
        native_window_api_disconnect(win, NATIVE_WINDOW_API_EGL);
        return VK_NULL_HANDLE;
    }

    if(!surface->GetWidth() || !surface->GetHeight()) {
        surface->SetWidth(ANativeWindow_getWidth(win));
        surface->SetHeight(ANativeWindow_getHeight(win));
    }

    /// Create a vk surface
    VkSurfaceKHR vkSurface;
    VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo;

    memset((void *)&surfaceCreateInfo, 0 ,sizeof(surfaceCreateInfo));
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.pNext = NULL;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.window = (ANativeWindow *)win;

    // Disconnect native window from EGL scope, in order to accomplish connection with WSI(currently both are using NATIVE_WINDOW_API_EGL)
    native_window_api_disconnect(win, NATIVE_WINDOW_API_EGL);

    if(VK_SUCCESS != mWsiAndroidCallbacks.fpCreateAndroidSurfaceKHR(mVkInterface->vkInstance, &surfaceCreateInfo, NULL, &vkSurface)) {
        native_window_set_buffers_format(win, 0);
        return VK_NULL_HANDLE;
    }

    return vkSurface;

}

