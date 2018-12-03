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
 *  @file       glslangCompiler.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      ESSL code compiler based on glslang TShader
 *
 */

#include "glslangCompiler.h"

GlslangCompiler::GlslangCompiler()
{
    FUN_ENTRY(GL_LOG_TRACE);
}

GlslangCompiler::~GlslangCompiler()
{
    FUN_ENTRY(GL_LOG_TRACE);

    Release();
}

void
GlslangCompiler::Release()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    for(auto shader : mShaderMap) {
        SafeDelete(shader.second);
    }
    mShaderMap.clear();
}

bool
GlslangCompiler::IsManageableError(const char* errors)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    size_t pragmaError  = string(errors).find("\"on\" or \"off\" expected after '(' for 'debug' pragma");
    size_t definedError = string(errors).find("cannot use in preprocessor expression when expanded from macros");

    return pragmaError != string::npos || definedError != string::npos;
}

bool
GlslangCompiler::IsNotFullySupported(const char* source, const char* errors)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    size_t nonConstError = string(errors).find("'non-constant initializer' : not supported with this profile: es");
    size_t matrixCompMult = string(source).find("matrixCompMult(");
    size_t mod = string(source).find("mod(");

    return nonConstError != string::npos && (matrixCompMult != string::npos || mod != string::npos);
}

glslang::TShader *
GlslangCompiler::GetShader(ESSL_VERSION version)
{ 
    FUN_ENTRY(GL_LOG_DEBUG); 
    
    return mShaderMap.find(version) != mShaderMap.end() ? mShaderMap[version] : nullptr;
}

bool
GlslangCompiler::CompileShader(const char* const* source, const TBuiltInResource* resources, EShLanguage language, ESSL_VERSION version)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    EShMessages messages = (version == ESSL_VERSION_100) ? EShMsgDefault : (EShMessages)(EShMsgVulkanRules | EShMsgSpvRules);
    EProfile    profile  = (version == ESSL_VERSION_100) ? EEsProfile    : ENoProfile;

    SafeDelete(mShaderMap[version]);
    mShaderMap[version] = new glslang::TShader(language);
    mShaderMap[version]->setStrings(source, 1);

    bool result = mShaderMap[version]->parse(resources, version, profile, false, false, messages);
    if(!result) {
        if(IsManageableError(mShaderMap[version]->getInfoLog()) || IsNotFullySupported(*source, mShaderMap[version]->getInfoLog()) ) {
            SafeDelete(mShaderMap[version]);
            mShaderMap[version] = new glslang::TShader(language);
            mShaderMap[version]->setStrings(source, 1);
            messages = static_cast<EShMessages>(EShMsgOnlyPreprocessor | EShMsgRelaxedErrors);
            if(version == ESSL_VERSION_400) {
                messages = static_cast<EShMessages>(messages | EShMsgVulkanRules | EShMsgSpvRules);
            }
            result = mShaderMap[version]->parse(resources, version, profile, false, false, messages);
        }

        GLOVE_PRINT_ERR("%s Shader Compiler v%s : %s\n", language == EShLangVertex ? "Vertex" : "Fragment", 
                                                         to_string(version) , mShaderMap[version]->getInfoLog());
    }

    return result;
}