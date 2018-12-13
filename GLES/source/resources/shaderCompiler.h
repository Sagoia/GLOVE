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
 *  @file       shaderCompiler.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Shader Compiler Definition
 *
 */

#ifndef __SHADERCOMPILER_H__
#define __SHADERCOMPILER_H__

#include "shaderReflection.h"

class ShaderCompiler {

public:
    ShaderCompiler() {}
    virtual ~ShaderCompiler() {}

/// Shader Functions
    virtual bool                PreprocessShader(uintptr_t program_ptr, shader_type_t shaderType, ESSL_VERSION version_in, ESSL_VERSION version_out, bool isYInverted) = 0;
    virtual bool                CompileShader(const char* const* source, shader_type_t shaderType, ESSL_VERSION version) = 0;

/// Shader Program Functions
    virtual bool                LinkProgram(uintptr_t program_ptr, ESSL_VERSION version, vector<uint32_t> &vertSpv, vector<uint32_t> &fragSpv) = 0;
    virtual bool                ValidateProgram(ESSL_VERSION version) = 0;
    
/// Reflection Functions
    virtual void                PrepareReflection(ESSL_VERSION version) = 0;
    virtual uint32_t            SerializeReflection(void* binary) = 0;
    virtual uint32_t            DeserializeReflection(const void* binary) = 0;
    
/// Print Functions
    virtual void                DumpUniforms(void) = 0;

/// Get Functions
    virtual ShaderReflection*   GetShaderReflection(void) = 0;
    virtual const char*         GetProgramInfoLog(ESSL_VERSION version) = 0;
    virtual const char*         GetShaderInfoLog(shader_type_t shaderType,
                                                 ESSL_VERSION version) = 0;

/// Enable Functions
    virtual void                EnablePrintReflection(ESSL_VERSION version) = 0;
    virtual void                EnablePrintConvertedShader(void) = 0;

    virtual void                EnableSaveBinaryToFiles(void) = 0;
    virtual void                EnableSaveSourceToFiles(void) = 0;
    virtual void                EnableSaveSpvTextToFile(void) = 0;
};

#endif // __SHADERCOMPILER_H__
