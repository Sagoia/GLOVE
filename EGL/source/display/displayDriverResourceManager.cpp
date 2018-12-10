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
 *  @file       DisplayDriverResourceManager.cpp
 *  @author     Think Silicon
 *  @date       07/12/2018
 *  @version    1.0
 *
 *  @brief      Driver Resources Manager. It handles resources related to the display driver.
 *
 */

#include "utils/egl_defs.h"
#include "displayDriverResourceManager.h"
#include "platform/platformWindowInterface.h"
#include "thread/renderingThread.h"
#include <algorithm>

EGLBoolean
DisplayDriverResourceManager::FindEGLConfig(const EGLConfig_t* eglConfig) const
{
    FUN_ENTRY(DEBUG_DEPTH);

    const auto iter = std::find(mConfigList.begin(), mConfigList.end(), eglConfig);
    if(iter == mConfigList.end()) {
        return EGL_FALSE;
    }
    return EGL_TRUE;
}

EGLBoolean
DisplayDriverResourceManager::AddEGLConfig(EGLConfig_t* eglConfig)
{
    FUN_ENTRY(DEBUG_DEPTH);

    if(FindEGLConfig(eglConfig) == EGL_FALSE) {
        mConfigList.push_back(eglConfig);
    }
    return EGL_TRUE;
}

EGLBoolean
DisplayDriverResourceManager::FindEGLSurface(const EGLSurface_t* eglSurface) const
{
    FUN_ENTRY(DEBUG_DEPTH);

    const auto iter = std::find(mSurfaceList.begin(), mSurfaceList.end(), eglSurface);
    if(iter == mSurfaceList.end()) {
        return EGL_FALSE;
    }
    return EGL_TRUE;
}

EGLSurface_t*
DisplayDriverResourceManager::CreateEGLSurface()
{
    FUN_ENTRY(DEBUG_DEPTH);

    return new EGLSurface_t();
}

EGLBoolean
DisplayDriverResourceManager::DeleteEGLSurface(PlatformWindowInterface *windowInterface, EGLSurface_t *eglSurface)
{
    FUN_ENTRY(DEBUG_DEPTH);

    //TODO: Delay deletion after surface is not current to any thread
    windowInterface->DestroySurfaceImages(eglSurface);
    windowInterface->DestroySurface(eglSurface);

    if(eglSurface->GetPlatformResources() != nullptr) {
        delete eglSurface->GetPlatformResources();
    }
    delete eglSurface;
    eglSurface = nullptr;

    return EGL_TRUE;
}

EGLSurface_t*
DisplayDriverResourceManager::AddEGLSurface()
{
    FUN_ENTRY(DEBUG_DEPTH);

    EGLSurface_t *eglSurface = CreateEGLSurface();

    if(eglSurface) {
        mSurfaceList.push_back(eglSurface);
    }

    return eglSurface;
}

EGLBoolean
DisplayDriverResourceManager::RemoveEGLSurface(PlatformWindowInterface *windowInterface, EGLSurface_t* eglSurface)
{
    FUN_ENTRY(DEBUG_DEPTH);

    const auto iter = std::find(mSurfaceList.begin(), mSurfaceList.end(), eglSurface);
    if(iter != mSurfaceList.end()) {
        mSurfaceList.erase(iter);
        return DeleteEGLSurface(windowInterface, eglSurface);
    }

    return EGL_FALSE;
}

EGLBoolean
DisplayDriverResourceManager::FindEGLContext(const EGLContext_t* eglContext) const
{
    FUN_ENTRY(DEBUG_DEPTH);

    const auto iter = std::find(mContextList.begin(), mContextList.end(), eglContext);
    if(iter == mContextList.end()) {
        return EGL_FALSE;
    }
    return EGL_TRUE;
}

EGLContext_t*
DisplayDriverResourceManager::CreateEGLContext(EGLDisplay_t *display, EGLenum rendering_api, EGLConfig_t *config, const EGLint *attribList)
{
    FUN_ENTRY(DEBUG_DEPTH);

    EGLContext_t * eglContext = new EGLContext_t(display, rendering_api, config, attribList);

    if(eglContext->Create() == EGL_FALSE) {
        delete eglContext;
        eglContext = nullptr;
    }

    return eglContext;
}

EGLBoolean
DisplayDriverResourceManager::DeleteEGLContext(EGLContext_t* eglContext)
{
    FUN_ENTRY(DEBUG_DEPTH);

    if(eglContext->Destroy() == EGL_FALSE) {
        return EGL_FALSE;
    }
    delete eglContext;
    return EGL_TRUE;
}

EGLContext_t*
DisplayDriverResourceManager::AddEGLContext(EGLDisplay_t *display, EGLenum rendering_api, EGLConfig_t *config, const EGLint *attribList)
{
    FUN_ENTRY(DEBUG_DEPTH);

    EGLContext_t *eglContext = CreateEGLContext(display, rendering_api, config, attribList);

    if(eglContext) {
        mContextList.push_back(eglContext);
    }

    return eglContext;
}

EGLBoolean
DisplayDriverResourceManager::RemoveEGLContext(EGLContext_t* eglContext)
{
    FUN_ENTRY(DEBUG_DEPTH);

    const auto iter = std::find(mContextList.begin(), mContextList.end(), eglContext);
    if(iter != mContextList.end()) {
        mContextList.erase(iter);
        return DeleteEGLContext(eglContext);
    }
    return EGL_FALSE;
}

void
DisplayDriverResourceManager::CleanMarkedResources(PlatformWindowInterface *windowInterface)
{
    FUN_ENTRY(DEBUG_DEPTH);

    // clear surfaces
    auto surfaceIter = mSurfaceList.begin();
    while (surfaceIter != mSurfaceList.end()) {
        EGLSurface_t* eglSurface = *surfaceIter;
        if((eglSurface->IsMarkedForDeletion() && eglSurface->FreeForDeletion())) {
            DeleteEGLSurface(windowInterface, eglSurface);
            surfaceIter = mSurfaceList.erase(surfaceIter);
        } else {
            surfaceIter++;
        }
    }

    // clear contexts
    auto contextIter = mContextList.begin();
    while(contextIter != mContextList.end()) {
        EGLContext_t* eglContext = *contextIter;
        if((eglContext->IsMarkedForDeletion() && eglContext->FreeForDeletion() && !eglContext->IsCurrent())) {
            DeleteEGLContext(eglContext);
            contextIter = mContextList.erase(contextIter);
        } else {
            contextIter++;
        }
    }
}

void
DisplayDriverResourceManager::CleanResources(PlatformWindowInterface *windowInterface)
{
    FUN_ENTRY(DEBUG_DEPTH);

    // clear surfaces
    for (auto surfaceIter : mSurfaceList) {
        DeleteEGLSurface(windowInterface, surfaceIter);
    }
    mSurfaceList.clear();

    // clear surfaces
    for (auto contextIter : mContextList) {
        DeleteEGLContext(contextIter);
    }
    mContextList.clear();
}
