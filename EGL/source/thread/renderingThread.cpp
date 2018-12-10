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
#include "display/displayDriver.h"
#include "utils/eglLogger.h"
#include "api/eglGlobalResourceManager.h"

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

    EGLContext_t *activeContext = GetCurrentContext();
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

void
RenderingThread::SetCurrentContext(EGLenum renderingAPI, EGLContext_t* eglContext)
{
    FUN_ENTRY(EGL_LOG_TRACE);

    switch(renderingAPI) {
        case EGL_OPENGL_ES_API: mGLESCurrentContext = eglContext; break;
        case EGL_OPENVG_API:    mVGCurrentContext   = eglContext; break;
        default:                break;
    }
}

EGLContext_t*
RenderingThread::GetCurrentContext(void)
{
    FUN_ENTRY(EGL_LOG_TRACE);

    switch(mCurrentAPI) {
        case EGL_OPENGL_ES_API:    return mGLESCurrentContext;
        case EGL_OPENVG_API:       return mVGCurrentContext;
        default:                   return nullptr;
    }
}

void
RenderingThread::UpdateCurrentContextResourcesRef(EGLContext_t *eglContext, bool incrementCounters)
{
    if(eglContext == nullptr) {
        return;
    }

    eglContext->UpdateRef(incrementCounters);

    if(eglContext->GetDrawSurface()) {
        eglContext->GetDrawSurface()->UpdateRef(incrementCounters);
    }
    if(eglContext->GetReadSurface()) {
        eglContext->GetReadSurface()->UpdateRef(incrementCounters);
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

    EGLContext_t *activeContext = GetCurrentContext();
    if(activeContext == nullptr) {
        return EGL_NO_SURFACE;
    }

    return (EGL_READ == readdraw) ? activeContext->GetReadSurface() : activeContext->GetDrawSurface();

}

EGLDisplay
RenderingThread::GetCurrentDisplay(void)
{
    FUN_ENTRY(EGL_LOG_TRACE);

    EGLContext_t *activeContext = GetCurrentContext();
    if (activeContext == nullptr) {
        return EGL_NO_DISPLAY;
    }

    return activeContext->GetDisplay();
}

EGLContext
RenderingThread::CreateContext(DisplayDriver* eglDriver, EGLConfig_t* eglConfig, EGLContext_t* eglShareContext, const EGLint *attrib_list)
{
    FUN_ENTRY(EGL_LOG_TRACE);

    if(mCurrentAPI == EGL_NONE) {
        RecordError(EGL_BAD_MATCH);
        return EGL_NO_CONTEXT;
    }

    return eglDriver->CreateContext(mCurrentAPI, eglConfig, attrib_list);
}

EGLBoolean
RenderingThread::DestroyContext(DisplayDriver* eglDriver, EGLContext_t* eglContext)
{
    FUN_ENTRY(EGL_LOG_TRACE);

    EGLBoolean result = EGL_FALSE;

    EGLenum renderingAPI = eglContext->GetRenderingAPI();

    // mark context for deletion
    eglContext->MarkForDeletion();


    // delete if not current to any thread
    if(eglContext->FreeForDeletion()) {
        result = eglDriver->DestroyContext(eglContext);

        // reset the current context for the rendering API
        SetCurrentContext(renderingAPI, nullptr);
    }

    return result;
}

EGLBoolean
RenderingThread::QueryContext(DisplayDriver* eglDriver, EGLContext_t* eglContext, EGLint attribute, EGLint *value)
{
    FUN_ENTRY(DEBUG_DEPTH);

    switch(attribute) {
        case EGL_CONFIG_ID:
            *value = eglContext->GetConfigID();
            break;
        case EGL_CONTEXT_CLIENT_TYPE:
            *value = eglContext->GetRenderingAPI();
            break;
        case EGL_CONTEXT_CLIENT_VERSION:
            *value = eglContext->GetClientVersion();
            break;
        case EGL_RENDER_BUFFER:
            *value = eglContext->GetRenderBuffer();
            break;
        default:
            currentThread.RecordError(EGL_BAD_ATTRIBUTE);
            return EGL_FALSE;
    }

    return EGL_TRUE;
}

EGLBoolean
RenderingThread::ValidateCurrentContext(DisplayDriver* eglDriver, EGLSurface_t* drawSurface, EGLSurface_t* readSurface, EGLContext_t* eglContext)
{
    // generate EGL_BAD_MATCH if EGL_NO_CONTEXT and EGL_NO_SURFACE are not specified together
    if((eglContext == EGL_NO_CONTEXT && (drawSurface != EGL_NO_SURFACE || readSurface != EGL_NO_SURFACE)) ||
       (eglContext != EGL_NO_CONTEXT && (drawSurface == EGL_NO_SURFACE || readSurface == EGL_NO_SURFACE))) {
        currentThread.RecordError(EGL_BAD_MATCH);
        return EGL_FALSE;
    }

    if(eglContext != EGL_NO_CONTEXT && eglDriver->CheckBadContext(eglContext) == EGL_FALSE) {
        currentThread.RecordError(EGL_BAD_CONTEXT);
        return EGL_FALSE;
    }

    // check for valid surfaces (that are note EGL_NO_SURFACE)
    if(drawSurface != EGL_NO_SURFACE && eglDriver->CheckBadSurface(drawSurface) == EGL_FALSE) {
        return EGL_FALSE;
    }

    if(readSurface != EGL_NO_SURFACE && eglDriver->CheckBadSurface(readSurface) == EGL_FALSE) {
        return EGL_FALSE;
    }

    // generate EGL_BAD_MATCH if read and draw surfaces are not the same on OpenVG
    if(eglContext && eglContext->GetRenderingAPI() == EGL_OPENVG_API && (readSurface != drawSurface)) {
        currentThread.RecordError(EGL_BAD_MATCH);
    }

    // generate EGL_BAD_MATCH if EGL_NO_CONTEXT and EGL_NO_SURFACE are not specified together
    if((eglContext == EGL_NO_CONTEXT && (drawSurface != EGL_NO_SURFACE || readSurface != EGL_NO_SURFACE)) ||
       (eglContext != EGL_NO_CONTEXT && drawSurface == EGL_NO_SURFACE && readSurface == EGL_NO_SURFACE)) {
        currentThread.RecordError(EGL_BAD_MATCH);
        return EGL_FALSE;
    }

    // TODO:: If ctx is current to some other thread, or if either draw or read are bound to
    // contexts in another thread, an EGL_BAD_ACCESS error is generated.

    // TODO:: If binding ctx would exceed the number of current contexts of that client
    // API type supported by the implementation, an EGL_BAD_ACCESS error is generated

    // TODO:: If either draw or read are pbuffers created with eglCreatePbufferFromClientBuffer,
    // and the underlying bound client API buffers are in use by the
    // client API that created them, an EGL_BAD_ACCESS error is generated.

    // TODO:: If a native window underlying either draw or read is no longer valid, an EGL_BAD_NATIVE_WINDOW error is generated.

    // TODO::If the previous context of the calling thread has unflushed commands, and
    // the previous surface is no longer valid, an EGL_BAD_CURRENT_SURFACE error is generated.

    // TODO:: If the ancillary buffers for draw and read cannot be allocated, an EGL_BAD_ALLOC error is generated

    // TODO:: If a power management event has occurred, an EGL_CONTEXT_LOST error is generated.

    return EGL_TRUE;
}

EGLBoolean
RenderingThread::MakeCurrent(DisplayDriver* eglDriver, EGLDisplay_t* dpy, EGLSurface draw, EGLSurface read, EGLContext ctx)
{
    FUN_ENTRY(DEBUG_DEPTH);

    EGLContext_t* eglContext = static_cast<EGLContext_t*>(ctx);
    EGLSurface_t *eglDrawSurface = static_cast<EGLSurface_t*>(draw);
    EGLSurface_t *eglReadSurface = static_cast<EGLSurface_t*>(read);
    if(ValidateCurrentContext(eglDriver, eglDrawSurface, eglReadSurface, eglContext) == EGL_FALSE) {
        return EGL_FALSE;
    }

    // Flush commands when changing contexts of the same client API type
    EGLContext_t* currentContext = GetCurrentContext();
    if(currentContext && currentContext != eglContext) {
        currentContext->Finish();
    }

   UpdateCurrentContextResourcesRef(currentContext, false);

    if(eglContext && eglContext->MakeCurrent(dpy, eglDrawSurface, eglReadSurface) != EGL_TRUE) {
        return EGL_FALSE;
    }

    // mark as not context the last context
    if(currentContext != nullptr) {
        currentContext->SetNotCurrent();
    }

    SetCurrentContext(mCurrentAPI, eglContext);

    currentContext = GetCurrentContext();
    UpdateCurrentContextResourcesRef(currentContext, true);

    eglDriver->SetActiveContext(currentContext);

    // clean any marked resources that may have been released after the current call to MakeCurrent
    eglDriver->CleanMarkedResources();

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
