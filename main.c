#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <fftw3.h>
#include "struct.h"
#include "FFT.h"
#include "bmp.h"

int main() {
    const char *inputFilename = "./Input images/lena.bmp";
    const char *fftDisplayFilename = "./Output images/fft_result.bmp";
    const char *ifftFilename = "./Output images/ifft_result.bmp";

    // Read BMP image.
    BMPImage *image = readBMP(inputFilename);
    if (!image) {
        printf("Failed to read BMP image.\n");
        return 1;
    }
    printf("Image read: width=%d, height=%d\n", image->width, image->height);
    
    int width = image->width, height = image->height;
    int total = width * height;
    
    complexRGB *fft = RGBforwardFFT(image);
    complexRGB *fftShifted = RGBFFTshift(fft, width, height);
    double ER = RGBenergyRatio(fftShifted, width, height);
    
    RGBsaveFFTbmp(fftShifted, width, height, fftDisplayFilename);
    printf("Energy ratio %f\n", ER);
    
    fft = RGBFFTshift(fftShifted, width, height);
    complexRGB *ifft = RGBinverseFFT(fft, width, height);

    BMPImage *ifftImage = RGBiFFTimage(ifft, width, height);
    if (saveBMP(ifftFilename, ifftImage) != 0) {
        printf("Error saving inverse FFT image.\n");
    } else {
        printf("Inverse FFT image saved to %s\n", ifftFilename);
    }

    freeBMPImage(ifftImage);    
    freeBMPImage(image);
    RGBfree(fft);
    RGBfree(fftShifted);
    RGBfree(ifft);

    return 0;
}

