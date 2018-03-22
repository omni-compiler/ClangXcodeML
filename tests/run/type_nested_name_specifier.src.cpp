#include <stdio.h>

namespace NS1 {

class ClassA {
public:
  ClassA(int i, int j) {
    x = i;
    y = j;
  }
  int x;
  int y;
};

} // namespace NS1

int
main() {
  NS1::ClassA obj(20, 30);
  printf("%d, %d\n", obj.x, obj.y);
}
