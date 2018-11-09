#include <system/graphics.h>

#if EGL_SUPPORT_ONLY_PBUFFER_SURFACE == 0
#   define EGL_AVAILABLE_SURFACES (EGL_PBUFFER_BIT | EGL_WINDOW_BIT)
#else
#   define EGL_AVAILABLE_SURFACES (EGL_PBUFFER_BIT)
#endif

const EGLConfig_t EglConfigs[4] = {
                                   { 0,   // Display
                                    32,   // BufferSize
                                     8,   // AlphaSize
                                     8,   // BlueSize
                                     8,   // GreenSize
                                     8,   // RedSize
                                    24,   // DepthSize
                                     8,   // StencilSize
                              EGL_NONE,   // ConfigCaveat
                                     1,   // ConfigID
                                     0,   // Level
                                  1080,   // MaxPbufferHeight
                           1920 * 1080,   // MaxPbufferPixels
                                  1920,   // MaxPbufferWidth
                             EGL_FALSE,   // NativeRenderable
            HAL_PIXEL_FORMAT_RGBA_8888,   // NativeVisualID
            HAL_PIXEL_FORMAT_RGBA_8888,   // NativeVisualType
                                     0,   // Samples
                                     0,   // SampleBuffers
                EGL_AVAILABLE_SURFACES,   // SurfaceType
                              EGL_NONE,   // TransparentType
                                     0,   // TransparentBlueValue
                                     0,   // TransparentGreenValue
                                     0,   // TransparentRedValue
                             EGL_FALSE,   // BindToTextureRGB
                              EGL_TRUE,   // BindToTextureRGBA
                                     0,   // MinSwapInterval
                                     1,   // MaxSwapInterval
                                     0,   // LuminanceSize
                                     0,   // AlphaMaskSize
                        EGL_RGB_BUFFER,   // ColorBufferType
                    EGL_OPENGL_ES2_BIT,   // RenderableType
                              EGL_NONE,   // MatchNativePixmap
                                   0x4,   // Conformant
                             EGL_FALSE,   // RecordableAndroid
                             EGL_FALSE},  // FramebufferTargetAndroid
                                   
                                   { 0,   // Display
                                    24,   // BufferSize
                                     0,   // AlphaSize
                                     8,   // BlueSize
                                     8,   // GreenSize
                                     8,   // RedSize
                                    24,   // DepthSize
                                     8,   // StencilSize
                              EGL_NONE,   // ConfigCaveat
                                     2,   // ConfigID
                                     0,   // Level
                                  1080,   // MaxPbufferHeight
                           1920 * 1080,   // MaxPbufferPixels
                                  1920,   // MaxPbufferWidth
                             EGL_FALSE,   // NativeRenderable
            HAL_PIXEL_FORMAT_RGBX_8888,   // NativeVisualID
            HAL_PIXEL_FORMAT_RGBX_8888,   // NativeVisualType
                                     0,   // Samples
                                     0,   // SampleBuffers
                EGL_AVAILABLE_SURFACES,   // SurfaceType
                              EGL_NONE,   // TransparentType
                                     0,   // TransparentBlueValue
                                     0,   // TransparentGreenValue
                                     0,   // TransparentRedValue
                             EGL_FALSE,   // BindToTextureRGB
                              EGL_TRUE,   // BindToTextureRGBA
                                     0,   // MinSwapInterval
                                     1,   // MaxSwapInterval
                                     0,   // LuminanceSize
                                     0,   // AlphaMaskSize
                        EGL_RGB_BUFFER,   // ColorBufferType
                    EGL_OPENGL_ES2_BIT,   // RenderableType
                              EGL_NONE,   // MatchNativePixmap
                                   0x4,   // Conformant
                              EGL_TRUE,   // RecordableAndroid
                              EGL_TRUE},  // FramebufferTargetAndroid
                                   
                                   { 0,   // Display
                                    24,   // BufferSize
                                     0,   // AlphaSize
                                     8,   // BlueSize
                                     8,   // GreenSize
                                     8,   // RedSize
                                     0,   // DepthSize
                                     0,   // StencilSize
                              EGL_NONE,   // ConfigCaveat
                                     3,   // ConfigID
                                     0,   // Level
                                  1080,   // MaxPbufferHeight
                           1920 * 1080,   // MaxPbufferPixels
                                  1920,   // MaxPbufferWidth
                             EGL_FALSE,   // NativeRenderable
            HAL_PIXEL_FORMAT_RGBX_8888,   // NativeVisualID
            HAL_PIXEL_FORMAT_RGBX_8888,   // NativeVisualType
                                     0,   // Samples
                                     0,   // SampleBuffers
                EGL_AVAILABLE_SURFACES,   // SurfaceType
                              EGL_NONE,   // TransparentType
                                     0,   // TransparentBlueValue
                                     0,   // TransparentGreenValue
                                     0,   // TransparentRedValue
                             EGL_FALSE,   // BindToTextureRGB
                              EGL_TRUE,   // BindToTextureRGBA
                                     0,   // MinSwapInterval
                                     1,   // MaxSwapInterval
                                     0,   // LuminanceSize
                                     0,   // AlphaMaskSize
                        EGL_RGB_BUFFER,   // ColorBufferType
                    EGL_OPENGL_ES2_BIT,   // RenderableType
                              EGL_NONE,   // MatchNativePixmap
                                   0x4,   // Conformant
                             EGL_FALSE,   // RecordableAndroid
                             EGL_FALSE},  // FramebufferTargetAndroid

                                   { 0,   // Display
                                    32,   // BufferSize
                                     8,   // AlphaSize
                                     8,   // BlueSize
                                     8,   // GreenSize
                                     8,   // RedSize
                                     0,   // DepthSize
                                     8,   // StencilSize
                              EGL_NONE,   // ConfigCaveat
                                     4,   // ConfigID
                                     0,   // Level
                                  1080,   // MaxPbufferHeight
                           1920 * 1080,   // MaxPbufferPixels
                                  1920,   // MaxPbufferWidth
                             EGL_FALSE,   // NativeRenderable
            HAL_PIXEL_FORMAT_RGBA_8888,   // NativeVisualID
            HAL_PIXEL_FORMAT_RGBA_8888,   // NativeVisualType
                                     0,   // Samples
                                     0,   // SampleBuffers
                EGL_AVAILABLE_SURFACES,   // SurfaceType
                              EGL_NONE,   // TransparentType
                                     0,   // TransparentBlueValue
                                     0,   // TransparentGreenValue
                                     0,   // TransparentRedValue
                             EGL_FALSE,   // BindToTextureRGB
                              EGL_TRUE,   // BindToTextureRGBA
                                     0,   // MinSwapInterval
                                     1,   // MaxSwapInterval
                                     0,   // LuminanceSize
                                     0,   // AlphaMaskSize
                        EGL_RGB_BUFFER,   // ColorBufferType
                    EGL_OPENGL_ES2_BIT,   // RenderableType
                              EGL_NONE,   // MatchNativePixmap
                                   0x4,   // Conformant
                             EGL_FALSE,   // RecordableAndroid
                             EGL_FALSE}   // FramebufferTargetAndroid
};
