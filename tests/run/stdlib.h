#ifndef __STDLIB_H_INCLUDED__
#define __STDLIB_H_INCLUDED__
#include <stddef.h>

extern "C" {

void *malloc(size_t);
void free(void *);
}

#endif
