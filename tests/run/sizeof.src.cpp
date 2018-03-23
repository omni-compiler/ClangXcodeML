#include <stdio.h>

int
main() {
  printf("%zu\n", sizeof(double));
  int i = 1;
  printf("%zu\n", sizeof(i));
  int a3i[100];
  printf("%zu\n", sizeof(a3i));
}
