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

namespace {

using namespace glslang;

struct ReportSamplerTraverser : public glslang::TIntermTraverser {
    ReportSamplerTraverser(AccessChainList& ACL) :
        glslang::TIntermTraverser(), Tracing(false), Chains(ACL), Chain() {}

    bool visitBinary(TVisit, TIntermBinary*) override;

    bool Tracing;
    AccessChainList& Chains;
    AccessChain Chain;
};

bool ReportSamplerTraverser::visitBinary(TVisit /* visit */, TIntermBinary* node) {
    FUN_ENTRY(GL_LOG_TRACE);

    auto Op = node->getOp();

    if (Op != EOpIndexDirectStruct && Op != EOpIndexDirect) {
        assert(!Tracing);
        return true;
    }

    // Either start tracing or in tracing
    const auto& Loc = node->getLoc();   // symbol location
    auto L = node->getLeft();
    const auto& TT = L->getType();
    auto I = node->getRight()->getAsConstantUnion()->getConstArray()[0].getIConst();
    const TType* Ty = nullptr;
    TSourceLoc TLoc {};

    if (Op == EOpIndexDirectStruct) {
        assert(TT.isStruct());
        const auto& ST = *TT.getStruct();
        Ty = ST[I].type;
        TLoc = ST[I].loc;
    } else {
        if (TT.getBasicType() == EbtSampler) {
            // It happens when there is a sampler array
            Ty = &TT;
        } else {
            if (!Tracing) {
                return true;
            }
            assert(TT.isArray());
            assert(TT.isStruct());
        }
        // No way to get the location of the type
    }


    if (!Tracing) {
        if (Ty->getBasicType() == EbtSampler) {
            Chain.emplace_back(&TT, Op == EOpIndexDirect, I, TLoc.line,
                TLoc.column, Loc.line, Loc.column);
            Tracing = true;
        }
    } else {
        // Always give the enclosing struct
        Chain.emplace_back(&TT, Op == EOpIndexDirect, I, TLoc.line,
                TLoc.column, Loc.line, Loc.column);
        if (L->getAsSymbolNode()) {
            Chains.push_back(Chain);
            Chain.clear();
            Tracing = false;
        }
    }

    return true;
}

AccessChainList ReportSampler(const glslang::TIntermediate *TI)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    AccessChainList AC {};
    if (!TI) {
        return AC;
    }
    if (auto Root = TI->getTreeRoot()) {
        ReportSamplerTraverser it {AC};
        Root->traverse(&it);
    }
    return AC;
}

} // end of anon namespace


bool             GlslangShaderCompiler::mInitialized = false;
TBuiltInResource GlslangShaderCompiler::mTBuiltInResource;

GlslangShaderCompiler::GlslangShaderCompiler()
: mProgramLinker(nullptr), mShaderConverter(nullptr), mShaderReflection(nullptr),
  mPrintConvertedShader(false),
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
GlslangShaderCompiler::ReCompileShader(GlslangCompiler* Compiler, ShaderProgram& shaderProgram, shader_type_t shaderType)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    const auto* IM = Compiler->GetSlangShader400()->getIntermediate();
    // Capture sampler-in-block
    AccessChainList samplers = ReportSampler(IM);
    if (samplers.empty()) {
        return false;
    }

    // Fix glslang at string level
    std::string *Src = (shaderType == SHADER_TYPE_VERTEX) ? &mVertSource400 : &mFragSource400;
    auto S = FixSamplers(samplers, *Src);
    Src->clear();
    *Src = std::move(S);

    if (mDumpProcessedShaderSource) {
        cout << "\n\nFINAL SOURCE:\n" << endl;
        cout << *Src << "\n\n" << endl;
    }

    if (mSaveSourceToFiles) {
        SaveShaderSourceToFile(&shaderProgram, true, Src->c_str(), shaderType);
    }

    EShMessages message = EShMsgVulkanRules | EShMsgSpvRules;
    EShLanguage language = (shaderType == SHADER_TYPE_VERTEX ? EShLangVertex : EShLangFragment);
    const char* source = Src->c_str();
    return Compiler->CompileShader400(&source, &slangShaderResources, language, message);
}

bool
GlslangShaderCompiler::CompileShader(ShaderProgram& shaderProgram, shader_type_t shaderType, bool isYInverted)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    const char* source = ConvertShader(shaderProgram, shaderType, isYInverted);
    GlslangCompiler* Compiler = (shaderType == SHADER_TYPE_VERTEX ? mSlangVertCompiler : mSlangFragCompiler);

    EShMessages message = EShMsgVulkanRules | EShMsgSpvRules | EShMsgCascadingErrors;
    EShLanguage language = (shaderType == SHADER_TYPE_VERTEX ? EShLangVertex : EShLangFragment);
    bool result = Compiler->CompileShader400(&source, &slangShaderResources, language, message);

    if (!result) {
        // If compile fails due to sampler-in-block, try to fix the glslang
        // and give it a second chance.
        return ReCompileShader(Compiler, shaderProgram, shaderType);
    }

    return true;
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

    SetUniformBlocksSizes(mProgramLinker->GetProgram(version));
    SetUniformBlocksOffset(mProgramLinker->GetProgram(version));
    BuildUniformReflection();

    mProgramLinker->GenerateSPV(vertSpv, EShLangVertex  , version);
    mProgramLinker->GenerateSPV(fragSpv, EShLangFragment, version);

    if(mSaveBinaryToFiles) {
        SaveBinaryToFiles(program_ptr, SHADER_COMPILER_VERTEX  , version);
        SaveBinaryToFiles(program_ptr, SHADER_COMPILER_FRAGMENT, version);
    }

    if(mSaveSpvTextToFile) {
        PrintReadableSPV(program_ptr, SHADER_COMPILER_VERTEX  , version);
        PrintReadableSPV(program_ptr, SHADER_COMPILER_FRAGMENT, version);
    }

    if(mPrintReflection[version]) {
        PrintReflection(mProgramLinker->GetProgram(version), version);
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
        PrintReflection(mProgramLinker->GetProgram(version), version);
    }

    return result;
}

void
GlslangShaderCompiler::PrintReadableSPV(uintptr_t program_ptr, shader_compiler_type_t type, ESSL_VERSION version)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    EShLanguage lang = (type == SHADER_COMPILER_VERTEX) ? EShLangVertex : EShLangFragment;

    mProgramLinker->GenerateSPV(mSpv[type], lang, version);
    mProgramLinker->GenerateSPV(mSpv[SHADER_COMPILER_FRAGMENT], EShLangFragment, version);

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
GlslangShaderCompiler::PrintReflection(const glslang::TProgram *prog, ESSL_VERSION version) const
{
    FUN_ENTRY(GL_LOG_DEBUG);
    
    int i;

    printf("\n\n-------- SHADER PROGRAM REFLECTION v%d --------\n\n", (int)version);
    printf("\n[ACTIVE UNIFORMS BLOCKS] [%d]\n\n", prog->getNumLiveUniformBlocks());
    for(i = 0; i < prog->getNumLiveUniformBlocks(); ++i) {
        printf("%s\n", prog->getUniformBlockName(i));
        printf("  hasLocation:  %d = %d\n", GetUniformHasLocation(prog, i), GetUniformLocation(prog, i));
        printf("  hasBinding:   %d = %d\n", GetUniformHasBinding(prog, i) , GetUniformBinding(prog, i));
        printf("  hasSet:       %d = %d\n", GetUniformHasSet(prog, i)     , GetUniformSet(prog, i));
        printf("  block size:   %d\n"     , prog->getUniformBlockSize(i));
    }

    printf("\n[ACTIVE UNIFORMS] [%d]\n\n", prog->getNumLiveUniformVariables());
    for(i = 0; i < prog->getNumLiveUniformVariables(); ++i) {
        printf("%s (0x%x)\n", prog->getUniformName(i), prog->getUniformType(i));
        printf("  hasLocation:  %d = %d\n", GetUniformHasLocation(prog, i), GetUniformLocation(prog, i));
        printf("  hasBinding:   %d = %d\n", GetUniformHasBinding(prog, i) , GetUniformBinding(prog, i));
        printf("  hasSet:       %d = %d\n", GetUniformHasSet(prog, i)     , GetUniformSet(prog, i));
        printf("  block index:  %d\n", prog->getUniformBlockIndex(i));
        printf("  block offset: %d\n", prog->getUniformBufferOffset(i));
        printf("  array size:   %d\n", prog->getUniformArraySize(i));
    }

    printf("\n[ACTIVE ATTRIBUTES] [%d]\n\n", prog->getNumLiveAttributes());
    for(i = 0; i < prog->getNumLiveAttributes(); ++i) {
        printf("%s (0x%x)\n", prog->getAttributeName(i), prog->getAttributeType(i));
        printf("  hasLocation:  %d = %d\n", GetAttributeHasLocation(prog, i), GetAttributeLocation(prog, i));
    }

    mProgramLinker->GetIoMapResolver()->PrintVaryingInfo();

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
GlslangShaderCompiler::SetUniformBlocksSizes(const glslang::TProgram* prog)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    /// Get uniform block sizes
    for(uint32_t index = 0; index < (uint32_t)prog->getNumLiveUniformBlocks(); ++index) {
        assert(prog->getUniformBlockSize(index) != -1);
        for(auto& block : mUniformBlocks) {
            if(!block.second.glslBlockName.compare(prog->getUniformBlockName(index))) {
                assert(!block.second.isOpaque);
                block.second.blockSize = prog->getUniformBlockSize(index);
            }
        }
    }
}

void
GlslangShaderCompiler::SetUniformBlocksOffset(const glslang::TProgram* prog)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    for(auto& block : mUniformBlocks) {
        if(block.second.pAggregate == nullptr) {
            continue;
        }

        const uniformBlock_t *pBlock = &block.second;
        size_t aggregateSize = pBlock->blockSize / pBlock->pAggregate->arraySize;
        for(auto& uni : mUniforms) {
            if(uni.pAggregate != pBlock->pAggregate) {
                continue;
            }

            assert(uni.pBlock);
            assert((uni.pBlock == pBlock && uni.pAggregate == pBlock->pAggregate) || (uni.pBlock != pBlock && uni.pAggregate != pBlock->pAggregate));
            string variableName = uni.variableName;

            /// Remove any other "[*]" from uniformName too
            size_t lastOpenBracket = variableName.find_last_of("[");
            while(lastOpenBracket > 0 && lastOpenBracket != string::npos){
                const size_t lastCloseBracket = variableName.find_last_of("]");
                variableName.erase(lastOpenBracket, lastCloseBracket - lastOpenBracket + 1);
                lastOpenBracket = variableName.find_last_of("[");
            }

            /// Query uniform's offset in block from glslang
            size_t offset = GLOVE_INVALID_OFFSET;
            string uniName;
            if(uni.pAggregate->name.compare("gl_DepthRange")) {
                uniName = uni.pAggregate->name + string(".") + variableName;
            } else {
                uniName = string(STRINGIFY_MACRO(GLOVE_VULKAN_DEPTH_RANGE)) + string(".") + variableName;
            }

            for(uint32_t index = 0; index < (uint32_t)prog->getNumLiveUniformVariables(); ++index) {
                if(!strcmp(uniName.c_str(), prog->getUniformName(index))) {
                    offset = prog->getUniformBufferOffset(index);
                    break;
                }
            }
            assert(offset != GLOVE_INVALID_OFFSET);

            uni.offset = (uni.indexIntoAggregateArray > -1 ? uni.indexIntoAggregateArray : 0) * aggregateSize + offset;
        }
    }
}

void
GlslangShaderCompiler::BuildUniformReflection(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    const int nLiveUniforms = mUniforms.size();
    const int nLiveUniformBlocks = mUniformBlocks.size();
    mShaderReflection->SetLiveUniforms(nLiveUniforms);
    mShaderReflection->SetLiveUniformBlocks(nLiveUniformBlocks);

    int i = 0;
    for(auto& block : mUniformBlocks) {
        mShaderReflection->SetUniformBlockGlslBlockName(block.second.glslBlockName.c_str(), i);
        mShaderReflection->SetUniformBlockBinding(block.second.binding, i);
        mShaderReflection->SetUniformBlockBlockSize(block.second.blockSize, i);
        mShaderReflection->SetUniformBlockBlockStage(block.second.blockStage, i);
        mShaderReflection->SetUniformBlockOpaque(block.second.isOpaque, i);
        ++i;
    }

    i = 0;
    for(auto& uni : mUniforms) {
        const uniformBlock_t *pBlock = uni.pBlock;
        uint32_t index = 0;
        for(auto& block : mUniformBlocks) {
            if(!block.second.glslBlockName.compare(pBlock->glslBlockName)) {
                mShaderReflection->SetUniformReflectionName(uni.reflectionName.c_str(), i);
                mShaderReflection->SetUniformLocation(uni.location, i);
                mShaderReflection->SetUniformBlockIndex(index, i);
                mShaderReflection->SetUniformArraySize(uni.arraySize, i);
                mShaderReflection->SetUniformType(uni.glType, i);
                mShaderReflection->SetUniformOffset(uni.offset, i);

                break;
            }
            ++index;
        }
        ++i;
    }
}

void
GlslangShaderCompiler::PrintUniforms(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    printf("[ACTIVE UNIFORM BLOCKS] [%zu]\n\n", mUniformBlocks.size());

    for(auto &it : mUniformBlocks) {
        printf("%s\n", it.second.glslBlockName.c_str());
        printf("  size: %zu\n", it.second.blockSize);
        printf("  encapsulates aggregate type: %s\n", it.second.pAggregate ? it.second.pAggregate->name.c_str() : "(none)");
    }

    printf("[ACTIVE UNIFORMS] [%zu]\n\n", mUniforms.size());
    for(const auto &uni : mUniforms) {
        /// Print aggregate name
        if(uni.pAggregate) {
            printf("%s", uni.pAggregate->name.c_str());
            if(uni.indexIntoAggregateArray > -1) {
                printf("[%d]", uni.indexIntoAggregateArray);
            }
            printf(".");
        }

        /// Rest of variable's name without base aggregate
        printf("%s\n", uni.variableName.c_str());

        printf("  reflection name: %s\n", uni.reflectionName.c_str());
        printf("  type: 0x%04x\n", uni.glType);
        printf("  location: %u\n", uni.location);
        printf("  array size: %d\n", uni.arraySize);
        printf("  belongs to block: [%s] with offset: %zu", uni.pBlock ? uni.pBlock->glslBlockName.c_str() : "", uni.offset);
        printf("\n\n");
    }
}

void
GlslangShaderCompiler::PrepareReflection(ESSL_VERSION version)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mShaderReflection->Reset();
    CompileAttributes(mProgramLinker->GetProgram(version));
    CompileUniforms(mProgramLinker->GetProgram(version));
}

void
GlslangShaderCompiler::CompileAttributes(const glslang::TProgram* prog)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    const int nLiveAttributes = prog->getNumLiveAttributes();
    mShaderReflection->SetLiveAttributes(nLiveAttributes);
    
    for(int i = 0; i < nLiveAttributes; ++i) {

        int     location = GetAttributeHasLocation(prog, i) ? GetAttributeLocation(prog, i) : GLOVE_INVALID_OFFSET;
        mShaderReflection->SetAttributeName(prog->getAttributeName(i), i);
        mShaderReflection->SetAttributeType(prog->getAttributeType(i), i);
        mShaderReflection->SetAttributeLocation((uint32_t)location, i);
    }
}

void
GlslangShaderCompiler::CompileUniforms(const glslang::TProgram* prog)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    /// Reset uniforms
    mAggregates.clear();
    mUniforms.clear();
    mUniformBlocks.clear();

    const int nLiveUniforms = prog->getNumLiveUniformVariables();
    if(!nLiveUniforms) {
        return;
    }

    /// NOTE: Reserve space in vector. This is very important when emplacing back uniform_t objects,
    /// as a change in vector's capacity can cause reallocation, which in turn will call uniform_t's destructor when we do not expect it
    mUniforms.reserve(nLiveUniforms);

    for(int i = 0; i < nLiveUniforms; ++i) {
        aggregate_t *pAggregate = nullptr;
        string uniformName = string(prog->getUniformName(i));
        int32_t arrayIndex = -1;

        /// Does this uniform belong to an aggregate type?
        const size_t dotPosition = uniformName.find_first_of(".");
        if(dotPosition != string::npos) {
            string aggregateName = string(uniformName, 0, dotPosition);

            /// Is this aggregate an array?
            const size_t firstOpenBracket = aggregateName.find_first_of("[");
            if(firstOpenBracket != string::npos) {
                /// Get array index
                const size_t firstCloseBracket = aggregateName.find_first_of("]");
                arrayIndex = stoi(string(aggregateName, firstOpenBracket + 1, firstCloseBracket - firstOpenBracket - 1));
                assert(arrayIndex >= 0);
                /// remove "[*]" from name
                aggregateName = string(aggregateName, 0, firstOpenBracket);
            }

            /// Register aggregate and keep track of highest array index (if any)
            aggregateMap_t::iterator aggrIt = mAggregates.find(aggregateName);
            if(aggrIt != mAggregates.end()) {
                if(arrayIndex > -1 && aggrIt->second.arraySize < arrayIndex + 1) {
                    aggrIt->second.arraySize = arrayIndex + 1;
                }
                pAggregate = &aggrIt->second;
            } else {
                mAggregates[aggregateName] = {aggregateName, arrayIndex > -1 ? arrayIndex + 1: 1};
                pAggregate = &mAggregates[aggregateName];
            }
        }

        assert((!pAggregate && dotPosition == string::npos) || (pAggregate && dotPosition != string::npos));

        /// Create uniform
        const int arraySize = prog->getUniformArraySize(i);
        const GLenum glType = static_cast<GLenum>(prog->getUniformType(i));
        const shader_type_t stage = prog->getUniformStages(i) == (EShLangVertexMask | EShLangFragmentMask) ? static_cast<shader_type_t>(SHADER_TYPE_VERTEX | SHADER_TYPE_FRAGMENT) :
                                    prog->getUniformStages(i) == EShLangVertexMask ? SHADER_TYPE_VERTEX : SHADER_TYPE_FRAGMENT;

        mUniforms.emplace_back(string(uniformName, dotPosition != string::npos ? dotPosition + 1 : 0),  /// variableName
                               uniformName,                                                             /// reflectionName
                               glType,                                                                  /// glType
                               GLOVE_MAX_COMBINED_UNIFORM_VECTORS,                                      /// location (not assigned yet)
                               arraySize,                                                               /// arraySize
                               arrayIndex,                                                              /// indexIntoAggregateArray
                               pAggregate,                                                              /// pAggregate
                               nullptr,                                                                 /// pBlock (not known yet)
                               stage,                                                                   /// stage
                               0);                                                                      /// offset (not known yet)
    }

    assert(mUniforms.size() == (uint32_t)nLiveUniforms);

    /// Generate locations for uniforms
    /// and bindings for uniform blocks
    uint32_t location = 0, binding = 0;
    for(const auto &aggr : mAggregates) {
        /// Each unique aggregate name will be encapsulated into a uniform block
        if(mUniformBlocks.find(aggr.second.name) == mUniformBlocks.end()) {
            mUniformBlocks[aggr.second.name] = {aggr.second.name,                                       /// Copy name for debugging
                                                string("uni") + to_string(binding),                     /// Construct uniform block's name
                                                binding,                                                /// Binding index
                                                false,                                                  /// Definitely not an opaque type
                                                0,                                                      /// blockSize (not known yet)
                                                INVALID_SHADER,                                         /// stage (not known yet)
                                                &aggr.second};                                          /// This block encapsulates an aggregate type (or an array of one)
            ++binding;
        }

        for(int32_t arrayIndex = 0; arrayIndex < aggr.second.arraySize; ++arrayIndex) {
            for(auto &uni : mUniforms) {
                if(uni.pAggregate == &aggr.second && (uni.indexIntoAggregateArray == arrayIndex || uni.indexIntoAggregateArray == -1)) {
                    uni.location = location;
                    assert(mUniformBlocks.find(aggr.second.name) != mUniformBlocks.end());
                    uni.pBlock = &mUniformBlocks.find(aggr.second.name)->second;
                    assert(uni.pBlock);
                    uni.pBlock->blockStage = static_cast<shader_type_t>(uni.pBlock->blockStage | uni.stage);
                    location += uni.arraySize;
                }
            }
        }
    }

    assert(mUniformBlocks.size() == mAggregates.size());

    /// For uniforms that do not belong to an aggregate type
    for(auto &uni : mUniforms) {
        if(!uni.pAggregate) {
            const bool isSampler = uni.glType == GL_SAMPLER_2D || uni.glType == GL_SAMPLER_CUBE;

            /// Each uniform that does not belong to an aggregate struct/array
            /// and is not of a sampler type, will be encapsulated into a uniform block
            if(mUniformBlocks.find(uni.variableName) == mUniformBlocks.end()) {
                mUniformBlocks[uni.variableName] = {uni.variableName,                                   /// Copy name for debugging
                                                    string("uni") + to_string(binding),                 /// Construct uniform block's name
                                                    binding,                                            /// Binding index
                                                    isSampler,                                          /// true for samplers
                                                    0,                                                  /// blockSize (not known yet)
                                                    uni.stage,                                          /// stage
                                                    nullptr};                                           /// This block encapsulates a base type (or an array of one)
                ++binding;
            }

            assert(uni.location == GLOVE_MAX_COMBINED_UNIFORM_VECTORS);
            assert(!uni.pBlock);
            uni.location = location;
            assert(mUniformBlocks.find(uni.variableName) != mUniformBlocks.end());
            uni.pBlock = &mUniformBlocks.find(uni.variableName)->second;
            assert(uni.pBlock);

            location += uni.arraySize ? uni.arraySize : 1;
        }
    }
}