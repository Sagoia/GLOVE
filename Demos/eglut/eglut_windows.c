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

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void
_eglutNativeInitDisplay(void)
{
   _eglut->native_dpy = EGL_DEFAULT_DISPLAY;
   _eglut->surface_type = EGL_WINDOW_BIT;
}

void
_eglutStoreName(const char *title)
{
    struct eglut_window* win = _eglut->current;
    SetWindowTextA(win->native.u.window, title);
}

void
_eglutNativeFiniDisplay(void)
{

}

void
_eglutNativeInitWindow(struct eglut_window *win, const char *title,
                       int x, int y, int w, int h)
{
    HINSTANCE hInstance = GetModuleHandle(NULL);
    // Initialize the window class.
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.lpszClassName = "eglut_class";
    RegisterClassEx(&windowClass);

    RECT windowRect = { x, y, (w + x), (h + y) };
    AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindow(
        windowClass.lpszClassName,
        title,
        WS_OVERLAPPEDWINDOW,
        x, y, w, h,
        NULL,
        NULL,
        hInstance,
        NULL);

    win->native.u.window = hwnd;
    win->native.width = w;
    win->native.height = h;
}

void
_eglutNativeFiniWindow(struct eglut_window *win)
{
    PostMessage(win->native.u.window, WM_CLOSE, 0, 0);
}

void
_eglutNativeEventLoop(void)
{
    struct eglut_window *win = _eglut->current;
    ShowWindow(win->native.u.window, SW_SHOW);

    MSG msg = { 0, };
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (_eglut->idle_cb) {
                _eglut->idle_cb();
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            UpdateWindow(win->native.u.window);
        }
    }
}

static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    struct eglut_window *win = _eglut->current;

    switch (message)
    {
    case WM_CREATE:
        break;

    case WM_KEYDOWN:
        break;

    case WM_KEYUP:
        {
            if (win)
            {
                if (win->keyboard_cb) {
                    win->keyboard_cb((UINT8)wParam);
                }
            }
        }
        _eglut->redisplay = 1;
        break;

    case WM_SIZE:
        {
            if (win)
            {
                RECT rect;
                GetWindowRect(hWnd, &rect);
                int w = rect.right - rect.left;
                int h = rect.bottom - rect.top;
                if (w != win->native.width && h != win->native.height)
                {
                    win->native.width = w;
                    win->native.height = h;
                    if (win->reshape_cb)
                        win->reshape_cb(win->native.width, win->native.height);
                }
            }
        }
        _eglut->redisplay = 1;
        break;

    case WM_PAINT:
        {
            if (win)
            {
                if (_eglut->redisplay) {
                    _eglut->redisplay = 0;
                }

                if(win->display_cb)
                    win->display_cb();
                 eglSwapBuffers(_eglut->dpy, win->surface);
             }
        }
        _eglut->redisplay = 1;
        return 0;

    case WM_CLOSE:
        DestroyWindow(hWnd);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}
