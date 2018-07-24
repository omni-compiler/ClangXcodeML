#include <stdio.h>

class A {
public:
  int x;
  int
  get() {
    return x;
  }
};

class B : public A {
private:
  int x;
};

int
main() {
  B b;
  b.A::x = 10;
  printf("%d\n", b.get());
}
