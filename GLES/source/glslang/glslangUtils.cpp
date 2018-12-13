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
 *  @date       10/12/2018
 *  @version    1.0
 *
 *  @brief      glslang Utility Source File
 *
 */

#include "glslangUtils.h"
#include "glslang/MachineIndependent/localintermediate.h"
#include "utils/parser_helpers.h"

int 
GetUniformOffset(const glslang::TProgram* prog, const std::string name)
{
    FUN_ENTRY(GL_LOG_TRACE);

    int offset = 0;
    for(int index = 0; index < prog->getNumLiveUniformVariables(); ++index) {
        string uniName = string(prog->getUniformName(index));
        RemoveBrackets(uniName);
        if(!strcmp(name.c_str(), uniName.c_str())) {
            offset = prog->getUniformBufferOffset(index);

            // this is used for correcting uniform offset which is erroneously generated from glslang (TODO)
            int size, stride, baseAlignment;
            const glslang::TType *uniformType = prog->getUniformTType(index);
            baseAlignment = glslang::TIntermediate::getBaseAlignment(*uniformType, size, stride, true, false);
            glslang::RoundToPow2(offset, baseAlignment); 

            break;
        }
    }

    return offset;
}