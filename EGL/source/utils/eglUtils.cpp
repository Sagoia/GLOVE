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
 *  @file       glUtils.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      OpengGL ES Utility Functions
 *
 *  @section
 *
 *  A variety of OpengGL ES utility functions.
 *
 */

#include "eglUtils.h"
#include "egl_defs.h"
#include "eglLogger.h"

void
setEGLVersion(EGLint *major, EGLint *minor)
{
    if(major) {
        *major = EGL_VERSION_MAJOR;
    }
    if(minor) {
        *minor = EGL_VERSION_MINOR;
    }
}

const char*
getEGLClientExtensions()
{
#ifdef VK_USE_PLATFORM_ANDROID_KHR
    return "EGL_EXT_client_extensions EGL_KHR_image_base EGL_ANDROID_image_native_buffer\0"; break;
#else
    return "EGL_EXT_client_extensions\0";
#endif
}
