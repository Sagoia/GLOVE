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
#include "utils/parser_helpers.h"
#include "resources/slangCompiler.h"
#include "glslang/Include/ShHandle.h"
#include "glslang/Public/ShaderLang.h"
#include "SPIRV/GlslangToSpv.h"
#include "SPIRV/doc.h"
#include "SPIRV/disassemble.h"

class GlslangCompiler : public SlangCompiler {
private:
    static TBuiltInResource * msSlangShaderResources;

    string            mSource;
    glslang::TShader* mSlangShader;
    glslang::TShader* mSlangShader400;

    void CleanUpShader(glslang::TShader* shader);
    bool IsManageableError(const char* errors);
    bool IsNotFullySupported(const char* source, const char* errors);

public:
    static inline void  SetShaderResource(TBuiltInResource* resources)          { FUN_ENTRY(GL_LOG_TRACE); msSlangShaderResources = resources; }

    GlslangCompiler();
    virtual ~GlslangCompiler();

    bool CompileShader(const char* const* source, EShLanguage language);
    bool CompileShader400(const char* const* source, EShLanguage language);
    virtual bool CompileShader(const char* const* source, shader_type_t language)
                                                                                { FUN_ENTRY(GL_LOG_TRACE) return CompileShader(source, language == SHADER_TYPE_VERTEX ? EShLangVertex : EShLangFragment); }
    virtual const char* GetInfoLog();
    virtual const char* GetSource()                                      const  { FUN_ENTRY(GL_LOG_TRACE); return mSource.c_str(); }

    glslang::TShader* GetSlangShader()                                   const  { FUN_ENTRY(GL_LOG_TRACE); return mSlangShader; }
    glslang::TShader* GetSlangShader400()                                const  { FUN_ENTRY(GL_LOG_TRACE); return mSlangShader400; }
};

#endif // __GLSLANGCOMPILER_H__
