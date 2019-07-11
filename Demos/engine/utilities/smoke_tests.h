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

#ifndef __SMOKE_TESTS_H_
#define __SMOKE_TESTS_H_

#include <stdbool.h>

#define MAX_GLOVE_DEMOS_MODE_STR 20

typedef void (*draw_cb_t)();

bool SmokeTestsRunning();
void TakeScreenshot(char *fileName, draw_cb_t draw_cb, int width, int height);

#endif // __SMOKE_TESTS_H_
