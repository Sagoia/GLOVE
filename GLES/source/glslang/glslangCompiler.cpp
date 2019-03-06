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
#include "utils/glLogger.h"

TBuiltInResource * GlslangCompiler::msSlangShaderResources = nullptr;

GlslangCompiler::GlslangCompiler()
: mSlangShader(nullptr), mSlangShader400(nullptr)
{
    FUN_ENTRY(GL_LOG_TRACE);
}

GlslangCompiler::~GlslangCompiler()
{
    FUN_ENTRY(GL_LOG_TRACE);

    CleanUpShader(mSlangShader);
    CleanUpShader(mSlangShader400);
}

void
GlslangCompiler::CleanUpShader(glslang::TShader* shader)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(shader) {
        delete shader;
        shader = nullptr;
    }
}

bool
GlslangCompiler::IsManageableError(const char* errors)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    size_t pragmaError = std::string(errors).find("\"on\" or \"off\" expected after '(' for 'debug' pragma");
    size_t definedError = std::string(errors).find("cannot use in preprocessor expression when expanded from macros");

    return pragmaError != std::string::npos || definedError != std::string::npos;
}

bool
GlslangCompiler::IsNotFullySupported(const char* source, const char* errors)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    size_t nonConstError = std::string(errors).find("'non-constant initializer' : not supported with this profile: es");
    size_t matrixCompMult = std::string(source).find("matrixCompMult(");
    size_t mod = std::string(source).find("mod(");

    return nonConstError != std::string::npos && (matrixCompMult != std::string::npos || mod != std::string::npos);
}

bool
GlslangCompiler::CompileShader(const char* const* source, EShLanguage language)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    TBuiltInResource* resources = msSlangShaderResources;

    mSource = std::string(*source);

    CleanUpShader(mSlangShader);

    mSlangShader = new glslang::TShader(language);
    assert(mSlangShader);

    mSlangShader->setStrings(source, 1);

    bool result = mSlangShader->parse(resources, 100, ENoProfile, false, false, static_cast<EShMessages>(EShMsgDefault));
    if(!result) {
        if(IsManageableError(mSlangShader->getInfoLog()) || IsNotFullySupported(*source, mSlangShader->getInfoLog())) {
            CleanUpShader(mSlangShader);
            mSlangShader = new glslang::TShader(language);
            mSlangShader->setStrings(source, 1);
            result = mSlangShader->parse(resources, 100, ENoProfile, false, false, static_cast<EShMessages>(EShMsgOnlyPreprocessor | EShMsgRelaxedErrors));
        }
        GLOVE_PRINT_ERR("shader 100:\n%s\n", mSlangShader->getInfoLog());
    }

    return result;
}

bool
GlslangCompiler::CompileShader400(const char* const* source, EShLanguage language)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    TBuiltInResource* resources = msSlangShaderResources;

    CleanUpShader(mSlangShader400);

    mSlangShader400 = new glslang::TShader(language);
    assert(mSlangShader400);

    mSlangShader400->setStrings(source, 1);
    bool result = mSlangShader400->parse(resources, 400, EEsProfile, false, false, static_cast<EShMessages>(EShMsgVulkanRules | EShMsgSpvRules));
    if(!result) {
        if(IsManageableError(mSlangShader400->getInfoLog())) {
            CleanUpShader(mSlangShader400);
            mSlangShader400 = new glslang::TShader(language);
            mSlangShader400->setStrings(source, 1);
            result = mSlangShader400->parse(resources, 400, EEsProfile, false, false, static_cast<EShMessages>(EShMsgVulkanRules | EShMsgSpvRules | EShMsgOnlyPreprocessor | EShMsgRelaxedErrors));
        }
        GLOVE_PRINT_ERR("shader 400:\n%s\n", mSlangShader400->getInfoLog());
        GLOVE_PRINT_ERR("shader Source:\n%s\n", *source);
    }

    return result;
}

const char*
GlslangCompiler::GetInfoLog()
{
    FUN_ENTRY(GL_LOG_TRACE);

    return mSlangShader->getInfoLog();
}
