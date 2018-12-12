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
 *  @file       FixSampler.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Fix sampler members in glsl struct
 *
 */

#ifndef __FIX_SAMPLER_H__
#define __FIX_SAMPLER_H__

#include "glslang/Include/Types.h"

#include <vector>

struct TypeLoc {
    TypeLoc (const glslang::TType *ty, bool array, int i, int l, int c) :
        type(ty), isArray(array), index(i), line(l), column(c) {}

    const glslang::TType *type; // This is the struct type
    bool isArray; // if isArray, ``index'' is the index of the array
    int index;    // use index to find field (the sampler) from type
    int line;     // location of the member field. however, if isArray,
    int column;   // location points to th symbol, i.e. the array.
};

struct TypeAccess {
    TypeAccess (const glslang::TType *ty, bool array, int i,
        int tl, int tc, int l, int c) :
        typeloc(ty, array, i, tl, tc),
        line(l), column(c) {}

    TypeLoc typeloc;
    int line;   // location of symbol
    int column;
};

using AccessChain = std::vector<TypeAccess>;
using AccessChainList = std::vector<AccessChain>;

std::string FixSamplers(const AccessChainList& samplers, std::string& source);

#endif
