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
 *  @file       androidLoggerImpl.h
 *  @author     Think Silicon
 *  @date
 *  @version    1.0
 *
 *  @brief      A Logger implementation for android
 *
 */

// #ifndef __ANDROIDLOGGERIMPL_H__
// #define __ANDROIDLOGGERIMPL_H__

#include "eglLoggerImpl.h"
#include <android/log.h>

#define PRINTABLE_LOG_LEVEL EGL_LOG_DEBUG

class AndroidEGLLoggerImpl : public EGLLoggerImpl {
public:
    AndroidEGLLoggerImpl() { }
    ~AndroidEGLLoggerImpl() { }

    void                   Initialize() override { }
    void                   Destroy() override    { }
    void                   WriteLog(eglLogLevel_e level, const char *log) override
    {
        if(level >= PRINTABLE_LOG_LEVEL) {
            __android_log_print(ANDROID_LOG_WARN, "GLOVE_EGL", "%s", log);
        }
    }
};
//#endif //__ANDROIDLOGGERIMPL_H__
