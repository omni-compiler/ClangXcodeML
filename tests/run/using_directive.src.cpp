#include <stdio.h>

namespace NS1 {

namespace NS2 {

void
func() {
  printf("NS1::func\n");
}

} // namespace NS2

} // namespace NS1

int
main() {
  using namespace NS1::NS2;
  func();
}
