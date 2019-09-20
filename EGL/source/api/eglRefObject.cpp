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
 *  @file       eglRefObject.cpp
 *  @author     Think Silicon
 *  @date       07/12/2018
 *  @version    1.0
 *
 *  @brief      EGL Reference Object. It manages lifetime properties of various EGL objects
 *
 */

#include "utils/egl_defs.h"
#include "eglRefObject.h"
#include <algorithm>

EGLRefObject::EGLRefObject():
    misMarkedForDeletion(false),
    mRefCounter(0)
{
    FUN_ENTRY(DEBUG_DEPTH);
}

EGLRefObject::~EGLRefObject()
{
    FUN_ENTRY(DEBUG_DEPTH);
}

void
EGLRefObject::UpdateRef(bool increaseRef)
{
    FUN_ENTRY(DEBUG_DEPTH);

    if(increaseRef) {
        mRefCounter++;
    } else {
        mRefCounter = std::max(mRefCounter - 1, 0);
    }
}
