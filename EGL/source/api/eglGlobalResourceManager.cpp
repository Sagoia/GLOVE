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
 *  @file       eglGlobalResourceManager.cpp
 *  @author     Think Silicon
 *  @date       07/12/2018
 *  @version    1.0
 *
 *  @brief      EGL Global Resources Manager. It handles EGL global resources
 *
 */

#include "utils/egl_defs.h"
#include "eglGlobalResourceManager.h"
#include "thread/renderingThread.h"
#include <algorithm>

EGLGlobalResourceManager::EGLGlobalResourceManager()
{
    FUN_ENTRY(DEBUG_DEPTH);
}

EGLGlobalResourceManager::~EGLGlobalResourceManager()
{
    FUN_ENTRY(DEBUG_DEPTH);
}

EGLDisplay_t*
EGLGlobalResourceManager::FindDisplayByID(EGLNativeDisplayType display_id)
{
    FUN_ENTRY(DEBUG_DEPTH);

    EGLDisplay_t* dis = nullptr;

    for(int32_t i = 0; i < MAX_NUM_DISPLAYS; ++i) {
        dis = &mEGLDisplayList[i];

        // return an already existing display
        if(dis->created && dis->display_id == display_id) {
            return dis;
        }

        // return the same display handle for same display_ids
        // as display handles are lifetime-valid
        if(dis->display_id == display_id) {
            break;
        }

#ifdef VK_USE_PLATFORM_MACOS_MVK
        // return the next available id
        if(dis->created == false && dis->display_id == 0) {
            break;
        }
#else
        // return the next available id
        if(dis->created == false && dis->display_id == nullptr) {
            break;
        }
#endif
     }

    return dis;
}

EGLDisplay_t*
EGLGlobalResourceManager::GetDisplayByID(EGLNativeDisplayType display_id)
{
    FUN_ENTRY(DEBUG_DEPTH);

    EGLDisplay_t* dis = FindDisplayByID(display_id);

   // create a new display if it does not exist
   EGLDisplay dpy = nullptr;
#if defined (VK_USE_PLATFORM_ANDROID_KHR)
   if (display_id == EGL_DEFAULT_DISPLAY) {
       dpy = (EGLDisplay)1;
   }
#endif

#if defined (VK_USE_PLATFORM_XCB_KHR) || defined (VK_USE_PLATFORM_WAYLAND_KHR)
   if(display_id == EGL_DEFAULT_DISPLAY) {
       dpy = nullptr;
   } else {
       dpy = display_id;
   }

#elif defined (VK_USE_PLATFORM_MACOS_MVK)
   // Hack: Should be changed to proper implementation
   if(display_id == EGL_DEFAULT_DISPLAY) {
       dpy = (EGLDisplay)1;
   }
#else
   dpy = display_id;
#endif

   dis->display = dpy;
   dis->display_id = display_id;
   dis->created = true;
   return dis;
}

EGLDisplay_t*
EGLGlobalResourceManager::FindDisplay(EGLDisplay display)
{
    FUN_ENTRY(DEBUG_DEPTH);

    EGLDisplay_t* dis = nullptr;
    for(int32_t i = 0; i < MAX_NUM_DISPLAYS; ++i) {
        dis = &mEGLDisplayList[i];
        // handle is the address of the element in the array
        if(dis->created == true && reinterpret_cast<eglDisplayHandle>(dis) == display) {
            return dis;
        }
     }

    return nullptr;
}

EGLBoolean
EGLGlobalResourceManager::InitializeDisplay(EGLDisplay dpy, void* displayDriver)
{
    FUN_ENTRY(DEBUG_DEPTH);

    EGLDisplay_t *eglDisplay = FindDisplay(dpy);
    if(eglDisplay == nullptr) {
        return EGL_FALSE;
    }

    // create a new default display
#ifdef VK_USE_PLATFORM_XCB_KHR
    if(eglDisplay->display_id == EGL_DEFAULT_DISPLAY && eglDisplay->display == nullptr) {
        eglDisplay->display = XOpenDisplay(nullptr);
    }
#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
    if(eglDisplay->display_id == EGL_DEFAULT_DISPLAY && eglDisplay->display == nullptr) {
        eglDisplay->display = wl_display_connect(NULL);
    }
#endif

    // Missing implementation for VK_USE_PLATFORM_MACOS_MVK

    eglDisplay->displayDriver = displayDriver;

    return EGL_TRUE;
}

void
EGLGlobalResourceManager::TerminateDisplay(EGLDisplay display)
{
    FUN_ENTRY(DEBUG_DEPTH);

    EGLDisplay_t *eglDisplay = FindDisplay(display);
    if(eglDisplay == nullptr) {
        return;
    }

#ifdef VK_USE_PLATFORM_XCB_KHR
    if(eglDisplay->display_id == EGL_NO_DISPLAY && eglDisplay->display != nullptr) {
        XCloseDisplay(static_cast<Display*>(eglDisplay->display));
        eglDisplay->display = nullptr;
    }
#endif

    eglDisplay->displayDriver = nullptr;
}

EGLBoolean
EGLGlobalResourceManager::CheckBadDisplay(const EGLDisplay_t* eglDisplay) const
{
    FUN_ENTRY(DEBUG_DEPTH);

    if(eglDisplay == nullptr) {
        currentThread.RecordError(EGL_BAD_DISPLAY);
        return EGL_FALSE;
    }
    return EGL_TRUE;
}
