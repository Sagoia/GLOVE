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
 *  @file       renderingThread.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Rendering thread module. It implements EGL API calls bound to the rendering thread.
 *
 */

#ifndef __RENDERINGTHREAD_H__
#define __RENDERINGTHREAD_H__

#include "api/eglContext.h"
#include "api/eglDisplay.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"

class RenderingThread {
private:
    static const char * const EGLErrors[];

    EGLenum                 mCurrentAPI;
    EGLContext_t           *mGLESCurrentContext;
    EGLContext_t           *mVGCurrentContext;
    EGLint                  mLastError;

    void                    ResetCurrentContext(EGLContext_t* context);
    void                    SetCurrentContext(EGLContext_t* eglContext);

public:
    RenderingThread(void);
    ~RenderingThread(void) { }

    /// EGL API core functions
    void                    RecordError(EGLenum error);
    EGLint                  GetError(void);
    EGLBoolean              BindAPI(EGLenum api);
    EGLenum                 QueryAPI(void);
    EGLBoolean              WaitClient(void);
    EGLBoolean              ReleaseThread(void);
    EGLContext              GetCurrentContext(void);
    EGLSurface              GetCurrentSurface(EGLint readdraw);
    EGLDisplay              GetCurrentDisplay(void);
    EGLContext              CreateContext(EGLDisplay_t* dpy, EGLConfig_t* eglConfig, EGLContext_t* eglShareContext, const EGLint *attrib_list);
    EGLBoolean              DestroyContext(EGLDisplay_t* dpy, EGLContext_t* eglContext);
    EGLBoolean              QueryContext(EGLDisplay_t* dpy, EGLContext_t* eglContext, EGLint attribute, EGLint *value);
    EGLBoolean              MakeCurrent(EGLDisplay_t* dpy, class EGLSurface_t* drawSurface, class EGLSurface_t* readSurface, EGLContext_t* eglContext);
    EGLBoolean              WaitGL(void);
    EGLBoolean              WaitNative(EGLint engine);
};

extern RenderingThread currentThread;

#endif // __RENDERINGTHREAD_H__
