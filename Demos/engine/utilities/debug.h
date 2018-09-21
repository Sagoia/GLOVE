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

#ifndef __ERROR_H_
#define __ERROR_H_

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#ifdef DEBUG_OPENGLES
#define ASSERT_NO_GL_ERROR() \
{   GLenum err = glGetError(); \
    switch (err) {\
    case GL_INVALID_ENUM:\
        printf("Error GL_INVALID_ENUM. An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag.\n");\
        break;\
    case GL_INVALID_VALUE:\
        printf("Error GL_INVALID_VALUE. A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag.\n");\
        break;\
    case GL_INVALID_OPERATION:\
        printf("Error GL_INVALID_OPERATION. The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag.\n");\
        break;\
    case GL_INVALID_FRAMEBUFFER_OPERATION:\
        printf("Error GL_INVALID_FRAMEBUFFER_OPERATION. The command is trying to render to or read from the framebuffer while the currently bound framebuffer is not framebuffer complete (i.e. the return value from glCheckFramebufferStatus is not GL_FRAMEBUFFER_COMPLETE). The offending command is ignored and has no other side effect than to set the error flag.\n");\
        break;\
    case GL_OUT_OF_MEMORY:\
        printf("Error GL_OUT_OF_MEMORY. There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.\n");\
        break;\
    }\
    assert(err == GL_NO_ERROR); \
}
#else
#define ASSERT_NO_GL_ERROR()
#endif

#endif // __ERROR_H_
