#include <stdio.h>

int
func(int a, int b = 100) {
  return a + b;
}

int
main() {
  printf("%d\n", func(10));
  return 0;
}
