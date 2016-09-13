void f(int i, int const ci, short * ps, short * const cps, short const * pcs, short const * const cpcs);
void f(int i, int const ci, short * ps, short * const cps, short const * pcs, short const * const cpcs) {
  double d = 0;
  double const cd = 0;
  int * pi = &i;
  int const * pci = &ci;
  int * const cpi = pi;
  int const * const cpci = pci;
}

void g(int i, int volatile ci, short * ps, short * volatile cps, short volatile * pcs, short volatile * volatile cpcs) {
  double d = 0;
  double volatile cd = 0;
  int * pi = &i;
  int volatile * pci = &ci;
  int * volatile cpi = pi;
  int volatile * volatile cpci = pci;
}

void h(int i, int const volatile ci, short * ps, short * const volatile cps, short const volatile * pcs, short const volatile * const volatile cpcs) {
  double d = 0;
  double const volatile cd = 0;
  int * pi = &i;
  int const volatile * pci = &ci;
  int * const volatile cpi = pi;
  int const volatile * const volatile cpci = pci;
}
