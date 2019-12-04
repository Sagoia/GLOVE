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
#include "eglRefObject.h"
#include "eglConfig.h"
#include "vector"

class EGLContext_t : public EGLRefObject {
private:
    api_context_t                mAPIContext;
    EGLenum                      mRenderingAPI;
    EGLint                       mRenderableAPIbit;
    rendering_api_interface_t   *mAPIInterface;
    struct EGLDisplay_t          *mDisplay;
    class EGLSurface_t          *mReadSurface;
    class EGLSurface_t          *mDrawSurface;
    struct EGLConfig_t          *mConfig;
    const EGLint                *mAttribList;
    EGLenum                      mClientVersion;
    bool                         mIsCurrent;

    EGLBoolean                   GetAPIRenderableType();
    EGLBoolean                   ParseAttributeList(const EGLint* attrib_list);
    EGLBoolean                   Validate();

public:
    EGLContext_t(struct EGLDisplay_t * display, EGLenum rendering_api, EGLConfig_t* config, const EGLint *attribList);
    ~EGLContext_t();

    EGLBoolean                   Create();
    EGLBoolean                   Destroy();
    EGLBoolean                   MakeCurrent(struct EGLDisplay_t *dpy, EGLSurface_t *draw, EGLSurface_t *read);
    //void                         SetNextImageIndex(uint32_t index);
    void                         Flush();
    void                         Finish();
    void                         BindToTexture(EGLint bind);
    void                         ReleaseSurfaceResources();

    inline void                  SetNotCurrent()                                { FUN_ENTRY(EGL_LOG_TRACE); mIsCurrent = false; }

    inline EGLenum               GetRenderingAPI()                        const { FUN_ENTRY(EGL_LOG_TRACE); return mRenderingAPI; }
    inline EGLDisplay_t         *GetDisplay()                             const { FUN_ENTRY(EGL_LOG_TRACE); return mDisplay; }
    inline EGLSurface_t         *GetReadSurface()                         const { FUN_ENTRY(EGL_LOG_TRACE); return mReadSurface; }
    inline EGLSurface_t         *GetDrawSurface()                         const { FUN_ENTRY(EGL_LOG_TRACE); return mDrawSurface; }
    inline EGLint                GetConfigID()                            const { FUN_ENTRY(EGL_LOG_TRACE); return GetConfigKey(mConfig, EGL_CONFIG_ID); }
    inline EGLint                GetClientVersion()                       const { FUN_ENTRY(EGL_LOG_TRACE); return mClientVersion; }
           EGLint                GetRenderBuffer()                        const;
    inline bool                  IsCurrent()                              const  { FUN_ENTRY(EGL_LOG_TRACE); return mIsCurrent; }

};

#endif // __EGL_CONTEXT_H__
