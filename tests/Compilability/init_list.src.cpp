class K {
public:
  K(int, int) {
  }
};

int
main() {
  K k(2, 3);
  K arrk[3] = {K(1, 2), K(3, 4), k};
  return 0;
}
