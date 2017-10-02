namespace N {
class A {
  int x, y;

public:
  int
  getX() {
    return x;
  }
  int getY();

protected:
  class B {
    int x;
    int operator*() {
      return x;
    }
  };
  B b;
};

namespace M {
class C {
  int x;
};
int
f() {
  return 42;
}
}
}

int
N::A::getY() {
  return y;
}
