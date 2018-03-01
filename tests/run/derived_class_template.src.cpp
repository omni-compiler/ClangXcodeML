#include <stdio.h>

template<typename T>
class ClassA {
  public:
  int func1() {
    return 100;
  }
};

template<typename T>
class ClassB : ClassA<T> {
  public:
  int func2(int k) {
    return k * this->func1();
  }
};

int main() {
  ClassB<int> b;
  printf("%d\n", b.func2(5));
  return 0;
}
