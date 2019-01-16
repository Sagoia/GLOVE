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
 *  @file       displayDriversContainer.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Container of EGL Display Drivers. It allocates a new DisplayDriver for each new EGL Display connection.
 *
 */

#ifndef __DISPLAY_DRIVERS_CONTAINER_H__
#define __DISPLAY_DRIVERS_CONTAINER_H__

#include <unordered_map>
#include <stdint.h>
#include "EGL/egl.h"
#include "displayDriver.h"

#ifdef DEBUG_DEPTH
#   undef DEBUG_DEPTH
#endif // DEBUG_DEPTH
#define DEBUG_DEPTH                    EGL_LOG_DEBUG

class DisplayDriversContainer {
private:
    using EGLDriverMap     = std::unordered_map<EGLDisplay_t*, DisplayDriver *>;

    EGLDriverMap                       mDriverMap;
    uint32_t                           mDriversNumber;
    static DisplayDriversContainer    *mInstance;

    DisplayDriversContainer();
    ~DisplayDriversContainer() {}

    DisplayDriver                     *FindDriver(EGLDisplay_t* display);
    DisplayDriver                     *GetDriver(EGLDisplay_t* display);
    DisplayDriver                     *RemoveDriver(EGLDisplay_t* display);

    static DisplayDriversContainer    *GetInstance();
    static void                        DestroyInstance();

public:
    static DisplayDriver              *FindDisplayDriver(EGLDisplay_t* display);
    static DisplayDriver              *AddDisplayDriver(EGLDisplay_t* display);
    static void                        RemoveDisplayDriver(EGLDisplay_t* display);
    static void                        Destroy();
    static EGLBoolean                  IsEmpty();

    inline uint32_t                    GetDriversNumber()                  const { FUN_ENTRY(EGL_LOG_TRACE); return mDriversNumber; }
};

#endif // __DISPLAY_DRIVERS_CONTAINER_H__
