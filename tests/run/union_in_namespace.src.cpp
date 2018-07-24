#include <stdio.h>

namespace NS1 {

union ClassA {
  int member_i1;
  int member_i2;
};

} // namespace NS1

int
main() {
  NS1::ClassA obja;
  obja.member_i1 = 100;
  printf("%d\n", obja.member_i2);
}
