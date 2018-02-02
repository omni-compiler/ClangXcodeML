template <typename T>
T
id(T x) {
  return x;
}

int
main() {
  return id('0') + id(1);
}
