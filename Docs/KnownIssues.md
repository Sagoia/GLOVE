# Known Issues

GLOVE is considered as work-in-progress, therefore there are known issues that have to be resolved or improved.

Most Important Known Issues list:

| **Issue** | **Description** | **Solution/Workaround** | **Status** |
| --- | --- | --- | --- |
| glColorMask() not working | Enabling or disabling writing of frame buffer color components r, g, b, a not working | TBD | see issue [#20](https://github.com/Think-Silicon/GLOVE/issues/20) |
| Compressed Textures not supported | glCompressed(Sub)TexImage2D() not implemented | Add missing functionality | see issue [#6](https://github.com/Think-Silicon/GLOVE/issues/6) |
| GL to Vulkan Depth Range conversion adds overhead| Adding ``` gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0; ``` in Vertex Shader | TBD | **unresolved** |
| Vulkan Textures allocated as RGBA in all cases  | Implicit conversion of all textures to GL_RGBA | Allocate Textures according to the input format | see issue [#7](https://github.com/Think-Silicon/GLOVE/issues/7) |
| --- | --- | --- | --- |
| Multiple EGLContexts not working  | Although multiple EGLContexts are supported in theory they are not working correctly| Correct errors related to multiple EGLContexts | **unresolved** |
| Multiple threads not supported  | Multiple threads not implemented yet in EGL | Support multiple threads in EGL | **unresolved** |
