#include <stdio.h>

class ClassA {
public:
  int x;
};

class ClassB {
public:
  int x;
};

class ClassC : public ClassA, public ClassB {
public:
  int
  func() {
    return ClassB::x;
  }
};

int
main() {
  ClassC obj;
  obj.ClassA::x = 100;
  printf("%d\n", obj.func());
}
