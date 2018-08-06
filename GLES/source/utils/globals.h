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
 *  @file       globals.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      A header file that includes several global variables.
 *
 */

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include <map>
#include <memory>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string.h>
#include <assert.h>

#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"
#include "vulkan/vulkan.h"
#include "rendering_api_interface.h"
#include "glLogger.h"

using namespace std;

// Compiler
#ifdef __GNUC__
#   define ASSERT_ONLY                                  __attribute__((unused))
#   define COMPILER_WARN_UNUSED_RESULT                  __attribute__((warn_unused_result))
#   define FORCE_INLINE                                 __attribute__((always_inline))
#else
#   define ASSERT_ONLY
#   define COMPILER_WARN_UNUSED_RESULT
#endif // __GNUC__

// GL ES Limits
#define GLOVE_MAX_VERTEX_ATTRIBS                        32    // MIN VALUE:   8
#define GLOVE_MAX_VARYING_VECTORS                       8     // MIN VALUE:   8
#define GLOVE_MAX_VERTEX_UNIFORM_VECTORS                128   // MIN VALUE: 128
#define GLOVE_MAX_FRAGMENT_UNIFORM_VECTORS              128   // MIN VALUE:  16
#define GLOVE_MAX_COMBINED_UNIFORM_VECTORS              GLOVE_MAX_VERTEX_UNIFORM_VECTORS     + GLOVE_MAX_FRAGMENT_UNIFORM_VECTORS

#define GLOVE_MAX_VERTEX_TEXTURE_IMAGE_UNITS            8     // MIN VALUE:  0
#define GLOVE_MAX_TEXTURE_IMAGE_UNITS                   32    // MIN VALUE:  8
#define GLOVE_MAX_COMBINED_TEXTURE_IMAGE_UNITS          GLOVE_MAX_VERTEX_TEXTURE_IMAGE_UNITS + GLOVE_MAX_TEXTURE_IMAGE_UNITS

#define GLOVE_MAX_DRAW_BUFFERS                          4     // MIN VALUE:  1

#define GLOVE_MAX_TEXTURE_SIZE                          4096
#define GLOVE_MAX_CUBE_MAP_TEXTURE_SIZE                 4096
#define GLOVE_MAX_RENDERBUFFER_SIZE                     4096

#define GLOVE_NUM_SHADER_BINARY_FORMATS                 0
#define GLOVE_NUM_PROGRAM_BINARY_FORMATS                1

/// Global switches
#define GLOVE_SAVE_SHADER_SOURCES_TO_FILES              false
#define GLOVE_SAVE_PROCESSED_SHADER_SOURCES_TO_FILES    false
#define GLOVE_SAVE_SPIRV_BINARY_TO_FILES                false
#define GLOVE_SAVE_SPIRV_TEXT_TO_FILE                   false

#define GLOVE_SAVE_READPIXELS_TO_FILE                   false
#define GLOVE_SAVE_TEXTURES_TO_FILE                     false

#define GLOVE_DUMP_INPUT_SHADER_REFLECTION              false

#ifdef TRACE_BUILD
#define GLOVE_DUMP_VULKAN_SHADER_REFLECTION             true
#else
#define GLOVE_DUMP_VULKAN_SHADER_REFLECTION             false
#endif

#define GLOVE_DUMP_ORIGINAL_SHADER_SOURCE               false
#define GLOVE_DUMP_PROCESSED_SHADER_SOURCE              false

#define GLOVE_INVALID_OFFSET                            UINT32_MAX

#ifndef GLOVE_EGL_SUPPORT_ONLY_PBUFFER_SURFACE
#   define GLOVE_EGL_SUPPORT_ONLY_PBUFFER_SURFACE       0
#else
#   undef GLOVE_EGL_SUPPORT_ONLY_PBUFFER_SURFACE
#   define GLOVE_EGL_SUPPORT_ONLY_PBUFFER_SURFACE       1
#endif // GLOVE_EGL_SUPPORT_ONLY_PBUFFER_SURFACE

#define VULKAN_DEPTH_RANGE                              vulkan_DepthRange

// TODO : remove
class Context;
class CommandBufferManager;

#endif // __GLOBALS_H__
