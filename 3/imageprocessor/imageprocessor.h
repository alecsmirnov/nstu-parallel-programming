#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <stdint.h>

typedef struct BitmapFileHeader {
    uint16_t type;   
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset_bits;
} BitmapFileHeader;

typedef struct BitmapInfoHeader {
    uint32_t size;
    int32_t  width;
    int32_t  height;
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

void readBMPFile(const char* filename, BMPImage* image);
void writeBMPFile(const char* filename, const BMPImage* image);

void changePixel(int32_t x, int32_t y, uint8_t r, uint8_t g, uint8_t b, BMPImage* image);

#endif