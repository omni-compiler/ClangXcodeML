#include <stdio.h>

int
func() {
  static int x = 0;
  return ++x;
}

int
main() {
  printf("%d\n", func());
  printf("%d\n", func());
  printf("%d\n", func());
}
