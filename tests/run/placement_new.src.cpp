#include <stddef.h>
#include <stdio.h>

struct ClassA {
  ClassA() : member_i(4) {}
  int member_i;
};

void *operator new(size_t size, int i, char c) {
  return operator new(size);
}

int
main() {
  ClassA *pv = (ClassA *)(new (100, 'c') ClassA);
  printf("%d\n", pv->member_i);
  delete pv;
}
