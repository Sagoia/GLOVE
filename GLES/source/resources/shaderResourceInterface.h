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
 *  @file       shaderResourceInterface.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Shader Resource Interface Functionality in GLOVE
 *
 */

#ifndef __SHADERRESOURCEINTERFACE_H__
#define __SHADERRESOURCEINTERFACE_H__

#include "shaderReflection.h"
#include "bufferObject.h"
#include "utils/cacheManager.h"
#include <vector>

class ShaderResourceInterface {
public:
    struct attribute {
        string                      name;
        GLenum                      type;
        uint32_t                    location;

        attribute(string n, GLenum t, uint32_t l)
         : name(n),
           type(t),
           location(l)
        {
            FUN_ENTRY(GL_LOG_TRACE);
        }
    };
    typedef struct attribute                attribute;
    typedef vector<attribute>               attributeInterface;

    struct uniform{
        string                      name;
        uint32_t                    location;
        uint32_t                    index;
        int32_t                     arraySize;
        GLenum                      type;
        size_t                      offset;

        uniform(string n, uint32_t l, uint32_t i, int32_t a, GLenum t, size_t o)
         : name(n),
           location(l),
           index(i),
           arraySize(a),
           type(t),
           offset(o)
        {
            FUN_ENTRY(GL_LOG_TRACE);
        }
    };
    typedef struct uniform uniform;
    typedef vector<uniform>                 uniformInterface;

    struct uniformData {
        uint8_t                    *pClientData;
        bool                        clientDataDirty;

        uniformData()
         : pClientData(nullptr),
           clientDataDirty(false)
        {
            FUN_ENTRY(GL_LOG_TRACE);
        }

        ~uniformData()
        {
            FUN_ENTRY(GL_LOG_TRACE);

            if(pClientData) {
                delete[] pClientData;
                pClientData = nullptr;
            }
        }
    };
    typedef struct uniformData              uniformData;
    typedef map<string, uniformData>        uniformDataInterface;

    struct uniformBlock {
        string                      name;
        uint32_t                    binding;
        size_t                      memorySize;
        shader_type_t               stage;
        bool                        isOpaque;

        uniformBlock(string n, uint32_t b, size_t m, shader_type_t s, bool o)
         : name(n),
           binding(b),
           memorySize(m),
           stage(s),
           isOpaque(o)
        {
            FUN_ENTRY(GL_LOG_TRACE);
        }
    };
    typedef struct uniformBlock             uniformBlock;
    typedef vector<uniformBlock>            uniformBlockInterface;

    struct uniformBlockData {
        UniformBufferObject *       pBufferObject;

        uniformBlockData()
         : pBufferObject(nullptr)
        {
            FUN_ENTRY(GL_LOG_TRACE);
        }

        ~uniformBlockData()
        {
            FUN_ENTRY(GL_LOG_TRACE);

            if(pBufferObject) {
                delete pBufferObject;
                pBufferObject = nullptr;
            }
        }
    };
    typedef struct uniformBlockData         uniformBlockData;
    typedef map<string, uniformBlockData>   uniformBlockDataInterface;

    typedef map<string, uint32_t>           attribsLayout_t;

private:
    uint32_t                                mLiveAttributes;
    uint32_t                                mLiveUniforms;
    uint32_t                                mLiveUniformBlocks;

    size_t                                  mActiveAttributeMaxLength;
    size_t                                  mActiveUniformMaxLength;
    uint32_t                                mReflectionSize;

    ShaderReflection*                       mShaderReflection;

    attributeInterface                      mAttributeInterface;

    uniformInterface                        mUniformInterface;
    uniformDataInterface                    mUniformDataInterface;

    uniformBlockInterface                   mUniformBlockInterface;
    uniformBlockDataInterface               mUniformBlockDataInterface;

    attribsLayout_t                         mCustomAttributesLayout;
    CacheManager*                           mCacheManager;

    void                                    Reset(void);

public:
    ShaderResourceInterface();
    ~ShaderResourceInterface();

/// Get Functions
    inline uint32_t                         GetLiveAttributes(void)                const { FUN_ENTRY(GL_LOG_TRACE); return mLiveAttributes;    }
    inline uint32_t                         GetLiveUniforms(void)                  const { FUN_ENTRY(GL_LOG_TRACE); return mLiveUniforms;      }
    inline uint32_t                         GetLiveUniformBlocks(void)             const { FUN_ENTRY(GL_LOG_TRACE); return mLiveUniformBlocks; }

    inline size_t                           GetActiveAttribMaxLen(void)            const { FUN_ENTRY(GL_LOG_TRACE); return mActiveAttributeMaxLength;  }
    inline size_t                           GetActiveUniformMaxLen(void)           const { FUN_ENTRY(GL_LOG_TRACE); return mActiveUniformMaxLength;    }


    inline uint32_t                         GetReflectionSize(void)                const { FUN_ENTRY(GL_LOG_TRACE); return mReflectionSize; }

    const  string&                          GetAttributeName(int index)            const { FUN_ENTRY(GL_LOG_TRACE); return mAttributeInterface[index].name; }
    int                                     GetAttributeType(int index)            const { FUN_ENTRY(GL_LOG_TRACE); return mAttributeInterface[index].type; }
    int                                     GetAttributeLocation(const char *name) const;
    inline uint32_t                         GetAttributeLocation(uint32_t index)   const { FUN_ENTRY(GL_LOG_TRACE); return mAttributeInterface[index].location; }
    inline GLenum                           GetAttributeType(uint32_t index)       const { FUN_ENTRY(GL_LOG_TRACE); return mAttributeInterface[index].type; }

    
    inline int32_t                          GetUniformBlockIndex(uint32_t index)   const { FUN_ENTRY(GL_LOG_TRACE); return mUniformInterface[index].index;     }
    inline int32_t                          GetUniformArraySize(uint32_t index)    const { FUN_ENTRY(GL_LOG_TRACE); return mUniformInterface[index].arraySize; }
    inline GLenum                           GetUniformType(uint32_t index)         const { FUN_ENTRY(GL_LOG_TRACE); return mUniformInterface[index].type;      }
           int                              GetUniformLocation(const char *name)   const;
           void                             GetUniformClientData(uint32_t loc,
                                                                 size_t size,
                                                                 void *ptr)        const;
	const  uint8_t                         *GetUniformClientData(uint32_t index)   const;
	UniformBufferObject                    *GetUniformBufferObject(uint32_t index) const;


    inline uint32_t                         GetUniformBlockBinding(uint32_t index) const { FUN_ENTRY(GL_LOG_TRACE); return mUniformBlockInterface[index].binding; }
    inline shader_type_t                    GetUniformBlockStage(uint32_t index)   const { FUN_ENTRY(GL_LOG_TRACE); return mUniformBlockInterface[index].stage; }
    inline bool                             IsUniformBlockOpaque(uint32_t index)   const { FUN_ENTRY(GL_LOG_TRACE); return mUniformBlockInterface[index].isOpaque; }

    const uniform                          *GetUniformAtLocation(uint32_t loc)     const;
    const uniform                          *GetUniform(uint32_t index)             const { FUN_ENTRY(GL_LOG_TRACE); return index < mUniformInterface.size() ? mUniformInterface.data() + index : nullptr; }

    const attribute                        *GetVertexAttribute(int index)          const { FUN_ENTRY(GL_LOG_TRACE); return &(*(mAttributeInterface.cbegin() + index)); }

/// Set Functions
    inline void                             SetCacheManager(CacheManager *cacheManager)          { FUN_ENTRY(GL_LOG_TRACE); mCacheManager     = cacheManager; }
    inline void                             SetReflection(ShaderReflection* reflection)          { FUN_ENTRY(GL_LOG_TRACE); mShaderReflection = reflection; };
    inline void                             SetReflectionSize(void)                              { FUN_ENTRY(GL_LOG_TRACE); mReflectionSize   = mShaderReflection->GetReflectionSize(); }
    inline void                             SetCustomAttribsLayout(const char *name, int index)  { FUN_ENTRY(GL_LOG_TRACE); mCustomAttributesLayout[std::string(name)] = index; }    
    void                                    SetActiveAttributeMaxLength(void);
    void                                    SetActiveUniformMaxLength(void);

    void                                    SetUniformClientData(uint32_t location,
                                                                 size_t size,
                                                                 const void *ptr);
    void                                    SetUniformSampler(uint32_t location,
                                                       int count,
                                                       const int *textureUnit);

/// Allocate Functions
    void                                    CreateInterface(void);
    void                                    AllocateUniformClientData(void);
	bool                                    AllocateUniformBufferObjects(const vulkanAPI::vkContext_t *vkContext);

/// Update Functions    
    bool                                    UpdateUniformBufferData(const vulkanAPI::vkContext_t *vkContext,
                                                                    bool *allocatedNewBufferObject);
    void                                    UpdateAttributeInterface(void);


};

#endif // __SHADERRESOURCEINTERFACE_H__
