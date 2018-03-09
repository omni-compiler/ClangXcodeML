#include <stdio.h>

class ClassA {
public:
  ClassA(int i, int j) : num(i * j) {
  }
  int num;
};

int
main() {
  ClassA a(10, 20);
  printf("%d\n", a.num);
}
