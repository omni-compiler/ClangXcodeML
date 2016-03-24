class A {
  public:
  int f();
} a;

int A::f() {
  return 0;
}

int main() {
  return a.f();
}
