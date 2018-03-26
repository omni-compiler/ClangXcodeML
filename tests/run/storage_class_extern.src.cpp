#include <stdio.h>

int global_variable;

int
func() {
  return global_variable;
}

int
main() {
  extern int global_variable;
  global_variable = 100;
  printf("%d\n", func());
}
