#include <stddef.h>
#include <stdio.h>

struct ClassA {
  int member_i;
};

void *operator new(size_t, int i, char c) {
  ClassA *pa = new ClassA;
  pa->member_i = i * i;
  return pa;
}

int
main() {
  ClassA *pv = (ClassA *)(new (100, 'c') ClassA);
  printf("%d\n", pv->member_i);
  delete pv;
}
