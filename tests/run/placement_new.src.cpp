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
  ClassA *pv1 = (ClassA *)(new (100, 'c') ClassA);
  ClassA *pv2 = (ClassA *)(new (100, 'c') ClassA());
  printf("%d %d\n", pv1->member_i, pv2->member_i);
  delete pv1;
  delete pv2;
}
