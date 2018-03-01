#include <stdio.h>

template <typename T>
const char *
func1(T x) {
  return "func1<T>";
}

template <>
const char *
func1(int x) {
  return "func1<int>";
}

const char *
func1(int x) {
  return "func1";
}

template<typename T>
const char *
func2() {
  return "func2<T>";
}

template<>
const char *
func2<int>() {
  return "func2<int>";
}

const char *
func2() {
  return "func2";
}

int
main() {
  printf("%s\n", func1('c'));
  printf("%s\n", func1<int>(10));
  printf("%s\n", func1(10));
  printf("%s\n", func2<char>());
  printf("%s\n", func2<int>());
  printf("%s\n", func2());
  return 0;
}
