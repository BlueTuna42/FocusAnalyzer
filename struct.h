#ifndef STRUCT_H 
#define STRUCT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <fftw3.h>

#pragma pack(push, 1)

typedef struct {
    uint16_t bfType;      // BM identifier, should be 0x4D42
    uint32_t bfSize;      // Size of the file in bytes
    uint16_t bfReserved1; // Reserved, must be 0
    uint16_t bfReserved2; // Reserved, must be 0
    uint32_t bfOffBits;   // Offset to start of pixel data
} BITMAPFILEHEADER;

typedef struct {
    uint32_t biSize;          // Size of this header (40 bytes)
    int32_t  biWidth;         // Width of the image in pixels
    int32_t  biHeight;        // Height of the image in pixels (can be negative)
    uint16_t biPlanes;        // Number of color planes, must be 1
    uint16_t biBitCount;      // Bits per pixel (e.g., 24 for 24-bit)
    uint32_t biCompression;   // Compression type (0 for uncompressed)
    uint32_t biSizeImage;     // Image size in bytes (may be 0 if uncompressed)
    int32_t  biXPelsPerMeter; // Horizontal resolution (pixels per meter)
    int32_t  biYPelsPerMeter; // Vertical resolution (pixels per meter)
    uint32_t biClrUsed;       // Number of colors in the color palette
    uint32_t biClrImportant;  // Number of important colors used
} BITMAPINFOHEADER;
#pragma pack(pop)

typedef struct {
    unsigned char blue;
    unsigned char green;
    unsigned char red;
} Pixel;

typedef struct {
    int width;
    int height;
    Pixel **pixels;
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;
} BMPImage;

typedef struct {
    fftw_complex *red;;
    fftw_complex *green;
    fftw_complex *blue;
} complexRGB;

#endif