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
#include <vector>

#ifdef DEBUG_DEPTH
#   undef DEBUG_DEPTH
#endif // DEBUG_DEPTH
#define DEBUG_DEPTH                                 EGL_LOG_DEBUG

void setCallingThread(RenderingThread *thread);

class DisplayDriver {
private:
    EGLContext_t                *mActiveContext;
    PlatformWindowInterface     *mWindowInterface;
    std::vector<EGLSurface_t*>   mSurfaceList;
    std::vector<EGLConfig_t*>    mConfigList;
    bool                         mInitialized;

    void                         UpdateConfigMap(EGLConfig_t* config);
    EGLImageKHR                  CreateImageNativeBufferAndroid(EGLDisplay_t* dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
    void                         CreateEGLSurfaceInterface(EGLSurface_t *eglSurface);
    void                         UpdateSurface(EGLDisplay_t* dpy, EGLSurface_t *eglSurface);

public:

    DisplayDriver(void);
    ~DisplayDriver(void)                                                        { FUN_ENTRY(EGL_LOG_TRACE); }

    inline void                  SetActiveContext(EGLContext ctx)               { FUN_ENTRY(EGL_LOG_TRACE); mActiveContext = static_cast<EGLContext_t *>(ctx); }
    inline bool                  Initialized()                            const { FUN_ENTRY(EGL_LOG_TRACE); return mInitialized; }

    // Error functions
    EGLBoolean                   CheckBadConfig(const EGLConfig_t *eglConfig) const;
    EGLBoolean                   CheckBadSurface(const EGLSurface_t *eglSurface) const;
    static EGLBoolean            CheckNonInitializedDisplay(const DisplayDriver* displayDriver);

    /// EGL API core functions
    EGLBoolean                   Initialize(EGLDisplay_t* dpy, EGLint *major, EGLint *minor);
    EGLBoolean                   Terminate(EGLDisplay_t* dpy);
    EGLBoolean                   GetConfigs(EGLDisplay_t* dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config);
    EGLBoolean                   ChooseConfig(EGLDisplay_t* dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
    EGLBoolean                   GetConfigAttrib(EGLDisplay_t* dpy, EGLConfig_t* eglConfig, EGLint attribute, EGLint *value);
    EGLSurface                   CreateWindowSurface(EGLDisplay_t* dpy, EGLConfig_t* eglConfig, EGLNativeWindowType win, const EGLint *attrib_list);
    EGLSurface                   CreatePbufferSurface(EGLDisplay_t* dpy, EGLConfig_t* eglConfig, const EGLint *attrib_list);
    EGLSurface                   CreatePixmapSurface(EGLDisplay_t* dpy, EGLConfig_t* eglConfig, EGLNativePixmapType pixmap, const EGLint *attrib_list);
    EGLBoolean                   DestroySurface(EGLDisplay_t* dpy, EGLSurface_t* eglSurface);
    EGLBoolean                   QuerySurface(EGLDisplay_t* dpy, EGLSurface_t* eglSurface, EGLint attribute, EGLint *value);
    EGLSurface                   CreatePbufferFromClientBuffer(EGLDisplay_t* dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig_t* eglConfig, const EGLint *attrib_list);
    EGLBoolean                   SurfaceAttrib(EGLDisplay_t* dpy, EGLSurface_t* eglSurface, EGLint attribute, EGLint value);
    EGLBoolean                   BindTexImage(EGLDisplay_t* dpy, EGLSurface_t* eglSurface, EGLint buffer);
    EGLBoolean                   ReleaseTexImage(EGLDisplay_t* dpy, EGLSurface_t* eglSurface, EGLint buffer);
    EGLBoolean                   SwapInterval(EGLDisplay_t* dpy, EGLint interval);
    EGLBoolean                   SwapBuffers(EGLDisplay_t* dpy, EGLSurface_t* eglSurface);
    EGLBoolean                   CopyBuffers(EGLDisplay_t* dpy, EGLSurface_t* eglSurface, EGLNativePixmapType target);
    __eglMustCastToProperFunctionPointerType
                                 GetProcAddress(const char *procname);
    const char*                  GetExtensions();


    /// EGL API extension functions
    EGLImageKHR                  CreateImageKHR(EGLDisplay_t* dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
    EGLBoolean                   DestroyImageKHR(EGLDisplay_t* dpy, EGLImageKHR image);
    EGLSyncKHR                   CreateSyncKHR(EGLDisplay_t* dpy, EGLenum type, const EGLint *attrib_list);
    EGLBoolean                   DestroySyncKHR(EGLDisplay_t* dpy, EGLSyncKHR sync);
    EGLint                       ClientWaitSyncKHR(EGLDisplay_t* dpy, EGLSyncKHR sync, EGLint flags, EGLTimeKHR timeout);
};

#endif // __DISPLAY_DRIVER_H__
