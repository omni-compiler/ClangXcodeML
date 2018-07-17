---
title: ClangXMLの要素
author: XcalableMP/Omni Compiler Project
---

この文書はClangXML形式の構文と意味内容を定義する。
ClangXML形式とは、C++プログラムをXMLで表現するための形式である。

# ClangXML文書の構造

ClangXML文書は次の構造に従う。

`<clangAST>`  
  `<clangDecl class="TranslationUnit">`  
    `<xcodemlTypeTable>`  
      _データ型定義要素_ ...  
    `</xcodemlTypeTable>`  
    `<xcodemlNnsTable>`  
      _NNS定義要素_ ...  
    `</xcodemlNnsTable>`  
    _C++プログラムを表現する`clangDecl`要素_ ...  
  `</clangDecl>`  
`</clangAST>`  

ひとつのClangXML文書は、C++のひとつの翻訳単位をXML文書として表現する。

ClangXML文書のルート要素は`clangAST`要素である。
`clangAST`要素はただひとつの子要素として`clangDecl`要素をもつ。
この`clangDecl`要素の`class 属性の値は`"TranslationUnit" である。

ClangXML文書は、
C++プログラム中で使用される型(データ型)をデータ型識別名とデータ型定義要素によって表現する。
データ型識別名はデータ型に与えられる名前である。
データ型定義要素はデータ型識別名とそれが指示するデータ型の内容を定義する。


# `clangAST`要素

`<clangAST`  
  `source` `=` _パス名_  
  `language=` `"C++"` | `"C"`  
  `time` `=` _時刻_  
`>`  
  _`clangDecl`要素_  
`</clangAST>`  

必須属性なし

オプショナル:

* `source`属性
* `language`属性(デフォルト値は`"C++"`)
* `time`属性

ClangXML文書のルート要素は`clangAST`要素である。

第1子要素は`clangDecl`要素で、このClangXML文書が表現する翻訳単位を表す。
この`clangDecl`要素の`class`属性の値は`"TranslationUnit"`でなければならない。

この要素は、オプションで`source`属性、`language`属性、`time`属性を利用できる。

`source`属性の値は文字列で、元となるプログラムのファイル名を表す。
逆変換では使用しない。

`language`属性の値は`"C++"`または`"C"`で、言語を表す。
`language`属性が与えられていない場合、逆変換は翻訳単位をC++で書かれたものとみなす。

`time`属性の値は文字列で、ClangXML文書が作られた時刻を表す。
逆変換では使用しない。

# `clangDecl`要素

`<clangDecl`  
  `class` `=` _宣言の種類(後述)_  
`>`  
_子要素_ ...  
`</clangDecl>`  

必須:

* `class`属性
* その他、以下の小節で必須属性が指定されることがある。

オプショナル属性なし
(ただし、以下の小節でオプショナル属性が指定されることがある)

`clangDecl`要素はC/C++の宣言を表現する。

この要素は、必須属性として`class`属性をもつ。

`class`属性の値は文字列であり、宣言の種類を表す。

*宣言の種類*は、
[`clang::Decl::Kind`](https://clang.llvm.org/doxygen/classclang_1_1Decl.html)
を表す文字列である。
以下に主要な宣言の種類を挙げる。

| 宣言の種類                             | `clang::Decl::Kind`の値              | 意味                             |
|----------------------------------------|--------------------------------------|----------------------------------|
| `"AccessSpec"`                         | `AccessSpec`                         | アクセス指定                     |
| `"ClassTemplate"`                      | `ClassTemplate`                      | クラステンプレート宣言           |
| `"ClassTemplatePartialSpecialization"` | `ClassTemplatePartialSpecialization` | クラステンプレートの部分的特殊化 |
| `"ClassTemplateSpecialization"`        | `ClassTemplateSpecialization`        | クラステンプレートの特殊化       |
| `"CXXConstructor"`                     | `CXXConstructor`                     | コンストラクター宣言             |
| `"CXXConversion"`                      | `CXXConversion`                      | 型変換関数宣言                   |
| `"CXXDestructor"`                      | `CXXDestructor`                      | デストラクター宣言               |
| `"CXXMethod"`                          | `CXXMethod`                          | メンバー関数宣言                 |
| `"CXXRecord"`                          | `CXXRecord`                          | クラス宣言                       |
| `"Field"`                              | `Field`                              | データメンバー宣言               |
| `"Friend"`                             | `Friend`                             | `friend`宣言                     |
| `"Function"`                           | `Function`                           | 関数宣言                         |
| `"FunctionTemplate"`                   | `"FunctionTemplate"`                 | 関数テンプレート宣言             |
| `"LinkageSpec"`                        | `LinkageSpec`                        | リンケージ指定                   |
| `"ParmVar"`                            | `ParmVar`                            | 仮引数                           |
| `"TemplateTypeParm"`                   | `TemplateTypeParm`                   | テンプレート型引数               |
| `"TranslationUnit"`                    | `TranslationUnit`                    | 翻訳単位                         |
| `"TypeAlias"`                          | `TypeAlias`                          | エイリアステンプレート宣言       |
| `"Typedef"`                            | `Typedef`                            | `typedef`宣言                    |
| `"Var"`                                | `Var`                                | 変数宣言                         |

## `AccessSpec`: アクセス指定

`<clangDecl class="AccessSpec"`  
  `access` `=` `"public"` | `"private"` | `"protected"`  
`/>`

必須属性なし

オプショナル：

* `access`属性

`AccessSpec`は、ソースコード中に明示的に書かれたアクセス指定子を表現する。

逆変換では使用しない。
代わりに、逆変換では、
メンバーのアクセス指定を`clangDecl`要素の`access`属性で表現する。

## `ClassTemplate`: クラステンプレート宣言

`<clangDecl class="ClassTemplate">`  
  _`name`要素_  
  _`xcodemlTypeTable`要素_  
  _`xcodemlNnsTable`要素_  
  _クラス本体を表す`clangDecl`要素_  
  _テンプレート仮引数を表す`clangDecl`要素_ ...  
`</clangDecl>`

`ClassTemplate`はクラステンプレート宣言を表現する。

第1子要素は`name`要素で、宣言するテンプレートの名前を表現する。

第2子要素は`xcodemlTypeTable`要素である。
`xcodemlTypeTable`要素は、0個以上のデータ型定義要素を子要素としてもつ。
各データ型定義要素は、このクラステンプレート宣言中で使われるデータ型を定義する。

第3子要素は`xcodemlNnsTable`要素である。
`xcodemlNnsTable`要素は、0個以上のNNS定義要素を子要素としてもつ。
各NNS定義要素は、このクラステンプレート宣言中で使われるスコープの情報を定義する。

第4子要素は`clangDecl`要素で、クラス本体を表現する。
この要素の`class`属性の値は`"CXXRecord"`である。

第5子要素以降の要素は`clangDecl`要素で、テンプレート仮引数宣言を表現する。
各要素の`class`属性の値は`"TemplateTypeParm"`である。
この要素は1個以上ある。
`clangDecl`要素の順序は仮引数の順序と一致している必要がある。

## `ClassTemplatePartialSpecialization`: クラステンプレートの部分的特殊化

<!-- TODO: not written -->

## `ClassTemplateSpecialization`: クラステンプレートの特殊化

<!-- TODO: not written -->

## `CXXConstructor`: コンストラクター宣言

`<clangDecl class="CXXConstructor"`  
   `is_implicit=` `"true"`  | `"false"` | `"1"` | `"0"`  
 `>`  
  _`name`要素_  
  _`clangTypeLoc`要素_  
  _`clangConstructorInitializer`要素_...  
  _`clangStmt`要素_  
`</clangDecl>`  

必須属性なし

オプショナル:

* `is_implicit`属性

`CXXConstructor`はコンストラクター定義を表現する。

`name`子要素は関数名を表現する。
これは`name_kind`属性を持ち、その値は`constructor`である。

params要素は仮引数リストを表現する。

`clangConstructorInitializer`子要素は初期化リストを表現する。

clangStmt子要素は関数本体を表現する。
これは`CompoundStmt`または`tryStmt`である。

この要素は、オプションで`is_implicit`属性を利用できる。
`is_implicit`属性の値は`"true"`, `"false"`, `"1"`, `"0"`のいずれかであり、
`"true"`または`"1"`のとき関数が暗黙に宣言されたことを表す。
このとき、逆変換はこの要素に対応するコンストラクター宣言を出力しない。

## `CXXConversion`: 型変換関数宣言

<!-- TODO: not written -->

## `CXXDestructor`: デストラクター宣言

`<clangDecl class="CXXDestructor"`  
  `is_defaulted` `=` `"true"`  | `"false"` | `"1"` | `"0"`  
  `is_deleted` `=` `"true"`  | `"false"` | `"1"` | `"0"`  
  `is_implicit` `=` `"true"`  | `"false"` | `"1"` | `"0"`  
  `is_pure` `=` `"true"`  | `"false"` | `"1"` | `"0"`  
  `is_virtual` `=` `"true"`  | `"false"` | `"1"` | `"0"`    
  `parent_class` `=` _データ型識別名_  
  `xcodemlType` `=` _データ型識別名_  
`>`  
  _`name`要素_  
  _`clangDeclarationNameInfo`要素_  
  _`clangTypeLoc`要素_  
  [ _`clangStmt`要素_ ]  
`</clangDecl>`

必須：

* `parent_class`属性
* `xcodemlType`属性

オプショナル：

* `is_defaulted`属性
* `is_deleted`属性
* `is_implicit`属性
* `is_pure`属性
* `is_virtual`属性

`CXXDestructor`はデストラクター宣言を表現する。

第1子要素は`name`要素で、デストラクターの名前を表現する。
この要素の`name_kind`属性の値は`"destructor"`である。

第2子要素は`clangDeclarationNameInfo`要素である。
逆変換では使用しない。

第3子要素は`clangTypeLoc`要素である。
逆変換では使用しない。

第4子要素は`clangStmt`要素で、デストラクターの本体を表現する。
この要素は省略されることがある。
省略された場合、その宣言は本体をもたない。

この要素は、必須属性として`parent_class`属性、`xcodemlType`属性をもつ。

`parent_class`属性の値はデータ型識別名で、
デストラクターが属するクラスの型を表す。

`xcodemlType`属性の値はデータ型識別名で、
デストラクターの型を表す。


この要素は、オプションで
`is_defaulted`属性、
`is_deleted`属性、
`is_implicit`属性、
`is_pure`属性、
`is_virtual`属性
を指定できる。
これらの属性の値は`"true"`, `"false"`, `"1"`, `"0"`のいずれかである。

`is_defaulted`属性の値が`"true"`または`"1"`のとき、
デストラクターが`default`定義されていることを表す。

`is_deleted`属性の値が`"true"`または`"1"`のとき、
デストラクターが`delete`定義されていることを表す。

`is_implicit`属性の値が`"true"`または`"1"`のとき、
デストラクターが暗黙に宣言されたことを表す。
このとき、逆変換はこの要素に対応するデストラクター宣言を出力しない。

`is_pure`属性の値が`"true"`または`"1"`のとき、
純粋`virtual`関数であることを表す。

`is_virtual`属性の値が`"true"`または`"1"`のとき、
`virtual`関数であることを表す。

## `CXXMethod`: メンバー関数宣言

`<clangDecl class="CXXMethod"`  
  `is_const` `=` `"true"`  | `"false"` | `"1"` | `"0"`  
  `is_defaulted` `=` `"true"`  | `"false"` | `"1"` | `"0"`  
  `is_deleted` `=` `"true"`  | `"false"` | `"1"` | `"0"`  
  `is_function_template_specialization` `=` `"true"`  | `"false"` | `"1"` | `"0"`  
  `is_implicit` `=` `"true"`  | `"false"` | `"1"` | `"0"`  
  `is_pure` `=` `"true"`  | `"false"` | `"1"` | `"0"`  
  `is_static` `=` `"true"`  | `"false"` | `"1"` | `"0"`  
  `is_variadic` `=` `"true"`  | `"false"` | `"1"` | `"0"`  
  `is_virtual` `=` `"true"`  | `"false"` | `"1"` | `"0"`    
  `parent_class` `=` _データ型識別名_  
  `xcodemlType` `=` _データ型識別名_  
`>`  
  _`name`要素_  
  _`clangDeclarationNameInfo`要素_  
  _`clangTypeLoc`要素_  
  [ _`clangStmt`要素_ ]  
`</clangDecl>`

必須：

* `parent_class`属性
* `xcodemlType`属性

オプショナル：

* `is_const`属性
* `is_defaulted`属性
* `is_deleted`属性
* `is_function_template_specialization`属性
* `is_implicit`属性
* `is_pure`属性
* `is_static`属性
* `is_variadic`属性
* `is_virtual`属性

`CXXMethod`はメンバー関数宣言を表現する。

第1子要素は`name`要素で、
メンバー名を表現する。

第2子要素は`clangDeclarationNameInfo`要素である。
逆変換では使用しない。

第3子要素は`clangTypeLoc`要素で、
仮引数リストを表現する。
次の小節で説明する。

第4子要素は`clangStmt`要素で、関数本体を表現する。
この要素は省略されることがある。
省略された場合、その宣言は関数本体をもたない。

この要素は、必須属性として`parent_class`属性、`xcodemlType`属性をもつ。

`parent_class`属性の値はデータ型識別名で、メンバー関数が所属するクラスの型を表す。

`xcodemlType`属性の値はデータ型識別名で、メンバー関数の型を表す。

この要素は、オプションで
`is_const`属性、
`is_defaulted`属性、
`is_deleted`属性、
`is_function_template_specialization`属性、
`is_implicit`属性、
`is_pure`属性、
`is_static`属性、
`is_variadic`属性、
`is_virtual`属性
を指定できる。
これらの属性の値は`"true"`, `"false"`, `"1"`, `"0"`のいずれかである。

`is_const`属性の値が`"true"`または`"1"`のとき、
`const`メンバー関数であることを表す。

`is_defaulted`属性の値が`"true"`または`"1"`のとき、
メンバー関数が`default`定義されていることを表す。

`is_deleted`属性の値が`"true"`または`"1"`のとき、
メンバー関数が`delete`定義されていることを表す。

`is_function_template_specialization`属性の値が`"true"`または`"1"`のとき、
メンバー関数テンプレートの明示的特殊化であることを表す。

`is_implicit`属性の値が`"true"`または`"1"`のとき、
メンバー関数が暗黙に宣言されたことを表す。
このとき、逆変換はこの要素に対応するメンバー関数宣言を出力しない。

`is_pure`属性の値が`"true"`または`"1"`のとき、
純粋`virtual`関数であることを表す。

`is_static`属性の値が`"true"`または`"1"`のとき、
`static`メンバー関数であることを表す。

`is_variadic`属性の値が`"true"`または`"1"`のとき、
可変長引数をとることを表す。

`is_virtual`属性の値が`"true"`または`"1"`のとき、
`virtual`関数であることを表す。

### `CXXMethod`の第3子要素(`clangTypeLoc`要素)

`<clangTypeLoc`  
  `class` `=` `"FunctionProto"`  
  `type` `=` _データ型識別名_  
`>`  
  _`clangTypeLoc`要素_  
  _`clangDecl`要素_  
`</clangTypeLoc>`

必須属性なし

オプショナル：

* `class`属性
* `type`属性

`CXXMethod`を表す`clangDecl`要素は、第3子要素として`clangTypeLoc`要素をもつ。

`clangTypeLoc`要素の第1子要素は`clangTypeLoc`要素で、
メンバー関数の返り値型を表現する。
ただし、コンストラクター、デストラクターに対しては、この要素は`void`型を表現する。
逆変換では使用しない。

第2子要素以降の子要素は`clangDecl`要素で、
関数の仮引数リストを表現する。
この要素は0個以上ある。
この要素の`class`属性の値は`"ParmVar"`である。
逆変換では、この要素の`name`子要素を使用して仮引数名を出力する。
`clangDecl`要素の順序は仮引数の順序と一致しなくてはならない。

この要素は、オプションで`class`属性、`type`属性を利用できる。

`class`属性の値は文字列で、
その値は`"FunctionProto"`である。
逆変換では使用しない。

`type`属性の値はデータ型識別名で、
メンバー関数の型を表現する。
逆変換では使用しない。

## `CXXRecord`: クラス宣言

`<clangDecl class="CXXRecord"`
  `is_implicit` `=` `"true"`  | `"false"` | `"1"` | `"0"`  
  `is_this_declaration_a_definition` `=` `"true"`  | `"false"` | `"1"` | `"0"`  
  `xcodemlType` `=` _データ型識別名_  
`>`  
  _`name`要素_  
  _`clangTypeLoc`要素_ ...  
  _`clangDecl`要素_ ...  
`</clangDecl>`

必須：

* `xcodemlType`属性

オプショナル：

* `is_implicit`属性
* `is_this_declaration_a_definition`属性

`CXXRecord`はC++のクラス宣言を表現する。

第1子要素は`name`要素で、クラス名を表現する。

第2子要素は`clangTypeLoc`要素である。
この要素は1個以上ある。
逆変換では使用しない。

第3子要素以降の子要素は`clangDecl`要素で、
メンバー指定(クラス内の宣言およびアクセス指定)を表現する。
この要素は1個以上ある。

この要素は必須属性として`xcodemlType`属性をもつ。

`xcodemlType`属性の値はデータ型識別名で、
宣言されるクラスの型を表現する。

この要素は、オプションで`is_implicit`属性、
`is_this_declaration_a_definition`属性を指定できる。

`is_implicit`属性の値は`"true"`, `"false"`, `"1"`, `"0"`のいずれかであり、
`"true"`または`"1"`のとき関数が暗黙に宣言されたことを表す。
このとき、逆変換はこの要素に対応するクラス宣言を出力しない。

`is_this_declaration_a_definition`属性の値は`"true"`, `"false"`, `"1"`, `"0"`のいずれかであり、
`"true"`または`"1"`のとき宣言がである(クラス本体をもつ)ことを表す。
そうでないとき、クラス宣言が定義でないことを表す。
省略時の値は`"false"`である。

## `Field`: データメンバー宣言

`<clangDecl class="Field"`  
  `is_bit_field` `=` `"true"`  | `"false"` | `"1"` | `"0"`  
  `is_unnamed_bit_field` `=` `"true"`  | `"false"` | `"1"` | `"0"`  
  `xcodemlType` `=` _データ型識別名_  
`>`  
  _`name`要素_  
  _`clangTypeLoc`要素_  
  [ _`clangStmt`要素_ ]  
`</clangDecl>`

`Field`はデータメンバー宣言を表現する。

第1子要素は`name`要素で、フィールド名を表現する。

第2子要素は`clangTypeLoc`要素である。
逆変換では使用しない。

第3子要素は`clangStmt`要素で、
ビットフィールドの定数式を表現する。
この要素は省略されることがある。
省略された場合、そのメンバーはビットフィールドではない。

この要素は、必須属性として`xcodemlType`属性をもつ。

`xcodemlType`属性の値はデータ型識別名であり、
データメンバーの型を表す。

`is_bit_field`属性の値は`"true"`, `"false"`, `"1"`, `"0"`のいずれかであり、
`"true"`または`"1"`のときそのメンバーがビットフィールドであることを表す。

`is_unnamed_bit_field`属性の値は`"true"`, `"false"`, `"1"`, `"0"`のいずれかであり、
`"true"`または`"1"`のときそのメンバーが無名ビットフィールドであることを表す。

## `Friend`: `friend`宣言

`Friend`は`friend`宣言を表現する。

### `friend`クラスの宣言

`<clangDecl class="Friend">`  
  _`clangTypeLoc`要素_  
`</clangDecl>`

第1子要素は`clangTypeLoc`要素で、`friend`指定するクラスを表現する。

この`clangTypeLoc`要素は、必須属性として`"type"`属性をもつ。
`"type"`属性の値は文字列で、
`friend`指定するクラス型に対応するデータ型識別名を表す。

### `friend`関数の宣言

`<clangDecl class="Friend">`  
  _`clangDecl`要素_  
`</clangDecl>`  

第1子要素は`clangDecl`要素で、`friend`指定する関数の宣言を表現する。
この要素の`class`属性の値は`"Function"`である。

## `Function`: 関数宣言

`<clangDecl class="Function"`  
   `is_implicit` = `"true"` | `"false"` | `"1"` | `"0"`  
 `>`  
  _`name`要素_  
  _`clangDeclarationNameInfo`要素_  
  _`clangTypeLoc`要素_  
  [ _`clangStmt`要素_ ]  
`</clangDecl>`  

必須属性なし

オプショナル:

* `is_implicit`属性

`Function`は関数宣言または関数定義を表現する。

第1子要素は`name`要素で、関数名を表現する。

第2子要素は`clangDeclarationNameInfo`要素である。逆変換では使用しない。

第3子要素は`clangTypeLoc`要素で、仮引数リストを表現する。
次の小節で説明する。

第4子要素は`clangStmt`要素で、関数本体を表現する。
この要素は省略されることがある。このとき関数本体はない。
この要素の`class`属性の値は`CompoundStmt`または`tryStmt`である。

この要素は、オプションで`is_implicit`属性を利用できる。
`is_implicit`属性の値は`"true"`, `"false"`, `"1"`, `"0"`のいずれかであり、
`"true"`または`"1"`のとき関数が暗黙に宣言されたことを表す。
このとき、逆変換はこの要素に対応する関数宣言を出力しない。


### `Function`の第3子要素(`clangTypeLoc`要素)

`<clangTypeLoc`  
  `class` `=` `"FunctionProto"`  
  `type` `=` _データ型識別名_  
`>`  
  _`clangTypeLoc`要素_  
  _`clangDecl`要素..._  
`</clangTypeLoc>`  

必須属性なし

オプショナル:

* `class`属性
* `type`属性

`Function`を表す`clangDecl`要素は、第3子要素として`clangTypeLoc`要素をもつ。

`clangTypeLoc`要素の第1子要素は`clangTypeLoc`要素で、関数の返り値型を表現する。
逆変換では使用しない。

第2子要素以降の要素は`clangDecl`要素で、関数の仮引数リストを表現する。
この要素の`class`属性の値は`"ParmVar"`である。
逆変換では、この要素の`"name"`子要素を使用して仮引数名を出力する。
`clangDecl`要素の順序は仮引数の順序と一致していなくてはならない。

この要素は、オプションで`class`、`type`属性を利用できる。

`class`属性の値は文字列で、その値は`"FunctionProto"`である。
逆変換では使用しない。

`type`属性の値は文字列で、
定義または宣言される関数の型に対応するデータ型識別名を表す。
逆変換では使用しない。

## `FunctionTemplate`: 関数テンプレート宣言

<!-- TODO: not written -->

## `LinkageSpec`: リンケージ指定

<!-- TODO: not written -->

## `ParmVar`: 仮引数

`<clangDecl class="ParmVar"`  
`has_init` `=` `"true"` | `"false"` | `"1"` | "`0`"  
`xcodemlType` `=` _データ型識別名_  
`>`  
  _`name`要素_  
  _`clangTypeLoc`要素_  
  [ _`clangStmt`要素_ ]  
`</clangDecl>`  

必須属性なし

オプショナル:

* `xcodemlType`属性
* `has_init`属性

`ParmVar`は関数宣言中の仮引数の宣言を表現する。

第1子要素は`name`要素で、引数名を表現する。

第2子要素は`clangTypeLoc`要素である。
逆変換では使用しない。

第3子要素は`clangStmt`要素で、デフォルト実引数を表現する。
この要素は省略されることがある。
そのとき、デフォルト実引数は指定されていない。

## `TemplateTypeParm`: テンプレート型引数

<!-- TODO: not written -->

## `TranslationUnit`: 翻訳単位

`<clangDecl class="TranslationUnit"`  
  _`xcodemlTypeTable`要素_  
  _`xcodemlNnsTable`要素_  
  _`clangDecl`要素_ ...  
`>`  

## `TypeAlias`: エイリアステンプレート宣言

<!-- TODO: not written -->

## `Typedef`: `typedef`宣言

`<clangDecl class="Typedef"`  
`xcodemlTypedefType` `=` _データ型識別名_  
`>`  
  _`name`要素_  
`</clangDecl>`  

必須属性:

* `xcodemlTypedefType`属性

`Typedef`は`typedef`宣言を表現する。

第1子要素は`name`要素で、`typedef`名を表現する。

この要素は必須属性として`xcodemlTypedefType`属性をもつ。
`xcodemlTypedefType`属性の値はデータ型識別名であり、
`typedef`名が表す型を表現する。

## `Var`: 変数宣言

`<clangDecl class="Var"`  
`xcodemlType` `=` _データ型識別名_  
`has_external_storage` `=` `"true"`  | `"false"` | `"1"` | `"0"`  
`is_out_of_line` `=` `"true"`  | `"false"` | `"1"` | `"0"`  
`is_static_data_member` `=` `"true"`  | `"false"` | `"1"` | `"0"`  
`is_static_local` `=` `"true"`  | `"false"` | `"1"` | `"0"`  
`>`  
  _`name`要素_  
  [ _`clangStmt`要素_ ]  
`</clangDecl>`  

必須属性:

* `xcodemlType`属性

オプショナル:

* `has_external_storage`属性
* `is_out_of_line`属性
* `is_static_data_member`属性
* `is_static_local`属性

`Var`は変数宣言を表現する。

第1子要素は`name`要素で、変数名を表現する。

第2子要素は`clangStmt`要素で、変数宣言の初期化子を表現する。
この要素は省略されることがある。
省略されたとき、その変数宣言は初期化子をもたない。

この要素は、必須属性として`xcodemlType`属性をもつ。

`xcodemlType`属性の値はデータ型識別名で、宣言する変数の型を表す。

この要素は、オプションで
`has_external_storage`属性、
`is_static_data_member`属性、
`is_out_of_line`属性、
`is_static_local`属性
を利用できる。
これらの属性の値は`"true"`, `"false"`, `"1"`, `"0"`のいずれかである。

`has_external_storage`属性の値が`"true"`または`"1"`のとき、
変数宣言が`extern`指定子をもつことを表す。

`is_out_of_line`属性の値が`"true"`または`"1"`のとき、
変数宣言が`static`データメンバーのout-of-line定義であることを表す。

`is_static_data_member`属性の値が`"true"`または`"1"`のとき、
変数宣言が`static`データメンバーに対するものであることを表す。

`is_static_local`属性の値が`"true"`または`"1"`のとき、
宣言された変数が`static`変数であることを表す。

# `clangStmt`要素

`<clangStmt`  
  `class` `=` _Stmtの種類(後述)_  
`>`  
_子要素_ ...  
`</clangStmt>`  

必須:

* `class`属性
* その他、以下の小節で必須属性が指定されることがある。

`clangStmt`要素は
Clang の `clang::Stmt` クラスから派生したクラスのデータを表す要素であり、
式や文を表現する。

この要素は、必須属性として`class`属性をもつ。

`class`属性の値は文字列であり、Stmtの種類を表す。

*Stmtの種類*は、
[clang::Stmt::StmtClass](https://clang.llvm.org/doxygen/classclang_1_1Stmt.html)
を表す文字列である。
以下に主要なStmtの種類を挙げる。

| Stmtの種類                 | `clang::Stmt::StmtClass`の値 | 意味                                    |
|----------------------------|------------------------------|-----------------------------------------|
| `"BinaryOperator"`         | BinaryOperatorClass          | 二項演算                                |
| `"BreakStmt"`              | BreakStmtClass               | `break`文                               |
| `"CallExpr"`               | CallExprClass                | 関数呼び出し                            |
| `"CaseStmt"`               | CaseStmtClass                | `case`ラベル                            |
| `"CharacterLiteral"`       | CharacterLiteralClass        | 文字リテラル                            |
| `"CompoundAssignOperator"` | CompoundAssignOperatorClass  | 複合代入演算                            |
| `"CompoundStmt"`           | CompoundStmtClass            | 複合文                                  |
| `"CStyleCastExpr"`         | CStyleCastExprClass          | Cスタイルキャスト形式による明示的型変換 |
| `"CXXConstCastExpr"`       | CXXConstCastExprClass        | `const_cast`式                          |
| `"CXXDynamicCastExpr"`     | CXXDynamicCastExprClass      | `dynamic_cast`式                        |
| `"CXXMemberCallExpr"`      | CXXMemberCallExprClass       | メンバー関数呼び出し                    |
| `"CXXStaticCastExpr"`      | CXXStaticCastExprClass       | `static_cast`式                         |
| `"CXXReinterpretCastExpr"` | CXXReinterpretCastExprClass  | `reinterpret_cast`式                    |
| `"CXXThisExpr"`            | CXXThisExprClass             | `this`ポインター                        |
| `"DeclRefExpr"`            | DeclRefExprClass             | 変数参照                                |
| `"DeclStmt"`               | DeclStmtClass                | 宣言文                                  |
| `"IfStmt"`                 | IfStmtClass                  | `if`文                                  |
| `"ImplicitCastExpr"`       | ImplicitCastExprClass        | 暗黙の型変換                            |
| `"IntegerLiteral"`         | IntegerLiteralClass          | 整数リテラル                            |
| `"MemberExpr"`             | MemberExprClass              | クラスメンバーアクセス                  |
| `"ReturnStmt"`             | ReturnStmtClass              | `return`文                              |
| `"StringLiteral"`          | StringLiteralClass           | 文字列リテラル                          |
| `"SwitchStmt"`             | SwitchStmtClass              | `switch`文                              |
| `"UnaryOperator"`          | UnaryOperatorClass           | 単項演算                                |

以下では逆変換に用いる部分について個別に解説する。
その他については Clang の実装を参照のこと。


## `BinaryOperator`: 二項演算

`<clangStmt class="BinaryOperator"`  
`binOpName` `=` _二項演算名(後述)_  
`xcodemlType` `=` _データ型識別名_  
`>`  
  _`clangStmt`要素_  
  _`clangStmt`要素_  
`</clangStmt>`  

必須:

* `binOpName`属性

オプショナル:

* `xcodemlType`属性

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
| `"memberIndirectRef"` | `BO_PtrMemI`                    | メンバーポインターによるメンバーアクセス `->*` |
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
| `"bitOrExpr"`         | `BO_Or`                         | ビットOR &#124;                                |
| `"logAndExpr"`        | `BO_LAnd`                       | 論理積 `&&`                                    |
| `"logOrExpr"`         | `BO_LOr`                        | 論理和 &#124;&#124;                            |
| `"assignExpr"`        | `BO_Assign`                     | 代入 `=`                                       |
| `"commaExpr"`         | `BO_Comma`                      | カンマ演算 `,`                                 |


## `BreakStmt`: `break`文

`<clangStmt class="BreakStmt">`  

必須属性なし

`BreakStmt`は`break`文を表現する。

この要素は子要素をもたない。


## `CallExpr`: 関数呼び出し

`<clangStmt class="CallExpr"`  
  `xcodemlType` `=` _データ型識別名_  
`>`  
  _`clangStmt`要素_  
  _`clangStmt`要素_ ...  
`</clangStmt>`  

オプショナル:

* `xcodemlType`属性

`CallExpr`は関数呼び出し式を表現する。

第1子要素は`clangStmt`要素で、呼び出される関数を表現する。

第2子要素以降の子要素は`clangStmt`要素で、関数に渡される実引数リストを表現する。
この要素は0個以上ある。
`clangStmt`要素の順序は実引数の順序と一致しなくてはならない。


## `CaseStmt`: caseラベル

`<clangStmt class="CaseStmt">`  
  _`clangStmt`要素_  
  _`clangStmt`要素_  
`</clangStmt>`  

必須属性なし

`CaseStmt`はcaseラベルを表現する。

第1子要素は式。

第2要素はcaseラベルに引き続く文。


## `CharacterLiteral`: 文字リテラル

`<clangStmt class="CharacterLiteral"`  
  `hexadecimalNotation` `=` _文字列_  
  `token` `=` _文字列_  
  `xcodemlType` `=` _データ型識別名_  
`/>`  

必須:

* `hexadecimalNotation`属性

オプショナル:

* `token`属性
* `xcodemlType`属性

`CharacterLiteral`は文字リテラルを表現する。

この要素は子要素をもたない。

この要素は、必須属性として`hexadecimalNotation`属性をもつ。

`hexadecimalNotation`属性の値は次の形式の文字列で、
文字リテラル値の十六進表現を表す。

```
0x[0-9]+
```

この要素は、オプションで`token`、`xcodemlType`属性を利用できる。

`token`属性の値は次の形式の文字列で、
接頭辞やシングルクオーテーション(`'`)を含む文字リテラルそれ自体を表す。

```
(u|U|L)?'.+'
```

`xcodemlType`属性の値はデータ型識別名で、文字列リテラルのデータ型を表す。


## `CompoundAssignOperator`: 複合代入演算

`<clangStmt class="CompoundAssignOperator"`  
  `binOpName` `=` _二項演算名(後述)_  
  `xcodemlType` `=` _データ型識別名_  
`>`  
  _`clangStmt`要素_  
  _`clangStmt`要素_  
`</clangStmt>`  

必須:

* `binOpName`属性
* `xcodemlType`属性

`CompoundAssignOperator`は複合代入演算を表現する。

第1、第2子要素はともに`clangStmt`要素で、
それぞれ複合代入演算の左辺、右辺を表現する。

この要素は、必須属性として`binOpName`属性をもつ。

`binOpName`属性の値は文字列で、二項演算名を表す。

*二項演算名*は、`clang::BinaryOperatorKind`を表す文字列である。
以下に主要な二項演算名を挙げる。

| 二項演算名        | `clang::BinaryOperatorKind`の値 | 意味                             |
|-------------------|---------------------------------|----------------------------------|
| `"asgMulExpr"`    | `BO_MulAssign`                  | 乗算の複合代入演算 `*=`          |
| `"asgDivExpr"`    | `BO_DivAssign`                  | 除算の複合代入演算 `/=`          |
| `"asgModExpr"`    | `BO_RemAssign`                  | 剰余の複合代入演算 `%=`          |
| `"asgPlusExpr"`   | `BO_AddAssign`                  | 加算の複合代入演算 `+=`          |
| `"asgMinusExpr"`  | `BO_SubAssign`                  | 減算の複合代入演算 `-=`          |
| `"asgLshiftExpr"` | `BO_ShlAssign`                  | 左シフトの複合代入演算 `<<=`     |
| `"asgRshiftExpr"` | `BO_ShrAssign`                  | 右シフトの複合代入演算 `>>=`     |
| `"asnBitAndExpr"` | `BO_AndAssign`                  | ビットANDの複合代入演算 `&=`     |
| `"asgBitOrExpr"`  | `BO_OrAssign`                   | ビットORの複合代入演算 &#124;=   |
| `"asgXorExpr"`    | `BO_XorAssign`                  | ビットXORの複合代入演算 `^=`     |

## `CompoundStmt`: 複合文

`<clangStmt class="CompoundStmt">`  
  _`clangStmt`要素_ ...  
`</clangStmt>`  

必須属性なし

`CompoundStmt`は複合文を表現する。

この要素は0個以上の`clangStmt`要素を子要素にもつ。
各`clangStmt`要素は複合文に含まれる各文を表現する。
`clangStmt`要素の順序は文の順序と一致しなくてはならない。


## `ConditionalOperator`: 条件演算式

`<clangStmt class="ConditionalOperator"`  
  `xcodemlType` `=` _データ型識別名_  
`>`  
  _`clangStmt`要素_  
  _`clangStmt`要素_  
  _`clangStmt`要素_  
`</clangStmt>`  

オプショナル:

* `xcodemlType`属性

`ConditionalOperator`は条件演算式(`E1 ? E2 : E3`)を表現する。

第1、 第2、第3子要素は`clangStmt`要素で、
それぞれ条件演算式の第1、第2、第3オペランドを表す。

この要素は、オプションで`xcodemlType`属性を利用できる。

`xcodemlType`属性の値はデータ型識別名で、条件演算式全体の型を表す。
逆変換では使用しない。

## `CXXMemberCallExpr`: メンバー関数呼び出し

`<clangStmt class="CXXMemberCallExpr"`  
  `xcodemlType` `=` _データ型識別名_  
`>`  
  _`clangStmt`要素_  
  _`clangStmt`要素_ ...  
`</clangStmt>`  

必須属性なし

オプショナル:

* `xcodemlType`属性

`CXXMemberCallExpr`はメンバー関数呼び出し式を表現する。

第1子要素は`clangStmt`要素で、呼び出される非`static`メンバー関数を表す。
この要素の`class`属性の値は`"MemberExpr"`でなければならない。

第2子要素以降の子要素は`clangStmt`要素で、
メンバー関数に渡される実引数リストを表す。
この要素は0個以上ある。
`clangStmt`要素の順序と実引数の順序は一致しなくてはならない。

## 型変換式

`<clangStmt class=` `"CStyleCastExpr"` | `"CXXConstCastExpr"` | `"CXXDynamicCastExpr"` `"CXXStaticCastExpr"` | `"CXXReinterpretCastExpr"`  
  `clangCastKind` `=` _型変換の種類_  
  `xcodemlType` `=` _データ型識別名_  
`>`  
  _`clangTypeLoc`要素_  
  _`clangStmt`要素_  
`</clangStmt>`

必須属性なし

オプショナル：

* `clangCastKind`属性
* `xcodemlType`属性

型変換式は`CStyleCastExpr`, `CXXConstCastExpr`, `CXXDynamicCastExpr`,
`CXXStaticCastExpr`, `CXXReinterpretCastExpr`によって表現する。

第1子要素は`clangTypeLoc`要素で、変換先の型を表現する。

第2子要素は`clangStmt`要素で、変換対象の式を表現する。

この要素は、オプションで`clangCastKind`属性、`xcodemlType`属性を利用できる。

`clangCastKind`属性の値は文字列で、型変換の種類を表す。
逆変換では使用しない。

`xcodemlType`属性の値はデータ型識別名で、型変換後のデータ型を表す。
逆変換では使用しない。


## `CXXThisExpr`: `this`ポインター

`<clangStmt class="CXXThisExpr"`  
  `xcodemlType` `=` _データ型識別名_  
`/>`  

必須属性なし

オプショナル:

* `xcodemlType`属性

`CXXThisExpr`は`this`ポインターを表現する。

この要素は子要素をもたない。

この要素は、オプションで`xcodemlType`属性を利用できる。

`xcodemlType`属性の値はデータ型識別名であり、`this`ポインターの型を表す。
逆変換では使用しない。

## `DeclRefExpr`: 変数参照

`<clangStmt class="DeclRefExpr"`  
  `xcodemlType` `=` _データ型識別名_  
`>`  
  _`name`要素_  
  _`clangDeclarationNameInfo`要素_  
`</clangStmt>`  

必須属性なし

オプショナル:

* `xcodemlType`属性

`DeclRefExpr`は変数参照を表現する。

第1子要素は`name`要素で、変数名を表現する。

第2子要素は`clangDeclarationNameInfo`要素で、逆変換では使用しない。

この要素は、オプションで`xcodemlType`属性を利用できる。

`xcodemlType`属性の値はデータ型識別名で、変数の型を表現する。
逆変換では使用しない。


## `DeclStmt`: 宣言文

`<clangStmt class="DeclStmt">`  
  _`clangDecl`要素_  
`</clangStmt>`  

必須属性なし

`DeclStmt`は宣言文を表現する。

第1子要素は`clangDecl`要素で、その宣言を表す。

## `IfStmt`: `if`文

`<clangStmt class="IfStmt">`  
  _`clangStmt`要素_  
  _`clangStmt`要素_  
  [ _`clangStmt`要素_ ]  
`</clangStmt>`  

必須属性なし

`IfStmt`は`if`文を表現する。

第1子要素は`clangStmt`要素で、条件式を表す。

第2子要素は`clangStmt`要素で、then節の文を表す。

第3子要素は`clangStmt`要素で、else節の文を表す。
第3子要素は省略されることがある。
省略された場合、元の`if`文がelse節をもたないことを表す。


## `ImplicitCastExpr`: 暗黙の型変換

`<clangStmt class="ImplicitCastExpr"`  
  `xcodemlType` `=` _データ型識別名_  
  `clangCastKind` `=` _型変換の種類(後述)_  
 `>`  
 _`clangStmt`要素_  
`</clangStmt>`  

必須属性なし

オプショナル:

* `xcodemlType`属性
* `clangCastKind`属性

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

| 型変換の種類               | `clang::CastKind`の値       | 意味                                       |
|----------------------------|-----------------------------|--------------------------------------------|
| `"NoOp"`                   | `CK_NoOp`                   | 何もしないか、または修飾子を付け加える     |
| `"ArrayToPointerDecay"`    | `CK_ArrayToPointerDecay`    | 配列からポインターへの型変換([conv.array]) |
| `"FunctionToPointerDecay"` | `CK_FunctionToPointerDecay` | 関数からポインターへの型変換([conv.func])  |
| `"LValueToRValue"`         | `CK_LValueToRValue`         | lvalueからrvalueへの型変換([conv.lval])    |


## `IntegerLiteral`: 整数リテラル

`<clangStmt class="IntegerLiteral"`  
  `token` `=` _文字列_  
  `decimalNotation` `=` _文字列_  
  `xcodemlType` `=` _データ型識別名_  
`/>`  

必須:

* `token`属性

オプショナル:

* `decimalNotation`属性
* `xcodemlType`属性

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

`<clangStmt class="MemberExpr"`  
  `is_arrow` `=` `"true"` | `"false"` | `"1"` | `"0"`  
  `xcodemlType` `=` _データ型識別名_ | `"_bound_member_function_type_"`  
`>`  
  _`name`要素_  
  _`clangDeclarationNameInfo`要素_  
  _`clangStmt`要素_  
`</clangStmt>`  

必須:

* `is_arrow`属性

オプショナル:

* `xcodemlType`属性

`MemberExpr`は、クラス型オブジェクトか、またはクラス型へのポインターのメンバーへのアクセス(`E1.E2`, `E1->E2`)を表現する。
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

`xcodemlType`属性の値はデータ型識別名であるか、または
`"_bound_member_function_type_"`である。
データ型識別名のとき、メンバーの型を表す。
`"_bound_member_function_type_"`のとき、
この式がオブジェクトの非`static`メンバー関数への参照であることを表す。
逆変換では使用しない。


## `ReturnStmt`: `return`文

`<clangStmt class="ReturnStmt">`  
  _`clangStmt`要素_  
`</clangStmt>`  

必須属性なし

`ReturnStmt`は`return`文を表現する。

第1子要素は`clangStmt`要素で、返す式を表す。


## `StringLiteral`: 文字列リテラル

`<clangStmt class="StringLiteral"`  
  `stringLiteral` `=` _文字列_  
  `xcodemlType` `=` _データ型識別名_  
`/>`  

必須:

* `stringLiteral`属性

オプショナル:

* `xcodemlType`属性

`StringLiteral`は、文字列リテラルを表現する。
現在`char`型の文字列リテラルにのみ対応している。

この要素は子要素をもたない。

この要素は、必須属性として`stringLiteral`属性をもつ。

`stringLiteral`属性の値は文字列で、(終端の`'\0'`を除いた)文字列リテラルの内容を表す。
ただし、終端以外の`'\0'`は`"\484848"`として表される。
<!-- 484848となる理由はよく分からない -->

この要素は、オプションで`xcodemlType`属性を利用できる。

`xcodemlType`属性の値はデータ型識別名で、文字列リテラルの型を表す。
逆変換では使用しない。


## `SwitchStmt`: switch文

`<clangStmt class="SwitchStmt">`  
  _`clangStmt`要素_  
  _`clangStmt`要素_ ...  
`</clangStmt>`  

必須属性なし

`SwitchStmt`はswitch文を表現する。

第1子要素は条件式を表現する。

第2子要素以降の子要素は`CaseStmt`または`DefaultStmt`で、

各要素はswitch文本体のラベルとそれに引き続く文を表現する。


## `UnaryOperator`: 単項演算式

`<clangStmt class="UnaryOperator"`  
  `unaryOpName` `=` _単項演算名(後述)_  
  `xcodemlType` `=` _データ型識別名_  
`>`  
  _`clangStmt`要素_  
`</clangStmt>`  

必須:

* `unaryOpName`属性

オプショナル:

* `xcodemlType`属性

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



# `clangTypeLoc`要素

`<clangTypeLoc`  
  `class` `=` _型の種類(後述)_  
  `type` `=` _データ型識別名_  
`>`  
_子要素_ ...  
`</clangTypeLoc>`  

必須属性なし

オプショナル:

* `type`属性
* `class`属性

`clangTypeLoc`要素は、
Clang の `clang::TypeLoc` のデータを表す要素であり、
C/C++のソースコードに明示的に書かれた型情報を表現する。

この要素は、オプションで`class`属性、`type`属性を指定できる。

`class`属性の値は文字列で、型の種類を表す。

`type`属性の値は文字列で、対応するデータ型識別名を表す。

ただし、いくつかの状況においてこれらの属性が必須となることがある。
その場合には該当する節で指定する。

*型の種類*は、
[`clang::Type::TypeClass`](https://clang.llvm.org/doxygen/classclang_1_1Type.html)
を表す文字列である。
以下に主要な型の種類を挙げる。

| 型の種類                 | `clang::Type::TypeClass`の値 | 意味                                               |
|--------------------------|------------------------------|----------------------------------------------------|
| "Builtin"                | `Builtin`                    | 普遍型                                             |
| "Elaborated"             | `Elaborated`                 | 修飾名により指定された型または複雑型指定子を持つ型 |
| "FunctionProto"          | `FunctionProto`              | 関数型                                             |
| "Paren"                  | `Paren`                      | `clang::ParenType` (括弧に包まれた型情報)          |
| "Pointer"                | `Pointer`                    | ポインター型                                       |
| "Record"                 | `Record`                     | Cの構造体型およびC++のクラス型                     |
| "TemplateTypeParm"       | `TemplateTypeParm`           | テンプレート型引数                                 |
| "TemplateSpecialization" | `TemplateSpecialization`     | テンプレートの特殊化により得られる型               |
| "Typedef"                | `Typedef`                    | `typedef`された型                                  |

逆変換の際には、
`clangTypeLoc`要素がどの要素の子要素として出現したかによってそれぞれ異なる用いられ方をするため、
詳細は該当する部分において解説する。

# `xcodemlTypeTable`要素

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

`<classType`  
  `cxx_class_kind` `=` `"class"` | `"struct"` | `"union"`  
  `is_anonymous` `=` `"true"` | `"false"` | `"1"` | `"0"`  
  `type` `=` _ユーザ定義されたデータ型識別名_  
  `>`  
  _`inheritedFrom`要素_  
  _`symbols`要素_  
`</classType>`  

`classType`要素はクラス型を表現する。

第1子要素は`inheritedFrom`要素で、このクラスの派生元クラスのリストを表現する。
このクラスが派生クラスでない場合、`inheritedFrom`要素は子要素をもたない。
このクラスが派生クラスである場合、
`inheritedFrom`要素は、派生元クラスを表す`typeName`要素を1個以上子要素にもつ。
`typeName`要素の順番は、派生クラスのリストの順番と等しい。

第2子要素は`symbols`要素で、このクラスのメンバーのリストを表現する。
`symbols`要素は、メンバー名を表す`id`要素を0個以上子要素にもつ。

この要素は、必須属性として`cxx_class_kind`属性、 `type`属性をもつ。

`type`属性の値はユーザ定義されたデータ型識別名であり、
この要素によって定義されるクラス型に与えられるデータ型識別名を表す。

`cxx_class_kind`属性の値は`"class"`,`"struct"`, `"union"`のいずれかであり、
C++プログラム中でこのクラスを宣言するのに使われたキーワードを表す。

この要素は、オプションで`is_anonymous`属性を利用できる。

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


# `xcodemlNnsTable`要素

## NNS


# clangConstructorInitializer要素

`<clangConstructorInitializer `  
  `is_written=` `"true"` | `"false"` | `"1"` | `"0"`  
  `member` `=` _メンバー名_  
  `>`  
  _`clangStmt`要素_  
`</clangStmt>`  


# `name`要素

`<name`  
  `name_kind` `=` _名前の種類(後述)_  
`>`  
  [ _名前_ ]  
`</name>`

必須：

* `name_kind`属性
* その他、以下の小節で必須属性が指定されることがある。

オプショナル属性なし
(ただし、以下の小節でオプショナル属性が指定されることがある)

`name`要素はC/C++の名前を表現する。

この要素は、必須属性として`name_kind`属性をもつ。

`name_kind`属性の値は文字列であり、名前の種類を表す。

*名前の種類*は、
[`clang::DeclarationName::NameKind`](https://clang.llvm.org/doxygen/classclang_1_1DeclarationName.html)
を表す文字列である。
以下に主要な名前の種類を挙げる。

| 名前の種類      | `clang::DeclarationName::NameKind`の値 | 意味               |
|-----------------|----------------------------------------|--------------------|
| `"operator"`    | `CXXOperatorName`                      | 演算子関数ID       |
| `"conversion"`  | `CXXConversionFunctionName`            | 変換関数ID         |
| `"constructor"` | `CXXConstructorName`                   | コンストラクター名 |
| `"destructor"`  | `CXXDestructorName`                    | デストラクター名   |
| `"name"`        | `Identifier`                           | その他の識別子     |

## `operator`: 演算子関数ID

## `conversion`: 変換関数ID

## `constructor`: コンストラクターを表す名前

## `destructor`: デストラクターを表す名前

## `name`: その他の識別子
