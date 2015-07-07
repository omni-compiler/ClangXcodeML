typedef struct complex {
  double real;
  double img;
} complex_t;
typedef char *charptr;

complex_t x __attribute__((aligned(64), unused));
complex_t complex_add(complex_t x, double y);

main()
{
  complex_t z;

  x.real = 1.0;
  x.img = 2.0;

  z = complex_add(x,1.0);

  printf("z=<%f,%f>\n",z.real,z.img);
}

typedef struct dummy {
  int dummy_int;
  unsigned dummy_unsigned;
  double dummy_double;
} dummy_t;

complex_t complex_add(complex_t x, double y)
{
  x.real += y;
  return x;
}

int iftest(int x, int y)
{
  for (int i = 0; i < 10; i++) {
    if (x < y) {
      return (x + y) * 3;
    } else {
      return y * x + 4;
    }
  }
}

double get_real(complex_t *p)
{
  return p->real;
}

int compoundstmtdecls(int x)
{
  int i = x + 1;
  printf("%d\n", i);
  int j = x + 2;
  printf("%d\n", j);
  int k = i + j;
  return ({static int tmp[10]; tmp[k % 10] = i; tmp[j];});
}

enum color {
  black, brown, red, orange, yellow, green, blue, purple, grey, white
};
  
union aho {
  int i[2];
  double d;
  enum color c;
};

union aho baka[10];

int gototest(int x)
{
  void *p = &&hunya;
  if (!x) {
    goto end;
  }
  return x * x;
  ;
  goto *p;
  ;
  ;
 end:
  return 1024;
 hoge:
 moge: return -1;
 hunya:;
}
