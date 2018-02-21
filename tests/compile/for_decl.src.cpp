int
func() {
  int acc = 0;
  for (int i = 0; i < 100; ++i) {
    i += acc;
  }
  return acc;
}
