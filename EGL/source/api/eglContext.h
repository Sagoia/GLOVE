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
 *  @file       eglContext.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      EGL Context functionality. It connects EGL to the client API
 *
 */

#ifndef __EGL_CONTEXT_H__
#define __EGL_CONTEXT_H__

#include "EGL/egl.h"
#include "rendering_api/rendering_api.h"
#include "utils/eglLogger.h"
#include "vector"

class EGLContext_t {
private:
    api_context_t                mAPIContext;
    EGLenum                      mRenderingAPI;
    EGLint                       mRenderableAPIbit;
    rendering_api_interface_t   *mAPIInterface;
    class EGLDisplay_t          *mDisplay;
    class EGLSurface_t          *mReadSurface;
    class EGLSurface_t          *mDrawSurface;
    struct EGLConfig_t          *mConfig;
    const EGLint                *mAttribList;
    EGLenum                      mClientVersion;

    static std::vector<class EGLContext_t*> globalEGLContextList;

    EGLBoolean                   GetAPIRenderableType();
    EGLBoolean                   ParseAttributeList(const EGLint* attrib_list);
    EGLBoolean                   Validate();

public:
    EGLContext_t(EGLenum rendering_api, EGLConfig_t* config, const EGLint *attribList);
    ~EGLContext_t();


    static EGLBoolean            FindEGLContext(const EGLContext_t* eglContext);
    static EGLBoolean            GetEGLContext(EGLContext_t* eglContext);
    static EGLBoolean            RemoveEGLContext(const EGLContext_t* eglContext);
    static EGLBoolean            CheckBadContext(const EGLContext_t* eglContext);

    EGLBoolean                   Create();
    EGLBoolean                   Destroy();
    EGLBoolean                   MakeCurrent(class EGLDisplay_t *dpy, EGLSurface_t *draw, EGLSurface_t *read);
    void                         SetNextImageIndex(uint32_t index);
    void                         Finish();
    void                         Release();
    GLPROC                       GetProcAddr(const char* procname);

    inline EGLenum               GetRenderingAPI()                        const { FUN_ENTRY(EGL_LOG_TRACE); return mRenderingAPI; }
    inline EGLDisplay            getDisplay()                             const { FUN_ENTRY(EGL_LOG_TRACE); return mDisplay; }
    inline EGLSurface            getReadSurface()                         const { FUN_ENTRY(EGL_LOG_TRACE); return mReadSurface; }
    inline EGLSurface            getDrawSurface()                         const { FUN_ENTRY(EGL_LOG_TRACE); return mDrawSurface; }

};

#endif // __EGL_CONTEXT_H__
