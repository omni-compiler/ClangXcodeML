#include <stdio.h>

class A {
public:
  int
  get() {
    return x;
  }

protected:
  int x;
};

class B : public A {
public:
  using A::x;
};

int
main() {
  B b;
  b.x = 10;
  printf("%d\n", b.get());
}
