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
 *  @file       glslangIoMapResolver.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Shader I/O gathering functionality based on glslang TIoMapResolver
 *
 */

#include "glslangIoMapResolver.h"

GlslangIoMapResolver::GlslangIoMapResolver()
{
    FUN_ENTRY(GL_LOG_TRACE);
}

GlslangIoMapResolver::~GlslangIoMapResolver()
{
    FUN_ENTRY(GL_LOG_TRACE);

    Release();
}

void
GlslangIoMapResolver::Release(void)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    mVaryingINMap.clear();
    mVaryingOUTMap.clear();
}

void
GlslangIoMapResolver::FillInVaryingInfo(VaryingInfo *varyingInfo, const glslang::TType& type, const char *name)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    varyingInfo->name        = name;
    varyingInfo->type        = type.getBasicString();
    varyingInfo->hasLocation = type.getQualifier().hasLocation();
    varyingInfo->location    = type.getQualifier().hasLocation() ? type.getQualifier().layoutLocation : -1;
    varyingInfo->vectorSize  = type.getVectorSize();
    varyingInfo->matrixCols  = type.getMatrixCols();
}

void
GlslangIoMapResolver::PrintVaryingInfo(void) const
{
    FUN_ENTRY(GL_LOG_DEBUG);

    printf("\nGL_ACTIVE_ATTRIBUTES varyings IN: %d\n", GetVaryingInNum());
    for(uint32_t i = 0; i < GetVaryingInNum(); ++i) {
        printf("Name: %s - Type: (%s)\n", GetVaryingInName(i), GetVaryingInType(i));
        printf("Location: %d - Value: %d\n", GetVaryingInHasLocation(i), GetVaryingInLocation(i));
    }

    printf("\nACTIVE VARYINGS varyings OUT: %d\n", GetVaryingOutNum());
    for(uint32_t i = 0; i < GetVaryingOutNum(); ++i) {
        printf("Name: %s - Type: (%s)\n", GetVaryingOutName(i), GetVaryingOutType(i));
        printf("Location: %d - Value: %d\n", GetVaryingOutHasLocation(i), GetVaryingOutLocation(i));
    }
}

void
GlslangIoMapResolver::CreateVaryingLocationMap(std::map<std::string, std::pair<int,bool>> *location_map)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    int location = 0;
    for(uint32_t out = 0; out < GetVaryingOutNum(); ++out) {
        for(uint32_t in = 0; in < GetVaryingInNum(); ++in) {
            const char *inName  = GetVaryingInName(in);
            const char *outName = GetVaryingOutName(out);
            const int   inSize  = GetVaryingInVectorSize(in);
            const int   outSize = GetVaryingOutVectorSize(out);

            if(!strcmp(inName, outName)) {
                assert(location_map->find(string(inName)) == location_map->end());

                (*location_map)[std::string(inName)] = std::make_pair(location, (inSize == outSize));
                location += GetVaryingInMatrixCols(in);
            }
        }
    }
}

void
GlslangIoMapResolver::notifyInOut(EShLanguage stage, const char* name, const glslang::TType& type, bool is_live)
{
    FUN_ENTRY(GL_LOG_DEBUG);

    // Do not store inactive varyings
    if(!is_live) {
        return;
    }

    if(glslang::EvqVaryingIn == type.getQualifier().storage) {
        VaryingInfo varyingInfo;

        FillInVaryingInfo(&varyingInfo, type, name);
        mVaryingINMap.push_back(varyingInfo);
    }

    if(glslang::EvqVaryingOut == type.getQualifier().storage) {
        VaryingInfo varyingInfo;

        FillInVaryingInfo(&varyingInfo, type, name);
        mVaryingOUTMap.push_back(varyingInfo);
    }
}