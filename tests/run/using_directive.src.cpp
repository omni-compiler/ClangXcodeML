#include <stdio.h>

namespace NS1 {

void
func() {
  printf("NS1::func\n");
}

} // namespace NS1

int
main() {
  using namespace NS1;
  func();
}
