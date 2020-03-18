#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include "filter.h"

typedef struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Color;

typedef struct BitmapFileHeader {
    uint16_t type;   
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset_bits;
} BitmapFileHeader;

typedef struct BitmapInfoHeader {
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bits_count;
    uint32_t compression;
    uint32_t size_image;
    int32_t  x_per_meter;
    int32_t  y_per_meter;
    uint32_t color_used;
    uint32_t color_important;
} BitmapInfoHeader;

typedef struct BMPImage {
    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;

    uint8_t* data;
} BMPImage;

void readImage(const char* filename, BMPImage* image);
void writeImage(const char* filename, const BMPImage* image);

void copyImage(BMPImage* image_dest, const BMPImage* image_src);
void filterImage(BMPImage* image, Filter filter);

void setPixelColor(BMPImage* image, uint32_t x, uint32_t y, Color pixel);
Color getPixelColor(BMPImage* image, uint32_t x, uint32_t y);

#endif