#include <stdio.h>

class ClassA {
public:
  int member_i;
};

int
main() {
  ClassA *pa = new ClassA[10];
  for (int i = 0; i < 10; ++i) {
    pa[i].member_i = i;
  }
  for (int i = 0; i < 10; ++i) {
    printf("%d\n", pa[i].member_i);
  }
  delete[] pa;
  return 0;
}
