#include <stdio.h>

struct ClassA {
  int member_i;
};

int
main() {
  int ClassA::*mpi_a = &ClassA::member_i;
  ClassA obja;
  obja.member_i = 20;
  printf("%d\n", obja.*mpi_a);
}
