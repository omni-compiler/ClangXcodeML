#include <stdio.h>

class ClassA {
public:
  ClassA() : num(42) {
  }
  ClassA(int i, int j) : num(i * j) {
  }
  ClassA(int i, int j, int k);
  int num;
};

ClassA::ClassA(int i, int j, int k) : num(i * j * k) {
}

class ClassB : public ClassA {
public:
  ClassB() : ClassA(), bnum(100) {
    bnum += num;
  }
  ClassB(int i) : ClassA(i, i), bnum(i) {
    bnum += num;
  }
  int bnum;
};

int
main() {
  ClassA a1(10, 20);
  printf("%d\n", a1.num);
  ClassA a2(10, 20, 30);
  printf("%d\n", a2.num);
  ClassB b1 = ClassB();
  printf("%d\n", b1.bnum);
  ClassB b2(100);
  printf("%d\n", b2.bnum);
}
