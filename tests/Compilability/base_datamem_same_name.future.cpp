class Base {
  Base(int) {
  }
};

class Derived : public Base {
public:
  Derived(int j) : Base::Base(j), Base(j) {
  }
  int Base;
};
