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
 *  @file       eglDisplay.cpp
 *  @author     Think Silicon
 *  @date       04/10/2018
 *  @version    1.0
 *
 *  @brief      Container of EGL Displays. It manages resources related to EGLDisplay.
 *
 */

#include "eglDisplay.h"
#include "thread/renderingThread.h"
#include <algorithm>

EGLDisplay_t EGLDisplay_t::globalDisplayList[MAX_NUM_DISPLAYS];

EGLDisplay_t*
EGLDisplay_t::GetDisplayByID(EGLNativeDisplayType display_id)
{
    EGLDisplay_t* dis = nullptr;

   for(int32_t i = 0; i < MAX_NUM_DISPLAYS; ++i) {
       dis = &globalDisplayList[i];

       // return an already existing display
       if(dis->created && dis->display_id == display_id) {
           return dis;
       }

       // return the same display handle for same display_ids
       // as display handles are lifetime-valid
       if(dis->display_id == display_id) {
           break;
       }

       // return the next available id
       if(dis->created == false && dis->display_id == nullptr) {
           break;
       }
    }

   // create a new display if it does not exist
   EGLDisplay dpy = nullptr;
#ifdef VK_USE_PLATFORM_ANDROID_KHR
   if (display_id == EGL_DEFAULT_DISPLAY) {
       dpy = (EGLDisplay)1;
   }
   return nullptr;
#endif

#ifdef VK_USE_PLATFORM_XCB_KHR
   if(display_id == EGL_DEFAULT_DISPLAY) {
       dpy = nullptr;
   } else {
       dpy = display_id;
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
EGLDisplay_t::FindDisplay(EGLDisplay display)
{
    FUN_ENTRY(DEBUG_DEPTH);

    EGLDisplay_t* dis = nullptr;
    for(int32_t i = 0; i < MAX_NUM_DISPLAYS; ++i) {
        dis = &globalDisplayList[i];
        // handle is the address of the element in the array
        if(dis->created == true && reinterpret_cast<eglDisplayHandle>(dis) == display) {
            return dis;
        }
     }

    return nullptr;
}

EGLBoolean
EGLDisplay_t::InitializeDisplay(EGLDisplay dpy, void* displayDriver)
{
    EGLDisplay_t *eglDisplay = EGLDisplay_t::FindDisplay(dpy);
    if(eglDisplay == nullptr) {
        return EGL_FALSE;
    }

    // create a new default display
#ifdef VK_USE_PLATFORM_XCB_KHR
    if(eglDisplay->display_id == EGL_DEFAULT_DISPLAY && eglDisplay->display == nullptr) {
        eglDisplay->display = XOpenDisplay(nullptr);
    }
#endif

    eglDisplay->displayDriver = displayDriver;

    return EGL_TRUE;
}

void EGLDisplay_t::TerminateDisplay(EGLDisplay display)
{
    EGLDisplay_t *eglDisplay = EGLDisplay_t::FindDisplay(display);
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
EGLDisplay_t::CheckBadDisplay(const EGLDisplay_t* eglDisplay)
{
    if(eglDisplay == nullptr) {
        currentThread.RecordError(EGL_BAD_DISPLAY);
        return EGL_FALSE;
    }
    return EGL_TRUE;
}
