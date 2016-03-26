struct Base {
  virtual void vv();
};
struct Derived : Base { };

void f(Base* pb) {
  const int ci = 0;
  int* pi = const_cast<int*>(&ci);
  double d = static_cast<double>(ci);
  void* pv = reinterpret_cast<void*>(ci);
  Derived* pd = dynamic_cast<Derived*>(pb);
}
