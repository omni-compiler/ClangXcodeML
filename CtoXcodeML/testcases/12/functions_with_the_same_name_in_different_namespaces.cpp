namespace {
int
f() {
  return 42;
}
}

namespace A {
int
f() {
  return 52;
}
}

inline namespace B {
int
f() {
  return 62;
}
}

namespace {
namespace {
int
f() {
  return 72;
}
}
}

int
f() {
  return 82;
}
