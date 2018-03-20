#include <stdio.h>

int
func() {
  return 100;
}

int
main() {
  if (sizeof(int) == sizeof(func())) {
    printf("OK\n");
  } else {
    printf("NG\n");
  }
}
