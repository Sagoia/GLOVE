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
 *  @file       shaderConverter.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      ESSL code converter from an ESSL version to another. Currently supports ESSL 100 to ESSL 400.
 *
 */

#ifndef __SHADER_CONVERTER_H__
#define __SHADER_CONVERTER_H__

#include "resources/shaderReflection.h"
#include "glslang/Include/ShHandle.h"
#include "glslangIoMapResolver.h"
#include "utils/parser_helpers.h"
#include "glslangUtils.h"

class ShaderConverter {
public:
    ShaderConverter();
    ~ShaderConverter();

           void Initialize(shader_type_t shaderType, ESSL_VERSION version_in, ESSL_VERSION version_out);
           void Convert(string& source, const uniformBlockMap_t &uniformBlockMap, ShaderReflection* reflection, bool isYInverted);

/// Set Functions
    inline void SetProgram(glslang::TProgram* slangProgram)                { FUN_ENTRY(GL_LOG_TRACE); mSlangProg     = slangProgram;  }
    inline void SetIoMapResolver(GlslangIoMapResolver *ioMapResolver)      { FUN_ENTRY(GL_LOG_TRACE); mIoMapResolver = ioMapResolver; }

private:
    typedef enum {
        INVALID_SHADER_CONVERSION,
        SHADER_CONVERSION_100_400
    } shader_conversion_type_t;

    static const char * const   shaderVersion;
    static const char * const   shaderExtensions;
    static const char * const   shaderPrecision;
    static const char * const   shaderTexture2d;
    static const char * const   shaderTextureCube;
    static const char * const   shaderDepthRange;
    static const char * const   shaderLimitsBuiltIns;

    shader_conversion_type_t    mConversionType;
    shader_type_t               mShaderType;
    string                      mMemLayoutQualifier;
    glslang::TProgram*          mSlangProg;
    GlslangIoMapResolver       *mIoMapResolver;

/// Process Functions
    void ProcessMacros(std::string& source);
    void ProcessHeader(string& source, const uniformBlockMap_t &uniformBlockMap);
    void ProcessUniforms(string& source, const uniformBlockMap_t &uniformBlockMap);
    void ProcessInvariantQualifier(std::string& source);
    void ProcessVaryings(string& source);
    void ProcessVertexAttributes(string& source, ShaderReflection* reflection);

/// Convert Functions
    void Convert100To400(string& source, const uniformBlockMap_t &uniformBlockMap, ShaderReflection* reflection, bool isYInverted);
    void ConvertGLToVulkanCoordSystem(string& source);
    void ConvertGLToVulkanDepthRange(string& source);

    shader_conversion_type_t EsslVersionToShaderConversionType(ESSL_VERSION version_in, 
                                                               ESSL_VERSION version_out);

};

#endif // __SHADER_CONVERTER_H__
