#include <stdio.h>
#include <string.h>

#include "imageprocessor.h"

#define INPUT_FILENAME  "kitten-gav.bmp"
#define OUTPUT_FILENAME "kitten-gav-filtered.bmp"

int main(int argc, char* argv[]) {
    BMPImage image;
    readBMPFile(INPUT_FILENAME, &image);

    double* filter = NULL;
    uint8_t filter_r = 0;

    filterCreate(double, filter, &filter_r, {
        0, 1, 0,
        1, 1, 1,
        0, 1, 0
    });

    filtration(&image, filter, filter_r);
    free(filter);

    writeBMPFile(OUTPUT_FILENAME, &image);

    return 0;
}