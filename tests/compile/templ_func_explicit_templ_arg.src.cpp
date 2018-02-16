template <typename T>
int
func(T *ptr) {
  return 0;
}

int
main() {
  int i = func<void>(0);
}
