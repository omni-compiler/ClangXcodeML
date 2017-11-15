% ClangXMLの要素
% XcalableMP/Omni Compiler Project
%

# `Program`要素

| `<Program`
|   `source="` _パス名_ `"`
|   `language=` `"C++"` | `"C"`
|   `time="` _時刻_ `"`
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

## `CXXConstructor`: コンストラクター定義

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

## `Function`: 関数定義

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
|   `class="` _属性_ `"`
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
|   `member="` _メンバー名_ `"`
|   `>`
|   _`clangStmt`要素_
| `</clangStmt>`


