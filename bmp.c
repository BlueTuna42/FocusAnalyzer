#include "struct.h"

BMPImage* readBMP(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        printf("Error opening file %s\n", filename);
        return NULL;
    }
    BMPImage *img = malloc(sizeof(BMPImage));
    if (!img) {
        printf("BMPImage Memory allocation error.\n");
        fclose(fp);
        return NULL;
    }
    if (fread(&img->fileHeader, sizeof(BITMAPFILEHEADER), 1, fp) != 1) {
        printf("Error reading file header.\n");
        free(img);
        fclose(fp);
        return NULL;
    }
    if (img->fileHeader.bfType != 0x4D42) {
        printf("Not a valid BMP file.\n");
        free(img);
        fclose(fp);
        return NULL;
    }
    if (fread(&img->infoHeader, sizeof(BITMAPINFOHEADER), 1, fp) != 1) {
        printf("Error reading info header.\n");
        free(img);
        fclose(fp);
        return NULL;
    }
    img->width = img->infoHeader.biWidth;
    img->height = abs(img->infoHeader.biHeight);
    int rowSize = (img->width * 3 + 3) & ~3;
    int padding = rowSize - (img->width * 3);
    img->pixels = malloc(img->height * sizeof(Pixel *));
    if (!img->pixels) {
        printf("Pixel rows Memory allocation error.\n");
        free(img);
        fclose(fp);
        return NULL;
    }

    printf("%i \n", img->height);
    for (int i = 0; i < img->height; i++) {
        img->pixels[i] = malloc(img->width * sizeof(Pixel));
        if (!img->pixels[i]) {
            printf("Pixel colums Memory allocation error.\n");
            for (int j = 0; j < i; j++)
                free(img->pixels[j]);
            free(img->pixels);
            free(img);
            fclose(fp);
            return NULL;
        }
    }
    fseek(fp, img->fileHeader.bfOffBits, SEEK_SET);
    for (int i = 0; i < img->height; i++) {
        int row = (img->infoHeader.biHeight > 0) ? (img->height - 1 - i) : i;
        for (int j = 0; j < img->width; j++) {
            if (fread(&img->pixels[row][j], sizeof(Pixel), 1, fp) != 1) {
                printf("Error reading pixel data.\n");
                for (int k = 0; k < img->height; k++)
                    free(img->pixels[k]);
                free(img->pixels);
                free(img);
                fclose(fp);
                return NULL;
            }
        }
        fseek(fp, padding, SEEK_CUR);
    }
    fclose(fp);
    return img;
}

int saveBMP(const char *filename, BMPImage *img) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        printf("Error opening output file %s\n", filename);
        return 1;
    }
    int rowSize = (img->width * 3 + 3) & ~3;
    int padding = rowSize - (img->width * 3);
    img->infoHeader.biSizeImage = rowSize * img->height;
    img->fileHeader.bfSize = img->fileHeader.bfOffBits + img->infoHeader.biSizeImage;
    if (fwrite(&img->fileHeader, sizeof(BITMAPFILEHEADER), 1, fp) != 1 ||
        fwrite(&img->infoHeader, sizeof(BITMAPINFOHEADER), 1, fp) != 1) {
        printf("Error writing headers.\n");
        fclose(fp);
        return 1;
    }
    unsigned char pad[3] = {0, 0, 0};
    for (int i = 0; i < img->height; i++) {
        int row = (img->infoHeader.biHeight > 0) ? (img->height - 1 - i) : i;
        for (int j = 0; j < img->width; j++) {
            if (fwrite(&img->pixels[row][j], sizeof(Pixel), 1, fp) != 1) {
                printf("Error writing pixel data.\n");
                fclose(fp);
                return 1;
            }
        }
        if (fwrite(pad, 1, padding, fp) != (size_t)padding) {
            printf("Error writing padding.\n");
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

void freeBMPImage(BMPImage *img) {
    if (!img) return;
    for (int i = 0; i < img->height; i++) {
        free(img->pixels[i]);
    }
    free(img->pixels);
    free(img);
}
