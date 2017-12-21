#include <stdio.h>

int
main() {
  union {
    int x;
    int y;
  };
  x = 0;
  printf("%d\n", x);
  y = 20;
  printf("%d\n", x);
}
