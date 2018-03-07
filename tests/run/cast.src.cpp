#include <stdio.h>

void
print_as_pint(void *pv) {
  int *pi = (int *)pv;
  printf("%d\n", *pi);
}

int
main() {
  int i = 100;
  print_as_pint(&i);
}
