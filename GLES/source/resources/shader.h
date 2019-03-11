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
 *  @file       shader.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Shader Functionality in GLOVE
 *
 */

#ifndef __SHADER_H__
#define __SHADER_H__

#include "shaderCompiler.h"

class SlangCompiler;

class Shader {
private:
    const vulkanAPI::vkContext_t *      mVkContext;
    VkShaderModule                      mVkShaderModule;
    SlangCompiler *                     mSlangCompiler;

    char *                              mSource;
    std::vector<uint32_t>               mSpv;

    uint32_t                            mSourceLength;
    shader_type_t                       mShaderType;
    int                                 mRefCounter;
    bool                                mMarkForDeletion;
    bool                                mCompiled;

    void                                FreeSources(void);
    void                                DestroyVkShader(void);

public:
    Shader(const vulkanAPI::vkContext_t *vkContext = nullptr);
    ~Shader();

    bool                                CompileShader(void);
    VkShaderModule                      CreateVkShaderModule(void);

    void                                RefShader(void);
    void                                UnrefShader(void);

// Get Functions
    char *                              GetInfoLog(void)                        const;
    int                                 GetInfoLogLength(void)                  const;
    char *                              GetShaderSource(void)                   const;
    int                                 GetShaderSourceLength(void)             const;
    shader_type_t                       GetShaderType(void)                     const   { FUN_ENTRY(GL_LOG_TRACE); return mShaderType; }
    std::vector<uint32_t> &             GetSPV(void)                                    { FUN_ENTRY(GL_LOG_TRACE); return mSpv; }
    int                                 GetRefCount(void)                       const   { FUN_ENTRY(GL_LOG_TRACE); return mRefCounter; }
    bool                                GetMarkForDeletion(void)                const   { FUN_ENTRY(GL_LOG_TRACE); return mMarkForDeletion; }
    SlangCompiler *                     GetSlangCompiler(void)                  const   { FUN_ENTRY(GL_LOG_TRACE); return mSlangCompiler; }

// Set Functions
    void                                SetShaderSource(GLsizei count, const GLchar *const *string, const GLint *length);
    void                                SetVkContext(const
                                                    vulkanAPI::vkContext_t *vkContext)  { FUN_ENTRY(GL_LOG_TRACE); mVkContext       = vkContext; }
    void                                SetSlangCompiler(SlangCompiler *compiler)       { FUN_ENTRY(GL_LOG_TRACE); mSlangCompiler   = compiler; }
    void                                SetShaderType(shader_type_t type)               { FUN_ENTRY(GL_LOG_TRACE); mShaderType      = type; }
    void                                MarkForDeletion(void)                           { FUN_ENTRY(GL_LOG_TRACE); mMarkForDeletion = true; }

// Is/Has Functions
    bool                                IsCompiled(void)                        const   { FUN_ENTRY(GL_LOG_TRACE); return mCompiled; }
    bool                                IsVertex(void)                          const   { FUN_ENTRY(GL_LOG_TRACE); return (mShaderType == SHADER_TYPE_VERTEX) ? true : false; }
    bool                                HasSource(void)                         const   { FUN_ENTRY(GL_LOG_TRACE); return mSource != nullptr; }
};

#endif // __SHADER_H__
