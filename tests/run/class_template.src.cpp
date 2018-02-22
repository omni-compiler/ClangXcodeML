#include <stdio.h>

template <typename T>
class ClassA {
public:
  ClassA(T x) {
    data = x;
  }
  T
  get() {
    return data;
  }

private:
  T data;
};

int
main() {
  ClassA<int> obj1(100);
  printf("%d\n", obj1.get());
}
