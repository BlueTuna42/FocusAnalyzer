#ifndef FFT_H 
#define FFT_H

#include "struct.h"
#include "bmp.h"

double energyRatio (fftw_complex *in, int width, int height);

void fftShiftComplex(fftw_complex *in, fftw_complex *out, int width, int height);

fftw_complex* computeForwardFFTChannel(BMPImage *img, int channel);

fftw_complex* computeInverseFFTChannel(fftw_complex *fftData, int width, int height);

BMPImage* createFFTDisplayImage(int width, int height, fftw_complex *fft_red, fftw_complex *fft_green, fftw_complex *fft_blue);

BMPImage* createImageFromIFFT(int width, int height, fftw_complex *inv_red, fftw_complex *inv_green, fftw_complex *inv_blue);

int saveFFTbmp( int width, int height, fftw_complex *fft_red, fftw_complex *fft_green, fftw_complex *fft_blue, const char *fftDisplayFilename);

complexRGB* RGBforwardFFT(BMPImage *img);

complexRGB* RGBFFTshift(complexRGB *data, int width, int height);

complexRGB* RGBinverseFFT (complexRGB *fft, int width, int height);

int RGBsaveFFTbmp(complexRGB *fft, int width, int height, const char* fftDisplayName);

BMPImage* RGBiFFTimage (complexRGB *ifft, int width, int height);

double RGBenergyRatio (complexRGB *fftShifted, int width, int height);

void RGBfree (complexRGB *data);

#endif