#ifndef MAPREDUCE_H
#define MAPREDUCE_H

#include <stdint.h>

// Указатели на функцию map, reduce
typedef double (*map_func_ptr)(double value);
typedef double (*reduce_func_ptr)(double result, double value);

// Обработка массива по модели mapReduce
double mapReduceArray(double* A, uint32_t size, 
                      map_func_ptr map, reduce_func_ptr reduce, 
                      uint8_t threads_count);

#endif