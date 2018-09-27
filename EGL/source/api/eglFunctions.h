#ifndef EGL_FUNC_PTR
#define EGL_FUNC_PTR(f) { #f, reinterpret_cast<__eglMustCastToProperFunctionPointerType>(f) }
#endif //  EGL_FUNC_PTR

#include "EGL/egl.h"
#include <unordered_map>
static const std::unordered_map<std::string, __eglMustCastToProperFunctionPointerType> eglFPMap = {
#ifdef EGL_VERSION_1_0
EGL_FUNC_PTR(eglChooseConfig),
EGL_FUNC_PTR(eglCopyBuffers),
EGL_FUNC_PTR(eglCreateContext),
EGL_FUNC_PTR(eglCreatePbufferSurface),
EGL_FUNC_PTR(eglCreatePixmapSurface),
EGL_FUNC_PTR(eglCreateWindowSurface),
EGL_FUNC_PTR(eglDestroyContext),
EGL_FUNC_PTR(eglDestroySurface),
EGL_FUNC_PTR(eglGetConfigAttrib),
EGL_FUNC_PTR(eglGetConfigs),
EGL_FUNC_PTR(eglGetCurrentDisplay),
EGL_FUNC_PTR(eglGetCurrentSurface),
EGL_FUNC_PTR(eglGetDisplay),
EGL_FUNC_PTR(eglGetError),
EGL_FUNC_PTR(eglGetProcAddress),
EGL_FUNC_PTR(eglInitialize),
EGL_FUNC_PTR(eglMakeCurrent),
EGL_FUNC_PTR(eglQueryContext),
EGL_FUNC_PTR(eglQueryString),
EGL_FUNC_PTR(eglQuerySurface),
EGL_FUNC_PTR(eglSwapBuffers),
EGL_FUNC_PTR(eglTerminate),
EGL_FUNC_PTR(eglWaitGL),
EGL_FUNC_PTR(eglWaitNative),
#endif /* EGL_VERSION_1_0 */
#ifdef EGL_VERSION_1_1
EGL_FUNC_PTR(eglBindTexImage),
EGL_FUNC_PTR(eglReleaseTexImage),
EGL_FUNC_PTR(eglSurfaceAttrib),
EGL_FUNC_PTR(eglSwapInterval),
#endif /* EGL_VERSION_1_1 */
#ifdef EGL_VERSION_1_2
EGL_FUNC_PTR(eglBindAPI),
EGL_FUNC_PTR(eglQueryAPI),
EGL_FUNC_PTR(eglCreatePbufferFromClientBuffer),
EGL_FUNC_PTR(eglReleaseThread),
EGL_FUNC_PTR(eglWaitClient),
#endif /* EGL_VERSION_1_2 */
#ifdef EGL_VERSION_1_3
#endif /* EGL_VERSION_1_3 */
#ifdef EGL_VERSION_1_4
EGL_FUNC_PTR(eglGetCurrentContext)
#endif /* EGL_VERSION_1_4 */
};
#undef EGL_FUNC_PTR

__eglMustCastToProperFunctionPointerType GetEGLProcAddr(const char *procname)
{
    if(strncmp(procname, "egl", 3) == 0) {
        const auto& it = eglFPMap.find(procname);
        if(it != eglFPMap.end()) {
            return it->second;
        }
    }
    return nullptr;
}
