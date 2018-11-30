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
 *  @file       glslangLinker.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      ESSL Shaders linker to a GL program, based on glslang TProgram
 *
 */

#ifndef __GLSLANGLINKER_H__
#define __GLSLANGLINKER_H__

#include "utils/glLogger.h"
#include "glslangUtils.h"
#include "glslang/Include/ShHandle.h"
#include "glslang/Public/ShaderLang.h"
#include "SPIRV/GlslangToSpv.h"
#include "SPIRV/doc.h"
#include "SPIRV/disassemble.h"
#include "glslangIoMapResolver.h"

class GlslangLinker {
private:
    
    std::map<ESSL_VERSION, glslang::TProgram *> mProgramMap;
    GlslangIoMapResolver                        mIoMapResolver;

public:
    GlslangLinker();
    ~GlslangLinker();

// Link/Validate Functions
    bool                                LinkProgram    (glslang::TShader* vertShader, glslang::TShader* fragShader, ESSL_VERSION version);
    bool                                ValidateProgram(glslang::TShader* vertShader, glslang::TShader* fragShader, ESSL_VERSION version);

// Generate Functions
    void                                GenerateSPV(std::vector<unsigned int>& spv, EShLanguage language, ESSL_VERSION version);

// Get Functions
    inline GlslangIoMapResolver        *GetIoMapResolver(void)                       { FUN_ENTRY(GL_LOG_TRACE); return &mIoMapResolver; }
           glslang::TProgram           *GetProgram(ESSL_VERSION version);
           const char                  *GetLinkInfoLog(ESSL_VERSION version)         { FUN_ENTRY(GL_LOG_TRACE); return GetProgram(version)->getInfoLog(); } 
};

#endif // __GLSLANGLINKER_H__
