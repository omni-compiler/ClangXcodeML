struct A {
  int x;
  int
  f(int a) {
    return x + a;
  }
};

void
f() {
  A a = {10};
  a.f(20);
}
