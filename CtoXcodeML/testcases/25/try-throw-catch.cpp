struct A {
  int x;
};

int
main() {
  try {
    A a;
    throw a;
  } catch (A a) {
    a.x = 0;
  }
  int k;
  return 0;
}
