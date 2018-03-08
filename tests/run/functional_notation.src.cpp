#include <stdio.h>

class ClassA {
public:
  ClassA() {
    x = 42;
  }
  ClassA(int i, int j) {
    x = i * j;
  }
  ClassA(int y) {
    x = y;
  }
  int x;
};

typedef void *PtrT;

void
print_as_pint(PtrT p) {
  printf("%d\n", *(int *)p);
}

int
main() {
  int i = 42;
  print_as_pint(PtrT(&i));

  printf("%d\n", unsigned(-1));

  ClassA obj = ClassA(1, 2);
  obj = ClassA(10, 20);
  printf("%d\n", obj.x);

  obj = ClassA(100);
  printf("%d\n", obj.x);

  obj = ClassA();
  printf("%d\n", obj.x);
}
