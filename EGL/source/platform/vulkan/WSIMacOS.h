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
 *  @file       WSIMacOS.h
 *  @author     Think Silicon
 *  @date       14/06/2019
 *  @version    1.0
 *
 *  @brief      WSI MacOS module. It gets VkSurface for MacOS Window platform.
 *
 */

#ifndef __WSI_MACOS_H__
#define __WSI_MACOS_H__

#include "vulkanWSI.h"

class WSIMacOS : public VulkanWSI
{
protected:
    typedef struct wsiWaylandCallbacks {
        // VK_MVK_macos_surface functions
        PFN_vkCreateMacOSSurfaceMVK                     fpCreateMacOSSurfaceMVK;
    } wsiMacOSCallbacks_t;

    wsiMacOSCallbacks_t                                  mWsiMacOSCallbacks;

    EGLBoolean         SetPlatformCallbacks() override;

public:
    WSIMacOS()  {}
    ~WSIMacOS() override {}

    EGLBoolean         Initialize() override;
    VkSurfaceKHR       CreateSurface(EGLDisplay_t* dpy,
                                    EGLNativeWindowType win,
                                    EGLSurface_t *surface) override;
};

#endif // __WSIMACOS_H__
