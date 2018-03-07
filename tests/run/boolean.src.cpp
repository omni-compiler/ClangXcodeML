#include <stdio.h>

void
print_boolean(bool b) {
  if (b) {
    printf("TRUE\n");
  } else {
    printf("FALSE\n");
  }
}

int
main() {
  bool b = true;
  print_boolean(b);
  b = false;
  print_boolean(b);
}
