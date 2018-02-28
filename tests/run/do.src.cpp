#include <stdio.h>

int
main() {
  int i = 1;
  int n = 0;
  do {
    n = n + i;
    ++i;
  } while (i <= 100);
  printf("%d\n", n);
  return 0;
}
