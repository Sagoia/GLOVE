#ifndef __ANDROID_APP_H_
#define __ANDROID_APP_H_

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <android/log.h>

#define GLOVE_LOG(...) __android_log_print(ANDROID_LOG_WARN, "GLOVE_Demo", __VA_ARGS__)
#define GLOVE_LOG_ERROR(...) __android_log_print(ANDROID_LOG_ERROR, "GLOVE_Demo", __VA_ARGS__)

typedef void (*AndroidAppDisplayCB)(void);
typedef void (*AndroidAppInitGLCB)(void);
typedef void (*AndroidAppResizeCB)(int width, int height);
typedef void (*AndroidAppIdleCB)(void);

void setAndroidAppCallbacks(AndroidAppDisplayCB display_cb,
                            AndroidAppInitGLCB  init_gl_cb,
                            AndroidAppResizeCB  resize_cb,
                            AndroidAppIdleCB    idle_cb);

void runAndroidApp(struct android_app *app);

#endif // __ANDROID_APP_H_
