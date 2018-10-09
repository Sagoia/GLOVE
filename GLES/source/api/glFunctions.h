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
 *  @file       glFunctions.h
 *  @author     Think Silicon
 *  @date       28/09/2018
 *  @version    1.0
 *
 *  @brief      GL function pointers helper file
 *
 */

#ifndef GL_FUNC_PTR
#define GL_FUNC_PTR(f) { #f, reinterpret_cast<GLPROC>(f) }
#endif //  GL_FUNC_PTR

#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"
#include <string>
#include <unordered_map>
static const std::unordered_map<std::string, GLPROC> glFPMap = {
GL_FUNC_PTR(glActiveTexture),
GL_FUNC_PTR(glAttachShader),
GL_FUNC_PTR(glBindAttribLocation),
GL_FUNC_PTR(glBindBuffer),
GL_FUNC_PTR(glBindFramebuffer),
GL_FUNC_PTR(glBindRenderbuffer),
GL_FUNC_PTR(glBindTexture),
GL_FUNC_PTR(glBlendColor),
GL_FUNC_PTR(glBlendEquation),
GL_FUNC_PTR(glBlendEquationSeparate),
GL_FUNC_PTR(glBlendFunc),
GL_FUNC_PTR(glBlendFuncSeparate),
GL_FUNC_PTR(glBufferData),
GL_FUNC_PTR(glBufferSubData),
GL_FUNC_PTR(glCheckFramebufferStatus),
GL_FUNC_PTR(glClear),
GL_FUNC_PTR(glClearColor),
GL_FUNC_PTR(glClearDepthf),
GL_FUNC_PTR(glClearStencil),
GL_FUNC_PTR(glColorMask),
GL_FUNC_PTR(glCompileShader),
GL_FUNC_PTR(glCompressedTexImage2D),
GL_FUNC_PTR(glCompressedTexSubImage2D),
GL_FUNC_PTR(glCopyTexImage2D),
GL_FUNC_PTR(glCopyTexSubImage2D),
GL_FUNC_PTR(glCreateProgram),
GL_FUNC_PTR(glCreateShader),
GL_FUNC_PTR(glCullFace),
GL_FUNC_PTR(glDeleteBuffers),
GL_FUNC_PTR(glDeleteFramebuffers),
GL_FUNC_PTR(glDeleteProgram),
GL_FUNC_PTR(glDeleteRenderbuffers),
GL_FUNC_PTR(glDeleteShader),
GL_FUNC_PTR(glDeleteTextures),
GL_FUNC_PTR(glDepthFunc),
GL_FUNC_PTR(glDepthMask),
GL_FUNC_PTR(glDepthRangef),
GL_FUNC_PTR(glDetachShader),
GL_FUNC_PTR(glDisable),
GL_FUNC_PTR(glDisableVertexAttribArray),
GL_FUNC_PTR(glDrawArrays),
GL_FUNC_PTR(glDrawElements),
GL_FUNC_PTR(glEnable),
GL_FUNC_PTR(glEnableVertexAttribArray),
GL_FUNC_PTR(glFinish),
GL_FUNC_PTR(glFlush),
GL_FUNC_PTR(glFramebufferRenderbuffer),
GL_FUNC_PTR(glFramebufferTexture2D),
GL_FUNC_PTR(glFrontFace),
GL_FUNC_PTR(glGenBuffers),
GL_FUNC_PTR(glGenerateMipmap),
GL_FUNC_PTR(glGenFramebuffers),
GL_FUNC_PTR(glGenRenderbuffers),
GL_FUNC_PTR(glGenTextures),
GL_FUNC_PTR(glGetActiveAttrib),
GL_FUNC_PTR(glGetActiveUniform),
GL_FUNC_PTR(glGetAttachedShaders),
GL_FUNC_PTR(glGetAttribLocation),
GL_FUNC_PTR(glGetBooleanv),
GL_FUNC_PTR(glGetBufferParameteriv),
GL_FUNC_PTR(glGetError),
GL_FUNC_PTR(glGetFloatv),
GL_FUNC_PTR(glGetFramebufferAttachmentParameteriv),
GL_FUNC_PTR(glGetIntegerv),
GL_FUNC_PTR(glGetProgramiv),
GL_FUNC_PTR(glGetProgramInfoLog),
GL_FUNC_PTR(glGetRenderbufferParameteriv),
GL_FUNC_PTR(glGetShaderiv),
GL_FUNC_PTR(glGetShaderInfoLog),
GL_FUNC_PTR(glGetShaderPrecisionFormat),
GL_FUNC_PTR(glGetShaderSource),
GL_FUNC_PTR(glGetString),
GL_FUNC_PTR(glGetTexParameterfv),
GL_FUNC_PTR(glGetTexParameteriv),
GL_FUNC_PTR(glGetUniformfv),
GL_FUNC_PTR(glGetUniformiv),
GL_FUNC_PTR(glGetUniformLocation),
GL_FUNC_PTR(glGetVertexAttribfv),
GL_FUNC_PTR(glGetVertexAttribiv),
GL_FUNC_PTR(glGetVertexAttribPointerv),
GL_FUNC_PTR(glHint),
GL_FUNC_PTR(glIsBuffer),
GL_FUNC_PTR(glIsEnabled),
GL_FUNC_PTR(glIsFramebuffer),
GL_FUNC_PTR(glIsProgram),
GL_FUNC_PTR(glIsRenderbuffer),
GL_FUNC_PTR(glIsShader),
GL_FUNC_PTR(glIsTexture),
GL_FUNC_PTR(glLineWidth),
GL_FUNC_PTR(glLinkProgram),
GL_FUNC_PTR(glPixelStorei),
GL_FUNC_PTR(glPolygonOffset),
GL_FUNC_PTR(glReadPixels),
GL_FUNC_PTR(glReleaseShaderCompiler),
GL_FUNC_PTR(glRenderbufferStorage),
GL_FUNC_PTR(glSampleCoverage),
GL_FUNC_PTR(glScissor),
GL_FUNC_PTR(glShaderBinary),
GL_FUNC_PTR(glShaderSource),
GL_FUNC_PTR(glStencilFunc),
GL_FUNC_PTR(glStencilFuncSeparate),
GL_FUNC_PTR(glStencilMask),
GL_FUNC_PTR(glStencilMaskSeparate),
GL_FUNC_PTR(glStencilOp),
GL_FUNC_PTR(glStencilOpSeparate),
GL_FUNC_PTR(glTexImage2D),
GL_FUNC_PTR(glTexParameterf),
GL_FUNC_PTR(glTexParameterfv),
GL_FUNC_PTR(glTexParameteri),
GL_FUNC_PTR(glTexParameteriv),
GL_FUNC_PTR(glTexSubImage2D),
GL_FUNC_PTR(glUniform1f),
GL_FUNC_PTR(glUniform1fv),
GL_FUNC_PTR(glUniform1i),
GL_FUNC_PTR(glUniform1iv),
GL_FUNC_PTR(glUniform2f),
GL_FUNC_PTR(glUniform2fv),
GL_FUNC_PTR(glUniform2i),
GL_FUNC_PTR(glUniform2iv),
GL_FUNC_PTR(glUniform3f),
GL_FUNC_PTR(glUniform3fv),
GL_FUNC_PTR(glUniform3i),
GL_FUNC_PTR(glUniform3iv),
GL_FUNC_PTR(glUniform4f),
GL_FUNC_PTR(glUniform4fv),
GL_FUNC_PTR(glUniform4i),
GL_FUNC_PTR(glUniform4iv),
GL_FUNC_PTR(glUniformMatrix2fv),
GL_FUNC_PTR(glUniformMatrix3fv),
GL_FUNC_PTR(glUniformMatrix4fv),
GL_FUNC_PTR(glUseProgram),
GL_FUNC_PTR(glValidateProgram),
GL_FUNC_PTR(glVertexAttrib1f),
GL_FUNC_PTR(glVertexAttrib1fv),
GL_FUNC_PTR(glVertexAttrib2f),
GL_FUNC_PTR(glVertexAttrib2fv),
GL_FUNC_PTR(glVertexAttrib3f),
GL_FUNC_PTR(glVertexAttrib3fv),
GL_FUNC_PTR(glVertexAttrib4f),
GL_FUNC_PTR(glVertexAttrib4fv),
GL_FUNC_PTR(glVertexAttribPointer),
GL_FUNC_PTR(glViewport)
#ifdef GL_OES_EGL_image
,GL_FUNC_PTR(glEGLImageTargetTexture2DOES),
GL_FUNC_PTR(glEGLImageTargetRenderbufferStorageOES)
#endif // GL_OES_EGL_image
#ifdef GL_EXT_debug_marker
,GL_FUNC_PTR(glInsertEventMarkerEXT),
GL_FUNC_PTR(glPushGroupMarkerEXT),
GL_FUNC_PTR(glPopGroupMarkerEXT)
#endif // GL_EXT_debug_marker
#ifdef GL_OES_get_program_binary
,GL_FUNC_PTR(glGetProgramBinaryOES),
GL_FUNC_PTR(glProgramBinaryOES)
#endif /* GL_OES_get_program_binary */
};
#undef GL_FUNC_PTR

GLPROC GetGLProcAddr(const char *procname)
{
    if(strncmp(procname, "gl", 2) == 0) {
        const auto& it = glFPMap.find(procname);
        if(it != glFPMap.end()) {
            return it->second;
        }
    }
    return nullptr;
}
