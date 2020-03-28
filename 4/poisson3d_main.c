#include <stdio.h>

#include "poisson3d.h"

int main(int argc, char* argv[]) {
    size_t size = 100;

    Point D = (Point){size, size, size};
    Point N = (Point){size, size, size};
    Point h = (Point){D.x / (N.x - 1), D.y / (N.y - 1), D.y / (N.y - 1)};

    double res = calculateEquation(D, h);

    printf("Result: %lf\n", res);

    return 0;
}