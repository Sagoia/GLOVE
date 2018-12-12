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
 *  @file       displayDriver.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      EGL Display driver module. It is responsible of communicating to the platform so as to init/terminate a display connection,
 *              create/destroy a surfaces, etc. It connects to the window platform via the abstract class PlatformWindowInterface.
 *
 */

#ifndef __DISPLAY_DRIVER_H__
#define __DISPLAY_DRIVER_H__

#include "api/eglContext.h"
#include "thread/renderingThread.h"
#include "utils/egl_defs.h"
#include "utils/eglLogger.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "platform/platformWindowInterface.h"
#include "api/eglDisplay.h"
#include "displayDriverResourceManager.h"
#include <vector>

#ifdef DEBUG_DEPTH
#   undef DEBUG_DEPTH
#endif // DEBUG_DEPTH
#define DEBUG_DEPTH                                 EGL_LOG_DEBUG

void setCallingThread(RenderingThread *thread);

class DisplayDriver {
private:
    EGLDisplay_t                *mEGLDisplay;
    EGLContext_t                *mActiveContext;
    PlatformWindowInterface     *mWindowInterface;
    DisplayDriverResourceManager mDisplayDriverResourceManager;
    bool                         mInitialized;

    EGLImageKHR                  CreateImageNativeBufferAndroid(EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
    void                         CreateEGLSurfaceInterface(EGLSurface_t *eglSurface);
    void                         UpdateSurface(EGLSurface_t *eglSurface);

public:

    DisplayDriver(EGLDisplay_t *eglDisplay);
    ~DisplayDriver(void);

    inline void                  SetActiveContext(EGLContext ctx)               { FUN_ENTRY(EGL_LOG_TRACE); mActiveContext = static_cast<EGLContext_t *>(ctx); }
    inline bool                  Initialized()                            const { FUN_ENTRY(EGL_LOG_TRACE); return mInitialized; }
    void                         CleanMarkedResources(void);

    // Error functions
    EGLBoolean                   CheckBadConfig(const EGLConfig_t *eglConfig) const;
    EGLBoolean                   CheckBadSurface(const EGLSurface_t *eglSurface) const;
    EGLBoolean                   CheckBadContext(const EGLContext_t* eglContext) const;
    static EGLBoolean            CheckNonInitializedDisplay(const DisplayDriver* displayDriver);

    /// EGL API core functions
    EGLBoolean                   Initialize(EGLint *major, EGLint *minor);
    EGLBoolean                   Terminate(void);
    EGLContext                   CreateContext(EGLenum rendering_api, EGLConfig_t *config, const EGLint *attribList);
    EGLBoolean                   DestroyContext(EGLContext_t *eglContext);
    EGLBoolean                   GetConfigs(EGLConfig *configs, EGLint config_size, EGLint *num_config);
    EGLBoolean                   ChooseConfig(const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
    EGLBoolean                   GetConfigAttrib(EGLConfig_t* eglConfig, EGLint attribute, EGLint *value);
    EGLSurface                   CreateWindowSurface(EGLConfig_t* eglConfig, EGLNativeWindowType win, const EGLint *attrib_list);
    EGLSurface                   CreatePbufferSurface(EGLConfig_t* eglConfig, const EGLint *attrib_list);
    EGLSurface                   CreatePixmapSurface(EGLConfig_t* eglConfig, EGLNativePixmapType pixmap, const EGLint *attrib_list);
    EGLBoolean                   DestroySurface(EGLSurface_t* eglSurface);
    EGLBoolean                   QuerySurface(EGLSurface_t* eglSurface, EGLint attribute, EGLint *value);
    EGLSurface                   CreatePbufferFromClientBuffer(EGLenum buftype, EGLClientBuffer buffer, EGLConfig_t* eglConfig, const EGLint *attrib_list);
    EGLBoolean                   SurfaceAttrib(EGLSurface_t* eglSurface, EGLint attribute, EGLint value);
    EGLBoolean                   BindTexImage(EGLSurface_t* eglSurface, EGLint buffer);
    EGLBoolean                   ReleaseTexImage(EGLSurface_t* eglSurface, EGLint buffer);
    EGLBoolean                   SwapInterval(EGLint interval);
    EGLBoolean                   SwapBuffers(EGLSurface_t* eglSurface);
    EGLBoolean                   CopyBuffers(EGLSurface_t* eglSurface, EGLNativePixmapType target);
    const char*                  GetExtensions();

    /// EGL API extension functions
    EGLImageKHR                  CreateImageKHR(EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
    EGLBoolean                   DestroyImageKHR(EGLImageKHR image);
    EGLSyncKHR                   CreateSyncKHR(EGLenum type, const EGLint *attrib_list);
    EGLBoolean                   DestroySyncKHR(EGLSyncKHR sync);
    EGLint                       ClientWaitSyncKHR(EGLSyncKHR sync, EGLint flags, EGLTimeKHR timeout);
};

#endif // __DISPLAY_DRIVER_H__
