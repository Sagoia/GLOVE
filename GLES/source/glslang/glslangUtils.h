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
 *  @file       glslangUtils.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      glslang Utility Header File
 *
 */

#ifndef __GLSLANG_UTILS_H__
#define __GLSLANG_UTILS_H__

#include <map>
#include <string>
#include "glslang/Public/ShaderLang.h"
#include "glslang/Include/Types.h"
#include "utils/glLogger.h"
#include "utils/glUtils.h"
#include "utils/glsl_types.h"

using namespace std;

typedef struct {
    std::string                     name;
    size_t                          memorySize;     /// Memory size 
    int32_t                         arraySize;      /// Array size (1 for non arrays)
    bool                            isBlock;        /// True if uniform block
} aggregate_t;

typedef map<string, aggregate_t>        aggregateMap_t;
typedef pair<aggregate_t *, int32_t>    aggregatePair_t;
typedef vector<aggregatePair_t>         aggregatePairList_t;

struct uniformBlock_t {
    string                          name;           /// For debug
    string                          glslName;       /// Block name that will be generated in final GLSL
    uint32_t                        binding;        /// layout decoration
    bool                            isOpaque;       /// true for opaque types (samplers)
    size_t                          memorySize;     /// Uniform block's size in bytes (including inactive variables)
    int32_t                         arraySize;      /// Uniform block's Array size 
    shader_type_t                   stage;          /// Uniform block's shader stage
    const aggregate_t *             pAggregate;

    uniformBlock_t():
        binding(0),
        isOpaque(false),
        memorySize(0),
        arraySize(0),
        stage(SHADER_TYPE_INVALID),
        pAggregate(nullptr)
    {
        FUN_ENTRY(GL_LOG_TRACE);
    }

    uniformBlock_t(string n, string gbn, uint32_t b, bool io, size_t bs, int32_t ba, shader_type_t bStage, const aggregate_t *pAggr)
     : name(n),
       glslName(gbn),
       binding(b),
       isOpaque(io),
       memorySize(bs),
       arraySize(ba),
       stage(bStage),
       pAggregate(pAggr)
    {
        FUN_ENTRY(GL_LOG_TRACE);
    }

    ~uniformBlock_t()
    {
        FUN_ENTRY(GL_LOG_TRACE);
    }
};
typedef struct uniformBlock_t       uniformBlock_t;
typedef map<string, uniformBlock_t> uniformBlockMap_t;

struct uniform_t {
    string                          name;               /// Full reflection name
    GLenum                          type;               /// format type
    uint32_t                        location;           /// Location assigned to uniform
    int32_t                         arraySize;          /// Array size (1 for non arrays)
    const aggregatePairList_t       aggregatePairList;  /// Pointer to aggregate type (if belongs to one)
    uniformBlock_t *                pBlock;             /// Pointer to uniformBlock_t this uniform belongs
    shader_type_t                   stage;              /// Uniform's shader stage
    size_t                          offset;             /// Offset from start of uniform block's data

    uniform_t(string rn, GLenum glt, uint32_t loc, int32_t as, const aggregatePairList_t pagl, uniformBlock_t *pub, shader_type_t stg, size_t off)
     : name(rn),
       type(glt),
       location(loc),
       arraySize(as),
       aggregatePairList(pagl),
       pBlock(pub),
       stage(stg),
       offset(off)
    {
        FUN_ENTRY(GL_LOG_TRACE);
    }

    ~uniform_t()
    {
        FUN_ENTRY(GL_LOG_TRACE);
    }
};
typedef struct uniform_t            uniform_t;

/// Get Functions
       int  GetUniformOffset(const glslang::TProgram* prog, string name);
inline bool GetAttributeHasLocation(const glslang::TProgram* prog, int index) { FUN_ENTRY(GL_LOG_TRACE); 
                                                                                const  glslang::TType *attributeType = prog->getAttributeTType(index);
                                                                                return attributeType->getQualifier().hasLocation(); }

inline int  GetAttributeLocation(const glslang::TProgram* prog, int index)    { FUN_ENTRY(GL_LOG_TRACE);
                                                                                const  glslang::TType *attributeType = prog->getAttributeTType(index);
                                                                                return attributeType->getQualifier().hasLocation() ? attributeType->getQualifier().layoutLocation : -1; }

inline bool GetUniformHasLocation(const glslang::TProgram* prog, int index)   { FUN_ENTRY(GL_LOG_TRACE);
                                                                                const glslang::TType *uniformType = prog->getUniformTType(index);
                                                                                return uniformType->getQualifier().hasLocation(); }

inline int  GetUniformLocation(const glslang::TProgram* prog, int index)       { FUN_ENTRY(GL_LOG_TRACE);
                                                                                 const glslang::TType *uniformType = prog->getUniformTType(index);
                                                                                 return uniformType->getQualifier().hasLocation() ? uniformType->getQualifier().layoutLocation : -1; }

inline bool GetUniformHasBinding(const glslang::TProgram* prog, int index)     { FUN_ENTRY(GL_LOG_TRACE);
                                                                                 const glslang::TType *uniformType = prog->getUniformTType(index);
                                                                                 return uniformType->getQualifier().hasBinding(); }

inline int  GetUniformBinding(const glslang::TProgram* prog, int index)        { FUN_ENTRY(GL_LOG_TRACE); return prog->getUniformBinding(index); }

inline bool GetUniformHasSet(const glslang::TProgram* prog, int index)         { FUN_ENTRY(GL_LOG_TRACE);
                                                                                 const glslang::TType *uniformType = prog->getUniformTType(index);
                                                                                 return uniformType->getQualifier().hasSet(); }

inline int  GetUniformSet(const glslang::TProgram* prog, int index)            { FUN_ENTRY(GL_LOG_TRACE);
                                                                                 const glslang::TType *uniformType = prog->getUniformTType(index);
                                                                                 return uniformType->getQualifier().hasSet() ? uniformType->getQualifier().layoutSet : -1; }

/// Delete Functions
template<typename T>  inline void SafeDelete(T*& ptr)                          { FUN_ENTRY(GL_LOG_TRACE); delete ptr; ptr = nullptr; }

#endif // __GLSLANG_UTILS_H__
