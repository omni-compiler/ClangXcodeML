#include <stdio.h>

template <typename T>
struct ClassA {
  T classa_x;
};

struct ClassB : ClassA<char> {
  int classb_y;
};

int
main() {
  ClassB objb;
  objb.classa_x = 'A';
  objb.classb_y = 100;
  printf("%c %d\n", objb.classa_x, objb.classb_y);
}
