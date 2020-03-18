#include <stdio.h>
#include <string.h>

#include "imageprocessor.h"

#define INPUT_FILENAME "kitten-gav.bmp"

int main(int argc, char* argv[]) {
    BMPImage image;
    readImage(INPUT_FILENAME, &image);

    //--------------------------------------
    BMPImage embos_image;
    copyImage(&embos_image, &image);

    Filter embos;
    filterCreate(embos, 1, 128, {
        -1, -1,  0,
        -1,  0,  1,
         0,  1,  1
    });

    filterImage(&embos_image, embos);

    writeImage("kitten-gav-embos.bmp", &embos_image);
    free(embos_image.data);
    
    //--------------------------------------
    BMPImage blur_image;
    copyImage(&blur_image, &image);

    Filter blur;
    filterCreate(blur, 1.0 / 9.0, 0, {
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

    filterImage(&blur_image, blur);

    writeImage("kitten-gav-blur.bmp", &blur_image);
    free(blur_image.data);

    //--------------------------------------
    free(image.data);

    return 0;
}