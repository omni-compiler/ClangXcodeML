#include <stdio.h>

void
func1() {
  int i = 1;
  int n = 0;
  while (i <= 100) {
    n = n + i;
    ++i;
  }
  printf("%d\n", n);
}

void
func2() {
  int j = 1;
  while (j < 10)
    printf("func2, %d\n", j++);
}

void
func3() {
  int i = 0;
  while (i < 20)
    struct A {
      A(int x) {
        printf("func3, %d\n", x * x);
      }
    } obj(++i);
}

int
main() {
  func1();
  func2();
  func3();
  return 0;
}
