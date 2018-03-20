class A {
public:
  explicit A(int, int) {
  }
};

int
main() {
  A *pi = new A(0, 0);
  delete pi;
  return 0;
}
