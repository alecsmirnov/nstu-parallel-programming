#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "imageprocessor.h"

#define IMAGE_BITNESS 32 
#define PIXEL_SIZE    4
#define BYTE_SIZE     8

#define throwErr(msg) do {          \
    fprintf(stderr, "%s\n", msg);   \
    exit(EXIT_FAILURE);             \
} while (0)

void readBitmap(FILE* fp, BMPImage* image) {
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

void readData(FILE* fp, BMPImage* image) {
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

void writeBitmap(FILE* fp, const BMPImage* image) {
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

void writeData(FILE* fp, const BMPImage* image) {
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

void changePixel(int32_t x, int32_t y, uint8_t r, uint8_t g, uint8_t b, BMPImage* image) {
    if (0 < x && x < image->info_header.width && 0 < y && y < image->info_header.height) {
        int32_t row = image->info_header.height ? image->info_header.height - y - 1 : y;
        int32_t row_size = ((image->info_header.bits_count * image->info_header.width + IMAGE_BITNESS - 1) / IMAGE_BITNESS) * PIXEL_SIZE;
        int32_t bits_per_pixel = image->info_header.bits_count / BYTE_SIZE;

        uint8_t* pixel = row * row_size + x * bits_per_pixel + image->data;

        pixel[0] = b;
        pixel[1] = g;
        pixel[2] = r;
    }
}