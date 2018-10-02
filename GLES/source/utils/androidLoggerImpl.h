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
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      A Logger implementation for android
 *
 */

#ifndef __ANDROIDGLESLOGGERIMPL_H__
#define __ANDROIDGLESLOGGERIMPL_H__

#include "glLoggerImpl.h"
#include <android/log.h>

#define PRINTABLE_LOG_LEVEL GL_LOG_DEBUG

class AndroidGLLoggerImpl : public GLLoggerImpl {
public:
    AndroidGLLoggerImpl() { }
    ~AndroidGLLoggerImpl() { }

    void                   Initialize() override { }
    void                   Destroy() override    { }
    void                   WriteLog(glLogLevel_e level, const char *log) override
    {
        if(level >= PRINTABLE_LOG_LEVEL) {
            __android_log_print(ANDROID_LOG_WARN, "GLOVE_GL", "%s", log);
        }
    }
};

#endif //__ANDROIDGLESLOGGERIMPL_H__
