#include <assert.h>

#include <android/log.h>
#include <android/native_window.h>
#include <android_native_app_glue.h>

#include "android_app.h"
#include "common.h"

typedef struct callbacks_s {
    AndroidAppDisplayCB display_cb;
    AndroidAppInitGLCB  init_gl_cb;
    AndroidAppResizeCB  resize_cb;
    AndroidAppIdleCB    idle_cb;
} callbacks_t;

typedef struct egl_params_s {
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    EGLint width, height;
} egl_params_t;

static bool initialized = false;
static bool active = false;

static callbacks_t callbacks;
egl_params_t egl_params;

static void initAppParams();
static void initEGLparams();
static EGLBoolean initAndroidEGL(struct android_app* app);
static void destroyAndroidEGL();
static void drawScene();

static int32_t processInput(struct android_app* app, AInputEvent* event);
static void processCommand(struct android_app* app, int32_t cmd);


static void initAppParams()
{
    initialized = false;
    active = false;
}

static void initEGLparams()
{
    memset(&egl_params, 0, sizeof(egl_params));
}

static EGLBoolean initAndroidEGL(struct android_app* app)
{
    const EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_DEPTH_SIZE, 16,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };

    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    EGLint format = 0;
    EGLint numConfigs = 0;
    EGLConfig config = NULL;

    initEGLparams();

    egl_params.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(egl_params.display == EGL_NO_DISPLAY) {
        GLOVE_LOG_ERROR("eglGetDisplay returned EGL_NO_DISPLAY");
        return EGL_FALSE;
    }

    if(eglInitialize(egl_params.display, NULL, NULL) == EGL_FALSE) {
        GLOVE_LOG_ERROR("eglInitialize failed");
        return EGL_FALSE;
    }

    /* Just pick the first EGLConfig that matches our criteria  */
    if((eglChooseConfig(egl_params.display, configAttribs, &config, 1, &numConfigs) == EGL_FALSE) || numConfigs == 0 ) {
        GLOVE_LOG_ERROR("eglChooseConfig failed");
        return EGL_FALSE;
    }

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID.
     */
    eglGetConfigAttrib(egl_params.display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(app->window, WIDTH, HEIGHT, format);

    egl_params.surface = eglCreateWindowSurface(egl_params.display, config, app->window, NULL);
    if(egl_params.surface == EGL_NO_SURFACE) {
        GLOVE_LOG_ERROR("eglCreateWindowSurface failed");
        return EGL_FALSE;
    }

    egl_params.context = eglCreateContext(egl_params.display, config, EGL_NO_CONTEXT, contextAttribs);
    if(egl_params.context == EGL_NO_CONTEXT) {
        GLOVE_LOG_ERROR("eglCreateContext failed");
        return EGL_FALSE;
    }

    if(eglMakeCurrent(egl_params.display, egl_params.surface, egl_params.surface, egl_params.context) == EGL_FALSE) {
        GLOVE_LOG_ERROR("Unable to eglMakeCurrent");
        return EGL_FALSE;
    }

    if(!eglQuerySurface(egl_params.display, egl_params.surface, EGL_WIDTH, &egl_params.width) ||
       !eglQuerySurface(egl_params.display, egl_params.surface, EGL_HEIGHT, &egl_params.height)) {
        GLOVE_LOG_ERROR("eglQuerySurface failed");
        return EGL_FALSE;
    }

    return EGL_TRUE;
}

static void destroyAndroidEGL()
{
    if(egl_params.display != EGL_NO_DISPLAY) {
        eglMakeCurrent(egl_params.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if(egl_params.context != EGL_NO_CONTEXT) {
            eglDestroyContext(egl_params.display, egl_params.context);
        }
        if(egl_params.surface != EGL_NO_SURFACE) {
            eglDestroySurface(egl_params.display, egl_params.surface);
        }
    }
    return;
}

static int32_t processInput(struct android_app* app, AInputEvent* event)
{
    return 0;
}

static void processCommand(struct android_app* app, int32_t cmd)
{
    switch(cmd) {
        case APP_CMD_INIT_WINDOW: {
            if(app->window) {
                initAppParams();
                if(initAndroidEGL(app)) {
                    callbacks.init_gl_cb();
                    callbacks.resize_cb(egl_params.width, egl_params.height);
                    initialized = true;
                }
            }
            break;
        }
        case APP_CMD_GAINED_FOCUS: {
            active = true;
            break;
        }
        case APP_CMD_LOST_FOCUS: {
            active = false;
            break;
        }
        default:
            break;
    }
}

static void drawScene()
{
    if(callbacks.idle_cb) {
        callbacks.idle_cb();
    }

    callbacks.display_cb();
    eglSwapBuffers(egl_params.display, egl_params.surface);
}

/**********************  PUBLIC FUNCTIONS ***************************/

void setAndroidAppCallbacks(AndroidAppDisplayCB display_cb,
                            AndroidAppInitGLCB  init_gl_cb,
                            AndroidAppResizeCB  resize_cb,
                            AndroidAppIdleCB    idle_cb)
{
    assert(display_cb);
    assert(init_gl_cb);
    assert(resize_cb);

    callbacks.display_cb = display_cb;
    callbacks.init_gl_cb = init_gl_cb;
    callbacks.resize_cb = resize_cb;
    callbacks.idle_cb = idle_cb;
}

void runAndroidApp(struct android_app *app)
{
    app->onAppCmd = processCommand;
    app->onInputEvent = processInput;

    while(1) {
        int events;
        struct android_poll_source* source;
        while(ALooper_pollAll(active ? 0 : -1, NULL, &events, (void**)&source) >= 0) {
            if(source) {
                source->process(app, source);
            }

            if(app->destroyRequested != 0) {
                destroyAndroidEGL();
                return;
            }
        }
        if(initialized && active) {
            drawScene();
        }
    }
}
