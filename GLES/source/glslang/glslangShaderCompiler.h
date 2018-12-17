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
 *  @file       glslangShaderCompiler.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Shaders compilation and linking module. It implements ShaderCompiler interface using glslang 
 *
 */

#ifndef __GLSLANGSHADERCOMPILER_H__
#define __GLSLANGSHADERCOMPILER_H__

#include "resources/shaderCompiler.h"
#include "shaderConverter.h"
#include "glslangCompiler.h"
#include "glslangLinker.h"
#include "resources/shaderReflection.h"
#include "resources/shaderProgram.h"

class GlslangShaderCompiler final : public ShaderCompiler {
private:

    typedef enum {
        SHADER_COMPILER_VERTEX   = 0,
        SHADER_COMPILER_FRAGMENT = 1,
        SHADER_COMPILER_TYPE_MAX
    } shader_compiler_type_t;

    static bool             mInitialized;
    static TBuiltInResource mTBuiltInResource;

    GlslangCompiler*        mShaderCompiler[SHADER_COMPILER_TYPE_MAX];
    GlslangLinker*          mProgramLinker;
    ShaderConverter*        mShaderConverter;
    ShaderReflection*       mShaderReflection;

    std::map<ESSL_VERSION, std::string[SHADER_COMPILER_TYPE_MAX]> 
                            mSourceMap;
    std::vector<uint32_t>   mSpv[SHADER_COMPILER_TYPE_MAX];

    std::map<ESSL_VERSION, bool> 
                            mPrintReflection;
    bool                    mPrintConvertedShader;
    bool                    mPrintSpv;
    bool                    mSaveBinaryToFiles;
    bool                    mSaveSourceToFiles;
    bool                    mSaveSpvTextToFile;

    /// All active uniform variables as reported by glslang
    std::vector<uniform_t>  mUniforms;

    /// uniforms that are an aggregation of basic types
    aggregateMap_t          mAggregates;

    /// All uniform blocks that will be constructed in the final source.
    /// This map associates all uniform variable names, as reported by glslang,
    /// with the corresponding uniform block that will encapsulate them in the final shader source.
    /// These variables can be any of the following types:
    /// 1. Basic types and arrays of them
    /// 2. Aggregates of basic types and arrays of them
    /// 3. Opaque types (although they can not be in a uniform block, they are stored here for easy reference)
    uniformBlockMap_t       mUniformBlocks;

/// Init Functions
    void                    InitCompiler(void);
    void                    InitCompilerResources(void);
    void                    InitReflection(void);

/// Terminate Functions
    void                    TerminateCompiler(void);

/// Release Functions
    void                    Release(void);

/// Convert Functions
    const char             *ConvertShader(uintptr_t program_ptr, shader_type_t shaderType, ESSL_VERSION version_in, ESSL_VERSION version_out, bool isYInverted);

/// In/Out File Functions
    void                    PrintReadableSPV(shader_compiler_type_t type, ESSL_VERSION version);
    void                    SaveReadableSPVToFiles(uintptr_t program_ptr, shader_compiler_type_t type, ESSL_VERSION version);
    void                    SaveBinaryToFiles(uintptr_t program_ptr, shader_compiler_type_t type, ESSL_VERSION version);
    void                    SaveShaderSourceToFile(uintptr_t program_ptr, bool processed, const char* source, shader_compiler_type_t type) const;

/// Reflection Functions (IN)
    void                    CreateUniforms(ESSL_VERSION version);
    void                    CreateUniformBlocks(void);
    aggregatePairList_t     CreateAggregates(const std::string uniformName);
    void                    LinkUniformsToUniformBlocks(void);
    void                    SetAttributesReflection(ESSL_VERSION version);

/// Reflection Functions (OUT)
    void                    UpdateUniformArraySizes(ESSL_VERSION version);
    void                    SetUniformLocations(void);
    void                    SetUniformBlocksSizes(ESSL_VERSION version);
    void                    SetUniformNestedStructSizes(ESSL_VERSION version);
    void                    SetUniformOffsets(ESSL_VERSION version);

    void                    SetUniformsReflection(void);
    
/// Reflection Functions (Print)
    void                    PrintReflection(ESSL_VERSION version) const;

public:
    GlslangShaderCompiler();
    ~GlslangShaderCompiler() override;

/// Linking Functions
    bool                     LinkProgram(uintptr_t program_ptr,
                                         ESSL_VERSION version,
                                         vector<uint32_t> &vertSpv, 
                                         vector<uint32_t> &fragSpv)           override;
    bool                     ValidateProgram(ESSL_VERSION version)            override;
    
/// Reflection Functions
    void                     PrepareReflection(ESSL_VERSION version)          override;
    uint32_t                 SerializeReflection(void* binary)                override { FUN_ENTRY(GL_LOG_TRACE); return mShaderReflection->Serialize(binary);  }
    uint32_t                 DeserializeReflection(const void* binary)        override { FUN_ENTRY(GL_LOG_TRACE); return mShaderReflection->Deserialize(binary);}

/// Shader Functions
    bool                     PreprocessShader(uintptr_t program_ptr,
                                              shader_type_t shaderType,
                                              ESSL_VERSION version_in,
                                              ESSL_VERSION version_out,
                                              bool isYInverted)               override;
    bool                     CompileShader(const char* const* source,
                                           shader_type_t shaderType,
                                           ESSL_VERSION version)              override;

/// Print Functions
    void                     PrintUniformReflection(void)                     override;

/// Get Functions
    const char              *GetProgramInfoLog(ESSL_VERSION version)          override;
    const char              *GetShaderInfoLog(shader_type_t shaderType,
                                              ESSL_VERSION  version)          override;
    inline ShaderReflection *GetShaderReflection(void)                        override { FUN_ENTRY(GL_LOG_TRACE); return mShaderReflection; }

/// Enable Functions
    inline void              EnablePrintReflection(ESSL_VERSION version)      override { FUN_ENTRY(GL_LOG_TRACE); mPrintReflection[version]   = true; }
    inline void              EnablePrintConvertedShader(void)                 override { FUN_ENTRY(GL_LOG_TRACE); mPrintConvertedShader       = true; }
    inline void              EnablePrintSpv(void)                             override { FUN_ENTRY(GL_LOG_TRACE); mPrintSpv                   = true; }
    inline void              EnableSaveBinaryToFiles(void)                    override { FUN_ENTRY(GL_LOG_TRACE); mSaveBinaryToFiles          = true; }
    inline void              EnableSaveSourceToFiles(void)                    override { FUN_ENTRY(GL_LOG_TRACE); mSaveSourceToFiles          = true; }
    inline void              EnableSaveSpvTextToFile(void)                    override { FUN_ENTRY(GL_LOG_TRACE); mSaveSpvTextToFile          = true; }
};

#endif // __GLSLANGSHADERCOMPILER_H__
