#include <stdio.h>

int main() {
  try {
    throw 1;
  } catch (int e) {
    printf("errno: %d\n", e);
  }
}
