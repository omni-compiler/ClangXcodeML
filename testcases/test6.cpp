struct A {
  int x;
  int f() {
    return 0;
  }
};

int g() {
  return 0;
}

int main() {
  struct A a;
  a.x = 0;
  return a.f();
}
