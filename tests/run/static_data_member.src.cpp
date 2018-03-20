#include <stdio.h>

class ClassA {
public:
  static int static_x;
  static void
  static_memfn() {
    printf("%d\n", static_x);
  }
};

int ClassA::static_x;

int
main() {
  ClassA::static_x = 10;
  ClassA::static_memfn();
  ClassA::static_x = 20;
  ClassA::static_memfn();
}
