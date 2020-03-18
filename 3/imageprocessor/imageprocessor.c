#include <stdio.h>
#include <string.h>

#include "imageprocessor.h"

#define IMAGE_BITNESS 32 

#define BYTE_SIZE  255
#define BYTE_VALUE 8
#define PIXEL_SIZE 4                // +1 байт для 32-х разрядного заголовка

// Функция обработки ошибок
#define throwErr(msg) do {          \
    fprintf(stderr, "%s\n", msg);   \
    exit(EXIT_FAILURE);             \
} while (0)

#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) < (y) ? (x) : (y))

// Корректировка цвета фильтра после преобразований
#define correctFilterColor(filter, color) \
    (min(max((filter.factor * color + filter.bias), 0), BYTE_SIZE))

// Чтение заголовков файла
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

// Чтение пиксельных данных
static void readData(FILE* fp, BMPImage* image) {
    image->data = (uint8_t*)malloc(sizeof(uint8_t) * image->info_header.size_image);
    if (image->data == NULL)
        throwErr("Error: data out of memmory!");

    fseek(fp, sizeof(uint8_t) * image->file_header.offset_bits, SEEK_SET);
    size_t err = fread(image->data, sizeof(uint8_t), image->info_header.size_image, fp);
    if (err == 0) 
        throwErr("Error: reading image->data!");
}

// Чтение изображения
void readImage(const char* filename, BMPImage* image) {
    if (image == NULL)
        throwErr("Error: image null ptr!");

    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) 
        throwErr("Error: read file opening!");

    readBitmap(fp, image);
    readData(fp, image);

    fclose(fp);
}

// Запись заголовков файла
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

// Запись пиксельных данных
static void writeData(FILE* fp, const BMPImage* image) {
    fseek(fp, sizeof(uint8_t) * image->file_header.offset_bits, SEEK_SET);
    fwrite(image->data, sizeof(uint8_t), image->info_header.size_image, fp);
}

// Запись изображения
void writeImage(const char* filename, const BMPImage* image) {
    if (image == NULL)
        throwErr("Error: image null ptr!");

    FILE* fp = fopen(filename, "wb");
    if (fp == NULL) 
        throwErr("Error: write file opening!");

    writeBitmap(fp, image);
    writeData(fp, image);

    fclose(fp);
}

// Копирование изображения
void copyImage(BMPImage* image_dest, const BMPImage* image_src) {
    if (image_dest == NULL)
        throwErr("Error: dest null ptr!");
    if (image_src == NULL)
        throwErr("Error: src null ptr!");

    image_dest->file_header = image_src->file_header;
    image_dest->info_header = image_src->info_header;
    
    image_dest->data = (uint8_t*)malloc(sizeof(uint8_t) * image_dest->info_header.size_image);
    if (image_dest->data == NULL)
        throwErr("Error: dest data out of memmory!");

    memcpy(image_dest->data, image_src->data, sizeof(uint8_t) * image_dest->info_header.size_image);
}

// Получить указатель пикселя из позиции в пиксельных данных
static uint8_t* getPixelPtr(BMPImage* image, uint32_t x, uint32_t y) {
    uint8_t* pixel_ptr = NULL;

    // Доступ к пикселю в пиксельных данных
    if (x < image->info_header.width && y < image->info_header.height) {       
        uint32_t row = 0 < image->info_header.height ? image->info_header.height - y - 1 : y;
        uint32_t row_size = ((image->info_header.bits_count * 
                              image->info_header.width + IMAGE_BITNESS - 1) / IMAGE_BITNESS) * PIXEL_SIZE;
        uint32_t bits_per_pixel = image->info_header.bits_count / BYTE_VALUE;

        pixel_ptr = row * row_size + x * bits_per_pixel + image->data;
    }

    return pixel_ptr;
}

// Изменить цвет пикселя
void setPixelColor(BMPImage* image, uint32_t x, uint32_t y, Color pixel) {
    uint8_t* pixel_ptr = getPixelPtr(image, x, y);
    if (pixel_ptr == NULL) 
        throwErr("Error: abroad pixel image!");

    pixel_ptr[0] = pixel.b;
    pixel_ptr[1] = pixel.g;
    pixel_ptr[2] = pixel.r;
}

// Получить цвет пикселя
Color getPixelColor(BMPImage* image, uint32_t x, uint32_t y) {
    uint8_t* pixel_ptr = getPixelPtr(image, x, y);
    if (pixel_ptr == NULL)
        throwErr("Error: abroad pixel image!");

    return (Color){pixel_ptr[2], pixel_ptr[1], pixel_ptr[0]};
}

// Фильтрация изображения
void filterImage(BMPImage* image, Filter filter) {
    // Инициализация пиксельного поля для преобразованного изображения
    Color** new_pixels = (Color**)malloc(sizeof(Color*) * image->info_header.width);
    if (new_pixels == NULL)
        throwErr("Error: new_pixels out of memmory!");
    for (uint32_t i = 0; i < image->info_header.width; ++i) {
        new_pixels[i] = (Color*)malloc(sizeof(Color) * image->info_header.height);
        if (new_pixels[i] == NULL)
            throwErr("Error: new_pixels[i] out of memmory!");
    }

    // Фильтрация всех пикселей изображения
    for(uint32_t x = 0; x < image->info_header.width; ++x)
        for(uint32_t y = 0; y < image->info_header.height; ++y) {
            FilterColor filter_color = (FilterColor){0, 0, 0};

            // Применение матрицы фильтра к пикселю и его соседям
            for(uint8_t filter_x = 0; filter_x < filter.r; ++filter_x)
                for(uint8_t filter_y = 0; filter_y < filter.r; ++filter_y) {
                    uint32_t image_x = (x - filter.r / 2 + filter_x + image->info_header.width) % image->info_header.width;
                    uint32_t image_y = (y - filter.r / 2 + filter_y + image->info_header.height) % image->info_header.height;

                    Color image_pixel = getPixelColor(image, image_x, image_y);

                    filter_color.r += image_pixel.r * filterAccess(filter, filter_x, filter_y);
                    filter_color.g += image_pixel.g * filterAccess(filter, filter_x, filter_y);
                    filter_color.b += image_pixel.b * filterAccess(filter, filter_x, filter_y);
                }
            
            // Сохранение преобразованного пикселя с выравниванием по размеру байта
            new_pixels[x][y].r = correctFilterColor(filter, filter_color.r);
            new_pixels[x][y].g = correctFilterColor(filter, filter_color.g);
            new_pixels[x][y].b = correctFilterColor(filter, filter_color.b);
        }

    // Замещение изображения на преобразованное
    for(uint32_t x = 0; x < image->info_header.width; ++x)
        for(uint32_t y = 0; y < image->info_header.height; ++y) 
            setPixelColor(image, x, y, new_pixels[x][y]);

    for(uint32_t x = 0; x < image->info_header.width; ++x)
        free(new_pixels[x]);
    free(new_pixels);
}