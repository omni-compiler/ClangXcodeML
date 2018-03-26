#include <stdio.h>

namespace {

class StructA {
public:
  int member_i;
};

StructA obja;

} // namespace

int
main() {
  obja.member_i = 10;
  printf("%d\n", obja.member_i);
}
