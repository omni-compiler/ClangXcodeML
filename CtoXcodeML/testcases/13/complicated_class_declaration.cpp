class A {
  A() = default;
  A(A &a) = delete;
  virtual int f() = 0;
  void
  g() {
    return;
  }

protected:
  const int x;
  static int a;
  const static int b = 42;
};

class B final : public A {
  int f() override;
};

int
B::f() {
  return x;
}

int A::a = 1;
