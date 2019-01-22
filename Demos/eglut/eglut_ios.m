/*
 * Modified code of eglut_x11.c.
 * Original license:
 *
 * Copyright (C) 2010 LunarG Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Chia-I Wu <olv@lunarg.com>
 *
 */

#include "eglutint.h"
#import "eglut_ios_app.h"

void
_eglutNativeInitDisplay(void)
{
   _eglut->native_dpy = EGL_DEFAULT_DISPLAY;
   _eglut->surface_type = EGL_WINDOW_BIT;
}

void
_eglutStoreName(const char *title)
{

}

void
_eglutNativeFiniDisplay(void)
{

}

void
_eglutNativeInitWindow(struct eglut_window *win, const char *title,
                       int x, int y, int w, int h)
{
    eglutAppDelegate *appDelegate = (eglutAppDelegate *)[[UIApplication sharedApplication] delegate];
    eglutViewController *viewController = (eglutViewController *)appDelegate.rootController;
    UIView *view = viewController.glView;
    
    win->native.u.window = (__bridge EGLNativeWindowType)(view);
    win->native.width = view.frame.size.width;
    win->native.height = view.frame.size.height;
}

void
_eglutNativeFiniWindow(struct eglut_window *win)
{

}

void
_eglutNativeEventLoop(void)
{
    eglutAppDelegate *appDelegate = (eglutAppDelegate *)[[UIApplication sharedApplication] delegate];
    eglutViewController *viewController = (eglutViewController *)appDelegate.rootController;
    [viewController startRun];
}

void
ios_draw_cb(void)
{
    struct eglut_window *win = _eglut->current;
    if (win) {
        if(win->display_cb)
            win->display_cb();
        eglSwapBuffers(_eglut->dpy, win->surface);
    }
}
