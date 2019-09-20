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
 *  @file       egl.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Entry points for the EGL API calls
 *
 */

#include "eglGlobalResourceManager.h"
#include "api/eglDisplay.h"
#include "display/displayDriver.h"
#include "utils/eglLogger.h"
#include "thread/renderingThread.h"
#include "utils/eglUtils.h"
#include "eglFunctions.h"

#ifdef DEBUG_DEPTH
#   undef DEBUG_DEPTH
#endif // DEBUG_DEPTH
#define DEBUG_DEPTH                          EGL_LOG_INFO

RenderingThread currentThread;
EGLGlobalResourceManager eglGlobalResourceManager;

#define THREAD_EXEC_RETURN(func)             FUN_ENTRY(DEBUG_DEPTH);                                                      \
                                             return currentThread.func;

#define CHECK_BAD_DISPLAY(eglDisplayPtr, dpy, erroRetValue)                                                               \
                                             EGLDisplay_t *eglDisplayPtr = eglGlobalResourceManager.FindDisplay(dpy);     \
                                             if(eglGlobalResourceManager.CheckBadDisplay(eglDisplayPtr) == EGL_FALSE)     \
                                             { return erroRetValue; }

#define CHECK_UNINITIALIZED_DISPLAY(eglDriverPtr, eglDisplayPtr, erroRetValue)                                                  \
                                             DisplayDriver *eglDriverPtr = eglGlobalResourceManager.FindDriver(eglDisplayPtr);  \
                                             if(DisplayDriver::CheckNonInitializedDisplay(eglDriver) == EGL_FALSE)              \
                                             { return erroRetValue; }

#define CHECK_BAD_CONFIG(eglDriverPtr, eglConfigPtr, eglConfig, erroRetValue)                                             \
                                             EGLConfig_t *eglConfigPtr = static_cast<EGLConfig_t*>(eglConfig);            \
                                             if(eglDriverPtr->CheckBadConfig(eglConfigPtr) == EGL_FALSE)                  \
                                             { return erroRetValue; }

#define CHECK_BAD_SURFACE(eglDriverPtr, eglSurfacePtr, eglSurface, erroRetValue)                                          \
                                             EGLSurface_t *eglSurfacePtr = static_cast<EGLSurface_t*>(eglSurface);        \
                                             if(eglDriverPtr->CheckBadSurface(eglSurfacePtr) == EGL_FALSE)                \
                                             { return erroRetValue; }

#define CHECK_BAD_CONTEXT(eglDriverPtr, eglContextPtr, eglContext, erroRetValue)                                          \
                                             EGLContext_t *eglContextPtr = static_cast<EGLContext_t*>(eglContext);        \
                                             if(eglDriverPtr->CheckBadContext(eglContextPtr) == EGL_FALSE)                \
                                             { return erroRetValue; }

EGLDisplay EGLAPIENTRY
eglGetDisplay(EGLNativeDisplayType display_id)
{
    FUN_ENTRY(DEBUG_DEPTH);

    EGLDisplay_t *eglDisplay = eglGlobalResourceManager.GetDisplayByID(display_id);
    return reinterpret_cast<EGLDisplay>(eglDisplay);
}

EGLint EGLAPIENTRY
eglGetError(void)
{
    THREAD_EXEC_RETURN(GetError());
}

EGLBoolean EGLAPIENTRY
eglBindAPI(EGLenum api)
{
    THREAD_EXEC_RETURN(BindAPI(api));
}

EGLenum EGLAPIENTRY
eglQueryAPI(void)
{
    THREAD_EXEC_RETURN(QueryAPI());
}

EGLBoolean EGLAPIENTRY
eglWaitClient(void)
{
    THREAD_EXEC_RETURN(WaitClient());
}

EGLBoolean EGLAPIENTRY
eglReleaseThread(void)
{
    THREAD_EXEC_RETURN(ReleaseThread());
}

EGLContext EGLAPIENTRY
eglGetCurrentContext(void)
{
    THREAD_EXEC_RETURN(GetCurrentContext());
}

EGLSurface EGLAPIENTRY
eglGetCurrentSurface(EGLint readdraw)
{
    THREAD_EXEC_RETURN(GetCurrentSurface(readdraw));
}

EGLDisplay EGLAPIENTRY
eglGetCurrentDisplay(void)
{
    THREAD_EXEC_RETURN(GetCurrentDisplay());
}

EGLContext EGLAPIENTRY
eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list)
{
    FUN_ENTRY(DEBUG_DEPTH);

    CHECK_BAD_DISPLAY(eglDisplay, dpy, EGL_NO_CONTEXT)
    CHECK_UNINITIALIZED_DISPLAY(eglDriver, eglDisplay, EGL_NO_CONTEXT)
    CHECK_BAD_CONFIG(eglDriver, eglConfig, config, EGL_NO_CONTEXT)
    EGLContext_t* eglShareContext = static_cast<EGLContext_t*>(share_context);
    // TODO::check for valid context
    THREAD_EXEC_RETURN(CreateContext(eglDriver, eglConfig, eglShareContext, attrib_list));
}

EGLBoolean EGLAPIENTRY
eglDestroyContext(EGLDisplay dpy, EGLContext ctx)
{
    FUN_ENTRY(DEBUG_DEPTH);

    CHECK_BAD_DISPLAY(eglDisplay, dpy, EGL_FALSE)
    CHECK_UNINITIALIZED_DISPLAY(eglDriver, eglDisplay, EGL_FALSE)
    CHECK_BAD_CONTEXT(eglDriver, eglContext, ctx, EGL_FALSE)
    THREAD_EXEC_RETURN(DestroyContext(eglDriver, eglContext));
}

EGLBoolean EGLAPIENTRY
eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx)
{
    FUN_ENTRY(DEBUG_DEPTH);

    CHECK_BAD_DISPLAY(eglDisplay, dpy, EGL_FALSE)
    DisplayDriver *eglDriver = eglGlobalResourceManager.FindDriver(eglDisplay);

    // check for non initialized display only if context/surfaces are not nullptr
    if((ctx != EGL_NO_CONTEXT || draw != EGL_NO_SURFACE || read != EGL_NO_SURFACE) &&
        DisplayDriver::CheckNonInitializedDisplay(eglDriver) == EGL_FALSE){
        return EGL_FALSE;
    }

    return currentThread.MakeCurrent(eglDriver, eglDisplay, draw, read, ctx);
}

EGLBoolean EGLAPIENTRY
eglQueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value)
{
    FUN_ENTRY(DEBUG_DEPTH);

    CHECK_BAD_DISPLAY(eglDisplay, dpy, EGL_FALSE)
    CHECK_UNINITIALIZED_DISPLAY(eglDriver, eglDisplay, EGL_FALSE)
    CHECK_BAD_CONTEXT(eglDriver, eglContext, ctx, EGL_FALSE)
    THREAD_EXEC_RETURN(QueryContext(eglDriver, eglContext, attribute, value));
}

EGLBoolean EGLAPIENTRY
eglWaitGL(void)
{
    THREAD_EXEC_RETURN(WaitGL());
}

EGLBoolean EGLAPIENTRY
eglWaitNative(EGLint engine)
{
    THREAD_EXEC_RETURN(WaitNative(engine));
}

EGLBoolean EGLAPIENTRY
eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor)
{
    FUN_ENTRY(DEBUG_DEPTH);

    CHECK_BAD_DISPLAY(eglDisplay, dpy, EGL_FALSE)
    // TODO:: EGLDisplay and DisplayDriver could be fused together, as they are 1-1
    DisplayDriver *eglDriver = eglGlobalResourceManager.AddDriver(eglDisplay);
    EGLBoolean res = eglDriver->Initialize(major, minor);
    if(eglDriver->Initialized()) {
        eglGlobalResourceManager.InitializeDisplay(dpy, eglDriver);
    }
    return res;
}

EGLBoolean EGLAPIENTRY
eglTerminate(EGLDisplay dpy)
{
    FUN_ENTRY(DEBUG_DEPTH);

    CHECK_BAD_DISPLAY(eglDisplay, dpy, EGL_FALSE)
    DisplayDriver *eglDriver = eglGlobalResourceManager.FindDriver(eglDisplay);
    if(eglDriver == nullptr || !eglDriver->Initialized()) {
        return EGL_FALSE;
    }

    // Note: eglTerminate specifies that if any display-related resources are current,
    // termination should occur later on, during eglReleaseThread or eglMakeCurrent
    // This is not yet supported and termination of these resources is forced here.
    EGLBoolean res = eglDriver->Terminate();
    eglGlobalResourceManager.RemoveDriver(eglDisplay);
    if(res == EGL_TRUE) {
       eglGlobalResourceManager.TerminateDisplay(dpy);
    }

    return res;
}

const char * EGLAPIENTRY
eglQueryString(EGLDisplay dpy, EGLint name)
{
    FUN_ENTRY(DEBUG_DEPTH);

    if(dpy == EGL_NO_DISPLAY && name == EGL_EXTENSIONS) {
        return getEGLClientExtensions();
    }

    CHECK_BAD_DISPLAY(eglDisplay, dpy, nullptr)
    CHECK_UNINITIALIZED_DISPLAY(eglDriver, eglDisplay, nullptr)

    switch(name) {
    case EGL_CLIENT_APIS:   return "OpenGL_ES\0";
    case EGL_VENDOR:        return "GLOVE (GL Over Vulkan)\0";
    case EGL_VERSION:       return "1.4\0";
    case EGL_EXTENSIONS:    return eglDriver->GetExtensions();
    default:                { currentThread.RecordError(EGL_BAD_PARAMETER); return nullptr; }
    }
}

EGLBoolean EGLAPIENTRY
eglGetConfigs(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
    FUN_ENTRY(DEBUG_DEPTH);

    CHECK_BAD_DISPLAY(eglDisplay, dpy, EGL_FALSE)
    CHECK_UNINITIALIZED_DISPLAY(eglDriver, eglDisplay, EGL_FALSE)
    return eglDriver->GetConfigs(configs, config_size, num_config);
}

EGLBoolean EGLAPIENTRY
eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
    FUN_ENTRY(DEBUG_DEPTH);

    CHECK_BAD_DISPLAY(eglDisplay, dpy, EGL_FALSE)
    CHECK_UNINITIALIZED_DISPLAY(eglDriver, eglDisplay, EGL_FALSE)
    return eglDriver->ChooseConfig(attrib_list, configs, config_size, num_config);
}

EGLBoolean EGLAPIENTRY
eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value)
{
    FUN_ENTRY(DEBUG_DEPTH);

    CHECK_BAD_DISPLAY(eglDisplay, dpy, EGL_FALSE)
    CHECK_UNINITIALIZED_DISPLAY(eglDriver, eglDisplay, EGL_FALSE)
    CHECK_BAD_CONFIG(eglDriver, eglConfig, config, EGL_FALSE)
    return eglDriver->GetConfigAttrib(eglConfig, attribute, value);
}

EGLSurface EGLAPIENTRY
eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list)
{
    FUN_ENTRY(DEBUG_DEPTH);

    CHECK_BAD_DISPLAY(eglDisplay, dpy, EGL_NO_SURFACE)
    CHECK_UNINITIALIZED_DISPLAY(eglDriver, eglDisplay, EGL_NO_SURFACE)
    CHECK_BAD_CONFIG(eglDriver, eglConfig, config, EGL_NO_SURFACE)
    return eglDriver->CreateWindowSurface(eglConfig, win, attrib_list);
}

EGLSurface EGLAPIENTRY
eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list)
{
    FUN_ENTRY(DEBUG_DEPTH);

    CHECK_BAD_DISPLAY(eglDisplay, dpy, EGL_NO_SURFACE)
    CHECK_UNINITIALIZED_DISPLAY(eglDriver, eglDisplay, EGL_NO_SURFACE)
    CHECK_BAD_CONFIG(eglDriver, eglConfig, config, EGL_NO_SURFACE)
    return eglDriver->CreatePbufferSurface(eglConfig, attrib_list);
}

EGLSurface EGLAPIENTRY
eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list)
{
    FUN_ENTRY(DEBUG_DEPTH);

    CHECK_BAD_DISPLAY(eglDisplay, dpy, EGL_NO_SURFACE)
    CHECK_UNINITIALIZED_DISPLAY(eglDriver, eglDisplay, EGL_NO_SURFACE)
    CHECK_BAD_CONFIG(eglDriver, eglConfig, config, EGL_NO_SURFACE)
    return eglDriver->CreatePixmapSurface(eglConfig, pixmap, attrib_list);
}

EGLBoolean EGLAPIENTRY
eglDestroySurface(EGLDisplay dpy, EGLSurface surface)
{
    FUN_ENTRY(DEBUG_DEPTH);

    CHECK_BAD_DISPLAY(eglDisplay, dpy, EGL_FALSE)
    CHECK_UNINITIALIZED_DISPLAY(eglDriver, eglDisplay, EGL_FALSE)
    CHECK_BAD_SURFACE(eglDriver, eglSurface, surface, EGL_FALSE)
    return eglDriver->DestroySurface(eglSurface);
}

EGLBoolean EGLAPIENTRY
eglQuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value)
{
    FUN_ENTRY(DEBUG_DEPTH);

    CHECK_BAD_DISPLAY(eglDisplay, dpy, EGL_FALSE)
    CHECK_UNINITIALIZED_DISPLAY(eglDriver, eglDisplay, EGL_FALSE)
    CHECK_BAD_SURFACE(eglDriver, eglSurface, surface, EGL_FALSE)
    return eglDriver->QuerySurface(eglSurface, attribute, value);

}

EGLSurface EGLAPIENTRY
eglCreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list)
{
    FUN_ENTRY(DEBUG_DEPTH);

    CHECK_BAD_DISPLAY(eglDisplay, dpy, EGL_NO_SURFACE)
    CHECK_UNINITIALIZED_DISPLAY(eglDriver, eglDisplay, EGL_NO_SURFACE)
    CHECK_BAD_CONFIG(eglDriver, eglConfig, config, EGL_NO_SURFACE)
    return eglDriver->CreatePbufferFromClientBuffer(buftype, buffer, eglConfig, attrib_list);
}

EGLBoolean EGLAPIENTRY
eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value)
{
    FUN_ENTRY(DEBUG_DEPTH);

    CHECK_BAD_DISPLAY(eglDisplay, dpy, EGL_FALSE)
    CHECK_UNINITIALIZED_DISPLAY(eglDriver, eglDisplay, EGL_FALSE)
    CHECK_BAD_SURFACE(eglDriver, eglSurface, surface, EGL_FALSE)
    return eglDriver->SurfaceAttrib(eglSurface, attribute, value);
}

EGLBoolean EGLAPIENTRY
eglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
    FUN_ENTRY(DEBUG_DEPTH);

    CHECK_BAD_DISPLAY(eglDisplay, dpy, EGL_FALSE)
    CHECK_UNINITIALIZED_DISPLAY(eglDriver, eglDisplay, EGL_FALSE)
    CHECK_BAD_SURFACE(eglDriver, eglSurface, surface, EGL_FALSE)
    return eglDriver->BindTexImage(eglSurface, buffer);
}

EGLBoolean EGLAPIENTRY
eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
    FUN_ENTRY(DEBUG_DEPTH);

    CHECK_BAD_DISPLAY(eglDisplay, dpy, EGL_FALSE)
    CHECK_UNINITIALIZED_DISPLAY(eglDriver, eglDisplay, EGL_FALSE)
    CHECK_BAD_SURFACE(eglDriver, eglSurface, surface, EGL_FALSE)
    return eglDriver->ReleaseTexImage(eglSurface, buffer);
}

EGLBoolean EGLAPIENTRY
eglSwapInterval(EGLDisplay dpy, EGLint interval)
{
    FUN_ENTRY(DEBUG_DEPTH);

    CHECK_BAD_DISPLAY(eglDisplay, dpy, EGL_FALSE)
    CHECK_UNINITIALIZED_DISPLAY(eglDriver, eglDisplay, EGL_FALSE)
    return eglDriver->SwapInterval(interval);
}

EGLBoolean EGLAPIENTRY
eglSwapBuffers(EGLDisplay dpy, EGLSurface surface)
{
    FUN_ENTRY(DEBUG_DEPTH);

    CHECK_BAD_DISPLAY(eglDisplay, dpy, EGL_FALSE)
    CHECK_UNINITIALIZED_DISPLAY(eglDriver, eglDisplay, EGL_FALSE)
    CHECK_BAD_SURFACE(eglDriver, eglSurface, surface, EGL_FALSE)
    return eglDriver->SwapBuffers(eglSurface);
}

EGLBoolean EGLAPIENTRY
eglCopyBuffers(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target)
{
    FUN_ENTRY(DEBUG_DEPTH);

    CHECK_BAD_DISPLAY(eglDisplay, dpy, EGL_FALSE)
    CHECK_UNINITIALIZED_DISPLAY(eglDriver, eglDisplay, EGL_FALSE)
    CHECK_BAD_SURFACE(eglDriver, eglSurface, surface, EGL_FALSE)
    return eglDriver->CopyBuffers(eglSurface, target);
}

__eglMustCastToProperFunctionPointerType EGLAPIENTRY
eglGetProcAddress(const char *procname)
{
    FUN_ENTRY(DEBUG_DEPTH);

    // get EGL function pointers
    __eglMustCastToProperFunctionPointerType fp = GetEGLProcAddr(procname);
    if(fp != nullptr || strncmp(procname, "egl", 3) == 0) {
        return fp;
    }

    // get GL function pointers
    EGLenum enumAPI = currentThread.QueryAPI();
    if(enumAPI == EGL_OPENGL_ES_API) {
        // Assuming only GLES2 for now
        rendering_api_interface_t* api = nullptr;
        rendering_api_return_e ret = RENDERING_API_load_api(EGL_OPENGL_ES_API, EGL_GL_VERSION_2, &api);
        if(ret == RENDERING_API_LOAD_SUCCESS) {
            fp = api->get_proc_addr_cb(procname);
        }
        // remove the refCounter for this call
        RENDERING_API_terminate_gles2_api();
    } else {
        NOT_IMPLEMENTED();
    }

    return fp;
}

EGLImageKHR EGLAPIENTRY
eglCreateImageKHR(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list)
{
    FUN_ENTRY(DEBUG_DEPTH);

    CHECK_BAD_DISPLAY(eglDisplay, dpy, EGL_NO_IMAGE_KHR)
    CHECK_UNINITIALIZED_DISPLAY(eglDriver, eglDisplay, EGL_NO_IMAGE_KHR)
    CHECK_BAD_CONTEXT(eglDriver, eglContext, ctx, EGL_NO_IMAGE_KHR)
    return eglDriver->CreateImageKHR(ctx, target, buffer, attrib_list);
}

EGLBoolean EGLAPIENTRY
eglDestroyImageKHR(EGLDisplay dpy, EGLImageKHR image)
{
    FUN_ENTRY(DEBUG_DEPTH);

    CHECK_BAD_DISPLAY(eglDisplay, dpy, EGL_FALSE)
    CHECK_UNINITIALIZED_DISPLAY(eglDriver, eglDisplay, EGL_FALSE)
    return eglDriver->DestroyImageKHR(image);
}

//TODO: Implement the KHR_fence_sync extension
EGLSyncKHR EGLAPIENTRY
eglCreateSyncKHR(EGLDisplay dpy, EGLenum type, const EGLint *attrib_list)
{
    FUN_ENTRY(DEBUG_DEPTH);

    CHECK_BAD_DISPLAY(eglDisplay, dpy, EGL_NO_SYNC_KHR)
    CHECK_UNINITIALIZED_DISPLAY(eglDriver, eglDisplay, EGL_NO_SYNC_KHR)
    return eglDriver->CreateSyncKHR(type, attrib_list);
}

EGLBoolean EGLAPIENTRY
eglDestroySyncKHR(EGLDisplay dpy, EGLSyncKHR sync)
{
    FUN_ENTRY(DEBUG_DEPTH);

    CHECK_BAD_DISPLAY(eglDisplay, dpy, EGL_FALSE)
    CHECK_UNINITIALIZED_DISPLAY(eglDriver, eglDisplay, EGL_FALSE)
    return eglDriver->DestroySyncKHR(sync);
}

EGLint EGLAPIENTRY
eglClientWaitSyncKHR(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags, EGLTimeKHR timeout)
{
    FUN_ENTRY(DEBUG_DEPTH);

    CHECK_BAD_DISPLAY(eglDisplay, dpy, EGL_FALSE)
    CHECK_UNINITIALIZED_DISPLAY(eglDriver, eglDisplay, EGL_FALSE)
    return eglDriver->ClientWaitSyncKHR(sync, flags, timeout);
}
