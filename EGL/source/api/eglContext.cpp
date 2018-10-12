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
 *  @file       eglContext.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      EGL Context functionality. It connects EGL to the client API
 *
 */

#include "utils/egl_defs.h"
#include "eglContext.h"
#include "api/eglDisplay.h"
#include "eglSurface.h"
#include "thread/renderingThread.h"
#include <algorithm>

std::vector<class EGLContext_t*> EGLContext_t::globalEGLContextList;

EGLBoolean
EGLContext_t::CheckBadContext(const EGLContext_t* eglContext)
{
    if(eglContext == nullptr) {
        currentThread.RecordError(EGL_BAD_CONTEXT);
        return EGL_FALSE;
    }

    const auto iter = std::find(globalEGLContextList.begin(), globalEGLContextList.end(), eglContext);
    if(iter == globalEGLContextList.end()) {
        currentThread.RecordError(EGL_BAD_CONTEXT);
        return EGL_FALSE;
    }
    return EGL_TRUE;
}

EGLBoolean
EGLContext_t::FindEGLContext(const EGLContext_t* eglContext)
{
    auto iter = std::find(globalEGLContextList.begin(), globalEGLContextList.end(), eglContext);
    if(iter == globalEGLContextList.end()) {
        return EGL_FALSE;
    }
    return EGL_TRUE;
}

EGLBoolean
EGLContext_t::GetEGLContext(EGLContext_t* eglContext)
{
    if(FindEGLContext(eglContext) == EGL_FALSE) {
        globalEGLContextList.push_back(eglContext);
    }
    return EGL_TRUE;
}

EGLBoolean
EGLContext_t::RemoveEGLContext(const EGLContext_t* eglContext)
{
    auto iter = std::find(globalEGLContextList.begin(), globalEGLContextList.end(), eglContext);
    if(iter == globalEGLContextList.end()) {
        return EGL_FALSE;
    }
    globalEGLContextList.erase(iter);
    return EGL_TRUE;
}

EGLContext_t::EGLContext_t(EGLDisplay_t* display, EGLenum rendering_api, EGLConfig_t *config, const EGLint *attribList)
: mAPIContext(nullptr), mRenderingAPI(rendering_api), mAPIInterface(nullptr),
mDisplay(display), mReadSurface(nullptr), mDrawSurface(nullptr),
mConfig(config), mAttribList(attribList), mClientVersion(1)
{
    FUN_ENTRY(EGL_LOG_TRACE);

    GetAPIRenderableType();
}

EGLContext_t::~EGLContext_t()
{
    FUN_ENTRY(EGL_LOG_TRACE);
}

void
EGLContext_t::Release()
{
    FUN_ENTRY(EGL_LOG_TRACE);

    mAPIInterface->release_system_fbo_cb(mAPIContext);
}

EGLBoolean
EGLContext_t::GetAPIRenderableType()
{
    switch(mRenderingAPI) {
        case EGL_OPENGL_ES_API:
        {
            switch(mClientVersion) {
                case 1: mRenderableAPIbit = EGL_OPENGL_ES_BIT; return EGL_TRUE;
                case 2: mRenderableAPIbit = EGL_OPENGL_ES2_BIT; return EGL_TRUE;
                default: NOT_REACHED(); return EGL_FALSE;
            }
        }
        case EGL_OPENGL_API: mRenderableAPIbit = EGL_OPENGL_BIT; return EGL_TRUE;
        case EGL_OPENVG_API: mRenderableAPIbit = EGL_OPENVG_BIT; return EGL_TRUE;
        default: NOT_REACHED(); return EGL_FALSE;
    }
}

EGLBoolean
EGLContext_t::Validate()
{
    FUN_ENTRY(EGL_LOG_DEBUG);

    // check the attribute list for errors
    if(ParseAttributeList(mAttribList) == EGL_FALSE) {
        return EGL_FALSE;
    }

    // check the config if it is incompatible with the current API
    GetAPIRenderableType();
    if(!(mConfig->RenderableType & mRenderableAPIbit)) {
        currentThread.RecordError(EGL_BAD_CONFIG);
        return EGL_FALSE;
    }

    // TODO:: generate EGL_BAD_MATCH for incompatible share_contets

    return EGL_TRUE;
}

EGLBoolean
EGLContext_t::Create()
{
    FUN_ENTRY(EGL_LOG_DEBUG);

    if(!Validate()) {
        return EGL_FALSE;
    }

    mAPIInterface = RENDERING_API_get_interface(mRenderingAPI, mClientVersion);

    assert(mAPIInterface != nullptr);

    if(mAPIInterface == nullptr) {
        return EGL_FALSE;
    }

    mAPIContext = mAPIInterface->create_context_cb();

    return mAPIContext != nullptr ? EGL_TRUE : EGL_FALSE;
}

EGLBoolean
EGLContext_t::Destroy()
{
    FUN_ENTRY(EGL_LOG_DEBUG);

    if (mAPIInterface == nullptr) {
        return EGL_FALSE;
    }

    //TODO: Include Error Handling in Final implementation
    mAPIInterface->delete_context_cb(mAPIContext);

    return EGL_TRUE;
}

EGLBoolean
EGLContext_t::MakeCurrent(EGLDisplay_t *dpy, EGLSurface_t *draw, EGLSurface_t *read)
{
    FUN_ENTRY(EGL_LOG_DEBUG);

    mDisplay = dpy;
    mDrawSurface = draw;
    mReadSurface = read;

    // TODO: support pbuffers/pixmaps
    if (draw && draw->GetType() != EGL_WINDOW_BIT) {
        return EGL_TRUE;
    }

    EGLSurfaceInterface* drawSurfaceInterface = nullptr;
    if(mDrawSurface) {
        drawSurfaceInterface = mDrawSurface->GetEGLSurfaceInterface();
    }

    EGLSurfaceInterface* readSurfaceInterface = nullptr;
    if(mReadSurface) {
        readSurfaceInterface = mReadSurface->GetEGLSurfaceInterface();
    }

    if(mReadSurface || mDrawSurface) {
        mAPIInterface->set_read_write_surface_cb(mAPIContext, drawSurfaceInterface, readSurfaceInterface);
    }

    return EGL_TRUE;
}

void
EGLContext_t::SetNextImageIndex(uint32_t index)
{
    FUN_ENTRY(EGL_LOG_TRACE);

    mAPIInterface->set_next_image_index_cb(mAPIContext, index);
}

void
EGLContext_t::Finish()
{
    FUN_ENTRY(EGL_LOG_DEBUG);

    mAPIInterface->finish_cb(mAPIContext);
}

void
EGLContext_t::BoundToTexture(EGLint bound)
{
    FUN_ENTRY(EGL_LOG_DEBUG);

    mAPIInterface->bound_to_texture_cb(mAPIContext, bound);
}

EGLBoolean
EGLContext_t::ParseAttributeList(const EGLint* attrib_list)
{
    if(attrib_list == nullptr || attrib_list[0] == EGL_NONE) {
       return EGL_TRUE;
    }

    // EGL_BAD_ATTRIBUTE is generated if there are any entries and API is not OpenGL ES
    if (mRenderingAPI != EGL_OPENGL_ES_API && attrib_list[0] != EGL_NONE) {
        currentThread.RecordError(EGL_BAD_ATTRIBUTE);
        return EGL_FALSE;
    }

    // EGL_BAD_ATTRIBUTE is also generated if
    // attribute is not EGL_CONTEXT_CLIENT_VERSION  with values 1 or 2
    for(int i = 0; attrib_list[i] != EGL_NONE; i++) {
        EGLint attr = attrib_list[i++];
        EGLint val = attrib_list[i];

        if(attr == EGL_CONTEXT_CLIENT_VERSION && val == 1) {
           mClientVersion = EGL_GL_VERSION_1;
        } else if(attr == EGL_CONTEXT_CLIENT_VERSION && val == 2) {
            mClientVersion = EGL_GL_VERSION_2;
        } else {
            currentThread.RecordError(EGL_BAD_ATTRIBUTE);
            return EGL_FALSE;
        }
    }
    return EGL_TRUE;
}

