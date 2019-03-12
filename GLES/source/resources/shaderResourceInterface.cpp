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
 *  @file       shaderResourceInterface.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Shader Resource Interface Functionality in GLOVE
 *
 *  @scope
 *
 *  A Shader-resource interface accesses shader (reflection) information.
 *
 */

#include "shaderResourceInterface.h"
#include "utils/parser_helpers.h"
#include "utils/glUtils.h"
#include "utils/glLogger.h"
#include <algorithm>
#include <string>

ShaderResourceInterface::ShaderResourceInterface()
: mLiveAttributes(0), mLiveUniforms(0), mLiveUniformBlocks(0), mLiveSamplers(0),
  mActiveAttributeMaxLength(0), mActiveUniformMaxLength(0), mReflectionSize(0), mCacheManager(nullptr)
{
    FUN_ENTRY(GL_LOG_TRACE);
}

ShaderResourceInterface::~ShaderResourceInterface()
{
    FUN_ENTRY(GL_LOG_TRACE);

    CleanUp();
}

void
ShaderResourceInterface::CleanUp(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mLiveAttributes = 0;
    mLiveUniforms = 0;
    mLiveUniformBlocks = 0;

    mAttributeInterface.clear();
    for (uint32_t i = 0; i < mUniforms.Size(); ++i) {
        delete mUniforms[i];
    }
    mUniforms.Clear();
    for (uint32_t i = 0; i < mUniformBlocks.Size(); ++i) {
        delete mUniformBlocks[i];
    }
    mUniformBlocks.Clear();
}

void
ShaderResourceInterface::CreateInterface(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    CleanUp();

    mLiveAttributes = mShaderReflection->GetLiveAttributes();
    mLiveUniforms = mShaderReflection->GetLiveUniforms();
    mLiveUniformBlocks = mShaderReflection->GetLiveUniformBlocks();

    assert(mLiveUniforms == mLiveUniformBlocks);

    mAttributeInterface.reserve(mLiveAttributes);
    mUniforms.Reserve(mLiveUniforms);
    mUniformBlocks.Reserve(mLiveUniformBlocks);

    for(uint32_t i = 0; i < mLiveAttributes; ++i) {
        mAttributeInterface.emplace_back(mShaderReflection->GetAttributeName(i),
                                         mShaderReflection->GetAttributeType(i),
                                         mShaderReflection->GetAttributeLocation(i));
    }

    for(uint32_t i = 0; i < mLiveUniforms; ++i) {
        uniform *u = new uniform(
            mShaderReflection->GetUniformReflectionName(i),
            mShaderReflection->GetUniformLocation(i),
            mShaderReflection->GetUniformBlockIndex(i),
            mShaderReflection->GetUniformArraySize(i),
            mShaderReflection->GetUniformType(i),
            mShaderReflection->GetUniformOffset(i));
        mUniforms.PushBack(u);
    }

    for(uint32_t i = 0; i < mLiveUniformBlocks; ++i) {
        uniformBlock *ub = new uniformBlock(
            mShaderReflection->GetUniformBlockGlslBlockName(i),
            mShaderReflection->GetUniformBlockBinding(i),
            mShaderReflection->GetUniformBlockBlockSize(i),
            mShaderReflection->GetUniformBlockBlockStage(i),
            mShaderReflection->GetUniformBlockOpaque(i));
        mUniformBlocks.PushBack(ub);
    }
}

void
ShaderResourceInterface::AllocateUniformClientData(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    for (uint32_t i = 0; i < mUniforms.Size(); ++i) {
        auto& uni = mUniforms[i];
        size_t clientDataSize = uni->arraySize * GlslTypeToSize(uni->glType);
        uni->pClientData = (uint8_t *)calloc(clientDataSize, sizeof(uint8_t));
    }
}

bool
ShaderResourceInterface::AllocateUniformBufferObjects(const vulkanAPI::XContext_t *xContext)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mLiveSamplers = 0;
    for (uint32_t i = 0; i < mUniformBlocks.Size(); ++i) {
        auto &uniBlock = mUniformBlocks[i];
        UniformBufferObject *ubo = nullptr;
        if(!uniBlock->isOpaque) {
            assert(uniBlock->blockSize);
            ubo = new UniformBufferObject(xContext);
            ubo->Allocate(uniBlock->requiredSize, nullptr, uniBlock->blockSize);
        } else {
            ++ mLiveSamplers;
        }
        uniBlock->pBufferObject = ubo;
    }

    return true;
}

bool
ShaderResourceInterface::UpdateUniformBufferData(const vulkanAPI::XContext_t *xContext, bool *allocatedNewBufferObject)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    for (uint32_t i = 0; i < mUniformBlocks.Size(); ++i) {
        auto &uniformBlock = mUniformBlocks[i];
        auto &uniform = mUniforms[i];

        if (uniform->clientDataDirty) {
            uniform->clientDataDirty = false;

            if (IsBuildInUniform(uniform->reflectionName)) {
                continue;
            }

            UniformBufferObject *bufferObject = uniformBlock->pBufferObject;
            if(bufferObject) {
                mCacheManager->CacheUBO(bufferObject);
            }

            bufferObject = mCacheManager->GetUBO(uniformBlock->cacheIndex);
            if (bufferObject) {
                bufferObject->UpdateData(uniformBlock->blockSize, 0, uniform->pClientData);
            } else {
                bufferObject = new UniformBufferObject(xContext);
                bufferObject->Allocate(uniformBlock->requiredSize, uniform->pClientData, uniformBlock->blockSize);
            }
            uniformBlock->pBufferObject = bufferObject;

            *allocatedNewBufferObject = true;
        }
    }

    return true;
}

const ShaderResourceInterface::uniform *
ShaderResourceInterface::GetUniformAtLocation(uint32_t location)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    for (uint32_t i = 0; i < mUniforms.Size(); ++i) {
        const auto &uniform = mUniforms[i];
        if(location >= (uint32_t)uniform->location && location < (uint32_t)(uniform->location + uniform->arraySize)) {
            return uniform;
        }
    }

    return nullptr;
}

int
ShaderResourceInterface::GetUniformLocation(const char *name) const
{
    FUN_ENTRY(GL_LOG_DEBUG);

    assert(name);

    if(name[strlen(name) - 1] != ']') {
        for (uint32_t i = 0; i < mUniforms.Size(); ++i) {
            auto &uni = mUniforms[i];
            if(!strcmp(uni->reflectionName.c_str(), name)) {
                return uni->location;
            }
        }
    } else {
        /// Adjust location according to the specific array index requested
        std::string requestedName = std::string(name);
        size_t leftBracketPos = requestedName.find_last_of("[");
        int index = std::stoi(requestedName.substr(leftBracketPos + 1));

        std::string ptrName = requestedName.substr(0, leftBracketPos);
        for (uint32_t i = 0; i < mUniforms.Size(); ++i) {
            auto &uni = mUniforms[i];
            if(!ptrName.compare(uni->reflectionName)) {
                if(index >= uni->arraySize) {
                    return -1;
                } else {
                    return uni->location + index;
                }
            }
        }
    }

    return -1;
}

void
ShaderResourceInterface::SetUniformClientData(uint32_t location, size_t size, const void *ptr)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    uniform *uniform = const_cast<ShaderResourceInterface::uniform *>(GetUniformAtLocation(location));
    assert(uniform);
    assert(uniform->location <= location);

    size_t arrayOffset = (location - uniform->location) * GlslTypeToSize(uniform->glType);
    assert(arrayOffset + size <= uniform->arraySize * GlslTypeToSize(uniform->glType));

    memcpy(static_cast<void *>(uniform->pClientData + arrayOffset), ptr, size);
    uniform->clientDataDirty = true;
}

void
ShaderResourceInterface::SetSampler(uint32_t location, int count, const int *textureUnit)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    while(count--) {
        assert((*textureUnit >= GL_TEXTURE0 && *textureUnit < GL_TEXTURE0 + GLOVE_MAX_COMBINED_TEXTURE_IMAGE_UNITS) ||
               (*textureUnit >= 0 && *textureUnit < GLOVE_MAX_COMBINED_TEXTURE_IMAGE_UNITS));

        uniform *uniformSampler = const_cast<ShaderResourceInterface::uniform *>(GetUniformAtLocation(location));
        assert(uniformSampler);

        assert(uniformSampler->location <= location);
        size_t arrayOffset = (location - uniformSampler->location) * GlslTypeToSize(uniformSampler->glType);

        /// Make sure textureUnit is inside [0, GLOVE_MAX_COMBINED_TEXTURE_IMAGE_UNITS)
        if(*textureUnit >= GL_TEXTURE0 && *textureUnit < GL_TEXTURE0 + GLOVE_MAX_COMBINED_TEXTURE_IMAGE_UNITS) {
            *((glsl_sampler_t *)(uniformSampler->pClientData + arrayOffset)) = (glsl_sampler_t)(*textureUnit - GL_TEXTURE0);
        } else {
            *((glsl_sampler_t *)(uniformSampler->pClientData + arrayOffset)) = (glsl_sampler_t)(*textureUnit);
        }

        ++textureUnit;
        ++location;
    }
}

void
ShaderResourceInterface::CopyUniformClientData(uint32_t location, size_t size, void *ptr)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    uniform *uniform = const_cast<ShaderResourceInterface::uniform *>(GetUniformAtLocation(location));

    assert(uniform);
    assert(uniform->location <= location);

    size_t arrayOffset = (location - uniform->location) * GlslTypeToSize(uniform->glType);
    assert(arrayOffset + size <= uniform->arraySize * GlslTypeToSize(uniform->glType));

    memcpy(ptr, static_cast<const void *>(uniform->pClientData + arrayOffset), size);
}

void
ShaderResourceInterface::UpdateAttributeInterface(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    assert(mShaderReflection);

    uint32_t i;
    uint32_t nLiveAttributes = mShaderReflection->GetLiveAttributes();
    if(!nLiveAttributes) {
        return;
    }

    std::vector<int> locations(nLiveAttributes);
    for(i = 0; i < nLiveAttributes; ++i) {
        attribsLayout_t::const_iterator it = mCustomAttributesLayout.find(std::string(mShaderReflection->GetAttributeName(i)));
        locations[i] = (it == mCustomAttributesLayout.end()) ? -1 : it->second;
    }

    std::vector<int> c;
    for(i = 0; i < nLiveAttributes; ++i) {
        if((uint32_t)mShaderReflection->GetAttributeLocation(i) == GLOVE_INVALID_OFFSET) {
            if(locations[i] != -1) {
                int location = locations[i];
                for(int j = 0; j < (int)OccupiedLocationsPerGlType(mShaderReflection->GetAttributeType(i)); ++j) {
                    c.push_back(location + j);
                }
                mShaderReflection->SetAttributeLocation(location, i);
            }
        }
    }

    for(i = 0; i < nLiveAttributes; ++i) {
        if((uint32_t)mShaderReflection->GetAttributeLocation(i) == GLOVE_INVALID_OFFSET) {
            if(locations[i] == -1) {
                int location = 0;
                for (int k = 0; k < GLOVE_MAX_VERTEX_ATTRIBS; k++) {
                    std::vector<int>::iterator it = std::find (c.begin(), c.end(), k);
                    if (it == c.end()) {
                        location = k;
                        for(int j = 0; j < (int)OccupiedLocationsPerGlType(mShaderReflection->GetAttributeType(i)); ++j) {
                            c.push_back(location + j);
                        }
                        break;
                    }
                }
                mShaderReflection->SetAttributeLocation(location, i);
            }
        }
    }
}

int
ShaderResourceInterface::GetAttributeLocation(const char *name) const
{
    FUN_ENTRY(GL_LOG_DEBUG);

    for(auto &it : mAttributeInterface) {
        if(!strcmp(it.name.c_str(), name)) {
            return it.location;
        }
    }

    return -1;
}

const std::string &
ShaderResourceInterface::GetAttributeName(int index) const
{
    FUN_ENTRY(GL_LOG_DEBUG);

    assert(index >= 0 && index < (int)mAttributeInterface.size());

    return mAttributeInterface[index].name;
}

int
ShaderResourceInterface::GetAttributeType(int index) const
{
    FUN_ENTRY(GL_LOG_DEBUG);

    assert(index >= 0 && index < (int)mAttributeInterface.size());

    return mAttributeInterface[index].glType;
}

const ShaderResourceInterface::attribute *
ShaderResourceInterface::GetVertexAttribute(int index) const
{
    FUN_ENTRY(GL_LOG_DEBUG);

    assert(index >= 0 && index < (int)mAttributeInterface.size());

    return &(*(mAttributeInterface.cbegin() + index));
}

void
ShaderResourceInterface::SetActiveAttributeMaxLength(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mActiveAttributeMaxLength = 0;
    for(const auto &attribute : mAttributeInterface) {
        size_t len = attribute.name.length() + 1;
        if(len > mActiveAttributeMaxLength) {
            mActiveAttributeMaxLength = len;
        }
    }
}

void
ShaderResourceInterface::SetActiveUniformMaxLength(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mActiveUniformMaxLength = 0;
    for (uint32_t i = 0; i < mUniforms.Size(); ++i) {
        auto &uniform = mUniforms[i];
        size_t len = uniform->reflectionName.length() + 1;
        if(len > mActiveUniformMaxLength) {
            mActiveUniformMaxLength = len;
        }
    }
}

void
ShaderResourceInterface::DumpGloveShaderVertexInputInterface(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    printf("Shader's vertex input interface:\n");

    for(const auto &vi : mAttributeInterface) {
        printf("%s: \n", vi.name.c_str());
#ifdef NDEBUG
        printf("  location: %d glsl type: %s\n", vi.location, GlAttribTypeToString(vi.glType));
#else
        printf("  location: %d glsl type: 0x%04x\n", vi.location, vi.glType);
#endif // NDEBUG
    }

    printf("\n\n");
}
