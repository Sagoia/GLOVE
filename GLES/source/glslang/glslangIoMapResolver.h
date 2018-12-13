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
 *  @file       glslangIoMapResolver.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Shader I/O gathering functionality based on glslang TIoMapResolver
 *
 */

#ifndef __GLSLANGIOMAPRESOLVER_H__
#define __GLSLANGIOMAPRESOLVER_H__

#include "utils/glLogger.h"
#include "glslang/Public/ShaderLang.h"
#include "glslang/Include/Types.h"

class GlslangIoMapResolver : public glslang::TIoMapResolver
{
private:
    typedef struct VaryingInfo {
        const char *name;
        const char *type;
        bool        hasLocation;
        int         location;
        int         vectorSize;
        int         matrixCols;
    } VaryingInfo;

    std::vector<VaryingInfo>    mVaryingINMap;
    std::vector<VaryingInfo>    mVaryingOUTMap;

    void               FillInVaryingInfo(VaryingInfo *varyinginfo, const glslang::TType& type, const char *name);

public:

    GlslangIoMapResolver(); 
    ~GlslangIoMapResolver();

    bool               validateBinding(EShLanguage stage, const char* name, const glslang::TType& type, bool is_live) override          { return true; }
    int                resolveBinding(EShLanguage stage, const char* name, const glslang::TType& type, bool is_live) override           { return -1;   }
    int                resolveSet(EShLanguage stage, const char* name, const glslang::TType& type, bool is_live) override               { return -1;   }
    int                resolveUniformLocation(EShLanguage stage, const char* name, const glslang::TType& type, bool is_live) override   { return -1;   }
    bool               validateInOut(EShLanguage stage, const char* name, const glslang::TType& type, bool is_live) override            { return true; }
    int                resolveInOutLocation(EShLanguage stage, const char* name, const glslang::TType& type, bool is_live) override     { return -1;   }
    int                resolveInOutComponent(EShLanguage stage, const char* name, const glslang::TType& type, bool is_live) override    { return -1;   }
    int                resolveInOutIndex(EShLanguage stage, const char* name, const glslang::TType& type, bool is_live) override        { return -1;   }
    void               notifyBinding(EShLanguage stage, const char* name, const glslang::TType& type, bool is_live) override            { }
    void               endNotifications(EShLanguage stage) override { }
    void               beginNotifications(EShLanguage stage) override { }
    void               beginResolve(EShLanguage stage) override { }
    void               endResolve(EShLanguage stage) override { }

    void               notifyInOut(EShLanguage stage, const char* name, const glslang::TType& type, bool is_live) override;

/// Release Functions
    void               Release(void);

///  Print Functions
    void               PrintVaryingInfo(void) const;

/// Create Functions
    void               CreateVaryingLocationMap(std::map<std::string, std::pair<int,bool>> *location_map);

/// Get Functions
    inline uint32_t    GetVaryingInNum(void)                        const { FUN_ENTRY(GL_LOG_TRACE); return mVaryingINMap.size(); }
    inline const char *GetVaryingInName(uint32_t index)             const { FUN_ENTRY(GL_LOG_TRACE); return (index > mVaryingINMap.size() - 1) ? "" : mVaryingINMap[index].name;        }
    inline const char *GetVaryingInType(uint32_t index)             const { FUN_ENTRY(GL_LOG_TRACE); return (index > mVaryingINMap.size() - 1) ? "" : mVaryingINMap[index].type;        }
    inline int         GetVaryingInLocation(uint32_t index)         const { FUN_ENTRY(GL_LOG_TRACE); return (index > mVaryingINMap.size() - 1) ? -1 : mVaryingINMap[index].location;    }
    inline int         GetVaryingInHasLocation(uint32_t index)      const { FUN_ENTRY(GL_LOG_TRACE); return (index > mVaryingINMap.size() - 1) ? -1 : mVaryingINMap[index].hasLocation; }
    inline int         GetVaryingInVectorSize(uint32_t index)       const { FUN_ENTRY(GL_LOG_TRACE); return (index > mVaryingINMap.size() - 1) ? -1 : mVaryingINMap[index].vectorSize;  }
    inline int         GetVaryingInMatrixCols(uint32_t index)       const { FUN_ENTRY(GL_LOG_TRACE); return (index > mVaryingINMap.size() - 1) ? -1 : mVaryingINMap[index].vectorSize == 0 ?
                                                                                                                                                     mVaryingINMap[index].matrixCols : 1;}

    inline uint32_t    GetVaryingOutNum(void)                       const { FUN_ENTRY(GL_LOG_TRACE); return mVaryingOUTMap.size(); }
    inline const char *GetVaryingOutName(uint32_t index)            const { FUN_ENTRY(GL_LOG_TRACE); return (index > mVaryingOUTMap.size() - 1) ? "" : mVaryingOUTMap[index].name;        }
    inline const char *GetVaryingOutType(uint32_t index)            const { FUN_ENTRY(GL_LOG_TRACE); return (index > mVaryingOUTMap.size() - 1) ? "" : mVaryingOUTMap[index].type;        }
    inline int         GetVaryingOutLocation(uint32_t index)        const { FUN_ENTRY(GL_LOG_TRACE); return (index > mVaryingOUTMap.size() - 1) ? -1 : mVaryingOUTMap[index].location;    }
    inline int         GetVaryingOutHasLocation(uint32_t index)     const { FUN_ENTRY(GL_LOG_TRACE); return (index > mVaryingOUTMap.size() - 1) ? -1 : mVaryingOUTMap[index].hasLocation; }
    inline int         GetVaryingOutVectorSize(uint32_t index)      const { FUN_ENTRY(GL_LOG_TRACE); return (index > mVaryingOUTMap.size() - 1) ? -1 : mVaryingOUTMap[index].vectorSize;  }
};

#endif // __GLSLANGIOMAPRESOLVER_H__
