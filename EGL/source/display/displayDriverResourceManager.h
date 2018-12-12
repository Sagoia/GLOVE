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
 *  @file       DisplayDriverResourceManager.h
 *  @author     Think Silicon
 *  @date       07/12/2018
 *  @version    1.0
 *
 *  @brief      Driver Resources Manager. It handles resources related to the display driver.
 *
 */

#ifndef __DISPLAYDRIVERRESOURCEMANAGER_H__
#define __DISPLAYDRIVERRESOURCEMANAGER_H__

#include "EGL/egl.h"
#include "utils/eglLogger.h"
#include "api/eglContext.h"
#include "api/eglConfig.h"
#include "api/eglSurface.h"
#include "vector"

class DisplayDriverResourceManager
{
protected:

    std::vector<EGLSurface_t*>   mSurfaceList;
    std::vector<EGLConfig_t*>    mConfigList;
    std::vector<EGLContext_t*>   mContextList;

    // EGLContext resources
    EGLContext_t                *CreateEGLContext(EGLDisplay_t *display, EGLenum rendering_api, EGLConfig_t *config, const EGLint *attribList);
    EGLBoolean                   DeleteEGLContext(EGLContext_t* eglContext);

    // EGLSurface resources
    EGLSurface_t                *CreateEGLSurface(void);
    EGLBoolean                   DeleteEGLSurface(class PlatformWindowInterface *windowInterface, EGLSurface_t *eglSurface);

public:
    DisplayDriverResourceManager() = default;
    ~DisplayDriverResourceManager() = default;

    // EGLConfig resources
    EGLBoolean                   AddEGLConfig(EGLConfig_t* eglConfig);
    EGLBoolean                   FindEGLConfig(const EGLConfig_t* eglConfig) const;

    // EGLSurface resources
    EGLSurface_t                *AddEGLSurface(void);
    EGLBoolean                   RemoveEGLSurface(class PlatformWindowInterface *windowInterface, EGLSurface_t* eglSurface);
    EGLBoolean                   FindEGLSurface(const EGLSurface_t* eglSurface) const;

    // EGLContext resources
    EGLContext_t                *AddEGLContext(EGLDisplay_t *display, EGLenum rendering_api, EGLConfig_t *config, const EGLint *attribList);
    EGLBoolean                   RemoveEGLContext(EGLContext_t* eglContext);
    EGLBoolean                   FindEGLContext(const EGLContext_t* eglContext) const;

    void                         CleanResources(class PlatformWindowInterface *windowInterface);
    void                         CleanMarkedResources(class PlatformWindowInterface *windowInterface);

};

#endif // __DISPLAYDRIVERRESOURCEMANAGER_H__
