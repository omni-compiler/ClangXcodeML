#include <stdio.h>

int factor[1000];

int
main() {
  factor[0] = factor[1] = 1;
  for (int p = 2; p < 1000; ++p) {
    if (factor[p]) {
      continue;
    }
    for (int n = 2 * p; n < 1000; n += p) {
      factor[n] = p;
    }
  }
  for (int i = 0; i < 1000; ++i) {
    if (!factor[i]) {
      printf("%d\n", i);
    }
  }

  const int array10_ci[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  for (int i = 0; i < 10; ++i) {
    printf("%d\n", array10_ci[i]);
  }
}
