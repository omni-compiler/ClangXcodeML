Compatibility テストおよび各テストケースの説明
----

# /XcodeMLtoCXX/tests/Compilability/ について

このテストはデコンパイラ(XcodeMLtoCXX)が合法(well-formed)なC/C++ソースコードを出力することを検証する。テストは次のコマンドで行う。

    cd XcodeMLtoCXX/tests/Compilability
    make check

テストではまず、Compilability/以下にあるテスト用ソースコード
(そのファイル名は テストの概要.src.c または テストの概要.src.cpp という形式である)
をフロントエンド(CtoXcodeML)に入力し、その出力をデコンパイラに入力する。

デコンパイラの出力はCompilability/以下に保存される。
そのファイル名はもとのファイル名の".src."を".dst."に変えたものである。

フロントエンドおよびデコンパイラが正常に終了し、なおかつデコンパイラの出力するC/C++ソースコードが合法であればテストは成功する。
そうでなければ、テストは失敗する。

フロントエンドおよびデコンパイラは意味を変えない範囲でプログラムの正規化を行うことがあるから、
もとの入力(src)と最終的な出力(dst)との単純な文字列比較や、それらの抽象構文木の比較をもって検証を行うことはできない。
このため、最終的な出力がソースコードとして正しいかどうかのみを試験している。

# C (.src.c)

## array_cv.src.c

配列型の引数に与えられたCV修飾子および要素数を扱うテスト。

    void f(const int a[const 10], double b[const volatile 20]) {
    }

## bitfield.src.c

構造体のビットフィールド宣言の名前とビット長を扱うテスト。

`: 5`のような無名ビットフィールドはunused fieldを表す。 `:0` は直後のビットフィールドが新しいメモリー境界から始まることを保証する。

    struct A {
      unsigned a : 3, b : 10, : 5, c : 10;
      unsigned :0;
      double g;
      signed d : 15, e : 17;
      signed f : 20;
      int h : 4;
    };

## fundamental_type.src.c

基本型(fundamental type)を扱うテスト。
特に、`long long`, `long long int`, `unsigned int`を正しく復元できることを確認する。

    char c;
    short s;
    int i;
    long l;
    unsigned u;
    float f;
    double d;
    long long ll;
    unsigned char uc;
    unsigned short us;
    unsigned int ui;
    unsigned long ul;
    unsigned long long ull;
    long double ld;

## several_var_in_single_decl.src.c

複数の変数を宣言する単一の宣言文を扱うテスト。

    int i, j, k,
        *pi, * const cpi, * volatile vpi, * const volatile cvpi,
        ** ppi, * const * pcpi,
        a10i[10], *a5pi[5],
        fi(),
        (*pfi)();

## struct_in_struct.src.c

別の構造体定義(`struct B`)を含む構造体の定義(`struct A`)を扱うテスト。

    struct A {
      struct B { int i; } b;
      struct B b2;
    } ;

## struct_type.src.c

単純な構造体定義を扱うテスト。

    struct A {
      int i;
    };
    struct B {
      double d;
      int i;
    };
    struct C {
      char *pc;
      double d;
      int i;
    };

# C++ (.src.cpp)

## cpp_struct.src.cpp

structキーワードで宣言されたクラスを扱うテスト。

    struct A {
      int x;
    };
    class B {
      int x;
    };

## cv_qualifier.src.cpp

引数および自動変数に与えられたCV修飾子を扱うテスト。

    void f(int i, int const ci, short * ps, short * const cps, short const * pcs, short const * const cpcs);
    void f(int i, int const ci, short * ps, short * const cps, short const * pcs, short const * const cpcs) {
      double d = 0;
      double const cd = 0;
      int * pi = &i;
      int const * pci = &ci;
      int * const cpi = pi;
      int const * const cpci = pci;
    }
    void g(int i, int volatile ci, short * ps, short * volatile cps, short volatile * pcs, short volatile * volatile cpcs) {
      double d = 0;
      double volatile cd = 0;
      int * pi = &i;
      int volatile * pci = &ci;
      int * volatile cpi = pi;
      int volatile * volatile cpci = pci;
    }
    void h(int i, int const volatile ci, short * ps, short * const volatile cps, short const volatile * pcs, short const volatile * const volatile cpcs) {
      double d = 0;
      double const volatile cd = 0;
      int * pi = &i;
      int const volatile * pci = &ci;
      int * const volatile cpi = pi;
      int const volatile * const volatile cpci = pci;
    }

## empty_do.src.cpp

本体が空のdo文を扱うテスト。

    void f() {
      do {
      } while (0);
    }

## empty_for.src.cpp

本体が空のfor文を扱うテスト。

    void f() {
      for (;;) { }
    }

## empty_while.src.cpp

本体が空のwhile文を扱うテスト。

    void f() {
      while(1) { }
    }

## for.src.cpp

本体が空でない繰り返し構文(for, while, do)を扱うテスト。

    void f() {
      int i = 0;
      for (i = 0; i - 10; i = i + 1) {
        int num = 1;
      }
      while (1 == 2) {
        return;
      }
      do {
        i = i - 1;
      } while (i);
    }
    int square(int x) {
      return x * x;
    }

## function_decl.src.cpp

関数のプロトタイプ宣言を扱うテスト。

    int f(int, int);

## function_param_same_name.src.cpp

関数名と同じ名前の引数をもつ関数定義を(特に型について)正しく復元できることを確認するテスト。

    int f(int f) {
      return f;
    }

## int_return.src.cpp

値を返すreturn文が正しく復元されることを確認するテスト。

    int return_the_answer() {
      return 42;
    }

## local_variable_declaration.src.cpp

自動変数の宣言を扱うテスト。

    void f() {
      int i = 1;
    }

## multi_dim_array.src.cpp

多重配列を扱うテスト。

    int b[1][2][3][4][5];

## params_of_function_declaration.src.cpp

引数名を含む関数プロトタイプ宣言を扱うテスト。

    void f();
    int g(int i);
    int h(double d, int i);
    double i(double d1, int, int i, double d2, short s);

## params_of_function_definition.src.cpp

引数名を含む関数プロトタイプ宣言を(特に `i` という名前で宣言された引数の型について)正しく復元できることを確認するテスト。

    void f() {
    }
    
    int g(int i) {
      return i;
    }
    
    int h(double d, int i) {
      return i;
    }
    
    double i(double d1, int, int i, double d2, short s) {
      return d2;
    }

## pointer_to_function.src.cpp

関数ポインターの宣言を扱うテスト。

    void declare_pointer() {
      void (*f)(int, int);
      void (*fp)(int (*)(int, int));
    }

## pointer_type.src.cpp

ポインターの宣言を扱うテスト。

    void declare_pointers() {
      int n;
      int *a = &n;
      double **b;
      const int *c;
      int * const d = 0;
      const int * const e = 0;
      void (*f)(int, int);
    }

## simple_class.src.cpp

単純なクラス定義を扱うテスト。

    class A {
      public:
        int x;
        int y;
        double dist();
      private:
        int pv;
    };
    
## variables_with_the_same_name_in_different_scopes.src.cpp

異なるスコープを持つ同じ名前を正しく復元できることを確認するテスト。

    void f() {
      int i = 0;
      {
        long i = 0;
      }
    }
    void g() {
      int g = 0;
    }
    void h(int g) {
    }

## void_return.src.cpp

空のreturn文を扱うテスト。

    void f() {
      return;
    }

## while.src.cpp

while文を扱うテスト。

    void f() {
      while (1) {
        int i = 1;
      }
    }
