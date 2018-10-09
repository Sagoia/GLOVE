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
 *  @file       glLogger.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      A Logging Library
 *
 *  @section
 *
 *  glLogger is an easy-to-use monitor interface that can be used for
 *  developing, debugging and running OpenGL ES applications.
 *
 */

#include "glLogger.h"

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    #include "androidLoggerImpl.h"
#else
    #include "simpleLoggerImpl.h"
#endif //VK_USE_PLATFORM_ANDROID_KHR
#include <string.h>

GLLogger *GLLogger::mInstance = nullptr;
GLLoggerImpl *GLLogger::mLoggerImpl = nullptr;

GLLogger::GLLogger()
{
    mLoggerImpl->Initialize();
}

GLLogger::~GLLogger()
{
    mLoggerImpl->Destroy();
}

void
GLLogger::SetLoggerImpl()
{
    if(!mLoggerImpl) {
        #ifdef VK_USE_PLATFORM_ANDROID_KHR
            mLoggerImpl = new AndroidGLLoggerImpl();
        #else
            mLoggerImpl = new SimpleLoggerImpl();
        #endif //VK_USE_PLATFORM_ANDROID_KHR
    }
}

GLLogger *
GLLogger::GetInstance()
{
    SetLoggerImpl();

    if(!mInstance) {
        mInstance = new GLLogger();
    }

    return mInstance;
}

void
GLLogger::DestroyInstance()
{
    if(mInstance) {
        delete mInstance;
        mInstance = nullptr;
    }

    if(mLoggerImpl) {
        delete mLoggerImpl;
        mLoggerImpl = nullptr;
    }
}

int
GLLogger::CalculateSpacesBefore(glLogLevel_e level)
{
    return ( 8 - (int)level*4 );
}

int
GLLogger::CalculateSpacesAfter(const char *func, glLogLevel_e level)
{
    return  funcWidth - (int)strlen(func)  - 8 + (int)level*4;
}

void
GLLogger::WriteFunEntry(glLogLevel_e level, const char* filename, const char *func, int line)
{
    char log[200];
    snprintf(log, 200, "%*s%s()%*s  [ %s: %d  Context: c1 ]", CalculateSpacesBefore(level), "", func, CalculateSpacesAfter(func, level), "", filename, line);
    mLoggerImpl->WriteLog(level, log);
}

void
GLLogger::FunEntry(glLogLevel_e level, const char* filename, const char *func, int line)
{
    GLLogger *glLogger = GLLogger::GetInstance();
    glLogger->WriteFunEntry(level, filename, func, line);
}

void
GLLogger::Log(glLogLevel_e level, const char *format, ... )
{
    char log[200];
    va_list args;
    va_start(args, format);
    vsnprintf(log, 200, format, args);
    va_end(args);
    mLoggerImpl->WriteLog(level, log);
}

void
GLLogger::Shutdown()
{
    GLLogger::DestroyInstance();
}
