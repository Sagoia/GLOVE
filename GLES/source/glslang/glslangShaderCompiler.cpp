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
 *  @file       glslangShaderCompiler.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Shaders compilation and linking module. It implements ShaderCompiler interface using glslang 
 *
 */

#include <vector>
#include "glslang/Include/intermediate.h"
#include "glslang/MachineIndependent/localintermediate.h"
#include "glslangShaderCompiler.h"
#include "FixSampler.h"
#include "utils/glLogger.h"
#include "utils/parser_helpers.h"

bool             GlslangShaderCompiler::mInitialized = false;
TBuiltInResource GlslangShaderCompiler::mTBuiltInResource;

GlslangShaderCompiler::GlslangShaderCompiler()
: mProgramLinker(nullptr), mShaderConverter(nullptr), mShaderReflection(nullptr),
  mPrintConvertedShader(false), mPrintSpv(false),
  mSaveBinaryToFiles(false), mSaveSourceToFiles(false), mSaveSpvTextToFile(false)
{
    FUN_ENTRY(GL_LOG_TRACE);

    mShaderCompiler[SHADER_COMPILER_VERTEX]   = nullptr;
    mShaderCompiler[SHADER_COMPILER_FRAGMENT] = nullptr;

    mPrintReflection[ESSL_VERSION_100] = false;
    mPrintReflection[ESSL_VERSION_400] = false;

    InitCompiler();
    InitReflection();
}

GlslangShaderCompiler::~GlslangShaderCompiler()
{
    FUN_ENTRY(GL_LOG_TRACE);

    TerminateCompiler();
    Release();
}

void
GlslangShaderCompiler::Release()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    SafeDelete(mShaderCompiler[SHADER_COMPILER_VERTEX]);
    SafeDelete(mShaderCompiler[SHADER_COMPILER_FRAGMENT]);
    SafeDelete(mProgramLinker);
    SafeDelete(mShaderReflection);
}

void
GlslangShaderCompiler::InitCompiler()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(!mInitialized) {
        mInitialized = glslang::InitializeProcess();
        assert(mInitialized);

        InitCompilerResources();
    }
}

void
GlslangShaderCompiler::InitCompilerResources(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mTBuiltInResource.maxVertexAttribs              = GLOVE_MAX_VERTEX_ATTRIBS;
    mTBuiltInResource.maxVaryingVectors             = GLOVE_MAX_VARYING_VECTORS;
    mTBuiltInResource.maxVertexUniformVectors       = GLOVE_MAX_VERTEX_UNIFORM_VECTORS;
    mTBuiltInResource.maxFragmentUniformVectors     = GLOVE_MAX_FRAGMENT_UNIFORM_VECTORS;
    mTBuiltInResource.maxVertexTextureImageUnits    = GLOVE_MAX_VERTEX_TEXTURE_IMAGE_UNITS;
    mTBuiltInResource.maxTextureUnits               = GLOVE_MAX_TEXTURE_IMAGE_UNITS;
    mTBuiltInResource.maxCombinedTextureImageUnits  = GLOVE_MAX_COMBINED_TEXTURE_IMAGE_UNITS;
    mTBuiltInResource.maxDrawBuffers                = GLOVE_MAX_DRAW_BUFFERS;
}

void
GlslangShaderCompiler::InitReflection()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mShaderReflection = new ShaderReflection();
}

void
GlslangShaderCompiler::TerminateCompiler()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(mInitialized) {
        glslang::FinalizeProcess();
        mInitialized = false;
    }
}

bool
GlslangShaderCompiler::CompileShader(const char* const* source, shader_type_t shaderType, ESSL_VERSION version)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    shader_compiler_type_t  type = (shaderType == SHADER_TYPE_VERTEX) ? SHADER_COMPILER_VERTEX : SHADER_COMPILER_FRAGMENT;
    EShLanguage             lang = (shaderType == SHADER_TYPE_VERTEX) ? EShLangVertex          : EShLangFragment;

    SafeDelete(mShaderCompiler[type]);
    mSourceMap[version][type] = string(*source);
    mShaderCompiler[type]     = new GlslangCompiler();
    return mShaderCompiler[type]->CompileShader(source, &mTBuiltInResource, lang, version);
}

const char*
GlslangShaderCompiler::ConvertShader(uintptr_t program_ptr, shader_type_t shaderType, ESSL_VERSION version_in, ESSL_VERSION version_out, bool isYInverted)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    shader_compiler_type_t type = (shaderType == SHADER_TYPE_VERTEX) ? SHADER_COMPILER_VERTEX : SHADER_COMPILER_FRAGMENT;
    if(mSaveSourceToFiles) {
        SaveShaderSourceToFile(program_ptr, false, mSourceMap[version_in][type].c_str(), type);
    }

    mShaderConverter = new ShaderConverter();
    mShaderConverter->Initialize(shaderType, version_in, version_out);
    mShaderConverter->SetProgram(mProgramLinker->GetProgram(version_in));
    mShaderConverter->SetIoMapResolver(mProgramLinker->GetIoMapResolver());
    mShaderConverter->Convert(mSourceMap[version_out][type], mUniformBlocks, mShaderReflection, isYInverted);

    if(mSaveSourceToFiles) {
        SaveShaderSourceToFile(program_ptr, true, mSourceMap[version_out][type].c_str(), type);
    }

    if(mPrintConvertedShader) {
        GlslPrintShaderSource(shaderType, version_out, mSourceMap[version_out][type]);
    }

    delete mShaderConverter;

    return mSourceMap[version_out][type].c_str();
}

bool
GlslangShaderCompiler::PreprocessShader(uintptr_t program_ptr, shader_type_t shaderType, ESSL_VERSION version_in, ESSL_VERSION version_out, bool isYInverted)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    shader_compiler_type_t type = (shaderType == SHADER_TYPE_VERTEX) ? SHADER_COMPILER_VERTEX : SHADER_COMPILER_FRAGMENT;
    EShLanguage            lang = (shaderType == SHADER_TYPE_VERTEX) ? EShLangVertex          : EShLangFragment;

    mSourceMap[version_out][type] = string(mSourceMap[version_in][type]);
    const char* source = ConvertShader(program_ptr, shaderType, version_in, version_out, isYInverted);
    return mShaderCompiler[type]->CompileShader(&source, &mTBuiltInResource, lang, version_out);
}

bool
GlslangShaderCompiler::LinkProgram(uintptr_t program_ptr, ESSL_VERSION version, vector<uint32_t> &vertSpv, vector<uint32_t> &fragSpv)
{
    bool result = mProgramLinker->LinkProgram(mShaderCompiler[SHADER_COMPILER_VERTEX]->GetShader(version),
                                              mShaderCompiler[SHADER_COMPILER_FRAGMENT]->GetShader(version),
                                              version);
    if(!result) {
        return false;
    }

    UpdateUniformArraySizes(version);
    SetUniformLocations();
    SetUniformBlocksSizes(version);
    SetUniformNestedStructSizes(version);
    SetUniformOffsets(version);
    SetUniformsReflection();

    mProgramLinker->GenerateSPV(vertSpv, EShLangVertex  , version);
    mProgramLinker->GenerateSPV(fragSpv, EShLangFragment, version);

    if(mSaveBinaryToFiles) {
        SaveBinaryToFiles(program_ptr, SHADER_COMPILER_VERTEX  , version);
        SaveBinaryToFiles(program_ptr, SHADER_COMPILER_FRAGMENT, version);
    }

    if(mSaveSpvTextToFile) {
        SaveReadableSPVToFiles(program_ptr, SHADER_COMPILER_VERTEX  , version);
        SaveReadableSPVToFiles(program_ptr, SHADER_COMPILER_FRAGMENT, version);
    }

    if(mPrintSpv) {
        PrintReadableSPV(SHADER_COMPILER_VERTEX  , version);
        PrintReadableSPV(SHADER_COMPILER_FRAGMENT, version);
    }

    if(mPrintReflection[version]) {
        PrintReflection(version);
    }

    if(mPrintReflection[version]) {
        PrintReflection(version);
    }

    return result;
}

bool
GlslangShaderCompiler::ValidateProgram(ESSL_VERSION version)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    SafeDelete(mProgramLinker);

    mProgramLinker = new GlslangLinker();
    bool result = mProgramLinker->ValidateProgram(mShaderCompiler[SHADER_COMPILER_VERTEX]->GetShader(version),
                                                  mShaderCompiler[SHADER_COMPILER_FRAGMENT]->GetShader(version),
                                                  version);
    if(!result) {
        return false;
    }

    if(mPrintReflection[version]) {
        PrintReflection(version);
    }

    return result;
}

void
GlslangShaderCompiler::PrintReadableSPV(shader_compiler_type_t type, ESSL_VERSION version)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    EShLanguage lang = (type == SHADER_COMPILER_VERTEX) ? EShLangVertex : EShLangFragment;

    mProgramLinker->GenerateSPV(mSpv[type], lang, version);

    if(!mSpv[type].empty()) {
        if(type == SHADER_COMPILER_VERTEX) {
            printf("\n\n------------- VERTEX SHADER SPIRV -------------\n\n");
        } else if(type == SHADER_COMPILER_FRAGMENT) {
            printf("\n\n------------ FRAGMENT SHADER SPIRV ------------\n\n");
        }
        stringstream out;
        spv::Disassemble(out, mSpv[type]);
        printf("%s", out.str().c_str());
        printf("------------------------------------------------\n\n");
    }
}

void
GlslangShaderCompiler::SaveReadableSPVToFiles(uintptr_t program_ptr, shader_compiler_type_t type, ESSL_VERSION version)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    EShLanguage lang = (type == SHADER_COMPILER_VERTEX) ? EShLangVertex : EShLangFragment;

    mProgramLinker->GenerateSPV(mSpv[type], lang, version);

    if(!mSpv[type].empty()) {
        stringstream filename;
        filename << (type == SHADER_COMPILER_VERTEX ? "vert_" : "frag_") << hex << program_ptr << ".spv.txt";

        ofstream out;
        out.open(filename.str(), ios::binary | ios::out);
        spv::Disassemble(out, mSpv[type]);
        out.close();
    }
}


void
GlslangShaderCompiler::SaveBinaryToFiles(uintptr_t program_ptr, shader_compiler_type_t type, ESSL_VERSION version)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    EShLanguage lang = (type == SHADER_COMPILER_VERTEX) ? EShLangVertex : EShLangFragment;

    if(mSpv[type].empty()) {
        mProgramLinker->GenerateSPV(mSpv[type], lang, version);
    }

    stringstream filename;
    filename << (type == SHADER_COMPILER_VERTEX ? "vert_" : "frag_") << hex << program_ptr << ".spv.bin";
    glslang::OutputSpvBin(mSpv[type], filename.str().c_str());
}

void
GlslangShaderCompiler::PrintReflection(ESSL_VERSION version) const
{
    FUN_ENTRY(GL_LOG_DEBUG);
    
    const glslang::TProgram *prog = mProgramLinker->GetProgram(version);

    printf("\n\n-------- SHADER PROGRAM REFLECTION v%d --------\n\n", (int)version);
    printf("\n[ACTIVE UNIFORMS BLOCKS] [%d]\n\n", prog->getNumLiveUniformBlocks());
    for(int i = 0; i < prog->getNumLiveUniformBlocks(); ++i) {
        printf("%s\n", prog->getUniformBlockName(i));
        printf("  hasLocation:  %d = %d\n", GetUniformHasLocation(prog, i), GetUniformLocation(prog, i));
        printf("  hasBinding:   %d = %d\n", GetUniformHasBinding(prog, i) , GetUniformBinding(prog, i));
        printf("  hasSet:       %d = %d\n", GetUniformHasSet(prog, i)     , GetUniformSet(prog, i));
        printf("  block size:   %d\n"     , prog->getUniformBlockSize(i));
    }

    printf("\n[ACTIVE UNIFORMS] [%d]\n\n", prog->getNumLiveUniformVariables());
    for(int i = 0; i < prog->getNumLiveUniformVariables(); ++i) {
        printf("%s (0x%x)\n", prog->getUniformName(i), prog->getUniformType(i));
        printf("  hasLocation:  %d = %d\n", GetUniformHasLocation(prog, i), GetUniformLocation(prog, i));
        printf("  hasBinding:   %d = %d\n", GetUniformHasBinding(prog, i) , GetUniformBinding(prog, i));
        printf("  hasSet:       %d = %d\n", GetUniformHasSet(prog, i)     , GetUniformSet(prog, i));
        printf("  block index:  %d\n", prog->getUniformBlockIndex(i));
        printf("  block offset: %d\n", prog->getUniformBufferOffset(i));
        printf("  array size:   %d\n", prog->getUniformArraySize(i));
    }

    printf("\n[ACTIVE ATTRIBUTES] [%d]\n\n", prog->getNumLiveAttributes());
    for(int i = 0; i < prog->getNumLiveAttributes(); ++i) {
        printf("%s (0x%x)\n", prog->getAttributeName(i), prog->getAttributeType(i));
        printf("  hasLocation:  %d = %d\n", GetAttributeHasLocation(prog, i), GetAttributeLocation(prog, i));
    }

    mProgramLinker->GetIoMapResolver()->PrintVaryingInfo();

    printf("-------------------------------------------------\n\n");

    const_cast<glslang::TProgram *>(prog)->dumpReflection();

    printf("-------------------------------------------------\n\n");
}

void
GlslangShaderCompiler::SaveShaderSourceToFile(uintptr_t program_ptr, bool processed, const char* source, shader_compiler_type_t type) const
{
    FUN_ENTRY(GL_LOG_DEBUG);

    string filename = static_cast<ostringstream *>(&(ostringstream().flush() << (processed ? "shader_processed" : "shader_") << hex << program_ptr))->str();
    filename = filename + string(type == SHADER_COMPILER_VERTEX ? ".vert" : ".frag");

    FILE *fp = fopen(filename.c_str(), "w");
    assert(fp);

    if(fp) {
        size_t ASSERT_ONLY bytes;
        bytes = fwrite(source, 1, strlen(source), fp);

        assert(bytes == strlen(source));

        fclose(fp);
    }
}

const char*
GlslangShaderCompiler::GetShaderInfoLog(shader_type_t shaderType, ESSL_VERSION version)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    shader_compiler_type_t type = (shaderType == SHADER_TYPE_VERTEX) ? SHADER_COMPILER_VERTEX : SHADER_COMPILER_FRAGMENT;
    return mShaderCompiler[type] ? mShaderCompiler[type]->GetCompileInfoLog(version) : "";
}

const char*
GlslangShaderCompiler::GetProgramInfoLog(ESSL_VERSION version)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    return mProgramLinker ? mProgramLinker->GetLinkInfoLog(version) : "";
}

void
GlslangShaderCompiler::UpdateUniformArraySizes(ESSL_VERSION version)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    const glslang::TProgram *prog = mProgramLinker->GetProgram(version);

    // Get uniform array size from glslang reflection [400] due to potential mismatch with glslang reflection [100]
    for(auto& uni : mUniforms) {   
        
        string uniformName = string(uni.name);
        RemoveBrackets(uniformName);

        for(int i = 0; i < prog->getNumLiveUniformVariables(); ++i) {
            string glslangName = prog->getUniformName(i);
            RemoveBrackets(glslangName);
            if(!uniformName.compare(glslangName)) {
                uni.arraySize = prog->getUniformArraySize(i);
                break;
            }
        }
    }
}

void
GlslangShaderCompiler::SetUniformLocations(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    // Since there is potential devation between 'arraySize' of [100] and [400] 
    // we have to recompute 'location' for each uniform variable
    uint32_t location = 0;
    for(auto &uni : mUniforms) {
        uni.location = location;
        location    += uni.arraySize;
    }
}

void
GlslangShaderCompiler::CreateUniformBlocks(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    /// Each unique aggregate name will be encapsulated into a uniform block
    mUniformBlocks.clear();

    uint32_t binding = 0;
    for(const auto &aggr : mAggregates) {
        if(aggr.second.isBlock) {
            mUniformBlocks[aggr.second.name] = {  aggr.second.name,                    /// Copy name for debugging
                                                  string("uni") + to_string(binding),  /// Construct uniform block's name
                                                  binding,                             /// Binding index
                                                  false,                               /// Definitely not an opaque type
                                                  0,                                   /// memorySize (not known yet)
                                                  aggr.second.arraySize,               /// ArraySize
                                                  SHADER_TYPE_INVALID,                 /// stage (not known yet)
                                                  &aggr.second
                                                };
            ++binding;
        }
    }

    /// For uniforms that do not belong to an aggregate will be encapsulated into a uniform block
    for(auto &uni : mUniforms) {
        if(!uni.aggregatePairList[0].first) {

            mUniformBlocks[uni.name] = {  uni.name,                                 /// Copy name for debugging
                                          string("uni") + to_string(binding),       /// Construct uniform block's name
                                          binding,                                  /// Binding index
                                          IsGlSampler(uni.type),                    /// true for samplers
                                          0,                                        /// memorySize (not known yet)
                                          0,                                        /// arraySize (not known yet)
                                          uni.stage,                                /// stage
                                          nullptr
                                       };
            ++binding;
        }
    }
}

void
GlslangShaderCompiler::SetUniformBlocksSizes(ESSL_VERSION version)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    const glslang::TProgram *prog = mProgramLinker->GetProgram(version);

    // Set uniform block size from glslang reflection [400] since uniform blocks do not exist in [100]
    for(int index = 0; index < prog->getNumLiveUniformBlocks(); ++index) {
        for(auto& block : mUniformBlocks) {
            if(!block.second.glslName.compare(prog->getUniformBlockName(index))) {
                block.second.memorySize = prog->getUniformBlockSize(index);
                break;
            }
        }
    }
}

void
GlslangShaderCompiler::SetUniformNestedStructSizes(ESSL_VERSION version)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    const glslang::TProgram *prog = mProgramLinker->GetProgram(version);

    // Set sizes for the nested struct size using the offsets of the glslang reflection [400]

    size_t offset, min_size, max_size, max_size_next, max_size_next_m;
    for(auto &aggr : mAggregates) {
        if(!aggr.second.isBlock) {
            min_size = UINT32_MAX;
            max_size = 0;

            // For each nested struct (e.g. '.name.' )
            string name = "." + aggr.first + ".";
            for(int i = 0; i < prog->getNumLiveUniformVariables(); ++i) {
                string glslangName = prog->getUniformName(i);
                RemoveBrackets(glslangName);

                offset = GetUniformOffset(prog, glslangName);
                
                // Find the minimum and maximum offsets
                if(glslangName.find(name) != std::string::npos) {
                    min_size = std::min(min_size, offset);
                    max_size = std::max(max_size, offset);
                }
            }

            max_size_next   = UINT32_MAX;
            max_size_next_m = max_size;
            for(int i = 0; i < prog->getNumLiveUniformVariables(); ++i) {

                string glslangName = prog->getUniformName(i);
                RemoveBrackets(glslangName);
                offset = GetUniformOffset(prog, glslangName);

                if(max_size < offset) {
                    max_size_next = std::min(max_size_next, offset);
                } else if(max_size == offset) { 
                    // if we cannot find (or does not exist) a member outsize this struct (: with an offset larger than the max of this struct)
                    // then compute the size of struct manually
                    max_size_next_m = offset + (prog->getUniformArraySize(i)-1)*GlslTypeToAllignment(prog->getUniformType(i));
                }
            }

            // Compute the total memory length of the (array of) struct
            if(max_size_next == UINT32_MAX) {
                aggr.second.memorySize = max_size_next_m*aggr.second.arraySize;
            } 
            else {
                aggr.second.memorySize = max_size_next-min_size;
            }
        }
    }
}

void
GlslangShaderCompiler::SetUniformOffsets(ESSL_VERSION version)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    const glslang::TProgram *prog = mProgramLinker->GetProgram(version);

    // Set uniform offset from glslang reflection [400]
    for(auto& block : mUniformBlocks) {

        // If uniform block does not encapsulate aggregate type then 
        // it contains only one uniform. So, we do not have to update its offset (= 0).
        if(!block.second.pAggregate) {
            continue;
        }

        int     zeroOffsetCount   = 0;
        int     blockUniformCount = 0;
        for(auto& uni : mUniforms) {
            
            if(block.second.glslName.compare(uni.pBlock->glslName)) {
                continue;
            }

            string uniformName = string(uni.name);
            if(IsBuildInUniform(uniformName)) {
                ReplaceString("gl", "vulkan", uniformName);
            } else {
                RemoveBrackets(uniformName);
            }

            /// Query uniform's offset in block from glslang           
            size_t offset = (size_t)GetUniformOffset(prog, uniformName);
            size_t aggregateSize;
            size_t aggregateIndex;

            uni.offset = offset;
            for(size_t i=0; i<uni.aggregatePairList.size(); ++i) {
                aggregateIndex   = uni.aggregatePairList[i].second > -1 ? uni.aggregatePairList[i].second : 0;
                aggregateSize    = (i==0) ? block.second.memorySize                    / (size_t)block.second.arraySize :
                                            uni.aggregatePairList[i].first->memorySize / (size_t)uni.aggregatePairList[i].first->arraySize;
                uni.offset      += aggregateIndex*aggregateSize;
            }          

            // Check if the glslang reflection erroneously returns '0' offset value to all members of the uniform struct
            ++blockUniformCount;
            if(uni.offset == 0) {
                ++zeroOffsetCount;
            }
        }

        // Compute manually the offset of each struct member
        // TODO: It currently covers simple struct cases
        if(zeroOffsetCount == blockUniformCount && blockUniformCount > 1) {
            size_t off = 0;
            for(auto& uni : mUniforms) {
                if(!block.second.glslName.compare(uni.pBlock->glslName)) {
                    uni.offset = off;
                    // if offset (of the last member) is outsize of the uniform block size 
                    // then readjust it in order to fit inside the allocated block memory
                    if(uni.offset >= block.second.memorySize) {
                        uni.offset -= GlslTypeToSize(uni.type);
                    }
                    off += GlslTypeToAllignment(uni.type);
                }
            }
        }
    }
}

void
GlslangShaderCompiler::PrepareReflection(ESSL_VERSION version)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    // Init Shader Reflection from glslang reflection [100]
    mShaderReflection->Reset();
    SetAttributesReflection(version);
    CreateUniforms(version);
    CreateUniformBlocks();
    LinkUniformsToUniformBlocks();
}

void
GlslangShaderCompiler::LinkUniformsToUniformBlocks(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    // Uniforms are initially linked with the base aggregate
    // thus we have to link them with the associated (newly created) uniform block
    string blockName;
    for(auto &uni : mUniforms) {

        if(!uni.aggregatePairList[0].first) {
            blockName = uni.name;
        } else {
            for(const auto &aggr : mAggregates) {
                if(uni.aggregatePairList[0].first == &aggr.second) {
                    blockName = aggr.second.name;
                    break;
                }
            }
        }

        uni.pBlock         = &(mUniformBlocks[blockName]);
        uni.pBlock->stage  = static_cast<shader_type_t>(uni.pBlock->stage | uni.stage);
    }
}

aggregatePairList_t
GlslangShaderCompiler::CreateAggregates(const std::string uniformName)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    size_t       dotPos1 = 0;
    size_t       dotPos2 = uniformName.find_first_of(".", dotPos1);
    vector<pair<string, int32_t>> aggregateList;
    while(dotPos2 != string::npos) {

        std::string name  = uniformName.substr(dotPos1, dotPos2-dotPos1);
        int32_t     index = RemoveBrackets(name);

        aggregateList.push_back(make_pair(name, index));

        dotPos1 = dotPos2 + 1;
        dotPos2 = uniformName.find_first_of(".", dotPos1);
    }

    aggregatePair_t     aggregatePair;
    aggregatePairList_t aggregatePairList;

    /// Register a list of aggregate pairs (i.e nested struct) and keep track of highest array index (if any)
    if(!aggregateList.empty()) {

        for(int i=0; i<(int)aggregateList.size(); ++i) {
            aggregate_t              *node   = nullptr;
            aggregateMap_t::iterator  aggrIt = mAggregates.find(aggregateList[i].first);
            if(aggrIt != mAggregates.end()) {
                
                if(aggregateList[i].second > -1 && aggrIt->second.arraySize < aggregateList[i].second + 1) {
                    aggrIt->second.arraySize = aggregateList[i].second + 1;
                }
                node = &aggrIt->second;

            } else {

                mAggregates[aggregateList[i].first] = { aggregateList[i].first,                                         /// name
                                                        0,                                                              /// memory size (unknown yet)
                                                        aggregateList[i].second > -1 ? aggregateList[i].second + 1: 1,  /// array size
                                                        i == 0 ? true : false                                           /// isBlock
                                                      };
                node = &mAggregates[aggregateList[i].first];
            }

            aggregatePair = make_pair(node, aggregateList[i].second);
            aggregatePairList.push_back(aggregatePair);
        }
    } else {
        aggregatePair = make_pair(nullptr, -1);
        aggregatePairList.push_back(aggregatePair);
    }
    
    return aggregatePairList;
}

void
GlslangShaderCompiler::CreateUniforms(ESSL_VERSION version)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    const glslang::TProgram *prog = mProgramLinker->GetProgram(version);

    // Init uniform (blocks) data [100]

    /// Reset uniforms
    mUniforms.clear();
    mAggregates.clear();

    /// NOTE: Reserve space in vector. This is very important when emplacing back uniform_t objects,
    /// as a change in vector's capacity can cause reallocation, which in turn will call uniform_t's destructor when we do not expect it
    mUniforms.reserve(prog->getNumLiveUniformVariables());
    for(int i = 0; i < prog->getNumLiveUniformVariables(); ++i) {
        
        /// Create Uniform
        const string        name      = string(prog->getUniformName(i));
        const int           arraySize = prog->getUniformArraySize(i);
        const GLenum        type      = static_cast<GLenum>(prog->getUniformType(i));
        const shader_type_t stage     = prog->getUniformStages(i) == (EShLangVertexMask | EShLangFragmentMask) ? static_cast<shader_type_t>(SHADER_TYPE_VERTEX | SHADER_TYPE_FRAGMENT) :
                                        prog->getUniformStages(i) == EShLangVertexMask ? SHADER_TYPE_VERTEX : SHADER_TYPE_FRAGMENT;

        /// Create Aggregate
        aggregatePairList_t aggregatePairList;
        if(type == GL_SAMPLER_2D || type == GL_SAMPLER_CUBE) {
            aggregatePairList.push_back(make_pair(nullptr, -1));
        } else {
            aggregatePairList = CreateAggregates(name);
        }

        mUniforms.emplace_back(name,                               /// reflectionName
                               type,                               /// type
                               GLOVE_MAX_COMBINED_UNIFORM_VECTORS, /// location (not assigned yet)
                               arraySize,                          /// arraySize
                               aggregatePairList,                  /// list of stuct pairs
                               nullptr,                            /// pBlock (not known yet)
                               stage,                              /// stage
                               0);                                 /// offset (not known yet)
    }
}

void
GlslangShaderCompiler::SetAttributesReflection(ESSL_VERSION version)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    const glslang::TProgram *prog = mProgramLinker->GetProgram(version);

    // Get Shader Reflection Attribute Data from glslang reflection [100]
    mShaderReflection->SetLiveAttributes(prog->getNumLiveAttributes());   
    for(int i = 0; i < prog->getNumLiveAttributes(); ++i) {
        int loc = GetAttributeHasLocation(prog, i) ? GetAttributeLocation(prog, i) : GLOVE_INVALID_OFFSET;
        mShaderReflection->SetAttributeName(prog->getAttributeName(i), i);
        mShaderReflection->SetAttributeType(prog->getAttributeType(i), i);
        mShaderReflection->SetAttributeLocation(static_cast<uint32_t>(loc), i);
    }
}


void
GlslangShaderCompiler::SetUniformsReflection(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    // Get Shader Reflection Uniform Data from updated uniform (blocks) data
    mShaderReflection->SetLiveUniforms(mUniforms.size());
    mShaderReflection->SetLiveUniformBlocks(mUniformBlocks.size());

    int uniformBlockIndex = 0;
    for(auto& block : mUniformBlocks) {
        mShaderReflection->SetUniformBlockGlslBlockName(block.second.glslName.c_str(), uniformBlockIndex);
        mShaderReflection->SetUniformBlockBinding(block.second.binding, uniformBlockIndex);
        mShaderReflection->SetUniformBlockBlockSize(block.second.memorySize, uniformBlockIndex);
        mShaderReflection->SetUniformBlockBlockStage(block.second.stage, uniformBlockIndex);
        mShaderReflection->SetUniformBlockOpaque(block.second.isOpaque, uniformBlockIndex);
        ++uniformBlockIndex;
    }

    int uniformIndex = 0;
    for(auto& uni : mUniforms) {
        uint32_t blockIndex = 0;
        for(auto& block : mUniformBlocks) {
            if(!block.second.glslName.compare(uni.pBlock->glslName)) {
                mShaderReflection->SetUniformReflectionName(uni.name.c_str(), uniformIndex);
                mShaderReflection->SetUniformLocation(uni.location, uniformIndex);
                mShaderReflection->SetUniformBlockIndex(blockIndex, uniformIndex);
                mShaderReflection->SetUniformArraySize(uni.arraySize, uniformIndex);
                mShaderReflection->SetUniformType(uni.type, uniformIndex);
                mShaderReflection->SetUniformOffset(uni.offset, uniformIndex);

                break;
            }
            ++blockIndex;
        }
        ++uniformIndex;
    }
}

void
GlslangShaderCompiler::PrintUniformReflection(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    printf("[ACTIVE UNIFORM BLOCKS] [%zu]\n\n", mUniformBlocks.size());

    for(const auto &it : mUniformBlocks) {
        printf("%s\n", it.second.glslName.c_str());
        printf("  size: %zu\n", it.second.memorySize);
        printf("  encapsulates aggregate type: %s\n", it.second.pAggregate ? it.second.pAggregate->name.c_str() : "(none)");
    }

    printf("\n[ACTIVE UNIFORMS] [%zu]\n\n", mUniforms.size());
    for(const auto &uni : mUniforms) {

        printf("%s\n", uni.name.c_str());
        printf("  type: 0x%04x\n", uni.type);
        printf("  location: %u\n", uni.location);
        printf("  array size: %d\n", uni.arraySize);
        printf("  belongs to block: [%s] with offset: %zu", uni.pBlock ? uni.pBlock->glslName.c_str() : "", uni.offset);
        printf("\n\n");
    }
}
