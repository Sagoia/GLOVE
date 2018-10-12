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
 *  @file       displayDriver.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      EGL Display driver module. It is responsible of communicating to the platform so as to init/terminate a display connection,
 *              create/destroy a Window Surface etc. It connects to the window platform via the abstract class PlatformWindowInterface
 *
 */

#include <vector>
#include "EGL/egl.h"
#include "displayDriver.h"
#include "api/eglConfig.h"
#include "api/eglSurface.h"
#include "utils/egl_defs.h"
#include "utils/eglUtils.h"
#include "platform/platformFactory.h"
#include <algorithm>

#ifdef VK_USE_PLATFORM_ANDROID_KHR
#include "system/window.h"
#endif
RenderingThread *callingThread = nullptr;

void setCallingThread(RenderingThread *thread) { callingThread = thread; }

DisplayDriver::DisplayDriver(void)
: mActiveContext(nullptr), mWindowInterface(nullptr),
  mInitialized(false)
{
    FUN_ENTRY(EGL_LOG_TRACE);
}

EGLBoolean DisplayDriver::CheckBadConfig(const EGLConfig_t* eglConfig) const
{
    if(eglConfig == nullptr) {
        currentThread.RecordError(EGL_BAD_CONFIG);
        return EGL_FALSE;
    }

    const auto iter = std::find(mConfigList.begin(), mConfigList.end(), eglConfig);
    if(iter == mConfigList.end()) {
        currentThread.RecordError(EGL_BAD_CONFIG);
        return EGL_FALSE;
    }
    return EGL_TRUE;
}

EGLBoolean DisplayDriver::CheckBadSurface(const EGLSurface_t *surface) const
{
    if(surface == nullptr) {
        currentThread.RecordError(EGL_BAD_SURFACE);
        return EGL_FALSE;
    }

    auto iter = std::find(mSurfaceList.begin(), mSurfaceList.end(), surface);
    if(iter == mSurfaceList.end()) {
        currentThread.RecordError(EGL_BAD_SURFACE);
        return EGL_FALSE;
    }
    return EGL_TRUE;
}

EGLBoolean
DisplayDriver::Initialize(EGLDisplay_t* dpy, EGLint *major, EGLint *minor)
{
    FUN_ENTRY(DEBUG_DEPTH);

    if(mInitialized == true) {
        setEGLVersion(major, minor);
        return EGL_TRUE;
    }

    PlatformFactory::ChoosePlatform();
    mWindowInterface = PlatformFactory::GetWindowInterface();
    if(mWindowInterface->Initialize() == EGL_FALSE) {
        currentThread.RecordError(EGL_NOT_INITIALIZED);
        return EGL_FALSE;
    }

    setEGLVersion(major, minor);

    mInitialized = true;
    return EGL_TRUE;
}

EGLBoolean
DisplayDriver::Terminate(EGLDisplay_t* dpy)
{
    FUN_ENTRY(DEBUG_DEPTH);

    if(mWindowInterface->Terminate() == EGL_FALSE) {
        return EGL_FALSE;
    }

    delete mWindowInterface;
    mWindowInterface = nullptr;

    PlatformFactory::DestroyInstance();

    mInitialized = false;

    return EGL_TRUE;
}

void DisplayDriver::UpdateConfigMap(EGLConfig_t* eglConfig)
{
    const auto iter = std::find(mConfigList.begin(), mConfigList.end(), eglConfig);
    if(iter == mConfigList.end()) {
        mConfigList.push_back(eglConfig);
    }
}

EGLBoolean
DisplayDriver::GetConfigs(EGLDisplay_t* dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
    FUN_ENTRY(DEBUG_DEPTH);

    int32_t count = 0;

    if(!num_config) {
        currentThread.RecordError(EGL_BAD_PARAMETER);
        return EGL_FALSE;
    }

    if(!configs) {
        *num_config = ARRAY_SIZE(EglConfigs);
        return EGL_TRUE;
    }

    if(config_size > count) {
        count = config_size < ARRAY_SIZE(EglConfigs) ? config_size : ARRAY_SIZE(EglConfigs);
    }

    *num_config = count;

    // set configs and update the config list
    for(int32_t i = 0; i < count; ++i) {
        configs[i] = (EGLConfig)&EglConfigs[i];
        UpdateConfigMap((EGLConfig_t*)&EglConfigs[i]);
    }

    return EGL_TRUE;
}

EGLBoolean
DisplayDriver::ChooseConfig(EGLDisplay_t* dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
    FUN_ENTRY(DEBUG_DEPTH);

    EGLConfig_t criteria;
    if(ParseConfigAttribList(&criteria, dpy, attrib_list) == EGL_FALSE) {
        currentThread.RecordError(EGL_BAD_ATTRIBUTE);
        return EGL_FALSE;
    }

    if(FilterConfigArray(reinterpret_cast<EGLConfig_t **>(configs), config_size, num_config, &criteria) == EGL_FALSE || configs == nullptr) {
        return EGL_FALSE;
    }

    // update the config list
    EGLConfig_t** matchedConfigs = reinterpret_cast<EGLConfig_t **>(configs);
    for (int i = 0; i < *num_config; ++i) {
        UpdateConfigMap(matchedConfigs[i]);
    }

    return EGL_TRUE;
}

EGLBoolean
DisplayDriver::GetConfigAttrib(EGLDisplay_t* dpy, EGLConfig_t* eglConfig, EGLint attribute, EGLint *value)
{
    FUN_ENTRY(DEBUG_DEPTH);

    if(attribute < EGL_BUFFER_SIZE || attribute > EGL_CONFORMANT) {
        currentThread.RecordError(EGL_BAD_ATTRIBUTE);
        return EGL_FALSE;
    }

    *value = GetConfigKey(eglConfig, attribute);
    return EGL_TRUE;
}

EGLSurface
DisplayDriver::CreateWindowSurface(EGLDisplay_t* dpy, EGLConfig_t* eglConfig, EGLNativeWindowType win, const EGLint *attrib_list)
{
    FUN_ENTRY(DEBUG_DEPTH);

    EGLSurface_t *eglSurface = new EGLSurface_t();
    if(!eglSurface) {
        currentThread.RecordError(EGL_BAD_ALLOC);
        return EGL_NO_SURFACE;
    }

    EGLint eglError = EGL_SUCCESS;
    if(eglSurface->InitSurface(EGL_WINDOW_BIT, eglConfig, attrib_list, &eglError) != EGL_TRUE) {
        currentThread.RecordError(eglError);
        delete eglSurface;
        return EGL_NO_SURFACE;
    }

    PlatformResources *platformResources = PlatformFactory::GetResources();
    eglSurface->SetPlatformResources(platformResources);

    if(mWindowInterface->CreateSurface(dpy, win, eglSurface) == EGL_FALSE) {
        delete eglSurface;
        return EGL_NO_SURFACE;
    }

    mWindowInterface->AllocateSurfaceImages(eglSurface);

    uint32_t imageNext = 0;
    mWindowInterface->AcquireNextImage(eglSurface, &imageNext);

    CreateEGLSurfaceInterface(eglSurface);

    mSurfaceList.push_back(eglSurface);

    return static_cast<EGLSurface>(eglSurface);
}

EGLSurface
DisplayDriver::CreatePbufferSurface(EGLDisplay_t* dpy, EGLConfig_t* eglConfig, const EGLint *attrib_list)
{
    FUN_ENTRY(DEBUG_DEPTH);

    EGLSurface_t *eglSurface = new EGLSurface_t();
    if(!eglSurface) {
        currentThread.RecordError(EGL_BAD_ALLOC);
        return EGL_NO_SURFACE;
    }

    EGLint eglError = EGL_SUCCESS;
    if(eglSurface->InitSurface(EGL_PBUFFER_BIT, eglConfig, attrib_list, &eglError) != EGL_TRUE) {
        currentThread.RecordError(eglError);
        delete eglSurface;
        return EGL_NO_SURFACE;
    }

    if(!eglSurface->GetWidth() || !eglSurface->GetHeight()) {
        eglSurface->SetWidth(EglConfigs[0].MaxPbufferWidth);
        eglSurface->SetHeight(EglConfigs[0].MaxPbufferHeight);
    }

    //TODO: Pbuffer is not supported
    NOT_IMPLEMENTED();

    return EGL_NO_SURFACE;
}

EGLSurface
DisplayDriver::CreatePixmapSurface(EGLDisplay_t* dpy, EGLConfig_t* eglConfig, EGLNativePixmapType pixmap, const EGLint *attrib_list)
{
    FUN_ENTRY(DEBUG_DEPTH);

    //TODO: Pixmap is not supported
    NOT_IMPLEMENTED();

    return EGL_NO_SURFACE;
}

void
DisplayDriver::CreateEGLSurfaceInterface(EGLSurface_t *eglSurface)
{
    FUN_ENTRY(DEBUG_DEPTH);

    EGLSurfaceInterface_t *surfaceInterface = eglSurface->GetEGLSurfaceInterface();

    memset(surfaceInterface, 0, sizeof(*surfaceInterface));

    surfaceInterface->surface = reinterpret_cast<void *>(eglSurface);
    if(eglSurface->GetType() == EGL_WINDOW_BIT) {
        surfaceInterface->images          = eglSurface->GetPlatformSurfaceImages();
        surfaceInterface->imageCount      = eglSurface->GetPlatformSurfaceImageCount();
    }
    surfaceInterface->type                = eglSurface->GetType();
    surfaceInterface->width               = eglSurface->GetWidth();
    surfaceInterface->height              = eglSurface->GetHeight();
    surfaceInterface->depthSize           = eglSurface->GetDepthSize();
    surfaceInterface->stencilSize         = eglSurface->GetStencilSize();
    surfaceInterface->surfaceColorFormat  = eglSurface->GetColorFormat();
    surfaceInterface->nextImageIndex      = eglSurface->GetCurrentImageIndex();
}

EGLBoolean
DisplayDriver::DestroySurface(EGLDisplay_t* dpy, EGLSurface_t* eglSurface)
{
    FUN_ENTRY(DEBUG_DEPTH);

    mWindowInterface->DestroySurfaceImages(eglSurface);
    mWindowInterface->DestroySurface(eglSurface);

    if(eglSurface->GetPlatformResources() != nullptr) {
        delete eglSurface->GetPlatformResources();
    }
    delete eglSurface;

    auto iter = std::find(mSurfaceList.begin(), mSurfaceList.end(), eglSurface);
    if (iter != mSurfaceList.end()) {
        mSurfaceList.erase(iter);
    }

    return EGL_TRUE;
}

EGLBoolean
DisplayDriver::QuerySurface(EGLDisplay_t* dpy, EGLSurface_t* eglSurface, EGLint attribute, EGLint *value)
{
    FUN_ENTRY(DEBUG_DEPTH);

    if(eglSurface->QuerySurface(attribute, value) == EGL_FALSE) {
        currentThread.RecordError(EGL_BAD_ATTRIBUTE);
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

EGLSurface
DisplayDriver::CreatePbufferFromClientBuffer(EGLDisplay_t* dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig_t* eglConfig, const EGLint *attrib_list)
{
    FUN_ENTRY(DEBUG_DEPTH);
    //TODO: Currently there is no support for OpenVG

    NOT_IMPLEMENTED();

    return EGL_NO_SURFACE;
}

EGLBoolean
DisplayDriver::SurfaceAttrib(EGLDisplay_t* dpy, EGLSurface_t* eglSurface, EGLint attribute, EGLint value)
{
    FUN_ENTRY(DEBUG_DEPTH);

    EGLint val = 0;
    eglSurface->QuerySurface(EGL_SURFACE_TYPE, &val);

    switch(attribute) {
    case EGL_MIPMAP_LEVEL:
        eglSurface->SetMipmapLevel(value);
        break;
    case EGL_MULTISAMPLE_RESOLVE:
        if((value == EGL_MULTISAMPLE_RESOLVE_BOX) && !(val & EGL_MULTISAMPLE_RESOLVE_BOX_BIT)) {
            currentThread.RecordError(EGL_BAD_MATCH);
            return EGL_FALSE;
        }
        eglSurface->SetMultisampleResolve(value);
        break;
    case EGL_SWAP_BEHAVIOR:
        if((value == EGL_BUFFER_PRESERVED) && !(val & EGL_SWAP_BEHAVIOR_PRESERVED_BIT)) {
            currentThread.RecordError(EGL_BAD_MATCH);
            return EGL_FALSE;
        }
   	    eglSurface->SetSwapBehavior(value);
        break;
    default:
        currentThread.RecordError(EGL_BAD_ATTRIBUTE);
        return EGL_FALSE;
    }
    //TODO: If OpenGL ES rendering is not supported by surface, generate EGL_BAD_PARAMETER error
    return EGL_TRUE;
}

EGLBoolean
DisplayDriver::BindTexImage(EGLDisplay_t* dpy, EGLSurface_t* eglSurface, EGLint buffer)
{
    FUN_ENTRY(DEBUG_DEPTH);

    EGLint textureFormat = 0;
    eglSurface->QuerySurface(EGL_TEXTURE_FORMAT, &textureFormat);
    if(textureFormat == EGL_NO_TEXTURE) {
        currentThread.RecordError(EGL_BAD_MATCH);
        return EGL_FALSE;
    }
    //TODO: EGL_BAD_ACCESS is generated if buffer is already bound to a texture.
    if (buffer != EGL_BACK_BUFFER) {
        currentThread.RecordError(EGL_BAD_MATCH);
        return EGL_FALSE;
    }
    EGLint surfacetype = eglSurface->GetType();
    if(surfacetype != EGL_PBUFFER_BIT) {
        return EGL_FALSE;
    }

    EGLint bindToTextureRGB  = eglSurface->GetBindToTextureRGB();
    EGLint bindToTextureRGBA = eglSurface->GetBindToTextureRGBA();
    if (!(bindToTextureRGB || bindToTextureRGBA)) {
        return EGL_FALSE;
    }
    //If display and surface are the display and surface for the calling thread's current context, eglBindTexImage performs an implicit glFlush
    mActiveContext->Finish();
    mActiveContext->BoundToTexture(EGL_TRUE);

    return EGL_FALSE;
}

EGLBoolean
DisplayDriver::ReleaseTexImage(EGLDisplay_t* dpy, EGLSurface_t* eglSurface, EGLint buffer)
{
    FUN_ENTRY(DEBUG_DEPTH);

    EGLint textureFormat = 0;
    eglSurface->QuerySurface(EGL_TEXTURE_FORMAT, &textureFormat);
    if(textureFormat == EGL_NO_TEXTURE) {
        currentThread.RecordError(EGL_BAD_MATCH);
        return EGL_FALSE;
    }
    if (buffer != EGL_BACK_BUFFER) {
        currentThread.RecordError(EGL_BAD_MATCH);
        return EGL_FALSE;
    }
    mActiveContext->BoundToTexture(EGL_FALSE);

    return EGL_FALSE;
}

EGLBoolean
DisplayDriver::SwapInterval(EGLDisplay_t* dpy, EGLint interval)
{
    FUN_ENTRY(DEBUG_DEPTH);

    if(currentThread.GetCurrentContext() == nullptr) {
        currentThread.RecordError(EGL_BAD_CONTEXT);
        return EGL_FALSE;
    }

    // 0 interval can be considered as supported with the current implementation
    if(!interval) {
        return EGL_TRUE;
    }

    NOT_IMPLEMENTED();

    return EGL_FALSE;
}

EGLBoolean
DisplayDriver::SwapBuffers(EGLDisplay_t* dpy, EGLSurface_t* eglSurface)
{
    FUN_ENTRY(DEBUG_DEPTH);

    if(eglSurface->GetType() != EGL_WINDOW_BIT) {
        return EGL_TRUE;
    }

    if(eglSurface->GetBoundToTexture() == EGL_TRUE) {
        return EGL_TRUE; //TODO: Is this right?
    }

    mActiveContext->Finish();

    if(mWindowInterface->PresentImage(eglSurface) == EGL_FALSE) {
        UpdateSurface(dpy, eglSurface);
    }

    uint32_t imageIndex;
    while(mWindowInterface->AcquireNextImage(eglSurface, &imageIndex) == EGL_FALSE) {
        UpdateSurface(dpy, eglSurface);
    }

    mActiveContext->SetNextImageIndex(imageIndex);

    return EGL_TRUE;
}

void
DisplayDriver::UpdateSurface(EGLDisplay_t* dpy, EGLSurface_t* eglSurface)
{
    FUN_ENTRY(DEBUG_DEPTH);

    assert(mActiveContext != nullptr);
    assert(mWindowInterface != nullptr);

    mActiveContext->Release();
    mWindowInterface->DestroySurfaceImages(eglSurface);
    mWindowInterface->AllocateSurfaceImages(eglSurface);
    CreateEGLSurfaceInterface(eglSurface);
    mActiveContext->MakeCurrent(dpy, eglSurface, eglSurface);
}

EGLBoolean
DisplayDriver::CopyBuffers(EGLDisplay_t* dpy, EGLSurface_t* eglSurface, EGLNativePixmapType target)
{
    FUN_ENTRY(DEBUG_DEPTH);

    //The implementation does not support native pixmaps
    currentThread.RecordError(EGL_BAD_NATIVE_PIXMAP);

    //TODO: EGL_BAD_MATCH is generated if the format of native_pixmap is not compatible with the color buffer of surface.
    //TODO: EGL_CONTEXT_LOST is generated if a power management event has occurred

    return EGL_FALSE;
}

EGLImageKHR
DisplayDriver::CreateImageNativeBufferAndroid(EGLDisplay_t* dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list)
{
    FUN_ENTRY(DEBUG_DEPTH);

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    (void)target;

    if(dpy == EGL_NO_DISPLAY) {
        currentThread.RecordError(EGL_BAD_DISPLAY);
        return EGL_NO_IMAGE_KHR;
    }

    if(ctx != EGL_NO_CONTEXT) {
        currentThread.RecordError(EGL_BAD_CONTEXT);
        return EGL_NO_IMAGE_KHR;
    }

    ANativeWindowBuffer *native_buffer = (ANativeWindowBuffer*)buffer;

    if(native_buffer->common.magic != ANDROID_NATIVE_BUFFER_MAGIC ||
       native_buffer->common.version != sizeof(ANativeWindowBuffer)) {
        currentThread.RecordError(EGL_BAD_PARAMETER);
        return EGL_NO_IMAGE_KHR;
    }

    switch(native_buffer->format) {
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_RGB_565:
            break;
        default:
            currentThread.RecordError(EGL_BAD_PARAMETER);
            return EGL_NO_IMAGE_KHR;
    }

    native_buffer->common.incRef(&native_buffer->common);

    return (EGLImageKHR)native_buffer;
#else
    return (EGLImageKHR)EGL_NO_IMAGE_KHR;
#endif
}

EGLImageKHR
DisplayDriver::CreateImageKHR(EGLDisplay_t* dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list)
{
    FUN_ENTRY(DEBUG_DEPTH);

    switch(target) {
        case EGL_NATIVE_BUFFER_ANDROID:
            return CreateImageNativeBufferAndroid(dpy, ctx, target, buffer, attrib_list);
        case EGL_GL_TEXTURE_2D_KHR:
        case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_X_KHR:
        case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_X_KHR:
        case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Y_KHR:
        case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_KHR:
        case EGL_GL_TEXTURE_CUBE_MAP_POSITIVE_Z_KHR:
        case EGL_GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_KHR:
        case EGL_GL_TEXTURE_3D_KHR:
        case EGL_GL_RENDERBUFFER_KHR:
          {  NOT_IMPLEMENTED();
            return EGL_NO_IMAGE_KHR;
          }
        default:
            currentThread.RecordError(EGL_BAD_PARAMETER);
            return EGL_NO_IMAGE_KHR;
    }
}

EGLBoolean
DisplayDriver::DestroyImageKHR(EGLDisplay_t* dpy, EGLImageKHR image)
{
    FUN_ENTRY(DEBUG_DEPTH);

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    if(dpy == EGL_NO_DISPLAY) {
        currentThread.RecordError(EGL_BAD_DISPLAY);
        return EGL_FALSE;
    }

    ANativeWindowBuffer* native_buffer = (ANativeWindowBuffer*)image;

    if(native_buffer->common.magic != ANDROID_NATIVE_BUFFER_MAGIC ||
       native_buffer->common.version != sizeof(ANativeWindowBuffer)) {
        currentThread.RecordError(EGL_BAD_PARAMETER);
        return EGL_FALSE;
    }

    native_buffer->common.decRef(&native_buffer->common);

    return EGL_TRUE;
#endif
    return EGL_FALSE;
}

EGLSyncKHR
DisplayDriver::CreateSyncKHR(EGLDisplay_t* dpy, EGLenum type, const EGLint *attrib_list)
{
    FUN_ENTRY(EGL_LOG_TRACE);

    return ((EGLSyncKHR)0xFE4CE000);
}

EGLBoolean
DisplayDriver::DestroySyncKHR(EGLDisplay_t* dpy, EGLSyncKHR sync)
{
    FUN_ENTRY(EGL_LOG_TRACE);

    return EGL_TRUE;
}

EGLint
DisplayDriver::ClientWaitSyncKHR(EGLDisplay_t* dpy, EGLSyncKHR sync, EGLint flags, EGLTimeKHR timeout)
{
    FUN_ENTRY(EGL_LOG_TRACE);

    return EGL_CONDITION_SATISFIED_KHR;
}

const char *DisplayDriver::GetExtensions()
{
    return "";
}

EGLBoolean
DisplayDriver::CheckNonInitializedDisplay(const DisplayDriver* displayDriver)
{
    if(displayDriver == nullptr || !displayDriver->Initialized()) {
        currentThread.RecordError(EGL_NOT_INITIALIZED);
        return EGL_FALSE;
    }
    return EGL_TRUE;
}
