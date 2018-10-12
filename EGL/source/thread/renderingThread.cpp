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
 *  @file       renderingThread.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Rendering thread module. It implements EGL API calls bound to the rendering thread.
 *
 */

#include "renderingThread.h"
#include "api/eglSurface.h"
#include "utils/eglLogger.h"

const char * const RenderingThread::EGLErrors[] = {   "EGL_SUCCESS",
                                                      "EGL_NOT_INITIALIZED",
                                                      "EGL_BAD_ACCESS",
                                                      "EGL_BAD_ALLOC",
                                                      "EGL_BAD_ATTRIBUTE",
                                                      "EGL_BAD_CONFIG",
                                                      "EGL_BAD_CONTEXT",
                                                      "EGL_BAD_CURRENT_SURFACE",
                                                      "EGL_BAD_DISPLAY",
                                                      "EGL_BAD_MATCH",
                                                      "EGL_BAD_NATIVE_PIXMAP",
                                                      "EGL_BAD_NATIVE_WINDOW",
                                                      "EGL_BAD_PARAMETER",
                                                      "EGL_BAD_SURFACE",
                                                      "EGL_CONTEXT_LOST"};

RenderingThread::RenderingThread()
: mCurrentAPI(EGL_OPENGL_ES_API), mLastError(EGL_SUCCESS)
{
    FUN_ENTRY(EGL_LOG_TRACE);
}

void
RenderingThread::RecordError(EGLenum error)
{
    FUN_ENTRY(EGL_LOG_TRACE);

    if (error != EGL_SUCCESS && mLastError == EGL_SUCCESS) {
        mLastError = error;
    }
}

EGLint
RenderingThread::GetError(void)
{
    FUN_ENTRY(EGL_LOG_TRACE);

    if(mLastError != 0x3000) {
        printf("Error: %s\n", EGLErrors[mLastError - 0x3000]); // TODO: to be removed
    }

    EGLint result = mLastError;
    mLastError    = EGL_SUCCESS;

    return result;
}

EGLBoolean
RenderingThread::BindAPI(EGLenum api)
{
    FUN_ENTRY(DEBUG_DEPTH);

    if(api != EGL_OPENGL_API && api != EGL_OPENGL_ES_API && api != EGL_OPENVG_API) {
        RecordError(EGL_BAD_PARAMETER);

        return EGL_FALSE;
    }

    mCurrentAPI = api;

    return EGL_TRUE;
}

EGLenum
RenderingThread::QueryAPI(void)
{
    FUN_ENTRY(EGL_LOG_TRACE);

    return mCurrentAPI;
}

EGLBoolean
RenderingThread::WaitClient(void)
{
    FUN_ENTRY(DEBUG_DEPTH);

    EGLContext_t *activeContext = static_cast<EGLContext_t *>(GetCurrentContext());
    if (activeContext == nullptr) {
        return EGL_FALSE;
    }

    activeContext->Finish();

    return EGL_TRUE;
}

EGLBoolean
RenderingThread::ReleaseThread(void)
{
    FUN_ENTRY(DEBUG_DEPTH);

    NOT_IMPLEMENTED();

    return EGL_FALSE;
}

EGLContext
RenderingThread::GetCurrentContext(void)
{
    FUN_ENTRY(EGL_LOG_TRACE);

    switch(mCurrentAPI) {
        case EGL_OPENGL_ES_API:    return mGLESCurrentContext; break;
        case EGL_OPENVG_API:       return mVGCurrentContext;   break;
        default:                   return nullptr;
    }
}

void
RenderingThread::SetCurrentContext(EGLContext_t* eglContext)
{
    FUN_ENTRY(EGL_LOG_TRACE);

    switch(mCurrentAPI) {
        case EGL_OPENGL_ES_API: mGLESCurrentContext = eglContext; break;
        case EGL_OPENVG_API:    mVGCurrentContext   = eglContext; break;
        default:                break;
    }
}

EGLSurface
RenderingThread::GetCurrentSurface(EGLint readdraw)
{
    FUN_ENTRY(EGL_LOG_TRACE);

    if(EGL_READ != readdraw && EGL_DRAW != readdraw) {
        RecordError(EGL_BAD_PARAMETER);
        return EGL_NO_SURFACE;
    }

    EGLContext_t *activeContext = static_cast<EGLContext_t *>(GetCurrentContext());
    if(!EGLContext_t::FindEGLContext(activeContext)) {
        return EGL_NO_SURFACE;
    }

    return (EGL_READ == readdraw) ? activeContext->GetReadSurface() : activeContext->GetDrawSurface();

}

EGLDisplay
RenderingThread::GetCurrentDisplay(void)
{
    FUN_ENTRY(EGL_LOG_TRACE);

    EGLContext_t *activeContext = static_cast<EGLContext_t *>(GetCurrentContext());
    if (activeContext == nullptr) {
        return EGL_NO_DISPLAY;
    }

    return activeContext->GetDisplay();
}

EGLContext
RenderingThread::CreateContext(EGLDisplay_t* dpy, EGLConfig_t* eglConfig, EGLContext_t* eglShareContext, const EGLint *attrib_list)
{
    FUN_ENTRY(EGL_LOG_TRACE);

    if(mCurrentAPI == EGL_NONE) {
        RecordError(EGL_BAD_MATCH);
        return EGL_NO_CONTEXT;
    }

    EGLContext_t *eglContext = new EGLContext_t(dpy, mCurrentAPI, eglConfig, attrib_list);
    if(eglContext->Create() == EGL_FALSE) {
        delete eglContext;
        return EGL_NO_CONTEXT;
    }

    EGLContext_t::GetEGLContext(eglContext);

    return eglContext;
}

EGLBoolean
RenderingThread::DestroyContext(EGLDisplay_t* dpy, EGLContext_t* eglContext)
{
    FUN_ENTRY(EGL_LOG_TRACE);

    if(eglContext->Destroy() == EGL_FALSE) {
        return EGL_FALSE;
    }

    EGLContext_t::RemoveEGLContext(eglContext);

    delete eglContext;

    return EGL_TRUE;
}

EGLBoolean
RenderingThread::QueryContext(EGLDisplay_t* dpy, EGLContext_t* eglContext, EGLint attribute, EGLint *value)
{
    FUN_ENTRY(DEBUG_DEPTH);

    switch(attribute) {
        case EGL_CONFIG_ID:
            *value = eglContext->getConfigID();
            break;
        case EGL_CONTEXT_CLIENT_TYPE:
            *value = QueryAPI();
            break;
        case EGL_CONTEXT_CLIENT_VERSION:
            *value = eglContext->getClientVersion();
        case EGL_RENDER_BUFFER:
            *value = EGL_BACK_BUFFER;
            break;
        default:
            currentThread.RecordError(EGL_BAD_ATTRIBUTE);
            return EGL_FALSE;
            break;
    }

    return EGL_TRUE;
}

EGLBoolean
RenderingThread::MakeCurrent(EGLDisplay_t* dpy, EGLSurface_t* drawSurface, EGLSurface_t* readSurface, EGLContext_t* eglContext)
{
    FUN_ENTRY(DEBUG_DEPTH);

    // Flush commands when changing contexts of the same client API type
    EGLContext_t* currentContext = static_cast<EGLContext_t*>(GetCurrentContext());
    if(currentContext && currentContext != eglContext) {
        currentContext->Finish();
    }

    if(eglContext && eglContext->MakeCurrent(dpy, drawSurface, readSurface) != EGL_TRUE) {
        return EGL_FALSE;
    }

    SetCurrentContext(eglContext);

    return EGL_TRUE;
}

EGLBoolean
RenderingThread::WaitGL(void)
{
    FUN_ENTRY(DEBUG_DEPTH);

    EGLBoolean ret = EGL_FALSE;

    EGLenum api = mCurrentAPI;
    mCurrentAPI = EGL_OPENGL_ES_API;
    ret         = WaitClient();
    mCurrentAPI = api;

    return ret;
}

EGLBoolean
RenderingThread::WaitNative(EGLint engine)
{
    FUN_ENTRY(DEBUG_DEPTH);

    if(engine != EGL_CORE_NATIVE_ENGINE) {
        RecordError(EGL_BAD_PARAMETER);
        return EGL_FALSE;
    }

    // TODO:: implement EGL_BAD_CURRENT_SURFACE
    NOT_IMPLEMENTED();

    return EGL_FALSE;
}
