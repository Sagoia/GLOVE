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
 *  @file       eglRefObject.h
 *  @author     Think Silicon
 *  @date       07/12/2018
 *  @version    1.0
 *
 *  @brief      EGL Reference Object. It manages lifetime properties of various EGL objects
 *
 */

#ifndef __EGL_REFOBJECT_H__
#define __EGL_REFOBJECT_H__

#include "EGL/egl.h"
#include "rendering_api/rendering_api.h"
#include "utils/eglLogger.h"
#include "eglConfig.h"
#include "vector"

class EGLRefObject
{
protected:
    bool                             misMarkedForDeletion;

public:
    int32_t                          mRefCounter;
    EGLRefObject();
    virtual ~EGLRefObject() = 0;

    void                      MarkForDeletion()                                          { FUN_ENTRY(EGL_LOG_TRACE); misMarkedForDeletion = true; }
    bool                      IsMarkedForDeletion()                                const { FUN_ENTRY(EGL_LOG_TRACE); return misMarkedForDeletion; }
    bool                      FreeForDeletion()                                    const { FUN_ENTRY(EGL_LOG_TRACE); return mRefCounter == 0; }
    virtual void              UpdateRef(bool increaseRef);
};

#endif // __EGL_REFOBJECT_H__
