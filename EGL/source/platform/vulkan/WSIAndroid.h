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
 *  @file       WSIAndroid.h
 *  @author     Think Silicon
 *  @date       
 *  @version    1.0
 *
 *  @brief      
 *
 */

#ifndef __WSI_ANDROID_H__
#define __WSI_ANDROID_H__

#include "vulkanWSI.h"
#include <system/window.h>
#include <android/native_window.h>
#include <android/log.h>


class WSIAndroid : public VulkanWSI
{
protected:
	typedef struct wsiAndroidCallbacks {
        // VK_KHR_display functions
        PFN_vkCreateAndroidSurfaceKHR                   fpCreateAndroidSurfaceKHR;
    }wsiAndroidCallbacks_t;

    wsiAndroidCallbacks_t                               mWsiAndroidCallbacks;

    EGLBoolean                                          SetPlatformCallbacks() override;

public:
    WSIAndroid() {}
    ~WSIAndroid() override {}

	EGLBoolean         Initialize() override;
    VkSurfaceKHR       CreateSurface(EGLDisplay_t* dpy,
                                     EGLNativeWindowType win,
                                     EGLSurface_t *surface) override;


};

#endif // __WSI_ANDROID_H__