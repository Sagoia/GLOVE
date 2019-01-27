/**
 * Copyright (C) 2015-2018 Think Silicon S.A. (https://think-silicon.com/)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public v3
 * License as published by the Free Software Foundation;
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 */

#ifndef __TEXLOADER_H__
#define __TEXLOADER_H__

#include "../utilities/debug.h"
#include <stdint.h>

#ifdef _MSC_VER
#define PACKED_BEGIN(_EXP_) __pragma( pack(push, 1) ) _EXP_
#define PACKED_END __pragma( pack(pop) )
#else
#define PACKED_BEGIN(_EXP_) _EXP_ __attribute__((packed))
#define PACKED_END
#endif

typedef	struct
{
    unsigned char * imageData;              // Image Data (Up To 32 Bits)
    unsigned int    bpp;                    // Image Color Depth In Bits Per Pixel
    unsigned int    width;                  // Image Width
    unsigned int    height;                 // Image Height
    unsigned int    texID;                  // Texture ID Used To Select A Texture
    unsigned int    type;                   // Image Type (GL_RGB, GL_RGBA)
} Texture;

typedef struct
{
    unsigned char   Header[12];             // TGA File Header
} TGAHeader;

typedef struct
{
    unsigned char   header[6];              // First 6 Useful Bytes From The Header
    unsigned int    bytesPerPixel;          // Holds Number Of Bytes Per Pixel Used In The TGA File
    unsigned int    imageSize;              // Used To Store The Image Size When Setting Aside Ram
    unsigned int    temp;                   // Temporary Variable
    unsigned int    type;
    unsigned int    Height;                 //Height of Image
    unsigned int    Width;                  //Width ofImage
    unsigned int    Bpp;                    // Bits Per Pixel
} TGA;

static unsigned char uTGAcompare[12] = {0,0,2, 0,0,0,0,0,0,0,0,0};  // Uncompressed TGA Header
static unsigned char cTGAcompare[12] = {0,0,10,0,0,0,0,0,0,0,0,0};  // Compressed TGA Header

int         LoadUncompressedTGA    (Texture *, const char *, FILE *);          // Load an Uncompressed file
int         LoadCompressedTGA      (Texture *, const char *, FILE *);          // Load a Compressed file
int         LoadTGA                (Texture * texture, const char *filename);

typedef struct
{
    GLsizei        width;
    GLsizei        height;
    GLubyte *      pixels;
    GLsizei        size;
} ImageSlice;

typedef struct 
{
    GLint           components;
    GLenum          format;
    int             numMipMaps;
    ImageSlice*     slices;
} ImageData;

extern const uint32_t DDS_ALPHAPIXELS;
extern const uint32_t DDSF_FOURCC;

// compressed texture types
extern const uint32_t FOURCC_DXT1;
extern const uint32_t FOURCC_DXT3;
extern const uint32_t FOURCC_DXT5;

typedef struct {
    uint32_t dwSize;
    uint32_t dwFlags;
    uint32_t dwFourCC;
    uint32_t dwRGBBitCount;
    uint32_t dwRBitMask;
    uint32_t dwGBitMask;
    uint32_t dwBBitMask;
    uint32_t dwABitMask;
} DDS_PIXELFORMAT;

typedef struct {
    uint32_t dwSize;
    uint32_t dwFlags;
    uint32_t dwHeight;
    uint32_t dwWidth;
    uint32_t dwPitchOrLinearSize;
    uint32_t dwDepth;
    uint32_t dwMipMapCount;
    uint32_t dwReserved1[11];
    DDS_PIXELFORMAT ddspf;
    uint32_t dwCaps1;
    uint32_t dwCaps2;
    uint32_t dwReserved2[3];
} DDS_HEADER;

typedef struct {
    uint16_t col0;
    uint16_t col1;
    uint8_t row[4];
} DXTColorBlock;

typedef struct {
    uint16_t row[4];
} DXT3AlphaBlock;

typedef struct {
    uint8_t alpha0;
    uint8_t alpha1;
    uint8_t row[6];
} DXT5AlphaBlock;

int         LoadDXTC                (Texture * texture, ImageData *image, const char *filename);

extern const uint32_t PVRLegacyMagic;
extern const uint32_t PVRv3Magic;

extern const uint32_t PVRLegacyPixelFormatPVRTC2;
extern const uint32_t PVRLegacyPixelFormatPVRTC4;

extern const uint32_t PVRv3PixelFormatPVRTC_2BPP_RGB;
extern const uint32_t PVRv3PixelFormatPVRTC_2BPP_RGBA;
extern const uint32_t PVRv3PixelFormatPVRTC_4BPP_RGB;
extern const uint32_t PVRv3PixelFormatPVRTC_4BPP_RGBA;

PACKED_BEGIN(typedef struct)
{
    uint32_t headerLength;
    uint32_t height;
    uint32_t width;
    uint32_t mipmapCount;
    uint32_t flags;
    uint32_t dataLength;
    uint32_t bitsPerPixel;
    uint32_t redBitmask;
    uint32_t greenBitmask;
    uint32_t blueBitmask;
    uint32_t alphaBitmask;
    uint32_t pvrTag;
    uint32_t surfaceCount;
} PVRv2Header;
PACKED_END

PACKED_BEGIN(typedef struct)
{
    uint32_t version;
    uint32_t flags;
    uint64_t pixelFormat;
    uint32_t colorSpace;
    uint32_t channelType;
    uint32_t height;
    uint32_t width;
    uint32_t depth;
    uint32_t surfaceCount;
    uint32_t faceCount;
    uint32_t mipmapCount;
    uint32_t metadataLength;
} PVRv3Header;
PACKED_END

int         LoadPVRTC               (Texture * texture, ImageData *image, const char *filename);

Texture *   LoadGLTexture           (const char *filename);

#endif //__TEXLOADER_H__
