#include <stdio.h>

int main() {
  for (int i = 0; i < 10; ++i) {
    if (i % 2) {
      printf("then\n");
    } else {
      printf("else\n");
    }
  }
  return 0;
}
