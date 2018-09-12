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
 *  @file       WSIPlaneDisplay.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      WSI Plane Display module. It gets Plane Display VkSurface.
 *
 */

#ifndef __WSI_PLANE_DISPLAY_H__
#define __WSI_PLANE_DISPLAY_H__

#include "vulkanWSI.h"
#include <vector>

class WSIPlaneDisplay : public VulkanWSI
{
protected:
    typedef struct wsiPlaneDisplayCallbacks {
        // VK_KHR_display functions
        PFN_vkCreateDisplayPlaneSurfaceKHR              fpCreateDisplayPlaneSurfaceKHR;
        PFN_vkGetPhysicalDeviceDisplayPropertiesKHR     fpGetPhysicalDeviceDisplayPropertiesKHR;
    } wsiPlaneDisplayCallbacks_t;

    wsiPlaneDisplayCallbacks_t                          mWsiPlaneDisplayCallbacks;
    std::vector<VkDisplayPropertiesKHR>                 mDisplayPropertiesList;

    void               SetPhysicalDeviceDisplayProperties();
    EGLBoolean         SetPlatformCallbacks() override;

public:
    WSIPlaneDisplay() {}
    ~WSIPlaneDisplay() override {}

    EGLBoolean         Initialize() override;
    VkSurfaceKHR       CreateSurface(EGLDisplay dpy,
                                     EGLNativeWindowType win,
                                     EGLSurface_t *surface) override;



};

#endif // __WSI_PLANE_DISPLAY_H__
