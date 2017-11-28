% ClangXMLの要素
% XcalableMP/Omni Compiler Project
%

この文書はClangXML形式の構文と意味内容を定義する。
ClangXML形式とは、C++プログラムをXMLで表現するための形式である。

# ClangXML文書の構造

ClangXML文書は次の構造に従う。

| `<Program>`
|   `<clangAST>`
|     `<clangDecl class="TranslationUnit">`
|       `<xcodemlTypeTable>`
|         _データ型定義要素_ ...
|       `</xcodemlTypeTable>`
|       `<xcodemlNnsTable>`
|         _NNS定義要素_ ...
|       `</xcodemlNnsTable>`
|       _C++プログラムを表現する`clangDecl`要素_ ...
|     `</clangDecl>`
|   `</clangAST>`
| `</Program>`

ひとつのClangXML文書は、C++のひとつの翻訳単位をXML文書として表現する。

ClangXML文書のルート要素は`Program`要素である。
`Program`要素はただひとつの子要素として`clangAST`要素をもつ。
`clangAST`要素はただひとつの子要素として`clangDecl`(`TranslationUnit`)要素をもつ。

ClangXML文書は、
C++プログラム中で使用される型(データ型)をデータ型識別名とデータ型定義要素によって表現する。
データ型識別名はデータ型に与えられる名前である。
データ型定義要素はデータ型識別名とそれが指示するデータ型の内容を定義する。

# 型

## データ型識別名とデータ型定義要素

*データ型識別名*はデータ型に与えられる名前で、アルファベット、数字、およびアンダースコア(`_`)
からなる有限長文字列である。
データ型識別名には、
予約データ型識別名とユーザ定義されたデータ型識別名と2種類がある。
ひとつのClangXML文書中で、ひとつのデータ型識別名はただひとつの同じデータ型を指示する。
ひとつのClangXML文書中で、同じデータ型に複数のデータ型識別名を与えてもよい。

*予約データ型識別名*はデータ型識別名の一種である。
予約データ型識別名は次のいずれかである。

* `bool`
* `char`
* `char16_t`
* `char32_t`
* `double`
* `double_complex`
* `double_imaginary`
* `float`
* `float_complex`
* `float_imaginary`
* `int`
* `long`
* `long_double`
* `long_double_complex`
* `long_double_imaginary`
* `long_long`
* `short`
* `unsigned`
* `unsigned_char`
* `unsigned_long`
* `unsigned_long_long`
* `unsigned_short`
* `void`
* `wchar_t`
* `__builtin_va_arg`

それ以外のデータ型識別名はユーザ定義されたデータ型識別名とよばれる。
*ユーザ定義されたデータ型識別名*は、
アルファベットの大文字で始まる。
また、連続したアンダースコアを含んではならない。


*データ型定義要素*は、`xcodemlTypeTable`要素の直接の子要素であって、
ユーザ定義されたデータ型識別名とそれが指示するデータ型を対応づける。
ひとつのデータ型識別名を複数のデータ型定義要素で定義してはならない。

## 組み込み・修飾型(`basicType`要素)

## クラス型・構造体型

### `classType`要素

| `<classType`
|   `cxx_class_kind` `=` `"class"` | `"struct"` | `"union"`
|   `is_anonymous` `=` `"true"` | `"false"` | `"1"` | `"0"`
|   `type` `=` _ユーザ定義されたデータ型識別名_
|   `>`
|   _`inheritedFrom`要素_
|   _`symbols`要素_
| `</classType>`

`classType`要素はクラス型を表現する。

第1子要素は`inheritedFrom`要素で、このクラスの派生元クラスのリストを表現する。
このクラスが派生クラスでない場合、`inheritedFrom`要素は子要素をもたない。
このクラスが派生クラスである場合、
`inheritedFrom`要素は、派生元クラスを表す`typeName`要素を1個以上子要素にもつ。
`typeName`要素の順番は、派生クラスのリストの順番と等しい。

第2子要素は`symbols`要素で、このクラスのメンバーのリストを表現する。
`symbols`要素は、メンバー名を表す`id`要素を0個以上子要素にもつ。

この要素は、必須属性として`type`属性をもつ。

`type`属性の値はユーザ定義されたデータ型識別名であり、
この要素によって定義されるクラス型に与えられるデータ型識別名を表す。

この要素は、オプションで`cxx_class_kind`属性、`is_anonymous`属性を利用できる。

`cxx_class_kind`属性の値は`"class"`,`"struct"`, `"union"`のいずれかであり、
C++プログラム中でこのクラスを宣言するのに使われたキーワードを表す。

`is_anonymous`属性の値は`"true"`, `"false"`, `"1"`, `"0"`のいずれかであり、
`"true"`または`"1"`のとき無名クラスであることを表す。

### `structType`要素

### `unionType`要素

## `arrayType`要素

## `complexType`要素

## `functionType`要素

## `pointerType`要素

## `typedefType`要素

## `vectorType`要素

## その他の型(`otherType`要素)

# NNS

# `Program`要素

| `<Program`
|   `source` `=` _パス名_
|   `language=` `"C++"` | `"C"`
|   `time` `=` _時刻_
|   `>`
|   _`clangAST`要素_
| `</Program>`

ClangXMLのルート要素は`Program`要素である。

# `xcodemlTypeTable`要素

# `xcodemlNnsTable`要素

# `clangAST`要素

| `<clangAST>`
|   _`clangDecl`要素_
| `</clangAST>`

`clangDecl`要素の`class`属性の値は`"TranslationUnit"`でなければならない。

# `clangDecl`要素

| `<clangDecl`
|   `class` `=` _宣言の種類(後述)_
| `>`
| _子要素_ ...
| `</clangDecl>`

`clangDecl`要素はC/C++の宣言を表現する。

この要素は、必須属性として`class`属性をもつ。

`class`属性の値は文字列であり、宣言の種類を表す。

*宣言の種類*は、
[`clang::Decl::Kind`](https://clang.llvm.org/doxygen/classclang_1_1Decl.html)
を表す文字列である。
以下に主要な宣言の種類を挙げる。

| 宣言の種類           | `clang::Decl::Kind`の値 | 意味                   |
|----------------------|-------------------------|------------------------|
| `"CXXConstructor"`   | `CXXConstructor`        | コンストラクター宣言      |
| `"Function"`         | `Function`              | 関数宣言               |
| `"LinkageSpec"`      | `LinkageSpec`           | リンケージ指定          |
| `"ParmVar"`          | `ParmVar`               | 仮引数                 |
| `"TranslationUnit"`  | `TranslationUnit`       | 翻訳単位               |
| `"Typedef"`          | `Typedef`               | `typedef`宣言          |

## `CXXConstructor`: コンストラクター宣言

| `<clangDecl class="CXXConstructor"`
|    `is_implicit=` `"true"`  | `"false"` | `"1"` | `"0"`
|  `>`
|   _`name`要素_
|   _`TypeLoc`要素_
|   _`clangConstructorInitializer`要素_...
|   _`clangStmt`要素_
| `</clangDecl>`

`CXXConstructor`はコンストラクター定義を表現する。

name子要素は関数名を表現する。
これは`name_kind`属性を持ち、その値は`constructor`である。

params要素は仮引数リストを表現する。

`clangConstructorInitializer`子要素は初期化リストを表現する。

clangStmt子要素は関数本体を表現する。
これは`CompoundStmt`または`tryStmt`である。

この要素は、オプションで`is_implicit`属性を利用できる。
`is_implicit`属性の値は`"true"`, `"false"`, `"1"`, `"0"`のいずれかであり、
`"true"`または`"1"`のとき関数が暗黙に定義されたことを表す。

## `Function`: 関数宣言

| `<clangDecl class="Function"`
|    `is_implicit` = `"true"` | `"false"` | `"1"` | `"0"`
|  `>`
|   _`name`要素_
|   _`params`要素_
|   _`clangStmt`要素_
| `</clangDecl>`

`Function`は関数定義を表現する。

第1子要素は関数名を表現する。

第2子要素は仮引数リストを表現する。

第3子要素は関数本体を表現する。
これは`CompoundStmt`または`tryStmt`である。

この要素は、オプションで`is_implicit`属性を利用できる。
`is_implicit`属性の値は`"true"`, `"false"`, `"1"`, `"0"`のいずれかであり、
`"true"`または`"1"`のとき関数が暗黙に定義されたことを表す。

## `LinkageSpec`: リンケージ指定


## `ParmVar`: 仮引数

| `<clangDecl class="ParmVar"`
| `has_init` `=` `"true"` | `"false"` | `"1"` | "`0`"
| `xcodemlType` `=` _データ型識別名_
| `>`
|   _`name`要素_
|   _`TypeLoc`要素_
|   [ _`clangStmt`要素_ ]
| `</clangDecl>`

`ParmVar`は関数宣言中の仮引数の宣言を表現する。

第1子要素は`name`要素で、引数名を表現する。

第2子要素は`TypeLoc`要素である。

第3子要素は`clangStmt`

## `TranslationUnit`: 翻訳単位

| `<clangDecl class="TranslationUnit"`
|   _`xcodemlTypeTable`要素_
|   _`xcodemlNnsTable`要素_
|   _`clangDecl`要素_ ...
| `>`


## `Typedef`: `typedef`宣言

| `<clangDecl class="Typedef"`
| `xcodemlTypedefType` `=` _データ型識別名_
| `>`
|   _`name`要素_
| `</clangDecl>`

`Typedef`は`typedef`宣言を表現する。

第1子要素は`name`要素で、`typedef`名を表現する。

この要素は必須属性として`xcodemlTypedefType`属性をもつ。
`xcodemlTypedefType`属性の値はデータ型識別名であり、
`typedef`名が表す型を表現する。

# `clangStmt`要素

| `<clangStmt`
|   `class` `=` _属性_
| `>`
| _子要素_ ...
| `</clangStmt>`

`clangStmt`要素は
Clang の `clang::Stmt` クラスから派生したクラスのデータを表す要素であり、
式や文を表現する。
`class`属性の値で具体的なクラス名を表す。
以下では逆変換に用いる部分について個別に解説する。
その他については Clang の実装を参照のこと。


## `BinaryOperator`: 二項演算

| `<clangStmt class="BinaryOperator"`
| `binOpName` `=` _二項演算名(後述)_
| `xcodemlType` `=` _データ型識別名_
| `>`
|   _`clangStmt`要素_
|   _`clangStmt`要素_
| `</clangStmt>`

`BinaryOperator`は(複合代入演算でない)二項演算を表現する。
複合代入演算は`CompoundAssignOperator`によって表現する。

第1、 第2子要素は`clangStmt`要素で、それぞれ二項演算の左辺式、右辺式を表現する。

この要素は、必須属性として`binOpName`属性をもつ。

`binOpName`属性の値は文字列で、二項演算名を表す。

*二項演算名*は、`clang::BinaryOperatorKind`を表す文字列である。
以下に主要な二項演算名を挙げる。

| 二項演算名            | `clang::BinaryOperatorKind`の値 | 意味                                           |
|-----------------------|---------------------------------|------------------------------------------------|
| `"memberPointerRef"`  | `BO_PtrMemD`                    | メンバーポインターによるメンバーアクセス `.*`  |
| `"memberIndirectRef"` | `BO_PtrMemI`                    | メンバーポインターによるメンバーアクセス `->*  |
| `"mulExpr"`           | `BO_Mul`                        | 乗算 `*`                                       |
| `"divExpr"`           | `BO_Div`                        | 除算 `/`                                       |
| `"modExpr"`           | `BO_Rem`                        | 剰余 `%`                                       |
| `"plusExpr"`          | `BO_Add`                        | 加算 `+`                                       |
| `"minusExpr"`         | `BO_Sub`                        | 減算 `-`                                       |
| `"LshiftExpr"`        | `BO_Shl`                        | 左シフト `<<`                                  |
| `"RshiftExpr"`        | `BO_Shr`                        | 右シフト `>>`                                  |
| `"logLTExpr"`         | `BO_LT`                         | 大小比較 `<`                                   |
| `"logGTExpr"`         | `BO_GT`                         | 大小比較 `>`                                   |
| `"logLEExpr"`         | `BO_LE`                         | 大小比較 `<=`                                  |
| `"logGEExpr"`         | `BO_GE`                         | 大小比較 `>=`                                  |
| `"logEQExpr"`         | `BO_EQ`                         | 等価比較 `==`                                  |
| `"logNEQExpr"`        | `BO_NE`                         | 等価比較 `!=`                                  |
| `"bitAndExpr"`        | `BO_And`                        | ビットAND `&`                                  |
| `"bitXorExpr"`        | `BO_Xor`                        | ビットXOR `^`                                  |
| `"bitOrExpr"`         | `BO_Or`                         | ビットOR `|`                                   |
| `"logAndExpr"`        | `BO_LAnd`                       | 論理積 `&&`                                    |
| `"logOrExpr"`         | `BO_LOr`                        | 論理和 `||`                                    |
| `"assignExpr"`        | `BO_Assign`                     | 代入 `=`                                       |
| `"commaExpr"`         | `BO_Comma`                      | カンマ演算 `,`                                 |


## `CaseStmt`: caseラベル

| `<clangStmt class="CaseStmt">`
|   _`clangStmt`要素_
|   _`clangStmt`要素_
| `</clangStmt>`

`CaseStmt`はcaseラベルを表現する。

第1子要素は式。

第2要素はcaseラベルに引き続く文。

## `CompoundAssignOperator`: 複合代入演算

| `<clangStmt class="CompoundAssignOperator"`
|   `binOpName` `=` _二項演算名(後述)_
|   `xcodemlType` `=` _データ型識別名_
| `>`
|   _`clangStmt`要素_
|   _`clangStmt`要素_
| `</clangStmt>`

`CompoundAssignOperator`は複合代入演算を表現する。

第1、第2子要素はともに`clangStmt`要素で、
それぞれ複合代入演算の左辺、右辺を表現する。

この要素は、必須属性として`binOpName`属性をもつ。

`binOpName`属性の値は文字列で、二項演算名を表す。

*二項演算名*は、`clang::BinaryOperatorKind`を表す文字列である。
以下に主要な二項演算名を挙げる。

| 二項演算名         | `clang::BinaryOperatorKind`の値 | 意味                         |
|--------------------|---------------------------------|------------------------------|
| `"asgMulExpr"`     | `BO_MulAssign`                  | 乗算の複合代入演算 `*=`      |
| `"asgDivExpr"`     | `BO_DivAssign`                  | 除算の複合代入演算 `/=`      |
| `"asgModExpr"`     | `BO_RemAssign`                  | 剰余の複合代入演算 `%=`      |
| `"asgPlusExpr"`    | `BO_AddAssign`                  | 加算の複合代入演算 `+=`      |
| `"asgMinusExpr"`   | `BO_SubAssign`                  | 減算の複合代入演算 `-=`      |
| `"asgLshiftExpr"`  | `BO_ShlAssign`                  | 左シフトの複合代入演算 `<<=` |
| `"asgRshiftExpr"`  | `BO_ShrAssign`                  | 右シフトの複合代入演算 `>>=` |
| `"asnBitAndExpr"`  | `BO_AndAssign`                  | ビットANDの複合代入演算 `&=` |
| `"asgBitOrExpr"`   | `BO_OrAssign`                   | ビットORの複合代入演算 `|=`  |
| `"asgXorExpr"`     | `BO_XorAssign`                  | ビットXORの複合代入演算 `^=` |

## `CompoundStmt`: 複合文

| `<clangStmt class="CompoundStmt">`
|   _`clangStmt`要素_ ...
| `</clangStmt>`

`CompoundStmt`は複合文を表現する。

この要素は0個以上の`clangStmt`要素を子要素にもつ。
各`clangStmt`要素は複合文に含まれる各文を表現する。
`clangStmt`要素の順序は文の順序と一致しなくてはならない。


## `ConditionalOperator`: 条件演算式

| `<clangStmt class="ConditionalOperator"`
|   `xcodemlType` `=` _データ型識別名_
| `>`
|   _`clangStmt`要素_
|   _`clangStmt`要素_
|   _`clangStmt`要素_
| `</clangStmt>`

`ConditionalOperator`は条件演算式(`E1 ? E2 : E3`)を表現する。

第1、 第2、第3子要素は`clangStmt`要素で、
それぞれ条件演算式の第1、第2、第3オペランドを表す。

この要素は、オプションで`xcodemlType`属性を利用できる。

`xcodemlType`属性の値はデータ型識別名で、条件演算式全体の型を表す。
逆変換では使用しない。


## `CXXMemberCallExpr`: メンバー関数呼び出し

| `<clangStmt class="CXXMemberCallExpr"`
|   `xcodemlType` `=` _データ型識別名_
| `>`
|   _`clangStmt`要素_
|   _`clangStmt`要素_ ...
| `</clangStmt>`

`CXXMemberCallExpr`はメンバー関数呼び出し式を表現する。

第1子要素は`clangStmt`要素で、呼び出される非`static`メンバー関数を表す。
この要素の`class`属性の値は`"MemberExpr"`でなければならない。

第2子要素以降の子要素は`clangStmt`要素で、
メンバー関数に渡される実引数リストを表す。
この要素は0個以上ある。
`clangStmt`要素の順序と実引数の順序は一致しなくてはならない。


## `CXXThisExpr`: `this`ポインター

| `<clangStmt class="CXXThisExpr"`
|   `xcodemlType` `=` _データ型識別名_
| `/>`

`CXXThisExpr`は`this`ポインターを表現する。

この要素は子要素をもたない。

この要素は、オプションで`xcodemlType`属性を利用できる。

`xcodemlType`属性の値はデータ型識別名であり、`this`ポインターの型を表す。
逆変換では使用しない。

## `DeclRefExpr`: 変数参照

| `<clangStmt class="DeclRefExpr"`
|   `xcodemlType` `=` _データ型識別名_
| `>`
|   _`name`要素_
|   _`clangDeclarationNameInfo`要素_
| `</clangStmt>`

`DeclRefExpr`は変数参照を表現する。

第1子要素は`name`要素で、変数名を表現する。

第2子要素は`clangDeclarationNameInfo`要素で、逆変換では使用しない。

この要素は、オプションで`xcodemlType`属性を利用できる。

`xcodemlType`属性の値はデータ型識別名で、変数の型を表現する。
逆変換では使用しない。


## `DeclStmt`: 宣言文

| `<clangStmt class="DeclStmt">`
|   _`clangDecl`要素_
| `</clangStmt>`

`DeclStmt`は宣言文を表現する。

第1子要素は`clangDecl`要素で、その宣言を表す。

## `ImplicitCastExpr`: 暗黙の型変換

| `<clangStmt class="ImplicitCastExpr"`
|   `xcodemlType` `=` _データ型識別名_
|   `clangCastKind` `=` _型変換の種類(後述)_
|  `>`
|  _`clangStmt`要素_
| `</clangStmt>`

`ImplicitCastExpr`は暗黙の型変換を表現する。

第1子要素は`clangStmt`要素で、型変換の対象になる式を表す。

この要素は、オプションで`xcodemlType`属性、`clangCastKind`属性を利用できる。

`xcodemlType`属性の値はデータ型識別名で、型変換後のデータ型を表す。
逆変換では用いない。

`clangCastKind`属性の値は文字列で、キャストの種類を表す。
逆変換では用いない。

*型変換の種類*は、
[`clang::CastKind`](https://clang.llvm.org/doxygen/classclang_1_1CastExpr.html)
を表す文字列である。
以下に主要な型変換の種類を挙げる。

| 型変換の種類               | `clang::CastKind`の値       | 意味                                      |
|----------------------------|-----------------------------|-------------------------------------------|
| `"NoOp"`                   | `CK_NoOp`                   | 何もしないか、または修飾子を付け加える        |
| `"ArrayToPointerDecay"`    | `CK_ArrayToPointerDecay`    | 配列からポインターへの型変換([conv.array]) |
| `"FunctionToPointerDecay"` | `CK_FunctionToPointerDecay` | 関数からポインターへの型変換([conv.func])  |
| `"LValueToRValue"`         | `CK_LValueToRValue`         | lvalueからrvalueへの型変換([conv.lval])   |


## `IntegerLiteral`: 整数リテラル

| `<clangStmt class="IntegerLiteral"`
|   `token` `=` _文字列_
|   `decimalNotation` `=` _文字列_
|   `xcodemlType` `=` _データ型識別名_
| `/>`

`IntegerLiteral`は整数リテラルを表現する。

この要素は子要素をもたない。

この要素は、必須属性として`token`属性をもつ。

`token`属性の値は文字列で、(接尾辞を含んだ)リテラルを表す。

この要素は、オプションで`decimalNotation`、`xcodemlType`属性を利用できる。

`decimalNotation`属性の値は数字またはハイフンマイナス(`-`)からなる文字列で、
整数の十進表現を表す。
逆変換では使用しない。

`xcodemlType`属性の値はデータ型識別名で、型を表す。
逆変換では使用しない。


## `MemberExpr`: クラスメンバーアクセス

| `<clangStmt class="MemberExpr"`
|   `is_arrow` `=` `"true"` | `"false"` | `"1"` | `"0"`
|   `xcodemlType` `=` _データ型識別名_
| `>`
|   _`name`要素_
|   _`clangDeclarationNameInfo`要素_
|   _`clangStmt`要素_
| `</clangStmt>`

`MemberExpr`は、クラス型オブジェクトか、またはクラス型へのポインターのデータメンバーへのアクセス(`E1.E2`, `E1->E2`)を表現する。
これら2つの演算が同じ形式で表されているのは、Clangでの内部表現
([`clang::MemberExpr`](https://clang.llvm.org/doxygen/classclang_1_1MemberExpr.html))
を反映している。

第1子要素は`name`要素で、アクセスするメンバー名を表現する。

第2子要素は`clangDeclarationNameInfo`要素で、逆変換では使用しない。

第3子要素は`clangStmt`要素で、アクセスするクラス型オブジェクトまたはクラス型へのポインターを表現する。

この要素は、必須属性として`is_arrow`属性をもつ。

`is_arrow`属性の値は`"true"`、`"false"`、`"1"`、`"0"`のいずれかであり、
`"true"`または`"1"`のときポインターに対するメンバーアクセス(`E1->E2`)であることを表す。

この要素は、オプションで`xcodemlType`属性を利用できる。

`xcodemlType`属性の値はデータ型識別名であり、データメンバーの型を表現する。
逆変換では使用しない。


## `ReturnStmt`: `return`文

| `<clangStmt class="ReturnStmt">`
|   _`clangStmt`要素_
| `</clangStmt>`

`ReturnStmt`は`return`文を表現する。

第1子要素は`clangStmt`要素で、返す式を表す。


## `SwitchStmt`: switch文

| `<clangStmt class="SwitchStmt">`
|   _`clangStmt`要素_
|   _`clangStmt`要素_ ...
| `</clangStmt>`

`SwitchStmt`はswitch文を表現する。

第1子要素は条件式を表現する。

第2子要素以降の子要素は`CaseStmt`または`DefaultStmt`で、

各要素はswitch文本体のラベルとそれに引き続く文を表現する。


## `UnaryOperator`: 単項演算式

| `<clangStmt class="UnaryOperator"`
|   `unaryOpName` `=` _単項演算名(後述)_
|   `xcodemlType` `=` _データ型識別名_
| `>`
|   _`clangStmt`要素_
| `</clangStmt>`


`UnaryOperator`は単項演算式を表現する。

第1子要素は`clangStmt`要素で、オペランドの式を表す。

この要素は、必須属性として`unaryOpName`属性をもつ。

`unaryOpName`属性の値は文字列で、単項演算名を表す。

*単項演算名*は、`clang::UnaryOperatorKind`を表す文字列である。
以下に主要な単項演算名を挙げる。

| 単項演算名         | `clang::UnaryOperatorKind`の値 | 意味                    |
|--------------------|--------------------------------|-------------------------|
| `"postIncrExpr"`   | `UO_PostInc`                   | 後置インクリメント `++` |
| `"postDecrExpr"`   | `UO_PostDec`                   | 後置デクリメント `--`   |
| `"preIncrExpr"`    | `UO_PreInc`                    | 前置インクリメント `++` |
| `"preDecrExpr"`    | `UO_PreDec`                    | 前置デクリメント `--`   |
| `"AddrOfExpr"`     | `UO_AddrOf`                    | アドレス取得 `&`        |
| `"pointerRef"`     | `UO_Deref`                     | 間接参照 `*`            |
| `"unaryPlusExpr"`  | `UO_Plus`                      | 符号指定 `+`            |
| `"unaryMinusExpr"` | `UO_MInus`                     | 符号指定 `-`            |
| `"bitNotExpr"`     | `UO_Not`                       | ビットNOT `~`           |
| `"logNotExpr"`     | `UO_LNot`                      | 論理否定 `!`            |

<!-- このほかに`unaryRealExpr`などがある(ClangOperator.cpp)が、用途は不明 -->

この要素は、オプションで`xcodemlType`属性を利用できる。

`xcodemlType`属性の値はデータ型識別名で、単項演算式の型を表す。
逆変換では使用しない。


# clangConstructorInitializer要素

| `<clangConstructorInitializer `
|   `is_written=` `"true"` | `"false"` | `"1"` | `"0"`
|   `member` `=` _メンバー名_
|   `>`
|   _`clangStmt`要素_
| `</clangStmt>`


