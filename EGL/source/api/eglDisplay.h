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
 *  @file       eglDisplay.h
 *  @author     Think Silicon
 *  @date       04/10/2018
 *  @version    1.0
 *
 *  @brief      Container of EGL Displays. It manages resources related to EGLDisplay.
 *
 */

#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <vector>
#include <unordered_map>
#include <stdint.h>
#include "utils/egl_defs.h"
#include "utils/eglLogger.h"
#include "EGL/egl.h"

#ifdef DEBUG_DEPTH
#   undef DEBUG_DEPTH
#endif // DEBUG_DEPTH
#define DEBUG_DEPTH                    EGL_LOG_DEBUG

#define MAX_NUM_DISPLAYS 16

typedef void* eglDisplayHandle;

struct EGLDisplay_t {
    EGLNativeDisplayType display_id;
    EGLDisplay display;
    void* displayDriver;
    bool created;
    EGLDisplay_t():
        display_id(nullptr), display(nullptr),
        displayDriver(nullptr), created(false)
    { }
};

#endif // __DISPLAY_CONTAINER_H__
