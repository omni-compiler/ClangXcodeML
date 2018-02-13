class A {};

class B {
  A func();
};

A
B::func() {
  return A();
}
