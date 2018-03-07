#include <stdio.h>

class ClassA {
public:
  void
  print() {
    printf("%d\n", i);
  }
  int i;
};

class ClassB {
public:
  void
  print() {
    printf("%lf\n", d);
  }
  double d;
};

class ClassC : public ClassA, public ClassB {};

int
main() {
  ClassC obj;
  obj.i = 100;
  obj.d = 3.14;
  static_cast<ClassA>(obj).print();
  static_cast<ClassB>(obj).print();
}
