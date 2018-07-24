#include <stdio.h>

namespace NS1 {

class ClassA {
public:
  ClassA() {
    x = 42;
  }
  ClassA(int i, int j) {
    x = i * j;
  }
  ClassA(int y) {
    x = y;
  }
  int x;
};

} // namespace NS1

int
main() {
  NS1::ClassA obj = NS1::ClassA(1, 2);
  obj = NS1::ClassA(10, 20);
  printf("%d\n", obj.x);

  obj = NS1::ClassA(100);
  printf("%d\n", obj.x);

  obj = NS1::ClassA();
  printf("%d\n", obj.x);
}
