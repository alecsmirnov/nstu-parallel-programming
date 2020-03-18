#include <stdio.h>
#include <string.h>

#include "imageprocessor.h"

#define INPUT_FILENAME "kitten-gav.bmp"

int main(int argc, char* argv[]) {
    BMPImage image;
    readBMPFile(INPUT_FILENAME, &image);

    //--------------------------------------
    BMPImage embos_image;
    copyImage(&embos_image, &image);

    Filter embos;
    embos.factor = 1;
    embos.bias = 128;

    filterCreate(embos, {
        -1, -1,  0,
        -1,  0,  1,
        0,  1,  1
    });

    filtration(&embos_image, embos);
    free(embos.data);

    writeBMPFile("kitten-gav-embos.bmp", &embos_image);
    free(embos_image.data);
    
    //--------------------------------------
    BMPImage blur_image;
    copyImage(&blur_image, &image);

    Filter blur;
    blur.factor = 1.0 / 9.0;
    blur.bias = 0;

    filterCreate(blur, {
        1, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 1, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 1, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 1, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 1,
    });

    filtration(&blur_image, blur);
    free(blur.data);

    writeBMPFile("kitten-gav-blur.bmp", &blur_image);
    free(blur_image.data);

    //--------------------------------------
    free(image.data);

    return 0;
}