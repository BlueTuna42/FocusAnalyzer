#ifndef BMP_H 
#define BMP_H

#include "struct.h"

BMPImage* readBMP(const char *filename);

int saveBMP(const char *filename, BMPImage *img);

void freeBMPImage(BMPImage *img);

#endif