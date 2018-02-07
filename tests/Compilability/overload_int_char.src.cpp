int
func(int) {
  return 100;
}

int
func(char) {
  return 200;
}

int
main() {
  char c = 'A';
  if (func(c) != 200) {
    return 1;
  }
  return 0;
}
