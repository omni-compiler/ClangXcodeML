#include <stddef.h>
#include <stdio.h>

void
print_as_pint(void *pv) {
  int *pi = (int *)pv;
  printf("%d\n", *pi);
}

void
print_as_string(size_t address) {
  const char *s = (const char*)address;
  printf("%s\n", s);
}

int
main() {
  int i = 100;
  print_as_pint(&i);
  const char *s = "const char *";
  print_as_string((size_t)s);
}
