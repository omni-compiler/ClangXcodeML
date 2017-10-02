struct A {
  int content;
};

A operator+=(A a, int x) {
  a.content += x;
  return a;
}

void
f() {
  A a = {0};
  a += 3;
}
