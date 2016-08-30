void declare_pointers() {
  int n;
  int *a = &n;
  double **b;
  const int *c;
  int * const d = 0;
  const int * const e = 0;
  void (*f)(int, int);
}
