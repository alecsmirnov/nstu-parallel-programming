#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include "filter.h"

// Цвет пикселя BMP изображения
typedef struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Color;

typedef struct BitmapFileHeader {
    uint16_t type;                  // Тип формата
    uint32_t size;                  // Размер файла в байтах
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset_bits;           // Сдвиг данных, относительно начала
} BitmapFileHeader;

// Основная структура BMP файла
typedef struct BitmapInfoHeader {
    uint32_t size;                  // Размер структуры в байтах
    uint32_t width;                 // Ширина в пикселях
    uint32_t height;                // Высота в пикселях
    uint16_t planes;                // Для Windows: значки, курсор
    uint16_t bits_count;            // Количество бит на пиксель
    uint32_t compression;           // Способ хранения пикселей
    uint32_t size_image;            // Размер пиксельных данных в байтах
    int32_t  x_per_meter;           // Кол-во пикселей на метр горизонтали
    int32_t  y_per_meter;           // Кол-во пикселей на метр по вертикали
    uint32_t color_used;            // Размер таблицы цветов в ячейках
    uint32_t color_important;       // Кол-во ячеек от начала таблицы цветов
} BitmapInfoHeader;

typedef struct BMPImage {
    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;

    uint8_t* data;                  // Пиксельные данные.
} BMPImage;                         // Каждый пиксель представляет из себя 3 подряд идущих цвета: b, g, r

// Чтение/запись изображения
void readImage(const char* filename, BMPImage* image);
void writeImage(const char* filename, const BMPImage* image);

// Копирование изображение
void copyImage(BMPImage* image_dest, const BMPImage* image_src);
// Фильтрация изображения
void filterImage(BMPImage* image, const Filter* filter, uint8_t threads_count);

// Доступ к отдельным пикселям изображения
void setPixelColor(BMPImage* image, uint32_t x, uint32_t y, Color pixel);
Color getPixelColor(BMPImage* image, uint32_t x, uint32_t y);

#endif