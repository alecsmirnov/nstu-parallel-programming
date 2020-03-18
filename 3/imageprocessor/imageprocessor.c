#include <stdio.h>
#include <string.h>

#if defined(_OPENMP)
#include <omp.h>
#endif

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
    (min(max(((filter)->factor * color + (filter)->bias), 0), BYTE_SIZE))

// Данные потока для обработки
typedef struct Chunk {
    Color** pixels;

    uint32_t x0;
    uint32_t x1;
    uint32_t y0;
    uint32_t y1;
} Chunk;

// Чтение заголовков файла
static void readBitmap(FILE* fp, BMPImage* image) {
    size_t err = 0;

    // Избегаем прагм упаковки структур и невозможности переноса.
    // Читаем все данные, без чтения округлённого размера структур в памяти
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
    // Избегаем прагм упаковки структур и невозможности переноса.
    // Записываем все данные, без записи округлённого размера структур в памяти
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
        throwErr("Error: pixel abroad when set!");

    pixel_ptr[0] = pixel.b;
    pixel_ptr[1] = pixel.g;
    pixel_ptr[2] = pixel.r;
}

// Получить цвет пикселя
Color getPixelColor(BMPImage* image, uint32_t x, uint32_t y) {
    uint8_t* pixel_ptr = getPixelPtr(image, x, y);
    if (pixel_ptr == NULL)
        throwErr("Error: pixel abroad when get!");

    return (Color){pixel_ptr[2], pixel_ptr[1], pixel_ptr[0]};
}

static void filterImageChunk(BMPImage* image, const Filter* filter, Chunk* chunk) {
    // Фильтрация всех пикселей изображения
    for(uint32_t x = chunk->x0; x < chunk->x1; ++x)
        for(uint32_t y = chunk->y0; y < chunk->y1; ++y) {
            FilterColor filter_color = (FilterColor){0, 0, 0};

            // Применение матрицы фильтра к пикселю и его соседям
            for(uint8_t filter_x = 0; filter_x < filter->r; ++filter_x)
                for(uint8_t filter_y = 0; filter_y < filter->r; ++filter_y) {
                    uint32_t image_x = (x - filter->r / 2 + filter_x + image->info_header.width) % image->info_header.width;
                    uint32_t image_y = (y - filter->r / 2 + filter_y + image->info_header.height) % image->info_header.height;

                    Color image_pixel = getPixelColor(image, image_x, image_y);

                    filter_color.r += image_pixel.r * filterAccess(filter, filter_x, filter_y);
                    filter_color.g += image_pixel.g * filterAccess(filter, filter_x, filter_y);
                    filter_color.b += image_pixel.b * filterAccess(filter, filter_x, filter_y);
                }
            
            // Сохранение преобразованного пикселя с выравниванием по размеру байта
            chunk->pixels[x][y].r = correctFilterColor(filter, filter_color.r);
            chunk->pixels[x][y].g = correctFilterColor(filter, filter_color.g);
            chunk->pixels[x][y].b = correctFilterColor(filter, filter_color.b);
        }

    // Замещение изображения на преобразованное
    for(uint32_t x = chunk->x0; x < chunk->x1; ++x)
        for(uint32_t y = chunk->y0; y < chunk->y1; ++y) 
            setPixelColor(image, x, y, chunk->pixels[x][y]);
}

// Фильтрация изображения
void filterImage(BMPImage* image, const Filter* filter, uint8_t threads_count) {
    #if defined(_OPENMP)
    omp_set_dynamic(0);
    omp_set_num_threads(threads_count);
    #endif

    // Инициализация пиксельного поля для преобразованного изображения
    Color** new_pixels = (Color**)malloc(sizeof(Color*) * image->info_header.width);
    if (new_pixels == NULL)
        throwErr("Error: new_pixels out of memmory!");

    #pragma omp parallel for
    for (uint32_t i = 0; i < image->info_header.width; ++i) {
        new_pixels[i] = (Color*)malloc(sizeof(Color) * image->info_header.height);
        if (new_pixels[i] == NULL)
            throwErr("Error: new_pixels[i] out of memmory!");
    }

    // Инициализация блоков данных
    Chunk* chunks = (Chunk*)malloc(sizeof(Chunk) * threads_count);
    if (new_pixels == NULL)
        throwErr("Error: chunks out of memmory!");

    uint32_t chunk_size = image->info_header.width / threads_count;
    uint8_t remainder = image->info_header.height * image->info_header.width % threads_count;

    // Определение размеров блоков данных
    uint8_t shift = 0;
    for (uint8_t i = 0; i < threads_count; ++i) {
        uint8_t step = i < remainder ? 1 : 0;

        chunks[i].pixels = new_pixels;
        chunks[i].x0 = i * chunk_size + shift;
        chunks[i].x1 = i * chunk_size + chunk_size + shift + step;
        chunks[i].y0 = 0;
        chunks[i].y1 = image->info_header.height;

        shift += step;
    }

    #pragma omp parallel
    {   
        uint8_t thread_num = 0;

        #if defined(_OPENMP)
        thread_num = omp_get_thread_num();
        #endif

        filterImageChunk(image, filter, &chunks[thread_num]);
    }

    #pragma omp parallel for
    for (uint32_t x = 0; x < image->info_header.width; ++x)
        free(new_pixels[x]);
    free(new_pixels);
    free(chunks);
}