#include <stdio.h>

void
func1() {
  int i = 1;
  int n = 0;
  do {
    n = n + i;
    ++i;
  } while (i <= 100);
  printf("%d\n", n);
}

void
func2() {
  int j = 1;
  do
    printf("func2, %d\n", j++);
  while (j < 10);
}

void
func3() {
  int i = 0;
  do
    struct A {
      A(int x) {
        printf("func3, %d\n", x * x);
      }
    } obj(++i);
  while (i < 20);
}

int
main() {
  func1();
  func2();
  func3();
  return 0;
}
