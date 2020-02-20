#ifndef ERRORHANDLE_H
#define ERRORHANDLE_H

#include <stdio.h>
#include <string.h>

#define PASS_CODE 0

#define errorHandle(test_code, pass_code, err_msg) do {				\
	if (test_code != pass_code) {									\
		fprintf(stderr, "%s%s\n", err_msg, strerror(test_code));	\
		exit(EXIT_FAILURE);											\
	}																\
} while (0)

/*
static inline void errorHandle(int test_code, int pass_code, const char* err_msg) {
	if (test_code != pass_code) {
		fprintf(stderr, "%s%s", err_msg, strerror(test_code));
		exit(EXIT_FAILURE);
	}
}
*/

#endif
