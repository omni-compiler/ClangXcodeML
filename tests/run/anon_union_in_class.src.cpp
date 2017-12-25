#include <stdio.h>

class ClassA {
public:
  union {
    int member_x;
    int member_y;
  };
  int member_z;
};

int
main() {
  ClassA obj;
  obj.member_z = 1000;
  obj.member_x = 100;
  obj.member_y = 42;
  printf("%d\n", obj.member_z + obj.member_x);
}
