#include <stdint.h>
#include <stdlib.h>

int8_t foo(int8_t a, int8_t b, int8_t c, int8_t d) {
  int8_t e = c + d;
  int8_t f = a + b;

  return e + f;
}

int main(int argc, char *argv[]) {
    int a = atoi(argv[1]), b = atoi(argv[2]), c = atoi(argv[3]), d = atoi(argv[4]);
    int8_t ret = foo(a, b, c, d);
    return ret;
}
