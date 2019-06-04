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
 *  @file       WSIXcb.h
 *  @author     Think Silicon
 *  @date       25/05/2019
 *  @version    1.0
 *
 *  @brief      WSI WAYLAND module. It gets VkSurface for WAYLAND Window platform.
 *
 */

#ifndef __WSI_WAYLAND_H__
#define __WSI_WAYLAND_H__

#include <wayland-client.h>
#include "vulkanWSI.h"

class WSIWayland : public VulkanWSI
{
protected:
    typedef struct wsiWaylandCallbacks {
        // VK_KHR_WAYLAND_surface functions
        PFN_vkCreateWaylandSurfaceKHR                    fpCreateWaylandSurfaceKHR;
    } wsiWaylandCallbacks_t;

    wsiWaylandCallbacks_t                                mWsiWaylandCallbacks;

    EGLBoolean         SetPlatformCallbacks() override;

public:
    WSIWayland()  {}
    ~WSIWayland() override {}

    EGLBoolean         Initialize() override;
    VkSurfaceKHR       CreateSurface(EGLDisplay_t* dpy,
                                    EGLNativeWindowType win,
                                    EGLSurface_t *surface) override;
};

#endif // __WSIWAYLAND_H__
