#include <stdio.h>

class ClassA;

class ClassB {
public:
  ClassB(int i) {
    num = i;
  }
  int value() {
    return num;
  }
  friend class ClassA;

private:
  int num;
};

class ClassA {
public:
  void func(ClassB &b) {
    b.num = num;
  }
  int num;
};

int
main() {
  ClassA a;
  a.num = 100;
  ClassB obj(20);
  a.func(obj);
  printf("%d\n", obj.value());
  return 0;
}
