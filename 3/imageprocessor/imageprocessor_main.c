#include <stdio.h>
#include <string.h>
#include <omp.h>

#include "imageprocessor.h"

#define DEMO_ARGS_COUNT 4

// Пресеты фильтров
typedef enum FilterNum {
    EMBOS,
    SHARPEN,
    EDGES,
    BLUR,
    FILTERNUM_SIZE
} FilterNum;

// Поличить пресет фильтра по номеру
static Filter getFilter(uint8_t filter_num) {
    Filter filter;
    Filter const_filter;

     switch (filter_num) {
        case EMBOS: {
            const_filter = filterCreate(1.0, 128.0, {
                -1, -1,  0,
                -1,  0,  1,
                 0,  1,  1
            });
            break;
        }
        case SHARPEN: {
            const_filter = filterCreate(1.0, 0.0, {
                -1, -1, -1,
                -1,  9, -1,
                -1, -1, -1
            });
            break;
        }
        case EDGES: {
            const_filter = filterCreate(1.0, 0.0, {
                0,  0, -1,  0,  0,
                0,  0, -1,  0,  0,
                0,  0,  2,  0,  0,
                0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,
            });
            break;
        }
        case BLUR: {
            const_filter = filterCreate(1.0 / 9.0, 0.0, {
                1, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 1, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 1, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 1, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 1, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 1, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 1, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 1, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 1
            });
            break;
        }
        default: {
            const_filter = filterCreate(1.0, 0.0, {1});
            break;
        }
    }

    // Создание динамической матрицы, для её передачи из функции 
    filter.r = const_filter.r;
    filter.factor = const_filter.factor;
    filter.bias = const_filter.bias;

    filter.matrix = (double*)malloc(sizeof(double) * filter.r * filter.r);
    if (filter.matrix == NULL) {
        fprintf(stderr, "Error: filter.matrix out of memmory!\n");
		exit(EXIT_FAILURE);
    }
    memcpy(filter.matrix, const_filter.matrix, sizeof(double) * filter.r * filter.r);

    return filter;
}

// Формирование результирующего названия файла по названию и суффиксу
static char* makeResultFilename(const char* filename, const char* suffix) {
    size_t result_size = strlen(filename) + strlen(suffix);
    char* result_filename = (char*)malloc(sizeof(char*) * result_size);

    strncpy(result_filename, filename, strlen(filename) - 4);
    result_filename[strlen(filename) - 4] = '\0';
    strcat(result_filename, suffix);
    strcat(result_filename, ".bmp");

    return result_filename;
}

char* getResultFilename(const char* filename, uint8_t filter_num) {
    char* result_filename = NULL;

    switch (filter_num) {
        case EMBOS:   result_filename = makeResultFilename(filename, "-embos");   break;
        case SHARPEN: result_filename = makeResultFilename(filename, "-sharpen"); break;
        case EDGES:   result_filename = makeResultFilename(filename, "-edges");   break;
        case BLUR:    result_filename = makeResultFilename(filename, "-blur");    break;
        default:      result_filename = makeResultFilename(filename, "-none");    break;
    }

    return result_filename;
}

static void demonstration(int argc, char* argv[]) {
    if (argc < DEMO_ARGS_COUNT) {
		fprintf(stderr, "Wrong number of arguments!\n");
		fprintf(stderr, "Enter: <filename> <filter number> <threads count>\n");
        fprintf(stderr, "(Filter number: 0 - embos, 1 - edges, 2 - sharpen, 3 - blur)\n");
		exit(EXIT_FAILURE);
	}

    const char* filename = argv[1];
    uint8_t filter_num = atoi(argv[2]);
    uint8_t threads_count = atoi(argv[3]);

    if (FILTERNUM_SIZE <= filter_num) {
        fprintf(stderr, "Error: invalid filter number!\n");
        exit(EXIT_FAILURE);
    }
    if (threads_count == 0) {
        fprintf(stderr, "Error: the number of threads cannot be equal 0!\n");
        exit(EXIT_FAILURE);
    }

    BMPImage image;
    readImage(filename, &image);

    Filter filter = getFilter(filter_num);
    
    double start = omp_get_wtime();
    filterImage(&image, &filter, threads_count);
    double stop = omp_get_wtime();

    char* result_filename = getResultFilename(filename, filter_num);

    writeImage(result_filename, &image);

    printf("\nElapdes time: %lf\n", stop - start);
    
    free(result_filename);
    free(filter.matrix);
    free(image.data);
}

int main(int argc, char* argv[]) {
    demonstration(argc, argv);

    return 0;
}