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

#include <string>
#include "shaderResourceInterface.h"
#include "utils/parser_helpers.h"
#include "utils/glUtils.h"
#include "utils/glLogger.h"
#include <algorithm>

ShaderResourceInterface::ShaderResourceInterface()
: mLiveAttributes(0), mLiveUniforms(0), mLiveUniformBlocks(0),
  mActiveAttributeMaxLength(0), mActiveUniformMaxLength(0), mReflectionSize(0), mCacheManager(nullptr)
{
    FUN_ENTRY(GL_LOG_TRACE);
}

ShaderResourceInterface::~ShaderResourceInterface()
{
    FUN_ENTRY(GL_LOG_TRACE);

    Reset();
}

void
ShaderResourceInterface::Reset(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mLiveAttributes     = 0;
    mLiveUniforms       = 0;
    mLiveUniformBlocks  = 0;

    mAttributeInterface.clear();
    mUniformInterface.clear();
    mUniformBlockInterface.clear();
}

void
ShaderResourceInterface::CreateInterface(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    Reset();

    mLiveAttributes     = mShaderReflection->GetLiveAttributes();
    mLiveUniforms       = mShaderReflection->GetLiveUniforms();
    mLiveUniformBlocks  = mShaderReflection->GetLiveUniformBlocks();

    mAttributeInterface.reserve(mLiveAttributes);
    mUniformInterface.reserve(mLiveUniforms);
    mUniformBlockInterface.reserve(mLiveUniformBlocks);

    for(uint32_t i = 0; i < mLiveAttributes; ++i) {
        mAttributeInterface.emplace_back(mShaderReflection->GetAttributeName(i),
                                         mShaderReflection->GetAttributeType(i),
                                         mShaderReflection->GetAttributeLocation(i));
    }

    for(uint32_t i = 0; i < mLiveUniforms; ++i) {
        mUniformInterface.emplace_back(mShaderReflection->GetUniformReflectionName(i),
                                       mShaderReflection->GetUniformLocation(i),
                                       mShaderReflection->GetUniformBlockIndex(i),
                                       mShaderReflection->GetUniformArraySize(i),
                                       mShaderReflection->GetUniformType(i),
                                       mShaderReflection->GetUniformOffset(i));
    }

    for(uint32_t i = 0; i < mShaderReflection->GetLiveUniformBlocks(); ++i) {
        mUniformBlockInterface.emplace_back(mShaderReflection->GetUniformBlockGlslBlockName(i),
                                            mShaderReflection->GetUniformBlockBinding(i),
                                            mShaderReflection->GetUniformBlockBlockSize(i),
                                            mShaderReflection->GetUniformBlockBlockStage(i),
                                            mShaderReflection->GetUniformBlockOpaque(i));
    }
}

void
ShaderResourceInterface::AllocateUniformClientData(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    for(auto& uni : mUniformInterface) {
        size_t   size = uni.arraySize * GlslTypeToSize(uni.type);
        uint8_t* data = new uint8_t[size];
        memset(static_cast<void *>(data), 0, size);

        mUniformDataInterface.insert(make_pair(uni.name, uniformData()));
        map<std::string, uniformData>::iterator it = mUniformDataInterface.find(uni.name);
        it->second.pClientData = data;
    }
}

bool
ShaderResourceInterface::AllocateUniformBufferObjects(const vulkanAPI::vkContext_t *vkContext)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    for(auto &uniBlock : mUniformBlockInterface) {
        if(!uniBlock.isOpaque) {

            mUniformBlockDataInterface.insert(make_pair(uniBlock.name, uniformBlockData()));
            map<std::string, uniformBlockData>::iterator it = mUniformBlockDataInterface.find(uniBlock.name);

            it->second.pBufferObject = new UniformBufferObject(vkContext);
            it->second.pBufferObject->Allocate(uniBlock.memorySize, nullptr);
        }
    }

    return true;
}

UniformBufferObject *
ShaderResourceInterface::GetUniformBufferObject(uint32_t index) const
{
    FUN_ENTRY(GL_LOG_DEBUG);

    map<std::string, uniformBlockData>::const_iterator itBlock = mUniformBlockDataInterface.find(mUniformBlockInterface[index].name);
    return itBlock->second.pBufferObject;
}

const ShaderResourceInterface::uniform *
ShaderResourceInterface::GetUniformAtLocation(uint32_t loc) const
{
    FUN_ENTRY(GL_LOG_DEBUG);

    for(const auto &uniform : mUniformInterface) {
        if(loc >= (uint32_t)uniform.location && loc < (uint32_t)(uniform.location + uniform.arraySize)) {
            return &uniform;
        }
    }

    return nullptr;
}

int
ShaderResourceInterface::GetUniformLocation(const char *name) const
{
    FUN_ENTRY(GL_LOG_DEBUG);

    if(name[strlen(name) - 1] != ']') {
        for(auto &uniIt : mUniformInterface) {
            if(!strcmp(uniIt.name.c_str(), name)) {
                return uniIt.location;
            }
        }
    } else {
        /// Adjust location according to the specific array index requested
        string requestedName = string(name);
        size_t leftBracketPos = requestedName.find_last_of("[");
        int index = std::stoi(requestedName.substr(leftBracketPos + 1));

        string ptrName = requestedName.substr(0, leftBracketPos);
        for(auto &uniIt : mUniformInterface) {
            if(!ptrName.compare(uniIt.name)) {
                if(index >= uniIt.arraySize) {
                    return -1;
                } else {
                    return uniIt.location + index;
                }
            }
        }
    }

    return -1;
}

void
ShaderResourceInterface::GetUniformClientData(uint32_t location, size_t size, void *ptr) const
{
    FUN_ENTRY(GL_LOG_DEBUG);

    const ShaderResourceInterface::uniform *uniform = GetUniformAtLocation(location);

    size_t arrayOffset = (location - uniform->location) * GlslTypeToSize(uniform->type);

    map<std::string, uniformData>::const_iterator it = mUniformDataInterface.find(uniform->name);
    memcpy(ptr, static_cast<const void *>(it->second.pClientData + arrayOffset), size);
}

const uint8_t*
ShaderResourceInterface::GetUniformClientData(uint32_t index) const
{
    FUN_ENTRY(GL_LOG_DEBUG);

    map<std::string, uniformData>::const_iterator it = mUniformDataInterface.find(mUniformInterface[index].name);
    return it->second.pClientData;
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
    for(const auto &uniform : mUniformInterface) {
        size_t len = uniform.name.length() + 1;
        if(len > mActiveUniformMaxLength) {
            mActiveUniformMaxLength = len;
        }
    }
}

void
ShaderResourceInterface::SetUniformClientData(uint32_t location, size_t size, const void *ptr)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    ShaderResourceInterface::uniform *uniform = nullptr;
    for(auto &uni: mUniformInterface) {
        if(location >= (uint32_t)uni.location && location < (uint32_t)(uni.location + uni.arraySize)) {
            uniform = &uni;
            break;
        }
    }

    size_t arrayOffset = (location - uniform->location) * GlslTypeToSize(uniform->type);

    map<std::string, uniformData>::iterator it = mUniformDataInterface.find(uniform->name);
    memcpy(static_cast<void *>(it->second.pClientData + arrayOffset), ptr, size);

    it->second.clientDataDirty = true;
}

void
ShaderResourceInterface::SetUniformSampler(uint32_t location, int count, const int *textureUnit)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    while(count--) {

        const ShaderResourceInterface::uniform *uniformSampler = GetUniformAtLocation(location);
        map<std::string, uniformData>::iterator it = mUniformDataInterface.find(uniformSampler->name);

        size_t arrayOffset = (location - uniformSampler->location) * GlslTypeToSize(uniformSampler->type);

        /// Make sure textureUnit is inside [0, GLOVE_MAX_COMBINED_TEXTURE_IMAGE_UNITS)
        if(*textureUnit >= GL_TEXTURE0 && *textureUnit < GL_TEXTURE0 + GLOVE_MAX_COMBINED_TEXTURE_IMAGE_UNITS) {
            *((glsl_sampler_t *)(it->second.pClientData + arrayOffset)) = (glsl_sampler_t)(*textureUnit - GL_TEXTURE0);
        } else {
            *((glsl_sampler_t *)(it->second.pClientData + arrayOffset)) = (glsl_sampler_t)(*textureUnit);
        }

        ++textureUnit;
        ++location;
    }
}

void
ShaderResourceInterface::UpdateAttributeInterface(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    uint32_t nLiveAttributes = mShaderReflection->GetLiveAttributes();
    if(!nLiveAttributes) {
        return;
    }

    vector<int> locations(nLiveAttributes);
    for(uint32_t i = 0; i < nLiveAttributes; ++i) {
        attribsLayout_t::const_iterator it = mCustomAttributesLayout.find(string(mShaderReflection->GetAttributeName(i)));
        locations[i] = (it == mCustomAttributesLayout.end()) ? -1 : it->second;
    }

    vector<int> c;
    for(uint32_t i = 0; i < nLiveAttributes; ++i) {
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

    for(uint32_t i = 0; i < nLiveAttributes; ++i) {
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

bool
ShaderResourceInterface::UpdateUniformBufferData(const vulkanAPI::vkContext_t *vkContext, bool *allocatedNewBufferObject)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    uint32_t blockIndex=0;
    bool     blockDataDirty;

    struct uniformDirty {
        size_t       offset;
        size_t       size;
        const void * data;
    };

    for(auto &uniBlock : mUniformBlockInterface) {

        map<std::string, uniformBlockData>::iterator itBlock = mUniformBlockDataInterface.find(uniBlock.name);

        blockDataDirty = false;
        std::vector<struct uniformDirty> uniformInterfaceDirty;
        for(auto &uniform : mUniformInterface) {

            // if does not belong to Block
            if(uniform.index != blockIndex) {
                continue;
            }

            map<std::string, uniformData>::iterator itUniform = mUniformDataInterface.find(uniform.name);

            // check if is updated
            if(!itUniform->second.clientDataDirty) {
               continue;
            }
            itUniform->second.clientDataDirty = false;

            blockDataDirty = true;
            if(IsBuildInUniform(uniform.name)) {
                blockDataDirty = false;
            }

            // compute uniform size
            for (size_t i = 0; i < (size_t)uniform.arraySize; i++) {
                struct uniformDirty newUniformDirty;
                newUniformDirty.size   = GlslTypeToSize(uniform.type);
                newUniformDirty.offset = uniform.offset + i*GlslTypeToAllignment(uniform.type);
                newUniformDirty.data   = static_cast<const void *>(itUniform->second.pClientData + i*newUniformDirty.size);
                uniformInterfaceDirty.push_back(newUniformDirty);
            }
        }

        if(blockDataDirty) {

            size_t   srcSize = 0;
            uint8_t *srcData = nullptr;

            if(itBlock->second.pBufferObject && itBlock->second.pBufferObject->GetSize() > 0) {
                mCacheManager->CacheUBO(itBlock->second.pBufferObject);

                // memcopy data
                srcSize = itBlock->second.pBufferObject->GetSize();
                srcData = new uint8_t[srcSize];
                itBlock->second.pBufferObject->GetData(srcSize, 0, srcData);

                *allocatedNewBufferObject = true;
            }

            itBlock->second.pBufferObject = new UniformBufferObject(vkContext);
            itBlock->second.pBufferObject->Allocate(uniBlock.memorySize, srcData);

            if(srcSize) {
                delete[] srcData;
            }
        }

        for (auto &u : uniformInterfaceDirty) {
            itBlock->second.pBufferObject->UpdateData(u.size, u.offset, u.data);
        }

        ++blockIndex;
    }

    return true;
}