void
declare_pointer() {
  void (*f)(int, int);
  void (*fp)(int (*)(int, int));
}
