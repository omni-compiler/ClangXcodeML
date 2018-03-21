#include <stdio.h>

int g_a4i[4];
const int g_a4ci[4] = {100, 200, 300, 400};
volatile int g_a4vi[4];
const volatile int g_a4cvi[4] = {1, 2, 3, 4};

void
func_pi(int *pi) {
  for (int k = 0; k < 4; ++k) {
    printf("int *: %d\n", pi[k]);
  }
}

void
func_pci(const int *pci) {
  for (int k = 0; k < 4; ++k) {
    printf("const int *: %d\n", pci[k]);
  }
}

void
func_pvi(volatile int *pvi) {
  for (int k = 0; k < 4; ++k) {
    printf("volatile int *: %d\n", pvi[k]);
  }
}

void
func_pcvi(const volatile int *pcvi) {
  for (int k = 0; k < 4; ++k) {
    printf("const volatile int *: %d\n", pcvi[k]);
  }
}

int
main() {
  int b_a4i[4] = {2, 4, 6, 8};
  const int b_a4ci[4] = {};
  volatile int b_a4vi[4] = {1, 10, 100, 1000};
  const volatile int b_a4cvi[4] = {3, 6, 9, 12};
  func_pi(b_a4i);
  func_pci(b_a4ci);
  func_pvi(b_a4vi);
  func_pcvi(b_a4cvi);
  func_pi(g_a4i);
  func_pci(g_a4ci);
  func_pvi(g_a4vi);
  func_pcvi(g_a4cvi);
}
