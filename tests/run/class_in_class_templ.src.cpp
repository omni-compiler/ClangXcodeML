#include <stdio.h>

template <typename T>
class ClassA {
public:
  class ClassB {
  public:
    T x;
    T
    get() {
      return x;
    }
  };
};

int
main() {
  ClassA<int>::ClassB obj;
  obj.x = 100;
  printf("%d\n", obj.get());
}
