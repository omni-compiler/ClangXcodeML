class A {
  int value;
  public:
  int operator*() {
    return value;
  }
  A operator*(A other);
};

A operator*(int k, A x);
