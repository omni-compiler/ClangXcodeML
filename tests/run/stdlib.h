#ifndef __STDLIB_H_INCLUDED__
#define __STDLIB_H_INCLUDED__
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void *malloc(size_t);
void free(void *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
