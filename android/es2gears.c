/*
 * Copyright (C) 1999-2001  Brian Paul   All Rights Reserved.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * Ported to GLES2.
 * Kristian Høgsberg <krh@bitplanet.net>
 * May 3, 2010
 *
 * Improve GLES2 port:
 *   * Refactor gear drawing.
 *   * Use correct normals for surfaces.
 *   * Improve shader.
 *   * Use perspective projection transformation.
 *   * Add FPS count.
 *   * Add comments.
 * Alexandros Frantzis <alexandros.frantzis@linaro.org>
 * Jul 13, 2010
 */

#define GL_GLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES

#define _GNU_SOURCE

#include "common.c"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "android_app.h"

#define VERTEX_SHADER SOURCES_PATH "es2gears.vert"
#define FRAGMENT_SHADER SOURCES_PATH "es2gears.frag"

#define STRIPS_PER_TOOTH 7
#define VERTICES_PER_TOOTH 34
#define GEAR_VERTEX_STRIDE 6

/**
 * Struct describing the vertices in triangle strip
 */
struct vertex_strip {
    /** The first vertex in the strip */
    GLint first;
    /** The number of consecutive vertices in the strip after the first */
    GLint count;
};

/* Each vertex consist of GEAR_VERTEX_STRIDE GLfloat attributes */
typedef GLfloat GearVertex[GEAR_VERTEX_STRIDE];

/**
 * Struct representing a gear.
 */
struct gear {
    /** The array of vertices comprising the gear */
    GearVertex *vertices;
    /** The number of vertices comprising the gear */
    int nvertices;
    /** The array of triangle strips comprising the gear */
    struct vertex_strip *strips;
    /** The number of triangle strips comprising the gear */
    int nstrips;
    /** The Vertex Buffer Object holding the vertices in the graphics card */
    GLuint vbo;
};

/** The view rotation [x, y, z] */
static GLfloat view_rot[3] = { 20.0, 30.0, 0.0 };
/** The gears */
static struct gear *gear1, *gear2, *gear3;
/** The current gear rotation angle */
static GLfloat angle = 0.0;
/** The location of the shader uniforms */
static GLuint ModelViewProjectionMatrix_location,
       NormalMatrix_location,
       LightSourcePosition_location,
       MaterialColor_location;
/** The projection matrix */
static GLfloat ProjectionMatrix[16];
/** The direction of the directional light for the scene */
static const GLfloat LightSourcePosition[4] = { 5.0, 5.0, 10.0, 1.0};
/** Shaders */
static GLuint vertexShader, fragmentShader, shaderProgram;

/**
 * Fills a gear vertex.
 *
 * @param v the vertex to fill
 * @param x the x coordinate
 * @param y the y coordinate
 * @param z the z coortinate
 * @param n pointer to the normal table
 *
 * @return the operation error code
 */
static GearVertex *
vert(GearVertex *v, GLfloat x, GLfloat y, GLfloat z, GLfloat n[3])
{
    v[0][0] = x;
    v[0][1] = y;
    v[0][2] = z;
    v[0][3] = n[0];
    v[0][4] = n[1];
    v[0][5] = n[2];

    return v + 1;
}

/**
 *  Create a gear wheel.
 *
 *  @param inner_radius radius of hole at center
 *  @param outer_radius radius at center of teeth
 *  @param width width of gear
 *  @param teeth number of teeth
 *  @param tooth_depth depth of tooth
 *
 *  @return pointer to the constructed struct gear
 */
static struct gear *
create_gear(GLfloat inner_radius, GLfloat outer_radius, GLfloat width,
            GLint teeth, GLfloat tooth_depth)
{
    GLfloat r0, r1, r2;
    GLfloat da;
    GearVertex *v;
    struct gear *gear;
    double s[5], c[5];
    GLfloat normal[3];
    int cur_strip = 0;
    int i;

    /* Allocate memory for the gear */
    gear = malloc(sizeof * gear);
    if(gear == NULL) {
        return NULL;
    }

    /* Calculate the radii used in the gear */
    r0 = inner_radius;
    r1 = outer_radius - tooth_depth / 2.0;
    r2 = outer_radius + tooth_depth / 2.0;

    da = 2.0 * M_PI / teeth / 4.0;

    /* Allocate memory for the triangle strip information */
    gear->nstrips = STRIPS_PER_TOOTH * teeth;
    gear->strips = calloc(gear->nstrips, sizeof(*gear->strips));

    /* Allocate memory for the vertices */
    gear->vertices = calloc(VERTICES_PER_TOOTH * teeth, sizeof(*gear->vertices));
    v = gear->vertices;

    for(i = 0; i < teeth; i++) {
        /* Calculate needed sin/cos for varius angles */
        sincos(i * 2.0 * M_PI / teeth, &s[0], &c[0]);
        sincos(i * 2.0 * M_PI / teeth + da, &s[1], &c[1]);
        sincos(i * 2.0 * M_PI / teeth + da * 2, &s[2], &c[2]);
        sincos(i * 2.0 * M_PI / teeth + da * 3, &s[3], &c[3]);
        sincos(i * 2.0 * M_PI / teeth + da * 4, &s[4], &c[4]);

        /* A set of macros for making the creation of the gears easier */
#define  GEAR_POINT(r, da) { (r) * c[(da)], (r) * s[(da)] }
#define  SET_NORMAL(x, y, z) do { \
        normal[0] = (x); normal[1] = (y); normal[2] = (z); \
    } while(0)

#define  GEAR_VERT(v, point, sign) vert((v), p[(point)].x, p[(point)].y, (sign) * width * 0.5, normal)

#define START_STRIP do { \
        gear->strips[cur_strip].first = v - gear->vertices; \
    } while(0);

#define END_STRIP do { \
        int _tmp = (v - gear->vertices); \
        gear->strips[cur_strip].count = _tmp - gear->strips[cur_strip].first; \
        cur_strip++; \
    } while (0)

#define QUAD_WITH_NORMAL(p1, p2) do { \
        SET_NORMAL((p[(p1)].y - p[(p2)].y), -(p[(p1)].x - p[(p2)].x), 0); \
        v = GEAR_VERT(v, (p1), -1); \
        v = GEAR_VERT(v, (p1), 1); \
        v = GEAR_VERT(v, (p2), -1); \
        v = GEAR_VERT(v, (p2), 1); \
    } while(0)

        struct point {
            GLfloat x;
            GLfloat y;
        };

        /* Create the 7 points (only x,y coords) used to draw a tooth */
        struct point p[7] = {
            GEAR_POINT(r2, 1), // 0
            GEAR_POINT(r2, 2), // 1
            GEAR_POINT(r1, 0), // 2
            GEAR_POINT(r1, 3), // 3
            GEAR_POINT(r0, 0), // 4
            GEAR_POINT(r1, 4), // 5
            GEAR_POINT(r0, 4), // 6
        };

        /* Front face */
        START_STRIP;
        SET_NORMAL(0, 0, 1.0);
        v = GEAR_VERT(v, 0, +1);
        v = GEAR_VERT(v, 1, +1);
        v = GEAR_VERT(v, 2, +1);
        v = GEAR_VERT(v, 3, +1);
        v = GEAR_VERT(v, 4, +1);
        v = GEAR_VERT(v, 5, +1);
        v = GEAR_VERT(v, 6, +1);
        END_STRIP;

        /* Inner face */
        START_STRIP;
        QUAD_WITH_NORMAL(4, 6);
        END_STRIP;

        /* Back face */
        START_STRIP;
        SET_NORMAL(0, 0, -1.0);
        v = GEAR_VERT(v, 6, -1);
        v = GEAR_VERT(v, 5, -1);
        v = GEAR_VERT(v, 4, -1);
        v = GEAR_VERT(v, 3, -1);
        v = GEAR_VERT(v, 2, -1);
        v = GEAR_VERT(v, 1, -1);
        v = GEAR_VERT(v, 0, -1);
        END_STRIP;

        /* Outer face */
        START_STRIP;
        QUAD_WITH_NORMAL(0, 2);
        END_STRIP;

        START_STRIP;
        QUAD_WITH_NORMAL(1, 0);
        END_STRIP;

        START_STRIP;
        QUAD_WITH_NORMAL(3, 1);
        END_STRIP;

        START_STRIP;
        QUAD_WITH_NORMAL(5, 3);
        END_STRIP;
    }

    gear->nvertices = (v - gear->vertices);

    /* Store the vertices in a vertex buffer object (VBO) */
    glGenBuffers(1, &gear->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, gear->vbo);
    glBufferData(GL_ARRAY_BUFFER, gear->nvertices * sizeof(GearVertex), gear->vertices, GL_STATIC_DRAW);

    return gear;
}

/**
 * Multiplies two 4x4 matrices.
 *
 * The result is stored in matrix m.
 *
 * @param m the first matrix to multiply
 * @param n the second matrix to multiply
 */
static void
multiply(GLfloat *m, const GLfloat *n)
{
    GLfloat tmp[16];
    const GLfloat *row, *column;
    div_t d;
    int i, j;

    for(i = 0; i < 16; i++) {
        tmp[i] = 0;
        d = div(i, 4);
        row = n + d.quot * 4;
        column = m + d.rem;
        for(j = 0; j < 4; j++) {
            tmp[i] += row[j] * column[j * 4];
        }
    }
    memcpy(m, &tmp, sizeof tmp);
}

/**
 * Rotates a 4x4 matrix.
 *
 * @param[in,out] m the matrix to rotate
 * @param angle the angle to rotate
 * @param x the x component of the direction to rotate to
 * @param y the y component of the direction to rotate to
 * @param z the z component of the direction to rotate to
 */
static void
rotate(GLfloat *m, GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    double s, c;

    sincos(angle, &s, &c);
    GLfloat r[16] = {
        x * x * (1 - c) + c,     y * x * (1 - c) + z * s, x * z * (1 - c) - y * s, 0,
        x * y * (1 - c) - z * s, y * y * (1 - c) + c,     y * z * (1 - c) + x * s, 0,
        x * z * (1 - c) + y * s, y * z * (1 - c) - x * s, z * z * (1 - c) + c,     0,
        0, 0, 0, 1
    };

    multiply(m, r);
}


/**
 * Translates a 4x4 matrix.
 *
 * @param[in,out] m the matrix to translate
 * @param x the x component of the direction to translate to
 * @param y the y component of the direction to translate to
 * @param z the z component of the direction to translate to
 */
static void
translate(GLfloat *m, GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat t[16] = { 1, 0, 0, 0,  0, 1, 0, 0,  0, 0, 1, 0,  x, y, z, 1 };

    multiply(m, t);
}

/**
 * Creates an identity 4x4 matrix.
 *
 * @param m the matrix make an identity matrix
 */
static void
identity(GLfloat *m)
{
    GLfloat t[16] = {
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
    };

    memcpy(m, t, sizeof(t));
}

/**
 * Transposes a 4x4 matrix.
 *
 * @param m the matrix to transpose
 */
static void
transpose(GLfloat *m)
{
    GLfloat t[16] = {
        m[0], m[4], m[8],  m[12],
        m[1], m[5], m[9],  m[13],
        m[2], m[6], m[10], m[14],
        m[3], m[7], m[11], m[15]
    };

    memcpy(m, t, sizeof(t));
}

/**
 * Inverts a 4x4 matrix.
 *
 * This function can currently handle only pure translation-rotation matrices.
 * Read http://www.gamedev.net/community/forums/topic.asp?topic_id=425118
 * for an explanation.
 */
static void
invert(GLfloat *m)
{
    GLfloat t[16];
    identity(t);

    // Extract and invert the translation part 't'. The inverse of a
    // translation matrix can be calculated by negating the translation
    // coordinates.
    t[12] = -m[12]; t[13] = -m[13]; t[14] = -m[14];

    // Invert the rotation part 'r'. The inverse of a rotation matrix is
    // equal to its transpose.
    m[12] = m[13] = m[14] = 0;
    transpose(m);

    // inv(m) = inv(r) * inv(t)
    multiply(m, t);
}

/**
 * Calculate a perspective projection transformation.
 *
 * @param m the matrix to save the transformation in
 * @param fovy the field of view in the y direction
 * @param aspect the view aspect ratio
 * @param zNear the near clipping plane
 * @param zFar the far clipping plane
 */
void
perspective(GLfloat *m, GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar)
{
    GLfloat tmp[16];
    identity(tmp);

    double sine, cosine, cotangent, deltaZ;
    GLfloat radians = fovy / 2 * M_PI / 180;

    deltaZ = zFar - zNear;
    sincos(radians, &sine, &cosine);

    if((deltaZ == 0) || (sine == 0) || (aspect == 0)) {
        return;
    }

    cotangent = cosine / sine;

    tmp[0] = cotangent / aspect;
    tmp[5] = cotangent;
    tmp[10] = -(zFar + zNear) / deltaZ;
    tmp[11] = -1;
    tmp[14] = -2 * zNear * zFar / deltaZ;
    tmp[15] = 0;

    memcpy(m, tmp, sizeof(tmp));
}

static void
gears_idle(void)
{
    static double t0 = 0.0;
    static double totalTime = 0.0;
    static int frames = 0;
    double t1 = 0.0;
    double timePerFrame = 0.0;
    struct timeval tim;

    if(t0 == 0.0) {
        gettimeofday(&tim, NULL);
        t0 = tim.tv_sec + (tim.tv_usec / 1000000.0);
    }

    /// Get time
    gettimeofday(&tim, NULL);
    t1 = tim.tv_sec + (tim.tv_usec / 1000000.0);
    timePerFrame = t1 - t0;
    totalTime += timePerFrame;

    /// Spin gears
    angle += 10.0 * timePerFrame;
    if(angle > 3600.0) {
        angle -= 3600.0;
    }

    #ifndef VK_USE_PLATFORM_ANDROID_KHR
        eglutPostRedisplay();
    #endif

    ++frames;

    /// Count fps
    if(totalTime >= 5.0) {
        float fps = (float)frames / (float)totalTime;
        GLOVE_LOG("%d frames in %3.1f seconds = %6.3f FPS", frames, totalTime, fps);

        frames = 0;
        totalTime = 0.0;
    }

    t0 = t1;
}


/**
 * Draws a gear.
 *
 * @param gear the gear to draw
 * @param transform the current transformation matrix
 * @param x the x position to draw the gear at
 * @param y the y position to draw the gear at
 * @param angle the rotation angle of the gear
 * @param color the color of the gear
 */
static void
draw_gear(struct gear *gear, GLfloat *transform,
          GLfloat x, GLfloat y, GLfloat angle, const GLfloat color[4])
{
    GLfloat model_view[16];
    GLfloat normal_matrix[16];
    GLfloat model_view_projection[16];

    /* Translate and rotate the gear */
    memcpy(model_view, transform, sizeof(model_view));
    translate(model_view, x, y, 0);
    rotate(model_view, 2 * M_PI * angle / 360.0, 0, 0, 1);

    /* Create and set the ModelViewProjectionMatrix */
    memcpy(model_view_projection, ProjectionMatrix, sizeof(model_view_projection));
    multiply(model_view_projection, model_view);

    glUniformMatrix4fv(ModelViewProjectionMatrix_location, 1, GL_FALSE, model_view_projection);

    /*
     * Create and set the NormalMatrix. It's the inverse transpose of the
     * ModelView matrix.
     */
    memcpy(normal_matrix, model_view, sizeof(normal_matrix));
    invert(normal_matrix);
    transpose(normal_matrix);
    glUniformMatrix4fv(NormalMatrix_location, 1, GL_FALSE, normal_matrix);

    /* Set the gear color */
    glUniform4fv(MaterialColor_location, 1, color);

    /* Set the vertex buffer object to use */
    glBindBuffer(GL_ARRAY_BUFFER, gear->vbo);

    int vertPos = glGetAttribLocation(shaderProgram, "position");
    int normPos = glGetAttribLocation(shaderProgram, "normal");
    assert(vertPos != -1);
    assert(normPos != -1);
    assert(normPos != vertPos);

    /* Enable the attributes */
    glEnableVertexAttribArray(vertPos);
    glEnableVertexAttribArray(normPos);

    /* Set up the position of the attributes in the vertex buffer object */
    glVertexAttribPointer(vertPos, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), NULL);
    glVertexAttribPointer(normPos, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLfloat *) 0 + 3);

    /* Draw the triangle strips that comprise the gear */
    int n;
    for(n = 0; n < gear->nstrips; n++) {
        glDrawArrays(GL_TRIANGLE_STRIP, gear->strips[n].first, gear->strips[n].count);
    }

    /* Disable the attributes */
    glDisableVertexAttribArray(vertPos);
    glDisableVertexAttribArray(normPos);
}

/**
 * Draws the gears.
 */
static void
gears_draw(void)
{
    static const GLfloat red[4] = { 0.8, 0.1, 0.0, 1.0 };
    static const GLfloat green[4] = { 0.0, 0.8, 0.2, 1.0 };
    static const GLfloat blue[4] = { 0.2, 0.2, 1.0, 1.0 };
    GLfloat transform[16];

    glClearColor(0.0, 0.0, 0.0, 1.0);

    identity(transform);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* Translate and rotate the view */
    translate(transform, 0, 0, -20);
    rotate(transform, 2 * M_PI * view_rot[0] / 360.0, 1, 0, 0);
    rotate(transform, 2 * M_PI * view_rot[1] / 360.0, 0, 1, 0);
    rotate(transform, 2 * M_PI * view_rot[2] / 360.0, 0, 0, 1);

    /* Draw the gears */
    draw_gear(gear1, transform, -3.0, -2.0, angle, red);
    draw_gear(gear2, transform, 3.1, -2.0, -2 * angle - 9.0, green);
    draw_gear(gear3, transform, -3.1, 4.2, -2 * angle - 25.0, blue);
}

/**
 * Handles a new window size or exposure.
 *
 * @param width the window width
 * @param height the window height
 */
static void
gears_reshape(int width, int height)
{
    /* Update the projection matrix */
    perspective(ProjectionMatrix, 60.0, width / (float)height, 1.0, 1024.0);

    /* Set the viewport */
    glViewport(0, 0, (GLint) width, (GLint) height);
}

static void
gears_init(void)
{
    char *pSource;
    char **ppSource;
    int length;

    const GLubyte *oglVendor = glGetString(GL_VENDOR);
    const GLubyte *renderer = glGetString(GL_RENDERER);
    const GLubyte *oglVersion = glGetString(GL_VERSION);

    GLOVE_LOG("\nSetting up OpenGL:");
	GLOVE_LOG("OpenGL Version: %s", oglVersion);
	GLOVE_LOG("OpenGL Vendor: %s", oglVendor);
	GLOVE_LOG("Renderer: %s", renderer);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    /* Compile the vertex shader */
    pSource = readShaderFile(VERTEX_SHADER);
    ppSource = &pSource;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, (const char **)ppSource, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &length);
    assert(length != GL_FALSE);
    free(pSource);
//   glGetShaderInfoLog(vertexShader, sizeof msg, NULL, msg);

    /* Compile the fragment shader */
    pSource = readShaderFile(FRAGMENT_SHADER);
    ppSource = &pSource;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, (const char **)ppSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &length);
    assert(length != GL_FALSE);
    free(pSource);
//   glGetShaderInfoLog(fragmentShader, sizeof msg, NULL, msg);

    /* Create and link the shader shaderProgram */
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindAttribLocation(shaderProgram, 0, "position");
    glBindAttribLocation(shaderProgram, 1, "normal");

    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &length);
    assert(length != GL_FALSE);
//   glGetProgramInfoLog(shaderProgram, sizeof msg, NULL, msg);

    /* Enable the shaders */
    glUseProgram(shaderProgram);

    /* Get the locations of the uniforms so we can access them */
    ModelViewProjectionMatrix_location = glGetUniformLocation(shaderProgram, "ModelViewProjectionMatrix");
    NormalMatrix_location = glGetUniformLocation(shaderProgram, "NormalMatrix");
    LightSourcePosition_location = glGetUniformLocation(shaderProgram, "LightSourcePosition");
    MaterialColor_location = glGetUniformLocation(shaderProgram, "MaterialColor");

    GLOVE_LOG("ModelViewProjectionMatrix_location = %d", ModelViewProjectionMatrix_location);
    GLOVE_LOG("NormalMatrix_location = %d", NormalMatrix_location);
    GLOVE_LOG("LightSourcePosition_location = %d", LightSourcePosition_location);
    GLOVE_LOG("MaterialColor_location = %d", MaterialColor_location);

    /* Set the LightSourcePosition uniform which is constant throught the shaderProgram */
   glUniform4fv(LightSourcePosition_location, 1, LightSourcePosition);

    /* make the gears */
    gear1 = create_gear(1.0, 4.0, 1.0, 20, 0.7);
    gear2 = create_gear(0.5, 2.0, 2.0, 10, 0.7);
    gear3 = create_gear(1.3, 2.0, 0.5, 10, 0.7);
}

void
destroyGL(void)
{
    GLOVE_LOG("**** %s ****", __FUNCTION__);

    glDeleteBuffers(1, &gear1->vbo);
    free(gear1->strips);
    free(gear1->vertices);
    free(gear1);

    glDeleteBuffers(1, &gear2->vbo);
    free(gear2->strips);
    free(gear2->vertices);
    free(gear2);

    glDeleteBuffers(1, &gear3->vbo);
    free(gear3->strips);
    free(gear3->vertices);
    free(gear3);

    glDetachShader(shaderProgram, vertexShader);
    glDetachShader(shaderProgram, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glDeleteProgram(shaderProgram);
}

void android_main(struct android_app *app)
{
    setAndroidAppCallbacks(gears_draw, gears_init, gears_reshape, gears_idle);
    runAndroidApp(app);
}
