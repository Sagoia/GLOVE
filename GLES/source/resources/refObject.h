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
 *  @file       refObject.h
 *  @author     Think Silicon
 *  @date       30/11/2018
 *  @version    1.0
 *
 *  @brief
 *
 */

#ifndef __REFOBJECT_H_
#define __REFOBJECT_H_

#include "utils/glLogger.h"

class refObject {
private:
    int   refCount;
    int   markForDeletion;

public:
// Constructor
    refObject();

// Destructor
    virtual ~refObject() = 0;

    int Bind();
    int Unbind();
    int GetRefCount()                             { FUN_ENTRY(GL_LOG_TRACE); return refCount; }
    bool FreeForDeletion()                  const { FUN_ENTRY(GL_LOG_TRACE); return refCount == 0; }
    bool GetMarkForDeletion()                     { FUN_ENTRY(GL_LOG_TRACE); return markForDeletion; }
    void SetMarkForDeletion(bool flag)            { FUN_ENTRY(GL_LOG_TRACE); markForDeletion = flag;}
};

#endif // __REFOBJECT_H_
