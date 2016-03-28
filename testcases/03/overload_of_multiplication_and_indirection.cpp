class A {
  public:
    A(int v): value(v) {}
    int operator*() {
      return value;
    }
    A operator*(A other);
  private:
    int value;
};

A operator*(int, A);

A operator*(int k, A x) {
  return A(k * *x);
}
