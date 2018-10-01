#ifndef __COMMON_H_
#define __COMMON_H_

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <android/log.h>

#define GLOVE_LOG_WARN(...) __android_log_print(ANDROID_LOG_WARN, "GLOVE_Demo", __VA_ARGS__)
#define GLOVE_LOG_ERROR(...)  __android_log_print(ANDROID_LOG_ERROR, "GLOVE_Demo", __VA_ARGS__)

#define ASSERT_NO_GL_ERROR() { GLenum err = glGetError(); assert(err == GL_NO_ERROR); }

static const float cube_vertex_buffer_data[] = {
    -1.0f, 1.0f,-1.0f, 1.0f,  // Left side - triangle 1
    -1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f, 1.0f,

    -1.0f,-1.0f, 1.0f, 1.0f,  // Left side - triangle 2
    -1.0f,-1.0f,-1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f, 1.0f,

    -1.0f, 1.0f,-1.0f, 1.0f,  // Back side - triangle 1
     1.0f,-1.0f,-1.0f, 1.0f,
     1.0f, 1.0f,-1.0f, 1.0f,

    -1.0f, 1.0f,-1.0f, 1.0f,  // Back side - triangle 2
    -1.0f,-1.0f,-1.0f, 1.0f,
     1.0f,-1.0f,-1.0f, 1.0f,

    -1.0f, 1.0f,-1.0f, 1.0f,  // Top side - triangle 1
     1.0f, 1.0f,-1.0f, 1.0f,
     1.0f, 1.0f, 1.0f, 1.0f,

    -1.0f, 1.0f,-1.0f, 1.0f,  // Top side - triangle 2
     1.0f, 1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f, 1.0f,

    -1.0f,-1.0f,-1.0f, 1.0f,  // Bottom side - triangle 1
    -1.0f,-1.0f, 1.0f, 1.0f,
     1.0f,-1.0f, 1.0f, 1.0f,

    -1.0f,-1.0f,-1.0f, 1.0f,  // Bottom side - triangle 2
     1.0f,-1.0f, 1.0f, 1.0f,
     1.0f,-1.0f,-1.0f, 1.0f,

     1.0f,-1.0f,-1.0f, 1.0f,  // Right side - triangle 1
     1.0f,-1.0f, 1.0f, 1.0f,
     1.0f, 1.0f, 1.0f, 1.0f,

     1.0f, 1.0f, 1.0f, 1.0f,  // Right side - triangle 2
     1.0f, 1.0f,-1.0f, 1.0f,
     1.0f,-1.0f,-1.0f, 1.0f,

    -1.0f,-1.0f, 1.0f, 1.0f,  // Front side - triangle 1
    -1.0f, 1.0f, 1.0f, 1.0f,
     1.0f,-1.0f, 1.0f, 1.0f,

    -1.0f, 1.0f, 1.0f, 1.0f,  // Front side - triangle 2
     1.0f, 1.0f, 1.0f, 1.0f,
     1.0f,-1.0f, 1.0f, 1.0f,
};

static const float cube_uv_buffer_data[] = {
    0.0f, 1.0f,  // Left side - triangle 1
    1.0f, 1.0f,
    1.0f, 0.0f,

    1.0f, 0.0f,  // Left side - triangle 2
    0.0f, 0.0f,
    0.0f, 1.0f,

    1.0f, 1.0f,  // Back side - triangle 1
    0.0f, 0.0f,
    0.0f, 1.0f,

    1.0f, 1.0f,  // Back side - triangle 2
    1.0f, 0.0f,
    0.0f, 0.0f,

    0.0f, 0.0f,  // Top side - triangle 1
    0.0f, 1.0f,
    1.0f, 1.0f,

    0.0f, 0.0f,  // Top side - triangle 2
    1.0f, 1.0f,
    1.0f, 0.0f,

    0.0f, 1.0f,  // Bottom side - triangle 1
    1.0f, 1.0f,
    1.0f, 0.0f,

    0.0f, 1.0f,  // Bottom side - triangle 2
    1.0f, 0.0f,
    0.0f, 0.0f,

    1.0f, 0.0f,  // Right side - triangle 1
    0.0f, 0.0f,
    0.0f, 1.0f,

    0.0f, 1.0f,  // Right side - triangle 2
    1.0f, 1.0f,
    1.0f, 0.0f,

    0.0f, 0.0f,  // Front side - triangle 1
    0.0f, 1.0f,
    1.0f, 0.0f,

    0.0f, 1.0f,  // Front side - triangle 2
    1.0f, 1.0f,
    1.0f, 0.0f,
};

static const GLfloat cube_color_buffer_data[] = {
    1.0f,  0.0f,  0.0f,         // LEFT
    0.0f,  1.0f,  0.0f,
    0.0f,  0.0f,  1.0f,

    0.0f,  0.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  0.0f,  0.0f,

    1.0f,  0.0f,  0.0f,         // BACK
    0.0f,  1.0f,  0.0f,
    0.0f,  0.0f,  1.0f,

    1.0f,  0.0f,  0.0f,
    1.0f,  1.0f,  1.0f,
    0.0f,  1.0f,  0.0f,

    1.0f,  0.0f,  0.0f,         // TOP
    0.0f,  1.0f,  0.0f,
    0.0f,  0.0f,  1.0f,

    1.0f,  0.0f,  0.0f,
    0.0f,  0.0f,  1.0f,
    1.0f,  1.0f,  1.0f,

    1.0f,  0.0f,  0.0f,         // BOTTOM
    0.0f,  1.0f,  0.0f,
    0.0f,  0.0f,  1.0f,

    1.0f,  0.0f,  0.0f,
    0.0f,  0.0f,  1.0f,
    1.0f,  1.0f,  1.0f,

    1.0f,  0.0f,  0.0f,         // RIGHT
    0.0f,  1.0f,  0.0f,
    0.0f,  0.0f,  1.0f,

    0.0f,  0.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  0.0f,  0.0f,

    1.0f,  0.0f,  0.0f,         // FRONT
    0.0f,  1.0f,  0.0f,
    0.0f,  0.0f,  1.0f,

    0.0f,  1.0f,  0.0f,
    1.0f,  1.0f,  1.0f,
    0.0f,  0.0f,  1.0f,
};


#define WIDTH   800
#define HEIGHT  552

#endif // __COMMON_H_
