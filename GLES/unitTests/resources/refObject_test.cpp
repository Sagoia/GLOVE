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

#include "refObject_test.h"

namespace Testing {

// Code here will be called immediately after the constructor (right
// before each test).
void refObjectTest::SetUp(void) {
    return;
}

// Code here will be called immediately after each test (right
// before the destructor).
void refObjectTest::TearDown() {
    return;
}

// Objects declared here can be used by all tests.

TEST_F(refObjectTest, Bind)
{
    ASSERT_EQ(0, RefObject.Bind());
}

TEST_F(refObjectTest, Unbind)
{
    ASSERT_EQ(0, RefObject.Unbind());
}

} //end of namespace
