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

namespace NS2 {

void
func() {
  printf("C\n");
}

} // namespace NS2

} // namespace NS1

int
main() {
  func();
  ::func();
  NS1::func();
  ::NS1::NS2::func();
}
