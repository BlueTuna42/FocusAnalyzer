#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <fftw3.h>
#include <string.h>

#include "struct.h"
#include "FFT.h"
#include "bmp.h"
#include "scan.h"

#define focusConst 1.2
#define PATH_MAX 256

int processFFT(const char *inputFilename, const char *fftDisplayFilename, const char *ifftFilename) {
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

int checkFocus (const char *inputFilename) {
    BMPImage *image = readBMP(inputFilename);
    int width = image->width, height = image->height;
    int total = width * height;

    complexRGB *fft = RGBforwardFFT(image);
    complexRGB *fftShifted = RGBFFTshift(fft, width, height);
    double ER = RGBenergyRatio(fftShifted, width, height);

    freeBMPImage(image);
    RGBfree(fft);
    RGBfree(fftShifted);

    if (ER < 1.2)
        return 0;
    else 
        return 1;
}

char* concat(const char *s1, const char *s2) {
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}


int main() {
    char dirpath[PATH_MAX];
    printf("Enter directory path: ");
    if (!fgets(dirpath, sizeof(dirpath), stdin)) {
        fprintf(stderr, "Error reading input.\n");
        return 1;
    }

    dirpath[strcspn(dirpath, "\n")] = '\0';
    size_t nfiles = 0;
    char **bmp_files = scan_bmp_files(dirpath, &nfiles);
    printf("Found %zu .bmp file(s):\n", nfiles);

    for (size_t i = 0; i < nfiles; i++) {
        if (checkFocus(concat(dirpath, bmp_files[i])))
            printf(concat(bmp_files[i], " is sharp\n"));
        else
            printf(concat(bmp_files[i], " is blurry\n"));

        free(bmp_files[i]);
        }
        free(bmp_files);

    return 0;
}

