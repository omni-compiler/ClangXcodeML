#include <stdio.h>

template <typename T>
struct ClassA {
  int
  func() {
    return 100;
  }
};

template <>
struct ClassA<int> {
  int x;
  int y;
  int
  func() {
    return x + y;
  }
};

int
main() {
  ClassA<double> a;
  printf("%d\n", a.func());

  ClassA<int> b;
  b.x = 100;
  b.y = 100;
  printf("%d\n", b.func());
  return 0;
}
