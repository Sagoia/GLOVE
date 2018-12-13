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
 *  @file       glslangCompiler.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      ESSL code compiler based on glslang TShader
 *
 */

#ifndef __GLSLANGCOMPILER_H__
#define __GLSLANGCOMPILER_H__


#include "utils/glLogger.h"
#include "glslangUtils.h"
#include "glslang/Include/ShHandle.h"
#include "glslang/Public/ShaderLang.h"
#include "SPIRV/GlslangToSpv.h"
#include "SPIRV/doc.h"
#include "SPIRV/disassemble.h"

// Treat EShMessages as bitmask
inline EShMessages operator|(EShMessages lsh, EShMessages rsh) {
    return static_cast<EShMessages>(static_cast<unsigned>(lsh) | static_cast<unsigned>(rsh));
}

class GlslangCompiler {

private:
    std::map<ESSL_VERSION, glslang::TShader *> mShaderMap;

    void                 Release(void);
    bool                 IsManageableError(const char* errors);
    bool                 IsNotFullySupported(const char* source, const char* errors);

public:
    GlslangCompiler();
    ~GlslangCompiler();

    /// Compile Functions
    bool                 CompileShader(const char* const* source, const TBuiltInResource* resources, EShLanguage language, ESSL_VERSION version);
    
    /// Get Functions
    glslang::TShader    *GetShader(ESSL_VERSION version);
    const  char         *GetCompileInfoLog(ESSL_VERSION version)     { FUN_ENTRY(GL_LOG_TRACE); return GetShader(version)->getInfoLog();}
};

#endif // __GLSLANGCOMPILER_H__
