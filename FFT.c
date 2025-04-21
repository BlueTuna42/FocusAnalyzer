#include "struct.h"
#include "bmp.h"

#define threshold 5

double energyRatio (fftw_complex *in, int width, int height) {
	int total = width * height;
	double mag_low = 0;
	double mag_high = 0;
        int nHigh = 0;
        int nLow = 0;
        for (int i = 0; i < height; i++) {
    		for (int j = 0; j < width; j++) {
    			int x = j - width/2;  
    			int y = i - height/2;
    			int k = i * width + j;
    			if ((abs(x) < width/(2*threshold) && abs(y) < height/(2*threshold)) || (abs(x) < width/(pow(threshold,2)) || abs(y) < height/(pow(threshold,2)))) {
    				mag_high += log(1 + sqrt(in[k][0]*in[k][0] + in[k][1]*in[k][1]));
    				nHigh++;
    			} else {
    				mag_low += log(1 + sqrt(in[k][0]*in[k][0] + in[k][1]*in[k][1]));
    				nLow++;
    			}
    		}
    	}
    	
    	return (mag_high / nHigh) / (mag_low / nLow);
}

void fftShiftComplex(fftw_complex *in, fftw_complex *out, int width, int height) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int srcIdx = i * width + j;
            int new_i = (i + height/2) % height;
            int new_j = (j + width/2) % width;
            int dstIdx = new_i * width + new_j;
            out[dstIdx][0] = in[srcIdx][0];
            out[dstIdx][1] = in[srcIdx][1];
        }
    }
}

fftw_complex* computeForwardFFTChannel(BMPImage *img, int channel) {
    int width = img->width, height = img->height;
    int total = width * height;
    fftw_complex *in = fftw_malloc(sizeof(fftw_complex) * total);
    fftw_complex *out = fftw_malloc(sizeof(fftw_complex) * total);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int idx = i * width + j;
            double val = 0.0;
            if (channel == 0)
                val = (double) img->pixels[i][j].red;
            else if (channel == 1)
                val = (double) img->pixels[i][j].green;
            else
                val = (double) img->pixels[i][j].blue;
            in[idx][0] = val;
            in[idx][1] = 0.0;
        }
    }
    fftw_plan plan = fftw_plan_dft_2d(height, width, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(plan);
    fftw_destroy_plan(plan);
    fftw_free(in);
    return out;
}

fftw_complex* computeInverseFFTChannel(fftw_complex *fftData, int width, int height) {
    int total = width * height;
    fftw_complex *in = fftw_malloc(sizeof(fftw_complex) * total);
    fftw_complex *out = fftw_malloc(sizeof(fftw_complex) * total);
    for (int i = 0; i < total; i++) {
        in[i][0] = fftData[i][0];
        in[i][1] = fftData[i][1];
    }
    fftw_plan plan = fftw_plan_dft_2d(height, width, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);
    fftw_execute(plan);
    fftw_destroy_plan(plan);
    fftw_free(in);
    // Normalize output.
    for (int i = 0; i < total; i++) {
        out[i][0] /= total;
        out[i][1] /= total;
    }
    return out;
}

BMPImage* createFFTDisplayImage(int width, int height, fftw_complex *fft_red, fftw_complex *fft_green, fftw_complex *fft_blue) {
    int total = width * height;
    double *mag_red   = malloc(sizeof(double) * total);
    double *mag_green = malloc(sizeof(double) * total);
    double *mag_blue  = malloc(sizeof(double) * total);
    double max_red = 0, max_green = 0, max_blue = 0;
    
    for (int i = 0; i < total; i++) {
        double m_r = log(1 + sqrt(fft_red[i][0]*fft_red[i][0] + fft_red[i][1]*fft_red[i][1]));
        double m_g = log(1 + sqrt(fft_green[i][0]*fft_green[i][0] + fft_green[i][1]*fft_green[i][1]));
        double m_b = log(1 + sqrt(fft_blue[i][0]*fft_blue[i][0] + fft_blue[i][1]*fft_blue[i][1]));
        mag_red[i] = m_r;
        mag_green[i] = m_g;
        mag_blue[i] = m_b;
        if (m_r > max_red) max_red = m_r;
        if (m_g > max_green) max_green = m_g;
        if (m_b > max_blue) max_blue = m_b;
    }
    
    BMPImage *dispImg = malloc(sizeof(BMPImage));
    dispImg->width = width;
    dispImg->height = height;
    // Set dummy headers.
    dispImg->fileHeader = (BITMAPFILEHEADER){0x4D42, 0, 0, 0, 54};
    dispImg->infoHeader = (BITMAPINFOHEADER){40, width, height, 1, 24, 0, 0, 0, 0, 0, 0};
    dispImg->pixels = malloc(height * sizeof(Pixel *));
    for (int i = 0; i < height; i++) {
        dispImg->pixels[i] = malloc(width * sizeof(Pixel));
    }
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int idx = i * width + j;
            unsigned char r = (unsigned char)((mag_red[idx] / max_red) * 255.0);
            unsigned char g = (unsigned char)((mag_green[idx] / max_green) * 255.0);
            unsigned char b = (unsigned char)((mag_blue[idx] / max_blue) * 255.0);
            dispImg->pixels[i][j].red = r;
            dispImg->pixels[i][j].green = g;
            dispImg->pixels[i][j].blue = b;
        }
    }
    free(mag_red); free(mag_green); free(mag_blue);
    return dispImg;
}

BMPImage* createImageFromIFFT(int width, int height, fftw_complex *inv_red, fftw_complex *inv_green, fftw_complex *inv_blue) {
    BMPImage *img = malloc(sizeof(BMPImage));
    img->width = width;
    img->height = height;
    img->fileHeader = (BITMAPFILEHEADER){0x4D42, 0, 0, 0, 54};
    img->infoHeader = (BITMAPINFOHEADER){40, width, height, 1, 24, 0, 0, 0, 0, 0, 0};
    img->pixels = malloc(height * sizeof(Pixel *));
    for (int i = 0; i < height; i++) {
        img->pixels[i] = malloc(width * sizeof(Pixel));
    }
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int idx = i * width + j;
            int r = (int) round(inv_red[idx][0]);
            int g = (int) round(inv_green[idx][0]);
            int b = (int) round(inv_blue[idx][0]);
            if (r < 0) r = 0; if (r > 255) r = 255;
            if (g < 0) g = 0; if (g > 255) g = 255;
            if (b < 0) b = 0; if (b > 255) b = 255;
            img->pixels[i][j].red = (unsigned char) r;
            img->pixels[i][j].green = (unsigned char) g;
            img->pixels[i][j].blue = (unsigned char) b;
        }
    }
    return img;
}

int saveFFTbmp( int width, int height, fftw_complex *fft_red, fftw_complex *fft_green, fftw_complex *fft_blue, const char *fftDisplayFilename) {
	BMPImage *fftDisplay = createFFTDisplayImage(width, height, fft_red, fft_green, fft_blue);
    if (saveBMP(fftDisplayFilename, fftDisplay) != 0) {
    	freeBMPImage(fftDisplay);
        printf("Error saving FFT display image.\n");
        return -1;
    } else {
        printf("FFT display image saved to %s\n", fftDisplayFilename);
    }
    freeBMPImage(fftDisplay);
    return 0;
}

complexRGB* RGBforwardFFT(BMPImage *img) {
    const int total = img->height * img->width;

    complexRGB *fft = malloc(sizeof(complexRGB));

    fft->red = fftw_malloc(sizeof(fftw_complex) * total);
    fft->green = fftw_malloc(sizeof(fftw_complex) * total);
    fft->blue = fftw_malloc(sizeof(fftw_complex) * total);


    fft->red = computeForwardFFTChannel(img, 0);
    fft->green = computeForwardFFTChannel(img, 1);
    fft->blue = computeForwardFFTChannel(img, 2);

    return fft;
}

complexRGB* RGBFFTshift(complexRGB *data, int width, int height) {
    const int total = width * height;

    complexRGB *shiftedData = malloc(sizeof(complexRGB));;
    shiftedData->red = fftw_malloc(sizeof(fftw_complex) * total);
    shiftedData->green = fftw_malloc(sizeof(fftw_complex) * total);
    shiftedData->blue = fftw_malloc(sizeof(fftw_complex) * total);

    fftShiftComplex(data->red, shiftedData->red, width, height);
    fftShiftComplex(data->green, shiftedData->green, width, height);
    fftShiftComplex(data->blue, shiftedData->blue, width, height);

    return shiftedData;
}

complexRGB* RGBinverseFFT (complexRGB *fft, int width, int height) {
    const int total = width * height;

    complexRGB *out = malloc(sizeof(complexRGB));;
    out->red = fftw_malloc(sizeof(fftw_complex) * total);
    out->green = fftw_malloc(sizeof(fftw_complex) * total);
    out->blue = fftw_malloc(sizeof(fftw_complex) * total);

    out->red = computeInverseFFTChannel(fft->red, width, height);
    out->green = computeInverseFFTChannel(fft->green, width, height);
    out->blue = computeInverseFFTChannel(fft->blue, width, height);

    return out;
}

int RGBsaveFFTbmp(complexRGB *fft, int width, int height, const char* fftDisplayName) {
    return saveFFTbmp(width, height, fft->red, fft->green, fft->blue, fftDisplayName);
}

BMPImage* RGBiFFTimage (complexRGB *ifft, int width, int height) {
    return createImageFromIFFT(width, height, ifft->red, ifft->green, ifft->blue);
}

double RGBenergyRatio (complexRGB *fftShifted, int width, int height) {
    return sqrt(pow(energyRatio(fftShifted->red, width, height), 2) + pow(energyRatio(fftShifted->green, width, height), 2) + pow(energyRatio(fftShifted->blue, width, height), 2));
}

void RGBfree (complexRGB *data) {
    fftw_free(data->red);
    fftw_free(data->green);
    fftw_free(data->blue);
}

