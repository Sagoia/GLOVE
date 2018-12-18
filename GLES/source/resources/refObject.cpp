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
 *  @file       refObject.cpp
 *  @author     Think Silicon
 *  @date       30/11/2018
 *  @version    1.0
 *
 *  @brief
 *
 */

#include "refObject.h"

refObject::refObject()
: refCount(0), markForDeletion(false)
{
    FUN_ENTRY(GL_LOG_TRACE);
}

refObject::~refObject()
{
    FUN_ENTRY(GL_LOG_TRACE);
}

int
refObject::Bind()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    //TODO: mutex lock
    refCount++;
    return 0;
}

int
refObject::Unbind()
{
    FUN_ENTRY(GL_LOG_DEBUG);

    //TODO: mutex lock
    refCount--;
    assert(refCount >= 0);
    return 0;
}
