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
|   `cxx_class_kind` `=` `"class"` | `"struct"`
|   `is_anonymous` `=` `"true"` | `"false"` | `"1"` | `"0"`
|   `type` `=` _ユーザ定義されたデータ型識別名_
|   `>`
|   _`inheritedFrom`要素_
|   _`symbols`要素_
| `</classType>`

`classType`要素はクラス型を表現する。

`inheritedFrom`子要素はこのクラスの派生元クラスのリストを表現する。
このクラスが派生クラスでない場合、`inheritedFrom`要素は子要素をもたない。
このクラスが派生クラスである場合、
`inheritedFrom`要素は、派生元クラスを表す`typeName`要素を1個以上子要素にもつ。
`typeName`要素の順番は、派生クラスのリストの順番と等しい。

`symbols`子要素はこのクラスのメンバーのリストを表現する。
`symbols`要素は、メンバー名を表す`id`要素を0個以上子要素にもつ。

この要素は、必須属性として`type`属性をもつ。

`type`属性の値はユーザ定義されたデータ型識別名であり、
この要素によって定義されるクラス型に与えられるデータ型識別名を表す。

この要素は、オプションで`cxx_class_kind`属性、`is_anonymous`属性を利用できる。

`cxx_class_kind`属性の値は`"class"`,`"struct"`のいずれかであり、
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

clangConstructorInitializer子要素は初期化リストを

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


## `TranslationUnit`: 翻訳単位

| `<clangDecl class="TranslationUnit"`
|   _`xcodemlTypeTable`要素_
|   _`xcodemlNnsTable`要素_
|   _`clangDecl`要素_ ...
| `>`


# `clangStmt`要素

| `<clangStmt`
|   `class` `=` _属性_
| `>`
| _子要素_
| `</clangStmt>`

`clangStmt`要素はC/C++の式または文を表す要素。
式または文の種類は`class`属性によって決められる。

## `CaseStmt`: caseラベル

| `<clangStmt class="CaseStmt">`
|   _`clangStmt`要素_
|   _`clangStmt`要素_
| `</clangStmt>`

`CaseStmt`はcaseラベルを表現する。

第1子要素は式。

第2要素はcaseラベルに引き続く文。

## `SwitchStmt`: switch文

| `<clangStmt class="SwitchStmt">`
|   _`clangStmt`要素_
|   _`clangStmt`要素_ ...
| `</clangStmt>`

`SwitchStmt`はswitch文を表現する。

第1子要素は条件式を表現する。

第2子要素以降の子要素は`CaseStmt`または`DefaultStmt`で、

各要素はswitch文本体のラベルとそれに引き続く文を表現する。

# clangConstructorInitializer要素

| `<clangConstructorInitializer `
|   `is_written=` `"true"` | `"false"` | `"1"` | `"0"`
|   `member` `=` _メンバー名_
|   `>`
|   _`clangStmt`要素_
| `</clangStmt>`


