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

#ifndef __REFOBJECT_TESTS_H__
#define __REFOBJECT_TESTS_H__

#include "gtest/gtest.h"
#include "resources/refObject.h"

namespace Testing {

class refObjectTest : public :: testing :: Test {
protected:
    void SetUp(void);
    void TearDown(void);

    class refObject RefObject;
};

} //end of namespace

#endif // __REFOBJECT_TESTS_H__
