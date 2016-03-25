class {
  public:
    int f() { return 0; }
} a;

class {
  class A {
    int f() { return 0; }
  };
} b;

int main() {
  class B {
    int x;
  };
  B t;
  class C {
    class {
      int f() { return 0; }
    } x;
  };
  C u;
  return a.f();
}
