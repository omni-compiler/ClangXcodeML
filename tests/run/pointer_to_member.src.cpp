#include <stdio.h>

struct ClassA {
  int member_i;
};

int
main() {
  int ClassA::*mpi_a = &ClassA::member_i;
}
