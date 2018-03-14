#include <stdio.h>

template <typename T1>
class ClassA {
public:
  T1 x;
  T1
  func_a() {
    return x + x;
  }

  template <typename T2>
  class ClassB {
  public:
    T2 y;
    T2
    func_b(T1 z) {
      return y + y;
    }
  };
};

int
main() {
  ClassA<int>::ClassB<char> obj;
  printf("%d\n", obj.func_b(1));
}
