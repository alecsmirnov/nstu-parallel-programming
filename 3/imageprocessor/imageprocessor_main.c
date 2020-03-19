#include <stdio.h>
#include <string.h>
#include <omp.h>

#include "imageprocessor.h"

#define DEMO_ARGS_COUNT 4
#define TEST_ARGS_COUNT 4

#define TEST_RESULT_FILENAME "result.txt"
#define TEST_SIZE 4

static const char* TEST_FOLDER = "test_images/";
static const char* TEST_FILENAMES[TEST_SIZE] = {"2k.bmp", "4k.bmp", "5k.bmp", "8k.bmp"};

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

static char* makeTestFilename(const char* folder, const char* filename) {
    size_t result_size = strlen(filename) + strlen(folder) + 1;
    char* result_filename = (char*)malloc(sizeof(char*) * result_size);

    snprintf(result_filename, sizeof(char) * result_size, "%s%s", folder, filename);

    return result_filename;
}

// Формирование результирующего названия файла по названию и суффиксу
static char* makeResultFilename(const char* filename, const char* suffix) {
    size_t result_size = strlen(filename) + strlen(suffix) + 1;
    char* result_filename = (char*)malloc(sizeof(char*) * result_size);

    strncpy(result_filename, filename, strlen(filename) - 4);
    result_filename[strlen(filename) - 4] = '\0';
    strcat(result_filename, suffix);
    strcat(result_filename, ".bmp");

    return result_filename;
}

static char* getResultFilename(const char* filename, uint8_t filter_num) {
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

static void test(int argc, char* argv[]) {
	if (argc < TEST_ARGS_COUNT) {
		fprintf(stderr, "Wrong number of arguments!\n");
		fprintf(stderr, "Enter: <filter num> <threads count> <measure count>\n");
        fprintf(stderr, "(Filter number: 0 - embos, 1 - edges, 2 - sharpen, 3 - blur)\n");
		exit(EXIT_FAILURE);
	}

    uint8_t filter_num = atoi(argv[1]);
	uint8_t threads_count = atoi(argv[2]);
	uint8_t measure_count = atoi(argv[3]);

	FILE* fp = fopen(TEST_RESULT_FILENAME, "w");

	printf("Program execution...\n");
	fprintf(fp, "size:\tthreads: 1\tthreads: 2\tthreads: 3\tthreads: 4\n");

    Filter filter = getFilter(filter_num);
	for (uint8_t test_num = 0; test_num != TEST_SIZE; ++test_num) {
        char* test_name = makeTestFilename(TEST_FOLDER, TEST_FILENAMES[test_num]);

        BMPImage image;
        readImage(test_name, &image);

		fprintf(fp, "%u x %u\t", image.info_header.width, image.info_header.height);

		for (uint8_t i = 0; i != threads_count; ++i) {
			double elapsed_time = 0;
			
			for (uint8_t j = 0; j != measure_count; ++j) {
                BMPImage test_image;
                copyImage(&test_image, &image);

                double start = omp_get_wtime();
                filterImage(&test_image, &filter, i + 1);
                double stop = omp_get_wtime();

                elapsed_time += stop - start;

                free(test_image.data);
			}

			fprintf(fp, "%lf\t", elapsed_time / measure_count);
		}

        free(test_name);
        free(image.data);

        printf("Test %hhu complete\n", test_num + 1);
		fprintf(fp, "\n");
	}

    free(filter.matrix);
    
	printf("Done.\n");
	fclose(fp);
}

int main(int argc, char* argv[]) {
    #ifdef TEST 
	test(argc, argv);
	#else 
	demonstration(argc, argv);
	#endif

    return 0;
}