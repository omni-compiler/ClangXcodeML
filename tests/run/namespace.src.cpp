#include <stdio.h>

namespace NS1 {

void
func() {
  printf("OK\n");
}

} // namespace NS1

void
func() {
  printf("NG\n");
}

int
main() {
  NS1::func();
}
