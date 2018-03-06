#include <stdio.h>

int
main() {
  int count = 0;
  int sum = 0;
  for (int i = 0; i < 20; ++i) {
    switch (i % 5) {
    case 0: ++count;
    case 1: printf("i % 5 < 2\n"); break;
    case 2: {
      printf("i % 5 = 2\n");
      int k = i * i;
      sum += k;
    }
    default: printf("i % 5 > 2\n");
    }
  }
  printf("%d %d\n", count, sum);
}
