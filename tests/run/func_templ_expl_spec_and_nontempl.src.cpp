#include <stdio.h>

template <typename T>
const char *
func(T x) {
  return "func<T>";
}

template <>
const char *
func(int x) {
  return "func<int>";
}

const char *
func(int x) {
  return "func";
}

int
main() {
  printf("%s\n", func('c'));
  printf("%s\n", func<int>(10));
  printf("%s\n", func(10));
  return 0;
}
