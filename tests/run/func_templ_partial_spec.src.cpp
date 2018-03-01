#include <stdio.h>

template<typename T>
int func(T x) {
  return 100;
}

template<typename T>
int func(T *x) {
  return 200;
}

int main() {
  int i = func('a');
  int j = func(&i);
  printf("%d,%d\n", i, j);
  return 0;
}
