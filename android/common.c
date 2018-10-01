#include <stdbool.h>
#include <string.h>
#include "common.h"
#include <sys/time.h>

#define __VMS

#define SOURCES_PATH "/data/local/tmp/"


static EGLDisplay   egl_display = EGL_NO_DISPLAY;
static EGLSurface   egl_surface = EGL_NO_SURFACE;
static EGLContext   context     = EGL_NO_CONTEXT;
static EGLConfig    config;

static bool         saveImages  = false;

static void
ParseArguments(int argc, const char **argv)
{
    if(argc == 2 && !strcmp(argv[1], "--save-images")) {
        saveImages = true;
    }
}

static const char *
ExecutableName(const char *name)
{
    return strchr(name, '/') ? strchr(name, '/') + 1 : name;
}

static void
TakeScreenshot(const char *name)
{
    char filename[64];
    if(!strcmp(name - 5, ".rgba")) {
        snprintf(filename, 64, "%s%s", SOURCES_PATH, name);
    } else {
        snprintf(filename, 64, "%s%s.rgba", SOURCES_PATH, name);
    }

    size_t size = WIDTH * HEIGHT * sizeof(uint32_t);
    uint32_t *pixels = (uint32_t *)malloc(size);
    assert(pixels);
    memset(pixels, 0xaa, size);

    glReadPixels(0, 0, WIDTH, HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    FILE *fp = fopen(filename, "w");
    size_t writtenBytes = fwrite(pixels, 1, size, fp);
    if(writtenBytes != size) {
        GLOVE_LOG_ERROR("fwrite had a problem.");
        exit(1);
    }
    fclose(fp);
    free(pixels);

}

static int
LoadShaderSource(const char* filename, char **shaderSource, int *length)
{
    FILE *file;
    file = fopen(filename, "r");
    if(!file) {
        GLOVE_LOG_ERROR("Error opening file %s", filename);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    unsigned int len = ftell(file) + 1;
    fseek(file, 0, SEEK_SET);

    if(!len) {
        return 0;
    }

    *length = (int)len;
    *shaderSource = (char *) malloc((len) * sizeof(char));
    assert(*shaderSource);
    char *src = *shaderSource;

    // len isn't always strlen cause some characters are stripped in ascii read...
    // it is important to 0-terminate the real length later, len is just max possible value...
    size_t ret = fread(src, 1, len - 1, file);
    assert(ret == len - 1);
    src[len - 1] = '\0';

    fclose(file);

    return 1;
}

static void
resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
}

static int
initEGL(void)
{
    egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if(egl_display == EGL_NO_DISPLAY) {
        GLOVE_LOG_ERROR("Can't get display!");
        return 0;
    }

    int majorVersion, minorVersion;
    if(!eglInitialize(egl_display, &majorVersion, &minorVersion)) {
        GLOVE_LOG_ERROR("Can't initialize EGL display");
        return 0;
    }

    EGLint count = 2;
    EGLConfig configs[2];

    if(eglGetConfigs(egl_display, configs, 2, &count) == EGL_FALSE) {
        assert(0);
        return 0;
    }

    EGLint attrib_list[] = { EGL_WIDTH,  WIDTH,
                             EGL_HEIGHT, HEIGHT,
                             EGL_NONE };

    egl_surface = eglCreatePbufferSurface(egl_display, configs[0], attrib_list);
    if(egl_surface == EGL_NO_SURFACE) {
        GLOVE_LOG_ERROR("Can't create EGL window");
        return 0;
    }

    context = eglCreateContext(egl_display, config, NULL, NULL);
    if(!context) {
        GLOVE_LOG_ERROR("Can't create a context!");
    }

    if(!eglMakeCurrent(egl_display, egl_surface, egl_surface, context)) {
        GLOVE_LOG_ERROR("Can't make context current");
        return 0;
    }

    GLOVE_LOG_WARN("GLES implementation info:");
    GLOVE_LOG_WARN("GL_VENDOR: %s", glGetString(GL_VENDOR));
    GLOVE_LOG_WARN("GL_RENDERER: %s", glGetString(GL_RENDERER));
    GLOVE_LOG_WARN("GL_VERSION: %s", glGetString(GL_VERSION));
    GLOVE_LOG_WARN("GL_SHADING_LANGUAGE_VERSION: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    GLOVE_LOG_WARN("GL_EXTENSIONS: %s", glGetString(GL_EXTENSIONS));

    return 1;
}

static void
destroyEGL(void)
{
    eglDestroySurface(egl_display, egl_surface);
    eglDestroyContext(egl_display, context);

    eglTerminate(egl_display);
}

char *
readShaderFile(const char *filename)
{
    assert(filename);

    FILE *fp = fopen(filename, "r");
    if(!fp) {
        GLOVE_LOG_ERROR("Can not open file \"%s\"", filename);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    size_t shaderSize = ftell(fp);
    rewind(fp);

    if(!shaderSize) {
        fclose(fp);
        return NULL;
    }

    char *shaderSource = (char *)malloc((shaderSize + 1) * sizeof(char));
    assert(shaderSource);

    size_t len = fread((void *)shaderSource, 1, shaderSize, fp);
    assert(len == shaderSize);

    shaderSource[len] = '\0';

    fclose(fp);
    return shaderSource;
}

bool
loadGlShaders(const char *vsFilename, GLuint *vs, const char *fsFilename, GLuint *fs, GLuint *prog)
{
    int length = 0;
    char *source = NULL;

    *vs = glCreateShader(GL_VERTEX_SHADER);
    *fs = glCreateShader(GL_FRAGMENT_SHADER);
    *prog = glCreateProgram();

/// Vertex shader
    LoadShaderSource(vsFilename, &source, &length);
    assert(source);
    assert(length);

    assert(glIsShader(*vs));
    glGetShaderiv(*vs, GL_SHADER_TYPE, &length);
    ASSERT_NO_GL_ERROR();
    assert(length == GL_VERTEX_SHADER);

    glShaderSource(*vs, 1, (const char **) &source, NULL);
    ASSERT_NO_GL_ERROR();
    glCompileShader(*vs);
    ASSERT_NO_GL_ERROR();

    glGetShaderiv(*vs, GL_SHADER_SOURCE_LENGTH, &length);
    ASSERT_NO_GL_ERROR();
    char *src = (char *)malloc(length * sizeof(char));
    glGetShaderSource(*vs, length, NULL, src);
    ASSERT_NO_GL_ERROR();
//    printf("\n\nVertex shader source:\n%s\n\n", src);
    free(src);

    glAttachShader(*prog, *vs);
    ASSERT_NO_GL_ERROR();

    glGetShaderiv(*vs, GL_COMPILE_STATUS, &length);
    ASSERT_NO_GL_ERROR();
    if(length == GL_FALSE) {
        glGetShaderiv(*vs, GL_INFO_LOG_LENGTH, &length);
        char *info = (char *)malloc(length * sizeof(char));
        glGetShaderInfoLog(*vs, length, NULL, info);
        GLOVE_LOG_ERROR("Failed to compile vertex shader.");
        GLOVE_LOG_ERROR("Compiler log:");
        GLOVE_LOG_ERROR("***********\n%s\n***********", info);
        assert(0);

        free(info);
        return false;
    }
    free(source);

/// Fragment shader
    LoadShaderSource(fsFilename, &source, &length);
    assert(source);
    assert(length);

    assert(glIsShader(*fs));
    glGetShaderiv(*fs, GL_SHADER_TYPE, &length);
    ASSERT_NO_GL_ERROR();
    assert(length == GL_FRAGMENT_SHADER);

    glShaderSource(*fs, 1, (const char **) &source, NULL);
    ASSERT_NO_GL_ERROR();
    glCompileShader(*fs);
    ASSERT_NO_GL_ERROR();

    glGetShaderiv(*fs, GL_COMPILE_STATUS, &length);
    ASSERT_NO_GL_ERROR();
    if(length == GL_FALSE) {
        glGetShaderiv(*fs, GL_INFO_LOG_LENGTH, &length);
        char *info = (char *)malloc(length * sizeof(char));
        glGetShaderInfoLog(*fs, length, NULL, info);
        GLOVE_LOG_ERROR("\n\nFailed to compile fragment shader.\n");
        GLOVE_LOG_ERROR("Compiler log:\n");
        GLOVE_LOG_ERROR("***********\n%s\n***********\n", info);
        assert(0);

        free(info);
        return false;
    }
    free(source);

    glGetShaderiv(*fs, GL_SHADER_SOURCE_LENGTH, &length);
    ASSERT_NO_GL_ERROR();
    src = (char *)malloc(length * sizeof(char));
    glGetShaderSource(*fs, length, NULL, src);
    ASSERT_NO_GL_ERROR();
    free(src);

    glAttachShader(*prog, *fs);

    assert(glIsProgram(*prog));

/// Link program
    glLinkProgram(*prog);

    glGetProgramiv(*prog, GL_LINK_STATUS, &length);
    ASSERT_NO_GL_ERROR();
    if(length == GL_FALSE) {
        glGetShaderiv(*fs, GL_INFO_LOG_LENGTH, &length);
        char *info = (char *)malloc(length * sizeof(char));
        glGetShaderInfoLog(*fs, length, NULL, info);
        GLOVE_LOG_ERROR("\n\nFailed to link shaders.\n");
        GLOVE_LOG_ERROR("Compiler log:\n");
        GLOVE_LOG_ERROR("***********\n%s\n***********\n", info);
        assert(0);

        return false;
    }

/// Query program
    /// Print active uniforms
    glGetProgramiv(*prog, GL_ACTIVE_UNIFORMS, &length);
    ASSERT_NO_GL_ERROR();
    GLOVE_LOG_WARN("%d active uniform(s)\n", length);
    for(int i = 0; i < length; ++i) {
        char buf[20];
        int ll = 0;
        glGetActiveUniform(*prog, i, 20, &ll, NULL, NULL, buf);
        GLOVE_LOG_ERROR("   %s (%d)\n", buf, ll);
    }

    /// Print active attributes
    glGetProgramiv(*prog, GL_ACTIVE_ATTRIBUTES, &length);
    ASSERT_NO_GL_ERROR();
    GLOVE_LOG_WARN("%d active attribute(s)\n", length);
    for(int i = 0; i < length; ++i) {
        char buf[20];
        int ll = 0;
        GLenum type;
        int size;
        glGetActiveAttrib(*prog, i, 20, &ll, &size, &type, buf);
        GLOVE_LOG_ERROR("   %s (%d)\n", buf, ll);
    }

    glUseProgram(*prog);

    return true;
}
