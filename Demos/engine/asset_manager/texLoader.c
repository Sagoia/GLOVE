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

#include "texLoader.h"
#include <stdbool.h>

#ifdef VK_USE_PLATFORM_IOS_MVK
extern FILE *ios_fopen(const char *filename, const char *mode);
#define fopen ios_fopen
#endif

static TGAHeader tgaheader;                        // TGA header
static TGA tga;                                    // TGA image data

int LoadTGA(Texture * texture, const char *filename)
{
    FILE * fTGA;												// File pointer to texture file
    fTGA = fopen(filename, "rb");				// Open file for reading

    if(fTGA == NULL)
        return 0;													// Exit function

    if(fread(&tgaheader, sizeof(TGAHeader), 1, fTGA) == 0)					// Attempt to read 12 byte header from file
    {
        if(fTGA != NULL)
            fclose(fTGA);
        return 0;
    }

    if(memcmp(uTGAcompare, &tgaheader, sizeof(tgaheader)) == 0)			// See if header matches the predefined header of an Uncompressed TGA image
    {
        LoadUncompressedTGA(texture, filename, fTGA);									// If so, jump to Uncompressed TGA loading code
    }
    else if(memcmp(cTGAcompare, &tgaheader, sizeof(tgaheader)) == 0)// See if header matches the predefined header of an RLE compressed TGA image
    {
        LoadCompressedTGA(texture, filename, fTGA);										// If so, jump to Compressed TGA loading code
    }
    else																														// If header matches neither type
    {
        fclose(fTGA);
        return 0;																											// Exit function
    }

    return 1;																												// All went well, continue on
}

int LoadUncompressedTGA(Texture * texture, const char *filename, FILE *fTGA)	// Load an uncompressed TGA
{
    if(fread(tga.header, sizeof(tga.header), 1, fTGA) == 0)					// Read TGA header
    {
        if(fTGA != NULL)
            fclose(fTGA);
        return 0;
    }

    texture->width  = tga.header[1] * 256 + tga.header[0];	// Determine The TGA Width	(highbyte*256+lowbyte)
    texture->height = tga.header[3] * 256 + tga.header[2];	// Determine The TGA Height	(highbyte*256+lowbyte)
    texture->bpp		= tga.header[4];												// Determine the bits per pixel
    tga.Width				= texture->width;												// Copy width into local structure
    tga.Height			= texture->height;											// Copy height into local structure
    tga.Bpp					= texture->bpp;													// Copy BPP into local structure

    if((texture->width <= 0) || (texture->height <= 0) || ((texture->bpp != 24) && (texture->bpp !=32)))	// Make sure all information is valid
    {
        if(fTGA != NULL)
            fclose(fTGA);
        return 0;
    }

    if(texture->bpp == 24)												// If the BPP of the image is 24...
        texture->type	= GL_RGB;											// Set Image type to GL_RGB
    else																					// Else if its 32 BPP
        texture->type	= GL_RGBA;										// Set image type to GL_RGBA

    tga.bytesPerPixel	= (tga.Bpp / 8);															// Compute the number of BYTES per pixel
    tga.imageSize		= (tga.bytesPerPixel * tga.Width * tga.Height);	// Compute the total amout ofmemory needed to store data
    texture->imageData	= (GLubyte *)malloc(tga.imageSize);					// Allocate that much memory

    if(texture->imageData == NULL)								// If no space was allocated
    {
        fclose(fTGA);
        return 0;
    }

    if(fread(texture->imageData, 1, tga.imageSize, fTGA) != tga.imageSize)	// Attempt to read image data
    {
        if(texture->imageData != NULL)							// If imagedata has data in it
        {
            free(texture->imageData);									// Delete data from memory
        }
        fclose(fTGA);																// Close file
        return 0;
    }

    // Byte Swapping Optimized By Steve Thomas
    for(GLuint cswap = 0; cswap < tga.imageSize; cswap += tga.bytesPerPixel)
    {
        unsigned char tmp = texture->imageData[cswap];
        texture->imageData[cswap] = texture->imageData[cswap + 2];
        texture->imageData[cswap + 1] = tmp;
    }

    fclose(fTGA);																// Close file
    return 1;																		// Return success
}

int LoadCompressedTGA(Texture * texture, const char *filename, FILE * fTGA)		// Load compressed TGAs
{
    if(fread(tga.header, sizeof(tga.header), 1, fTGA) == 0)					// Attempt to read header
    {
        if(fTGA != NULL)
            fclose(fTGA);
        return 0;
    }

    texture->width  = tga.header[1] * 256 + tga.header[0];					// Determine The TGA Width	(highbyte*256+lowbyte)
    texture->height = tga.header[3] * 256 + tga.header[2];					// Determine The TGA Height	(highbyte*256+lowbyte)
    texture->bpp	= tga.header[4];																	// Determine Bits Per Pixel
    tga.Width		= texture->width;																		// Copy width to local structure
    tga.Height		= texture->height;																// Copy width to local structure
    tga.Bpp			= texture->bpp;																			// Copy width to local structure

    if((texture->width <= 0) || (texture->height <= 0) || ((texture->bpp != 24) && (texture->bpp !=32)))	//Make sure all texture info is ok
    {
        if(fTGA != NULL)
            fclose(fTGA);
        return 0;
    }

    if(texture->bpp == 24)													// If the BPP of the image is 24...
        texture->type	= GL_RGB;												// Set Image type to GL_RGB
    else																						// Else if its 32 BPP
        texture->type	= GL_RGBA;											// Set image type to GL_RGBA

    tga.bytesPerPixel	= (tga.Bpp / 8);							// Compute BYTES per pixel
    tga.imageSize		= (tga.bytesPerPixel * tga.Width * tga.Height);	// Compute amout of memory needed to store image
    texture->imageData	= (GLubyte *)malloc(tga.imageSize);					// Allocate that much memory

    if(texture->imageData == NULL)									// If it wasnt allocated correctly..
    {
        fclose(fTGA);
        return 0;
    }

    GLuint pixelcount	= tga.Height * tga.Width;			// Nuber of pixels in the image
    GLuint currentpixel	= 0;												// Current pixel being read
    GLuint currentbyte	= 0;												// Current byte
    GLubyte * colorbuffer = (GLubyte *)malloc(tga.bytesPerPixel);			// Storage for 1 pixel

    do
    {
        GLubyte chunkheader = 0;											// Storage for "chunk" header

        if(fread(&chunkheader, sizeof(GLubyte), 1, fTGA) == 0)				// Read in the 1 byte header
        {
            if(fTGA != NULL)
                fclose(fTGA);
            if(texture->imageData != NULL)							// If there is stored image data
                free(texture->imageData);									// Delete image data
            return 0;
        }

        if(chunkheader < 128)												// If the ehader is < 128, it means the that is the number of RAW color packets minus 1 that follow the header
        {
            chunkheader++;														// add 1 to get number of following color values
            for(short counter = 0; counter < chunkheader; counter++)		// Read RAW color values
            {
                if(fread(colorbuffer, 1, tga.bytesPerPixel, fTGA) != tga.bytesPerPixel) // Try to read 1 pixel
                {
                    if(fTGA != NULL)
                        fclose(fTGA);

                    if(colorbuffer != NULL)								// See if colorbuffer has data in it
                        free(colorbuffer);									// If so, delete it

                    if(texture->imageData != NULL)				// See if there is stored Image data
                        free(texture->imageData);						// If so, delete it too

                    return 0;
                }
                                                                                                                                                    // write to memory
                texture->imageData[currentbyte			] = colorbuffer[2];                 // Flip R and B vcolor values around in the process
                texture->imageData[currentbyte + 1	] = colorbuffer[1];
                texture->imageData[currentbyte + 2	] = colorbuffer[0];

                if(tga.bytesPerPixel == 4)							// if its a 32 bpp image
                {
                    texture->imageData[currentbyte + 3] = colorbuffer[3];				// copy the 4th byte
                }

                currentbyte += tga.bytesPerPixel;				// Increase thecurrent byte by the number of bytes per pixel
                currentpixel++;													// Increase current pixel by 1

                if(currentpixel > pixelcount)						// Make sure we havent read too many pixels
                {
                    if(fTGA != NULL)
                        fclose(fTGA);

                    if(colorbuffer != NULL)								// If there is data in colorbuffer
                        free(colorbuffer);									// Delete it

                    if(texture->imageData != NULL)				// If there is Image data
                        free(texture->imageData);						// delete it

                    return 0;
                }
            }
        }
        else																				// chunkheader > 128 RLE data, next color reapeated chunkheader - 127 times
        {
            chunkheader -= 127;												// Subteact 127 to get rid of the ID bit
            if(fread(colorbuffer, 1, tga.bytesPerPixel, fTGA) != tga.bytesPerPixel)		// Attempt to read following color values
            {
                if(fTGA != NULL)
                    fclose(fTGA);
                if(colorbuffer != NULL)									// If there is data in the colorbuffer
                    free(colorbuffer);										// delete it

                if(texture->imageData != NULL)					// If thereis image data
                    free(texture->imageData);							// delete it

                return 0;
            }

            for(short counter = 0; counter < chunkheader; counter++)				// copy the color into the image data as many times as dictated by the header
            {
                texture->imageData[currentbyte		] = colorbuffer[2];					// switch R and B bytes areound while copying
                texture->imageData[currentbyte + 1	] = colorbuffer[1];
                texture->imageData[currentbyte + 2	] = colorbuffer[0];

                if(tga.bytesPerPixel == 4)																		// If TGA images is 32 bpp
                {
                    texture->imageData[currentbyte + 3] = colorbuffer[3];				// Copy 4th byte
                }

                currentbyte += tga.bytesPerPixel;															// Increase current byte by the number of bytes per pixel
                currentpixel++;																								// Increase pixel count by 1

                if(currentpixel > pixelcount)																	// Make sure we havent written too many pixels
                {
                    if(fTGA != NULL)
                        fclose(fTGA);

                    if(colorbuffer != NULL)											// If there is data in colorbuffer
                        free(colorbuffer);												// Delete it

                    if(texture->imageData != NULL)							// If there is Image data
                        free(texture->imageData);									// delete it

                    return 0;														// Return failed
                }
            }
        }
    }

    while(currentpixel < pixelcount);										// Loop while there are still pixels left
    fclose(fTGA);																				// Close the file
    free(colorbuffer);
    return 1;																						// return success
}

const uint32_t DDS_ALPHAPIXELS = 0x00000001;
const uint32_t DDSF_FOURCC = 0x00000004;

// compressed texture types
const uint32_t FOURCC_DXT1 = 0x31545844; //(MAKEFOURCC('D','X','T','1'))
const uint32_t FOURCC_DXT3 = 0x33545844; //(MAKEFOURCC('D','X','T','3'))
const uint32_t FOURCC_DXT5 = 0x35545844; //(MAKEFOURCC('D','X','T','5'))

static void swapUint8(uint8_t *a, uint8_t *b) 
{
    uint8_t temp = *a;
    *a = *b;
    *b = temp;
}

static void swapUint16(uint16_t *a, uint16_t *b)
{
    uint16_t temp = *a;
    *a = *b;
    *b = temp;
}

static void FlipBlocksDXTC1(DXTColorBlock *line, uint32_t numBlocks) {
    DXTColorBlock *curblock = line;

    for (uint32_t i = 0; i < numBlocks; i++) {
        swapUint8(curblock->row + 0, curblock->row + 3);
        swapUint8(curblock->row + 1, curblock->row + 2);

        curblock++;
    }
}

static void FlipBlocksDXTC3(DXTColorBlock *line, uint32_t numBlocks) {
    DXTColorBlock *curblock = line;
    DXT3AlphaBlock *alphablock;

    for (uint32_t i = 0; i < numBlocks; i++) {
        alphablock = (DXT3AlphaBlock*)curblock;

        swapUint16(alphablock->row + 0, alphablock->row + 3);
        swapUint16(alphablock->row + 1, alphablock->row + 2);

        curblock++;

        swapUint8(curblock->row + 0, curblock->row + 3);
        swapUint8(curblock->row + 1, curblock->row + 2);

        curblock++;
    }
}

static void FlipDXTC5Alpha(DXT5AlphaBlock *block) {
    uint8_t gBits[4][4];

    const uint32_t mask = 0x00000007;          // bits = 00 00 01 11
    uint32_t bits = 0;
    memcpy(&bits, &block->row[0], sizeof(uint8_t) * 3);

    gBits[0][0] = (uint8_t)(bits & mask);
    bits >>= 3;
    gBits[0][1] = (uint8_t)(bits & mask);
    bits >>= 3;
    gBits[0][2] = (uint8_t)(bits & mask);
    bits >>= 3;
    gBits[0][3] = (uint8_t)(bits & mask);
    bits >>= 3;
    gBits[1][0] = (uint8_t)(bits & mask);
    bits >>= 3;
    gBits[1][1] = (uint8_t)(bits & mask);
    bits >>= 3;
    gBits[1][2] = (uint8_t)(bits & mask);
    bits >>= 3;
    gBits[1][3] = (uint8_t)(bits & mask);

    bits = 0;
    memcpy(&bits, &block->row[3], sizeof(uint8_t) * 3);

    gBits[2][0] = (uint8_t)(bits & mask);
    bits >>= 3;
    gBits[2][1] = (uint8_t)(bits & mask);
    bits >>= 3;
    gBits[2][2] = (uint8_t)(bits & mask);
    bits >>= 3;
    gBits[2][3] = (uint8_t)(bits & mask);
    bits >>= 3;
    gBits[3][0] = (uint8_t)(bits & mask);
    bits >>= 3;
    gBits[3][1] = (uint8_t)(bits & mask);
    bits >>= 3;
    gBits[3][2] = (uint8_t)(bits & mask);
    bits >>= 3;
    gBits[3][3] = (uint8_t)(bits & mask);

    uint32_t *pBits = ((uint32_t*) &(block->row[0]));

    *pBits = *pBits | (gBits[3][0] << 0);
    *pBits = *pBits | (gBits[3][1] << 3);
    *pBits = *pBits | (gBits[3][2] << 6);
    *pBits = *pBits | (gBits[3][3] << 9);

    *pBits = *pBits | (gBits[2][0] << 12);
    *pBits = *pBits | (gBits[2][1] << 15);
    *pBits = *pBits | (gBits[2][2] << 18);
    *pBits = *pBits | (gBits[2][3] << 21);

    pBits = ((uint32_t*) &(block->row[3]));

#ifdef MACOS
    *pBits &= 0x000000ff;
#else
    *pBits &= 0xff000000;
#endif

    *pBits = *pBits | (gBits[1][0] << 0);
    *pBits = *pBits | (gBits[1][1] << 3);
    *pBits = *pBits | (gBits[1][2] << 6);
    *pBits = *pBits | (gBits[1][3] << 9);

    *pBits = *pBits | (gBits[0][0] << 12);
    *pBits = *pBits | (gBits[0][1] << 15);
    *pBits = *pBits | (gBits[0][2] << 18);
    *pBits = *pBits | (gBits[0][3] << 21);
}

static void FlipBlocksDXTC5(DXTColorBlock *line, uint32_t numBlocks) {
    DXTColorBlock *curblock = line;
    DXT5AlphaBlock *alphablock;

    for (uint32_t i = 0; i < numBlocks; i++) {
        alphablock = (DXT5AlphaBlock*)curblock;

        FlipDXTC5Alpha(alphablock);

        curblock++;

        swapUint8(curblock->row + 0, curblock->row + 3);
        swapUint8(curblock->row + 1, curblock->row + 2);

        curblock++;
    }
}

int LoadDXTC(Texture * texture, ImageData *image, const char *filename)
{
    DDS_HEADER ddsh;
    char filecode[4];

    // Open the file
    FILE *pFile = fopen(filename, "rb");
    if (pFile == NULL)  {
        printf("File %s open failed!\n", filename);
        return 0;
    }

    // Verify the file is a true .dds file
    fread(filecode, 1, 4, pFile);
    if (strncmp(filecode, "DDS ", 4) != 0) {
        printf("File %s doesn't appear to be a valid .dds file!\n", filename);
        fclose(pFile);
        return 0;
    }

    // Get the surface descriptor
    fread(&ddsh, sizeof(ddsh), 1, pFile);
    memset(image, 0, sizeof(ImageData));

    //
    // This .dds loader supports the loading of compressed formats DXT1, DXT3 
    // and DXT5.
    //

    if (!(ddsh.ddspf.dwFlags & DDSF_FOURCC)) {
        printf("File %s doesn't appear to be a compressed .dds file!\n", filename);
        fclose(pFile);
        return 0;
    }

    if (ddsh.ddspf.dwFourCC == FOURCC_DXT1) {
        // DXT1's compression ratio is 8:1
        if (ddsh.ddspf.dwFlags & DDS_ALPHAPIXELS) {
            image->format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        } else {
            image->format = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
        }
    } else if (ddsh.ddspf.dwFourCC == FOURCC_DXT3) {
        // DXT3's compression ratio is 4:1
        image->format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    } else if (ddsh.ddspf.dwFourCC == FOURCC_DXT5) {
        // DXT5's compression ratio is 4:1
        image->format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    } else {
        printf("File %s load failed: unsupported dds format! \n", filename);
        fclose(pFile);
        return 0;
    }

    if (ddsh.dwPitchOrLinearSize == 0) {
        printf("dwLinearSize is 0! \n");
    }

    uint32_t width = ddsh.dwWidth;
    uint32_t height = ddsh.dwHeight;
    image->slices = (ImageSlice *)malloc(ddsh.dwMipMapCount * sizeof(ImageSlice));
    uint32_t blockSize = (image->format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT || image->format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ? 8 : 16);
    for (uint32_t i = 0; i < ddsh.dwMipMapCount; ++i) {
        uint32_t xblocks = (width + 3) / 4;
        uint32_t yblocks = (height + 3) / 4;

        uint32_t size = xblocks * yblocks * blockSize;
        GLubyte *pixels = (GLubyte *)malloc(size * sizeof(GLubyte));
        fread(pixels, 1, size, pFile);

        // flip date
        uint32_t lineSize = xblocks * blockSize;
        uint8_t *tmp = (uint8_t *)malloc(lineSize * sizeof(uint8_t));
        for (unsigned int j = 0; j < (yblocks >> 1); j++) {
            DXTColorBlock *top = (DXTColorBlock*)(pixels + j * lineSize);
            DXTColorBlock *bottom = (DXTColorBlock*)(pixels + (((yblocks - j) - 1) * lineSize));

            switch (image->format)
            {
            case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
            case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
                FlipBlocksDXTC1(top, xblocks);
                FlipBlocksDXTC1(bottom, xblocks);
                break;
            case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
                FlipBlocksDXTC3(top, xblocks);
                FlipBlocksDXTC3(bottom, xblocks);
                break;
            case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
                FlipBlocksDXTC5(top, xblocks);
                FlipBlocksDXTC5(bottom, xblocks);
                break;
            }

            // swap
            memcpy(tmp, bottom, lineSize);
            memcpy(bottom, top, lineSize);
            memcpy(top, tmp, lineSize);
        }
        free(tmp);

        image->slices[i].width = width;
        image->slices[i].height = height;
        image->slices[i].pixels = pixels;
        image->slices[i].size = size;

        width = width >> 1; if (width == 0) { width = 1; }
        height = height >> 1; if (height == 0) { height = 1; }
    }

    // Close the file
    fclose(pFile);

    image->numMipMaps = ddsh.dwMipMapCount;

    if (ddsh.ddspf.dwFourCC == FOURCC_DXT1) {
        image->components = 3;
    } else {
        image->components = 4;
    }

    texture->imageData = NULL;
    texture->bpp = image->components;
    texture->width = ddsh.dwWidth;
    texture->height = ddsh.dwHeight;
    texture->type = image->format;

    return 1;
}

const uint32_t PVRLegacyMagic   = 0x21525650;
const uint32_t PVRv3Magic       = 0x03525650;

const uint32_t PVRLegacyPixelFormatPVRTC2 = 0x18;
const uint32_t PVRLegacyPixelFormatPVRTC4 = 0x19;

const uint32_t PVRv3PixelFormatPVRTC_2BPP_RGB  = 0x0;
const uint32_t PVRv3PixelFormatPVRTC_2BPP_RGBA = 0x1;
const uint32_t PVRv3PixelFormatPVRTC_4BPP_RGB  = 0x2;
const uint32_t PVRv3PixelFormatPVRTC_4BPP_RGBA = 0x3;

static uint32_t SwapInt32LittleToHost(uint32_t arg)
{
    return arg;
}

static uint64_t SwapInt64LittleToHost(uint64_t arg)
{
    return arg;
}

static int IsPVRv2Container(void *data, size_t size)
{
    if (size < sizeof(PVRv2Header)) {
        return 0;
    }
    
    PVRv2Header *header = (PVRv2Header *)data;
    uint32_t fileMagic = SwapInt32LittleToHost(header->pvrTag);
    return (fileMagic == PVRLegacyMagic);
}

static int IsPVRv3Container(void *data, size_t size)
{
    if (size < sizeof(PVRv3Header)) {
        return 0;
    }
    
    PVRv3Header *header = (PVRv3Header *)data;
    uint32_t fileMagic = SwapInt32LittleToHost(header->version);
    return (fileMagic == PVRv3Magic);
}

static int LoadPVRv2Image(ImageData *data, void *fileData, size_t fileSize)
{
    PVRv2Header *header = (PVRv2Header *)fileData;
    
    uint32_t flags = SwapInt32LittleToHost(header->flags);
    uint32_t format = flags & 0xFF;
    uint32_t mipCount = SwapInt32LittleToHost(header->mipmapCount);
    uint32_t width = 0;
    uint32_t height = 0;
    bool     hasAlpha = ((flags & 0x8000) > 0);
    uint32_t componentCount = hasAlpha ? 4 : 3;
    bool     colorSpaceIsLinear = true;
    
    if (format != PVRLegacyPixelFormatPVRTC2 && format != PVRLegacyPixelFormatPVRTC4) {
        printf("LoadPVRv2Image: unsupported pvr format 0x%x! \n", format);
        return 0;
    }

    if (mipCount == 0) {
        mipCount = 1;
    }
    ImageSlice *slices = malloc(mipCount * sizeof(ImageSlice));

    width = SwapInt32LittleToHost(header->width);
    height = SwapInt32LittleToHost(header->height);
    
    uint32_t dataLength = SwapInt32LittleToHost(header->dataLength);
    
    uint8_t *bytes = ((uint8_t *)fileData) + sizeof(PVRv2Header);
    
    // PVRLegacyPixelFormatPVRTC4
    uint32_t blockWidth = 4;
    uint32_t blockHeight = 4;
    uint32_t bitsPerPixel = 4;
    if (format == PVRLegacyPixelFormatPVRTC2) {
        blockWidth = 8;
        blockHeight = 4;
        bitsPerPixel = 2;
    }
    uint32_t blockSize = blockWidth * blockHeight;

    uint32_t dataOffset = 0;
    uint32_t levelWidth = width;
    uint32_t levelHeight = height;
    uint32_t index = 0;
    while (dataOffset < dataLength) {
        uint32_t widthInBlocks = levelWidth / blockWidth;
        uint32_t heightInBlocks = levelHeight / blockHeight;
        
        if (widthInBlocks < 2) {
            widthInBlocks = 2;
        }
        if (heightInBlocks < 2) {
            heightInBlocks = 2;
        }
        
        uint32_t mipSize = widthInBlocks * heightInBlocks * ((blockSize * bitsPerPixel) / 8);
        uint8_t *levelData = (uint8_t *)malloc(mipSize * sizeof(uint8_t));
        memcpy(levelData, bytes + dataOffset, mipSize);
        
        slices[index].width = levelWidth;
        slices[index].height = levelHeight;
        slices[index].pixels = levelData;
        slices[index].size = mipSize;
        
        levelWidth = levelWidth / 2;
        levelHeight = levelHeight / 2;
        
        if (levelWidth < 1) {
            levelWidth = 1;
        }
        if (levelHeight < 1) {
            levelHeight = 1;
        }
        
        dataOffset += mipSize;
        ++ index;
    }
    
    data->components = componentCount;
    data->numMipMaps = mipCount;
    data->slices = slices;
    if (bitsPerPixel == 2) {
        if (componentCount == 3) {
            data->format = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
        } else if (componentCount == 4) {
            data->format = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
        }
    } else if (bitsPerPixel == 4) {
        if (componentCount == 3) {
            data->format = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
        } else if (componentCount == 4) {
            data->format = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
        }
    }
    
    return 1;
}

static int LoadPVRv3Image(ImageData *data, void *fileData, size_t fileSize)
{
    PVRv3Header *header = (PVRv3Header *)fileData;
    
    uint32_t width = SwapInt32LittleToHost(header->width);
    uint32_t height = SwapInt32LittleToHost(header->height);
    uint32_t format = SwapInt64LittleToHost(header->pixelFormat) & 0xffffffff;
    uint32_t mipCount = SwapInt32LittleToHost(header->mipmapCount);
    uint32_t metadataLength = SwapInt32LittleToHost(header->metadataLength);
    bool     colorSpaceIsLinear = (SwapInt32LittleToHost(header->colorSpace) == 0);
    
    if (format != PVRv3PixelFormatPVRTC_2BPP_RGB &&
        format != PVRv3PixelFormatPVRTC_2BPP_RGBA &&
        format != PVRv3PixelFormatPVRTC_4BPP_RGB &&
        format != PVRv3PixelFormatPVRTC_4BPP_RGBA) {
        printf("LoadPVRv3Image: unsupported pvr format 0x%x! \n", format);
        return 0;
    }
    
    if (mipCount == 0) {
        mipCount = 1;
    }
    ImageSlice *slices = malloc(mipCount * sizeof(ImageSlice));

    uint32_t dataLength = (uint32_t)(fileSize - (sizeof(PVRv3Header) + metadataLength));
    
    uint8_t *bytes = ((uint8_t *)fileData) + sizeof(PVRv3Header) + metadataLength;
    
    // PVRv3PixelFormatPVRTC_4BPP_RGB or PVRv3PixelFormatPVRTC_4BPP_RGBA
    uint32_t blockWidth = 4;
    uint32_t blockHeight = 4;
    uint32_t bitsPerPixel = 4;
    if (format == PVRv3PixelFormatPVRTC_2BPP_RGB || format == PVRv3PixelFormatPVRTC_2BPP_RGBA) {
        blockWidth = 8;
        blockHeight = 4;
        bitsPerPixel = 2;
    }

    uint32_t blockSize = blockWidth * blockHeight;
    
    uint32_t dataOffset = 0;
    uint32_t levelWidth = width;
    uint32_t levelHeight = height;
    uint32_t index = 0;
    while (dataOffset < dataLength) {
        uint32_t widthInBlocks = levelWidth / blockWidth;
        uint32_t heightInBlocks = levelHeight / blockHeight;
        
        uint32_t mipSize = widthInBlocks * heightInBlocks * ((blockSize * bitsPerPixel) / 8);
        
        if (mipSize < 32) {
            mipSize = 32;
        }
        
        uint8_t *levelData = (uint8_t *)malloc(mipSize * sizeof(uint8_t));
        memcpy(levelData, bytes + dataOffset, mipSize);
        
        slices[index].width = levelWidth;
        slices[index].height = levelHeight;
        slices[index].pixels = levelData;
        slices[index].size = mipSize;
        
        levelWidth = levelWidth / 2;
        levelHeight = levelHeight / 2;
        
        if (levelWidth < 1) {
            levelWidth = 1;
        }
        if (levelHeight < 1) {
            levelHeight = 1;
        }
        
        dataOffset += mipSize;
        ++ index;
    }
    
    uint32_t componentCount = ((format == PVRv3PixelFormatPVRTC_2BPP_RGB) ||
                               (format == PVRv3PixelFormatPVRTC_4BPP_RGB)) ? 3 : 4;

    data->components = componentCount;
    data->numMipMaps = mipCount;
    data->slices = slices;
    if (bitsPerPixel == 2) {
        if (componentCount == 3) {
            data->format = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
        } else if (componentCount == 4) {
            data->format = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
        }
    } else if (bitsPerPixel == 4) {
        if (componentCount == 3) {
            data->format = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
        } else if (componentCount == 4) {
            data->format = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
        }
    }
    
    return 1;
}

int LoadPVRTC (Texture * texture, ImageData *image, const char *filename)
{
    void *fileData = NULL;
    FILE *pFile = fopen(filename, "rb");
    if (pFile == NULL)  {
        printf("File %s open failed!\n", filename);
        return 0;
    }
    
    fseek(pFile, 0, SEEK_END);
    size_t fileSize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);
    
    fileData = malloc(fileSize * sizeof(uint8_t));
    fread(fileData, sizeof(uint8_t), fileSize, pFile);
    
    fclose(pFile);
    pFile = NULL;
    
    int32_t loadRet = 0;
    if (IsPVRv2Container(fileData, fileSize)) {
        loadRet = LoadPVRv2Image(image, fileData, fileSize);
    } else if (IsPVRv3Container(fileData, fileSize)) {
        loadRet = LoadPVRv3Image(image, fileData, fileSize);
    } else {
        printf("File %s load failed: unsupported pvr format! \n", filename);
    }
    
    if (loadRet) {
        texture->imageData = NULL;
        texture->bpp = image->components;
        texture->width = image->slices[0].width;
        texture->height = image->slices[0].height;
        texture->type = image->format;
    }
    
    free(fileData);
    return loadRet;
    
}

Texture *LoadGLTexture(const char *filename)
{
    Texture *texture = (Texture *)malloc(sizeof(Texture));

    const char *prefix = filename + strlen(filename) - 3;
    if (!strcmp(prefix, "tga") || !strcmp(prefix, "TGA")) {
        if(LoadTGA(texture, filename)) {
            glGenTextures   (1, &texture->texID);
            glBindTexture   (GL_TEXTURE_2D, texture->texID);
            glTexImage2D    (GL_TEXTURE_2D, 0, texture->type, texture->width, texture->height, 0, texture->type, GL_UNSIGNED_BYTE, texture->imageData);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            ASSERT_NO_GL_ERROR();
            free(texture->imageData);
            texture->imageData = NULL;
            return texture;
        }
    } else if (!strcmp(prefix, "dds") || !strcmp(prefix, "DDS")) {
        ImageData ddsImage;
        if (LoadDXTC(texture, &ddsImage, filename)) {
            glGenTextures(1, &texture->texID);
            glBindTexture(GL_TEXTURE_2D, texture->texID);
            for (int i = 0; i < ddsImage.numMipMaps; ++i) {
                glCompressedTexImage2D(GL_TEXTURE_2D, i, ddsImage.format, ddsImage.slices[i].width, ddsImage.slices[i].height, 0, ddsImage.slices[i].size, ddsImage.slices[i].pixels);
            }
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            ASSERT_NO_GL_ERROR();

            for (int i = 0; i < ddsImage.numMipMaps; ++i) {
                free(ddsImage.slices[i].pixels);
            }
            free(ddsImage.slices);

            return texture;
        }
    } else if (!strcmp(prefix, "pvr") || !strcmp(prefix, "PVR")) {
        ImageData pvrImage;
        if (LoadPVRTC(texture, &pvrImage, filename)) {
            glGenTextures(1, &texture->texID);
            glBindTexture(GL_TEXTURE_2D, texture->texID);
            for (int i = 0; i < pvrImage.numMipMaps; ++i) {
                glCompressedTexImage2D(GL_TEXTURE_2D, i, pvrImage.format, pvrImage.slices[i].width, pvrImage.slices[i].height, 0, pvrImage.slices[i].size, pvrImage.slices[i].pixels);
                break;
            }
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            ASSERT_NO_GL_ERROR();
            
            for (int i = 0; i < pvrImage.numMipMaps; ++i) {
                free(pvrImage.slices[i].pixels);
            }
            free(pvrImage.slices);
            
            return texture;
        }
    }

#ifdef DEBUG_ASSET_MANAGEMENT
    fprintf(stderr, "[LoadGLTexture()]: Error loading texture %s\n", filename);
#endif
    assert(0);
    free(texture);
    return NULL;
}
