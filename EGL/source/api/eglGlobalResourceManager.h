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
 *  @file       eglGlobalResourceManager.h
 *  @author     Think Silicon
 *  @date       07/12/2018
 *  @version    1.0
 *
 *  @brief      EGL Global Resources Manager. It handles EGL global resources
 *
 */

#ifndef __EGL_GLOBALRESOURCEMANAGER_H__
#define __EGL_GLOBALRESOURCEMANAGER_H__

#include "EGL/egl.h"
#include "rendering_api/rendering_api.h"
#include "display/displayDriversContainer.h"
#include "utils/eglLogger.h"
#include "eglConfig.h"
#include "eglContext.h"
#include "eglDisplay.h"
#include "vector"

class EGLGlobalResourceManager
{
protected:

    // Display Drivers
    DisplayDriversContainer     mDisplayDriversContainer;

    struct EGLDisplay_t         mEGLDisplayList[MAX_NUM_DISPLAYS];
    EGLDisplay_t                *FindDisplayByID(EGLNativeDisplayType display_id);

public:
    EGLGlobalResourceManager();
    ~EGLGlobalResourceManager();

    // EGLDisplay resources
    EGLBoolean                  InitializeDisplay(EGLDisplay dpy, void* displayDriver);
    EGLDisplay_t               *FindDisplay(EGLDisplay display);
    EGLDisplay_t               *GetDisplayByID(EGLNativeDisplayType display_id);
    void                        TerminateDisplay(EGLDisplay display);

    // Error handling
    EGLBoolean                  CheckBadDisplay(const EGLDisplay_t* eglDisplay) const;

    //DisplayDriversContainer functions
    inline DisplayDriver        *FindDriver(EGLDisplay_t* display)                  { FUN_ENTRY(EGL_LOG_TRACE); return mDisplayDriversContainer.FindDriver(display); }
    inline DisplayDriver        *AddDriver(EGLDisplay_t* display)                   { FUN_ENTRY(EGL_LOG_TRACE); return mDisplayDriversContainer.AddDriver(display); }
    inline void                  RemoveDriver(EGLDisplay_t* display)                { FUN_ENTRY(EGL_LOG_TRACE); mDisplayDriversContainer.RemoveDriver(display); }
    inline EGLBoolean            IsDriverListEmpty()                                { FUN_ENTRY(EGL_LOG_TRACE); return mDisplayDriversContainer.IsEmpty(); }
};

extern EGLGlobalResourceManager eglGlobalResourceManager;

#endif // __EGL_GLOBALRESOURCEMANAGER_H__
