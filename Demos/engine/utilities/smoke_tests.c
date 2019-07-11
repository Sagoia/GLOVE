#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "GLES2/gl2.h"
#include "smoke_tests.h"

static void saveFramebufferToFile(char *fileName, int width, int height);

static void saveFramebufferToFile(char *fileName, int width, int height)
{
    char fullFilename[256];
    if(!strcmp(fileName - 5, ".rgba")) {
        snprintf(fullFilename, 256, "%s", fileName);
    } else {
        snprintf(fullFilename, 256, "%s.rgba", fileName);
    }

    size_t size = width * height * sizeof(uint32_t);
    uint32_t *pixels = (uint32_t *)malloc(size);

    memset(pixels, 0xaa, size);

    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    FILE *fp = fopen(fullFilename, "w");
    if(fp) {
        size_t writtenBytes = fwrite(pixels, 1, size, fp);
        if(writtenBytes != size) {
            printf("Could not write file %s.\n", fullFilename);
        }
        fclose(fp);
    } else {
        printf("Could not open file %s.\n", fullFilename);
    }

    free(pixels);
}


/******************* PUBLIC FUNCTIONS ***************************************/

bool SmokeTestsRunning()
{
    const char *gloveDemosMode = getenv("GLOVE_DEMOS_MODE");

    if(!gloveDemosMode) {
        return false;
    }

    return (0 == strncmp(gloveDemosMode, "GLOVE_CI", MAX_GLOVE_DEMOS_MODE_STR)) ? true : false;;
}

void TakeScreenshot(char *fileName, draw_cb_t draw_cb, int width, int height)
{
    draw_cb();
    saveFramebufferToFile(fileName, width, height);
}
