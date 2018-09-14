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
 *  @file       genericVertexAttribute.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Generic Vertex Attribute Functionality in GLOVE
 *
 */

#ifndef __GENERICVERTEXATTRIBUTE_H__
#define __GENERICVERTEXATTRIBUTE_H__

#include "bufferObject.h"
#include "utils/GlToVkConverter.h"
#include "utils/cacheManager.h"

class GenericVertexAttribute {
private:
    GLint                               mElements;
    GLenum                              mType;
    GLboolean                           mNormalized;
    GLsizei                             mStride;
    GLfloat                             mGenericValue[4];
    bool                                mEnabled;

    uint32_t                            mOffset;
    uintptr_t                           mPtr;
    BufferObject *                      mVbo;
    bool                                mInternalVBO;
    CacheManager *                      mCacheManager;

public:
    GenericVertexAttribute();
    ~GenericVertexAttribute();

    // Release Functions
    void                                MoveToCache(void);
    void                                Release(void);

    // Get Functions
    inline bool                         GetEnabled(void)                  const { FUN_ENTRY(GL_LOG_TRACE); return mEnabled;    }
    inline GLint                        GetNumElements(void)              const { FUN_ENTRY(GL_LOG_TRACE); return mElements;   }
    inline GLenum                       GetType(void)                     const { FUN_ENTRY(GL_LOG_TRACE); return mType;       }
    inline GLboolean                    GetNormalized(void)               const { FUN_ENTRY(GL_LOG_TRACE); return mNormalized; }
    inline GLsizei                      GetStride(void)                   const { FUN_ENTRY(GL_LOG_TRACE); return mStride;     }
    inline uint32_t                     GetOffset(void)                   const { FUN_ENTRY(GL_LOG_TRACE); return mOffset;     }
    inline uintptr_t                    GetPointer(void)                  const { FUN_ENTRY(GL_LOG_TRACE); return mPtr;        }
    inline BufferObject *               GetVbo(void)                      const { FUN_ENTRY(GL_LOG_TRACE); return mVbo;        }
    inline VkFormat                     GetVkFormat(void)                 const { FUN_ENTRY(GL_LOG_TRACE); return GlAttribPointerToVkFormat(mElements, mType, mNormalized); }
    inline bool                         GetInternalVBOStatus(void)        const { FUN_ENTRY(GL_LOG_TRACE); return mInternalVBO;}
    inline void                         GetGenericValue(GLint *ptr)       const { FUN_ENTRY(GL_LOG_TRACE); ptr[0] = static_cast<GLint>(mGenericValue[0]);
                                                                                                           ptr[1] = static_cast<GLint>(mGenericValue[1]);
                                                                                                           ptr[2] = static_cast<GLint>(mGenericValue[2]);
                                                                                                           ptr[3] = static_cast<GLint>(mGenericValue[3]); }
    inline void                         GetGenericValue(GLfloat *ptr)     const { FUN_ENTRY(GL_LOG_TRACE); ptr[0] = mGenericValue[0];
                                                                                                           ptr[1] = mGenericValue[1];
                                                                                                           ptr[2] = mGenericValue[2];
                                                                                                           ptr[3] = mGenericValue[3]; }

    // Set Functions
           void                         Set(GLint nElements, GLenum type, GLboolean normalized, GLsizei stride, const void *ptr, BufferObject *vbo);
           void                         SetVbo(BufferObject *vbo);
    inline void                         SetEnabled(bool enabled)                    { FUN_ENTRY(GL_LOG_TRACE); mEnabled         = enabled;     }
    inline void                         SetNumElements(GLint nElements)             { FUN_ENTRY(GL_LOG_TRACE); mElements        = nElements;   }
    inline void                         SetType(GLenum type)                        { FUN_ENTRY(GL_LOG_TRACE); mType            = type;        }
    inline void                         SetNormalized(GLboolean normalized)         { FUN_ENTRY(GL_LOG_TRACE); mNormalized      = normalized;  }
    inline void                         SetStride(GLsizei stride)                   { FUN_ENTRY(GL_LOG_TRACE); mStride          = stride;      }
    inline void                         SetOffset(uint32_t offset)                  { FUN_ENTRY(GL_LOG_TRACE); mOffset          = offset;      }
    inline void                         SetPointer(uintptr_t ptr)                   { FUN_ENTRY(GL_LOG_TRACE); mPtr             = ptr;         }
    inline void                         SetInternalVBOStatus(bool internalVBO)      { FUN_ENTRY(GL_LOG_TRACE); mInternalVBO     = internalVBO; }
    inline void                         SetCacheManager(CacheManager *cacheManager) { FUN_ENTRY(GL_LOG_TRACE); mCacheManager = cacheManager; }
    inline void                         SetGenericValue(const GLfloat *ptr)         { FUN_ENTRY(GL_LOG_TRACE); mGenericValue[0] = ptr[0];
                                                                                                               mGenericValue[1] = ptr[1];
                                                                                                               mGenericValue[2] = ptr[2];
                                                                                                               mGenericValue[3] = ptr[3]; }
};

#endif // __GENERICVERTEXATTRIBUTE_H__
