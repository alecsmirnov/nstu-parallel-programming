#include <stdio.h>
#include <string.h>

#include "imageprocessor.h"

#define IMAGE_BITNESS 32 
#define PIXEL_SIZE    4
#define BYTE_SIZE     8

#define throwErr(msg) do {          \
    fprintf(stderr, "%s\n", msg);   \
    exit(EXIT_FAILURE);             \
} while (0)

#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) < (y) ? (x) : (y))

#define doubleToByte(color) (floor(1.0 <= color ? 255 : color * 256.0))
#define byteToDouble(color) (color / 255.0)

static void readBitmap(FILE* fp, BMPImage* image) {
    size_t err = 0;

    err = fread(&image->file_header.type, sizeof image->file_header.type, 1, fp);
    if (err == 0) 
        throwErr("Error: reading file_header.type!");
    err = fread(&image->file_header.size, sizeof image->file_header.size, 1, fp);
    if (err == 0) 
        throwErr("Error: reading file_header.size!");
    err = fread(&image->file_header.reserved1, sizeof image->file_header.reserved1, 1, fp);
    if (err == 0) 
        throwErr("Error: reading file_header.reserved1!");
    err = fread(&image->file_header.reserved2, sizeof image->file_header.reserved2, 1, fp);
    if (err == 0) 
        throwErr("Error: reading file_header.reserved2!");
    err = fread(&image->file_header.offset_bits, sizeof image->file_header.offset_bits, 1, fp);
    if (err == 0) 
        throwErr("Error: reading file_header.offset_bits!");

    err = fread(&image->info_header.size, sizeof image->info_header.size, 1, fp);
    if (err == 0) 
        throwErr("Error: reading info_header.size!");
    err = fread(&image->info_header.width, sizeof image->info_header.width, 1, fp);
    if (err == 0) 
        throwErr("Error: reading info_header.width!");
    err = fread(&image->info_header.height, sizeof image->info_header.height, 1, fp);
     if (err == 0) 
        throwErr("Error: reading info_header.height!");
    err = fread(&image->info_header.planes, sizeof image->info_header.planes, 1, fp);
    if (err == 0) 
        throwErr("Error: reading info_header.planes!");
    err = fread(&image->info_header.bits_count, sizeof image->info_header.bits_count, 1, fp);
    if (err == 0) 
        throwErr("Error: reading info_header.bits_count!");
    err = fread(&image->info_header.compression, sizeof image->info_header.compression, 1, fp);
    if (err == 0) 
        throwErr("Error: reading info_header.compression!");
    err = fread(&image->info_header.size_image, sizeof image->info_header.size_image, 1, fp);
    if (err == 0) 
        throwErr("Error: reading info_header.size_image!");
    err = fread(&image->info_header.x_per_meter, sizeof image->info_header.x_per_meter, 1, fp);
    if (err == 0) 
        throwErr("Error: reading info_header.x_per_meter!");
    err = fread(&image->info_header.y_per_meter, sizeof image->info_header.y_per_meter, 1, fp);
    if (err == 0) 
        throwErr("Error: reading info_header.y_per_meter!");
    err = fread(&image->info_header.color_used, sizeof image->info_header.color_used, 1, fp);
    if (err == 0) 
        throwErr("Error: reading info_header.color_used!");
    err = fread(&image->info_header.color_important, sizeof image->info_header.color_important, 1, fp);
    if (err == 0) 
        throwErr("Error: reading info_header.color_important!");
}

static void readData(FILE* fp, BMPImage* image) {
    image->data = (uint8_t*)malloc(sizeof(uint8_t) * image->info_header.size_image);
    if (image->data == NULL)
        throwErr("Error: data out of memmory!");

    fseek(fp, sizeof(uint8_t) * image->file_header.offset_bits, SEEK_SET);
    size_t err = fread(image->data, sizeof(uint8_t), image->info_header.size_image, fp);
    if (err == 0) 
        throwErr("Error: reading image->data!");
}

void readBMPFile(const char* filename, BMPImage* image) {
    if (image == NULL)
        throwErr("Error: image null ptr!");

    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) 
        throwErr("Error: read file opening!");

    readBitmap(fp, image);
    readData(fp, image);

    fclose(fp);
}

static void writeBitmap(FILE* fp, const BMPImage* image) {
    fwrite(&image->file_header.type, sizeof(uint8_t), sizeof image->file_header.type, fp);
    fwrite(&image->file_header.size, sizeof(uint8_t), sizeof image->file_header.size, fp);
    fwrite(&image->file_header.reserved1, sizeof(uint8_t), sizeof image->file_header.reserved1, fp);
    fwrite(&image->file_header.reserved2, sizeof(uint8_t), sizeof image->file_header.reserved2, fp);
    fwrite(&image->file_header.offset_bits, sizeof(uint8_t), sizeof image->file_header.offset_bits, fp);
    
    fwrite(&image->info_header.size, sizeof(uint8_t), sizeof image->info_header.size, fp);
    fwrite(&image->info_header.width, sizeof(uint8_t), sizeof image->info_header.width, fp);
    fwrite(&image->info_header.height, sizeof(uint8_t), sizeof image->info_header.height, fp);
    fwrite(&image->info_header.planes, sizeof(uint8_t), sizeof image->info_header.planes, fp);
    fwrite(&image->info_header.bits_count, sizeof(uint8_t), sizeof image->info_header.bits_count, fp);
    fwrite(&image->info_header.compression, sizeof(uint8_t), sizeof image->info_header.compression, fp);
    fwrite(&image->info_header.size_image, sizeof(uint8_t), sizeof image->info_header.size_image, fp);
    fwrite(&image->info_header.x_per_meter, sizeof(uint8_t), sizeof image->info_header.x_per_meter, fp);
    fwrite(&image->info_header.y_per_meter, sizeof(uint8_t), sizeof image->info_header.y_per_meter, fp);
    fwrite(&image->info_header.color_used, sizeof(uint8_t), sizeof image->info_header.color_used, fp);
    fwrite(&image->info_header.color_important, sizeof(uint8_t), sizeof image->info_header.color_important, fp);
}

static void writeData(FILE* fp, const BMPImage* image) {
    fseek(fp, sizeof(uint8_t) * image->file_header.offset_bits, SEEK_SET);
    fwrite(image->data, sizeof(uint8_t), image->info_header.size_image, fp);
}

void writeBMPFile(const char* filename, const BMPImage* image) {
    if (image == NULL)
        throwErr("Error: image null ptr!");

    FILE* fp = fopen(filename, "wb");
    if (fp == NULL) 
        throwErr("Error: write file opening!");

    writeBitmap(fp, image);
    writeData(fp, image);

    free(image->data);

    fclose(fp);
}

static uint8_t* getPixelPtr(BMPImage* image, uint32_t x, uint32_t y) {
    uint8_t* pixel_ptr = NULL;

    if (x < image->info_header.width && y < image->info_header.height) {
        if (x == 0) ++x;
        if (y == 0) ++y;
        
        uint32_t row = 0 < image->info_header.height ? image->info_header.height - y - 1 : y;
        uint32_t row_size = ((image->info_header.bits_count * image->info_header.width + IMAGE_BITNESS - 1) / IMAGE_BITNESS) * PIXEL_SIZE;
        uint32_t bits_per_pixel = image->info_header.bits_count / BYTE_SIZE;

        pixel_ptr = row * row_size + x * bits_per_pixel + image->data;
    }

    return pixel_ptr;
}

void setPixel(BMPImage* image, uint32_t x, uint32_t y, Pixel pixel) {
    uint8_t* pixel_ptr = getPixelPtr(image, x, y);
    if (pixel_ptr == NULL) 
        throwErr("Error: abroad pixel image!");

    pixel_ptr[0] = pixel.b;
    pixel_ptr[1] = pixel.g;
    pixel_ptr[2] = pixel.r;
}

Pixel getPixel(BMPImage* image, uint32_t x, uint32_t y) {
    uint8_t* pixel_ptr = getPixelPtr(image, x, y);
    if (pixel_ptr == NULL)
        throwErr("Error: abroad pixel image!");

    return (Pixel){pixel_ptr[2], pixel_ptr[1], pixel_ptr[0]};
}

void filtration(BMPImage* image, double* filter, uint8_t filter_r) {
    double factor = 1.0;
    double bias = 0.0;

    Pixel** new_pixels = (Pixel**)malloc(sizeof(Pixel*) * image->info_header.width);
    for (uint32_t i = 0; i != image->info_header.width; ++i)
        new_pixels[i] = (Pixel*)malloc(sizeof(Pixel) * image->info_header.height);

    for(uint32_t x = 0; x != image->info_header.width; ++x)
        for(uint32_t y = 0; y != image->info_header.height; ++y) {
            double r = 0, g = 0, b = 0;

            for(uint8_t filter_x = 0; filter_x < filter_r; ++filter_x)
                for(uint8_t filter_y = 0; filter_y < filter_r; ++filter_y) {
                    uint32_t image_x = (x - filter_r / 2 + filter_x + image->info_header.width) % image->info_header.width;
                    uint32_t image_y = (y - filter_r / 2 + filter_y + image->info_header.height) % image->info_header.height;

                    Pixel pixel = getPixel(image, image_x, image_y);

                    r += byteToDouble(pixel.r) * filterAccess(filter, filter_r, filter_x, filter_y);
                    g += byteToDouble(pixel.g) * filterAccess(filter, filter_r, filter_x, filter_y);
                    b += byteToDouble(pixel.b) * filterAccess(filter, filter_r, filter_x, filter_y);
                }

            new_pixels[x][y].r = min(max(doubleToByte(factor * r + bias), 0), 255);
            new_pixels[x][y].g = min(max(doubleToByte(factor * g + bias), 0), 255);
            new_pixels[x][y].b = min(max(doubleToByte(factor * b + bias), 0), 255);  
        }

    for(uint32_t x = 0; x != image->info_header.width; ++x)
        for(uint32_t y = 0; y != image->info_header.height; ++y) 
            setPixel(image, x, y, new_pixels[x][y]);

    for(uint32_t x = 0; x != image->info_header.width; ++x)
        free(new_pixels[x]);
    free(new_pixels);
}