#include <stdio.h>

class ClassA {
public:
  ClassA(int i, int j) : num(i * j) {
  }
  ClassA(int i, int j, int k);
  int num;
};

ClassA::ClassA(int i, int j, int k) : num(i * j * k) {
}

int
main() {
  ClassA a1(10, 20);
  printf("%d\n", a1.num);
  ClassA a2(10, 20, 30);
  printf("%d\n", a2.num);
}
