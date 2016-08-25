class A {
  public:
    A(int a, int b):
      x(a * b),
      y(a + b),
      z(0) {}
  private:
    int x, y, z;
};

class B {
  public:
    B(int a, int b);
  private:
    int x, y, z;
};

B::B(int a, int b):
  x(a * b),
  y(a + b),
  z(0)
{}

void f() {
  A a(42, 54);
}
