struct A {
  static const int foo = 42;
  static const int bar = 45;
};

void
func() {
  int i = A::foo + A::bar;
}
