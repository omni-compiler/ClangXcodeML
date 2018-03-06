#include <stdio.h>

int
main() {
  for (int i = 0; int const j = 10 - i; ++i) {
    printf("%d\n", j);
  }
}
