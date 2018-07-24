#include <stdio.h>

class Outer {
public:
  class Inner {
  public:
    static int member_i;
  };
};

int Outer::Inner::member_i;

int
main() {
  Outer::Inner::member_i = 20;
  printf("%d\n", Outer::Inner::member_i);
}
