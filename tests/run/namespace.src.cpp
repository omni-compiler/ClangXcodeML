#include <stdio.h>

void
func() {
  printf("A\n");
}

namespace NS1 {

void
func() {
  ::func();
  printf("B\n");
}

} // namespace NS1

int
main() {
  func();
  ::func();
  NS1::func();
}
