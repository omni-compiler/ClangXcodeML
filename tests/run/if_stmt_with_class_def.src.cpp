#include <stdio.h>

int
main() {
  for (int i = 0; i < 10; ++i) {
    if (i % 2)
      struct ClassThen {
        ClassThen() {
          printf("then\n");
        }
      } Then;
    else
      struct ClassElse {
        ClassElse() {
          printf("else\n");
        }
      } Else;
  }
  return 0;
}
