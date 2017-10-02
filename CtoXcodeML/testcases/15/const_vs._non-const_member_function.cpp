class A {
public:
  int f();
  int f() const;

private:
  int x;
};

int
A::f() {
  x++;
  return x;
}

int
A::f() const {
  return x;
}
