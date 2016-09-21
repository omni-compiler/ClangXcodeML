class A {
public:
  A(int);
  int get();
private:
  int val;
  double d;
};

A::A(int x):
  val(x),
  d(x)
{}

int A::get() {
  return val * d;
}
