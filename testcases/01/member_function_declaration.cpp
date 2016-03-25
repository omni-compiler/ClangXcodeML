class A {
  void f(int x, int y);
  int operator* () { return 42; }
};

void A::f(int x, int y) {
  return;
};
