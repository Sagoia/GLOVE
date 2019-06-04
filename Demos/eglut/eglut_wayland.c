/*
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
 */

#include <wayland-client.h>
#include <wayland-egl.h>

#include "eglutint.h"

struct wl_compositor *compositor = NULL;
struct wl_shell *shell;
struct wl_surface *surface;
struct wl_shell_surface *shell_surface;

// Input devices
struct wl_seat *seat;
struct wl_keyboard *keyboard;

static char*
iso8859_1_to_utf8(const char *str)
{
    char *utf8 = malloc(1 + (2 * strlen(str)));

    while(*str) {
        uint8_t ch = (uint8_t)(*str);
        if(ch < 0x80) {
            *utf8++ = ch;
        } else {
            *utf8++ = 0xc0 | ch >> 6;
            *utf8++ = 0x80 | (ch & 0x3f);
        }
        str++;
    }
    return utf8;
}

static void
keyboard_handle_keymap(void *data, struct wl_keyboard *keyboard,
                       uint32_t format, int fd, uint32_t size)
{
}

static void
keyboard_handle_enter(void *data, struct wl_keyboard *keyboard,
                      uint32_t serial, struct wl_surface *surface,
                      struct wl_array *keys)
{
    fprintf(stdout, "[eglut_wayland.c]: Keyboard gained focus\n");
}

static void
keyboard_handle_leave(void *data, struct wl_keyboard *keyboard,
                      uint32_t serial, struct wl_surface *surface)
{
    fprintf(stdout, "[eglut_wayland.c]: Keyboard lost focus\n");
}

static void
keyboard_handle_key(void *data, struct wl_keyboard *keyboard,
                    uint32_t serial, uint32_t time, uint32_t key,
                    uint32_t state)
{
    struct eglut_window *win = _eglut->current;
    if(state == 1 && key == 1) { // ESC KEY handling
        win->keyboard_cb(27);
    }
}

static void
keyboard_handle_modifiers(void *data, struct wl_keyboard *keyboard,
                          uint32_t serial, uint32_t mods_depressed,
                          uint32_t mods_latched, uint32_t mods_locked,
                          uint32_t group)
{
}

static void
keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard,
                     int32_t rate, int32_t delay)
{
}


static const struct wl_keyboard_listener keyboard_listener = {
    keyboard_handle_keymap,
    keyboard_handle_enter,
    keyboard_handle_leave,
    keyboard_handle_key,
    keyboard_handle_modifiers,
    keyboard_repeat_info
};

static void
seat_handle_capabilities(void *data, struct wl_seat *seat,
                         enum wl_seat_capability caps)
{
    if (caps & WL_SEAT_CAPABILITY_KEYBOARD) {
        keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(keyboard, &keyboard_listener, NULL);
    } else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD)) {
        wl_keyboard_destroy(keyboard);
        keyboard = NULL;
    }
}

static void
seat_handle_name(void *data, struct wl_seat *wl_seat,
                 const char *name)
{
}

static const struct wl_seat_listener seat_listener = {
    seat_handle_capabilities,
    seat_handle_name
};

static void
global_registry_handler(void *data, struct wl_registry *registry, uint32_t id,
           const char *interface, uint32_t version)
{
    if (strcmp(interface, "wl_compositor") == 0) {
        compositor = wl_registry_bind(registry,
                                      id,
                                      &wl_compositor_interface,
                                      1);
    } else if (strcmp(interface, "wl_shell") == 0) {
        shell = wl_registry_bind(registry, id,
                                 &wl_shell_interface, 1);

    } else if (strcmp(interface, "wl_seat") == 0) {
        seat = wl_registry_bind(registry, id,
                                &wl_seat_interface, 1);
        wl_seat_add_listener(seat, &seat_listener, NULL);
    }
}

static void
global_registry_remover(void *data, struct wl_registry *registry, uint32_t id)
{
}

static const struct wl_registry_listener registry_listener = {
    global_registry_handler,
    global_registry_remover
};

void
_eglutNativeInitDisplay(void)
{
    _eglut->native_dpy = wl_display_connect(NULL);
    if (!_eglut->native_dpy)
        fprintf(stderr, "[eglut_wayland.c] [_eglutNativeInitDisplay()]: Failed to initialize native display %s\n", _eglut->display_name);

    _eglut->surface_type = _eglut->native_dpy ? EGL_WINDOW_BIT : EGL_PBUFFER_BIT;

    struct wl_registry *registry = wl_display_get_registry(_eglut->native_dpy);
    wl_registry_add_listener(registry, &registry_listener, NULL);

    wl_display_dispatch(_eglut->native_dpy);
    wl_display_roundtrip(_eglut->native_dpy);

    if (compositor == NULL || shell == NULL) {
        fprintf(stderr, "[eglut_wayland.c] [_eglutNativeInitDisplay()]: Can't find compositor or shell\n");
        exit(1);
    } else {
        fprintf(stdout, "[eglut_wayland.c] [_eglutNativeInitDisplay()]: Found compositor and shell\n");
    }
}

void
_eglutStoreName(const char *title)
{
}

void
_eglutNativeFiniDisplay(void)
{
    wl_display_disconnect(_eglut->native_dpy);
}

void
_eglutNativeInitWindow(struct eglut_window *win, const char *title,
                       int x, int y, int w, int h)
{
    surface = wl_compositor_create_surface(compositor);
    if (surface == NULL) {
        fprintf(stderr, "[eglut_wayland.c] [_eglutNativeInitWindow()]: Can't create surface\n");
        exit(1);
    } else {
        fprintf(stdout, "[eglut_wayland.c] [_eglutNativeInitWindow()]: Created surface\n");
    }

    shell_surface = wl_shell_get_shell_surface(shell, surface);
    wl_shell_surface_set_title(shell_surface, iso8859_1_to_utf8("experiment"));
    wl_shell_surface_set_toplevel(shell_surface);

    win->native.u.window = wl_egl_window_create(surface, w, h);
    if (win->native.u.window == EGL_NO_SURFACE) {
        fprintf(stderr, "[eglut_wayland.c] [_eglutNativeInitWindow()]: Can't create egl window\n");
        exit(1);
    } else {
        fprintf(stdout, "[eglut_wayland.c] [_eglutNativeInitWindow()]: Created egl window\n");
    }

    win->native.width = w;
    win->native.height = h;
}

void
_eglutNativeFiniWindow(struct eglut_window *win)
{
}

static void
next_event(struct eglut_window *win)
{
    if(_eglut->idle_cb) {
      _eglut->idle_cb();
    }
    _eglut->redisplay = 1;
}

void
_eglutNativeEventLoop(void)
{
   while (1) {
      struct eglut_window *win = _eglut->current;

      next_event(win);

      if (_eglut->redisplay) {
          _eglut->redisplay = 0;

          if (win->display_cb)
              win->display_cb();
          eglSwapBuffers(_eglut->dpy, win->surface);

          wl_display_roundtrip(_eglut->native_dpy);
      }
   }
}
