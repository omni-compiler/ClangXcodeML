#include <stdio.h>

struct ClassA {
  int member_i;
};

struct Outer {
  struct Inner {
    char member_c;
  };
};

int
main() {
  int ClassA::*mpi_a = &ClassA::member_i;
  ClassA obja;
  obja.member_i = 20;
  printf("%d\n", obja.*mpi_a);

  char ::Outer::Inner::*mpc_oi = &Outer::Inner::member_c;
  Outer::Inner objoi;
  objoi.member_c = '#';
  printf("%c\n", objoi.*mpc_oi);
}
