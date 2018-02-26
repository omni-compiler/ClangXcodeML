struct A {
public:
  struct B &r1;
};

struct B {
public:
  struct B &r2;
};
