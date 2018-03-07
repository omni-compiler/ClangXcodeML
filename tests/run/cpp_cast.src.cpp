#include <stdio.h>

class ClassA {
public:
  void
  print() {
    printf("%d\n", i);
  }
  virtual void
  f() {
  }
  int i;
};

class ClassB {
public:
  void
  print() {
    printf("%lf\n", d);
  }
  double d;
};

class ClassC : public ClassA, public ClassB {
public:
  void
  print() {
    printf("%c\n", c);
  }
  char c;
};

void
print_as_c(ClassA *pa) {
  ClassC *pc = dynamic_cast<ClassC *>(pa);
  if (pc) {
    pc->print();
  } else {
    printf("pc is null");
  }
}

void
print_const(const ClassC &rcc) {
  const_cast<ClassC &>(rcc).print();
}

void
print_reinterpret_cast(short *ps) {
  int *pi = reinterpret_cast<int *>(ps);
  printf("%d\n", *pi);
}

int
main() {
  ClassC obj;
  obj.i = 100;
  obj.d = 3.14;
  obj.c = 'c';
  static_cast<ClassA>(obj).print();
  static_cast<ClassB>(obj).print();
  print_as_c(&obj);
  print_const(obj);
  int i = 100;
  print_reinterpret_cast(reinterpret_cast<short *>(&i));
}
