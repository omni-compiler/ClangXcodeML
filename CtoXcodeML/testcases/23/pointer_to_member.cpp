struct A {
  int x, y;
  void f(){};
};

int
main() {
  int A::*p = &A::x;
  A a = {0, 1};
  void (A::*func)() = &A::f;
  (a.*p)++;
  (a.*func)();
}
