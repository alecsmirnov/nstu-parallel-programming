#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#define merge(x, y) x##_##y

#define filterCreate(type, array_ptr, r_ptr, ...) do {                                       \
    static type merge(array_ptr, const)[] = __VA_ARGS__;                                     \
    static uint16_t merge(array_ptr, size) = sizeof(merge(array_ptr, const)) / sizeof(type); \
    array_ptr = malloc(merge(array_ptr, size) * sizeof(type));                               \
    if (array_ptr == NULL)                                                                   \
        fprintf(stderr, "Error: filter out of memmory!\n");                                  \
    memcpy(array_ptr, merge(array_ptr, const), merge(array_ptr, size) * sizeof(type));       \
    if (r_ptr == NULL)                                                                       \
        fprintf(stderr, "Error: pass NULL ptr!\n");                                          \
    *r_ptr = sqrt(merge(array_ptr, size));                                                   \
} while (0)

#define filterAccess(filter, filter_r, i, j) \
    (filter[(i) * (filter_r) + (j)])

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

typedef struct Pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Pixel;

void readBMPFile(const char* filename, BMPImage* image);
void writeBMPFile(const char* filename, const BMPImage* image);

void setPixel(BMPImage* image, uint32_t x, uint32_t y, Pixel pixel);
Pixel getPixel(BMPImage* image, uint32_t x, uint32_t y);

void filtration(BMPImage* image, double* filter, uint8_t filter_r);

#endif