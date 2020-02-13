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
 *  @file       glslangLinker.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      ESSL Shaders linker to a GL program, based on glslang TProgram
 *
 */

#include "glslangLinker.h"

GlslangLinker::GlslangLinker()
{
    FUN_ENTRY(GL_LOG_TRACE);
}

GlslangLinker::~GlslangLinker()
{
    FUN_ENTRY(GL_LOG_TRACE);

    Release();
}

void
GlslangLinker::Release()
{
    FUN_ENTRY(GL_LOG_DEBUG);

#if defined(VK_USE_PLATFORM_WIN32_KHR) && ! defined(NDEBUG)
    // HACK: Deletion of glslang::TProgram cuases exception in MSCV debug versions
    // Skip for the moment till solving the issue with glslang
#else
    for(auto shader : mProgramMap) {
        SafeDelete(shader.second);
    }
#endif
    mProgramMap.clear();
}

glslang::TProgram *
GlslangLinker::GetProgram(ESSL_VERSION version)
{ 
    FUN_ENTRY(GL_LOG_DEBUG); 
    
    return mProgramMap.find(version) != mProgramMap.end() ? mProgramMap[version] : nullptr;
}

void
GlslangLinker::GenerateSPV(std::vector<unsigned int>& spv, EShLanguage language, ESSL_VERSION version)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    spv.clear();
    glslang::GlslangToSpv(*mProgramMap[version]->getIntermediate(language), spv);
}

bool
GlslangLinker::LinkProgram(glslang::TShader* vertShader, glslang::TShader* fragShader, ESSL_VERSION version)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    SafeDelete(mProgramMap[version]);
    mProgramMap[version] = new glslang::TProgram();
    mProgramMap[version]->addShader(vertShader);
    mProgramMap[version]->addShader(fragShader);
   
    bool result = mProgramMap[version]->link(EShMsgDefault);
    if(!result) {
        GLOVE_PRINT_ERR("Program Link v%s :\n %s\n", to_string(version).c_str(), mProgramMap[version]->getInfoLog());
        return false;
    }

    return mProgramMap[version]->buildReflection(); 
}

bool
GlslangLinker::ValidateProgram(glslang::TShader* vertShader, glslang::TShader* fragShader, ESSL_VERSION version)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(!LinkProgram(vertShader, fragShader, version))
        return false;

    mIoMapResolver.Release();
    mProgramMap[version]->mapIO(&mIoMapResolver);

    return true;
}
