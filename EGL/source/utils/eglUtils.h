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
 *  @file       eglUtils.h
 *  @author     Think Silicon
 *  @date       03/10/2018
 *  @version    1.0
 *
 *  @brief      EGL Utility Functions
 *
 */

#ifndef __EGLUTILS_H__
#define __EGLUTILS_H__

#include <cstdio>
#include "EGL/egl.h"
#include "EGL/eglext.h"

void                    setEGLVersion(EGLint *major, EGLint *minor);
const char*             getEGLClientExtensions();

#endif // __GLUTILS_H__
