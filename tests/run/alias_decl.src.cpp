#include <stdio.h>

using T = int (*)(int);

int
func1(int x) {
  return x * x + x + 1;
}

int
func2(int x) {
  return 2 * x;
}

int
func3(int x) {
  return x - 1;
}

int
main() {
  T a[3];
  a[0] = func1;
  a[1] = func2;
  a[2] = func3;
  for (int i = 0; i < 10; ++i) {
    printf("%d\n", (a[i % 3])(i));
  }
}
