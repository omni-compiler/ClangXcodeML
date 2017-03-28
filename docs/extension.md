ClangXcodeML拡張
================

## 独自属性

### is\_access\_declaration属性

usingDecl要素はis\_access\_declarationを持ってもよい。
その属性値は、usingDecl要素がアクセス宣言(access-declaration)を表現するとき真("1"または"true")、
そうでないとき偽("0"または"false")である。
省略時の値は偽である。

### is\_const属性、is\_volatile属性(functionType要素)

メンバー関数の型を表現するfunctionType要素はis\_const属性、is\_volatile属性を持ってもよい。
その属性値は、それぞれconst、volatileメンバー関数であるとき真("1"または"true")、
そうでないとき偽("0"または"false")である。
省略時の値は偽である。

### is\_pure属性

非staticメンバー関数を表現するfunctionDecl要素はis\_pure要素を持ってもよい。
その属性値は、純粋virtualメンバー関数であるとき真("1"または"true")、
そうでないとき偽("0"または"false")である。
省略時の値は偽である。

### is\_static\_data\_member属性

クラスのデータメンバーを表現するvarDecl要素はis\_static\_data\_member属性を持ってもよい。
その属性値は、staticデータメンバーを表現するとき真("1"または"true")、
そうでないとき偽("0"または"false")である。
省略時の値は偽である。

### is\_this\_declaration\_a\_definition属性

classDecl要素はis\_this\_declaration\_a\_definition属性を持ってもよい。
その属性値は、クラス宣言が定義本体を持つとき真("1"または"true")、
そうでないとき偽("0"または"false")である。
省略時の値は偽である。

### language\_linkage属性

functionDecl要素・functionDefinition要素・varDecl要素はlanguage\_linkage属性を持ってもよい。
その属性値は"C"または"C++"である。
省略時の値は"C++"である。

### nns属性(memberRef要素)

修飾子付きの名前へのクラスメンバーアクセス(例: `a->T::x`)を表現するmemberRef要素はnns属性を持つ。
その属性値は対応するNNS識別名である。

### nns属性(Var要素)

修飾子付きの変数参照を表現するVar要素はnns属性を持つ。
その属性値は対応するNNS識別名である。

### parent\_class属性

メンバー関数を表現するfunctionDecl要素・functionDefinition要素はparent\_class属性を持つ。
その属性値は所属するクラスのデータ型識別名である。

constructor要素([-@sec:decl.ctor])またはdescructor要素([-@sec:decl.dtor])を持つ
functionDecl要素・functionDefinition要素から識別子を復元するのに使われている。

### type属性(functionDecl要素・functionDefinition要素・varDecl要素)

functionDecl要素・functionDefinition要素・varDecl要素はtype属性を持ってもよい。
その属性値は関数(または変数)の型に対応するデータ型識別名である。
省略された場合、name要素(またはoperator要素・constructor要素・destructor要素)
および入力されたXML文書のもつsymbols要素・globalSymbols要素から適切なデータ型識別名を決定する。
適切なデータ型識別名が一意に決定できない場合はエラーである。

## clangDecl要素とclangStmt要素

Clang ASTの定義するノードであって、XcodeML/C++に対応する要素を持たないものを表現する。

### `clangDecl[@class="Friend"]`

friend宣言を表現する。

friend関数の宣言に対応するclangDecl要素は次の形式に従う。

| `<clangDecl class="Friend">`
|    functionDecl要素 or functionDefinition要素
| `</clangDecl>`

friendクラスの宣言に対応するclangDecl要素は次の形式に従う。

| `<clangDecl class="Friend">`
|   `<typeLoc type=` データ型識別名 `>`
| `</clangDecl>`

### `clangStmt[@class="CXXMemberCallExpr"]`

| `<clangStmt class="CXXMemberCallExpr">`
|   式の要素
|   [ 式の要素
|    ... ]
| `</clangStmt>`

非staticメンバー関数の呼び出しを表現する。

### `clangStmt[@class="CXXTemporaryObjectExpr"]`

| `<clangStmt class="CXXTemporaryObjectExpr`
|   `type=` データ型識別名 `>`
|   `<typeLoc/>`
|   [ 式の要素
|     ... ]
| `</clangStmt>`

C++における関数形式の明示的型変換を表現する。

## 独自要素

### classDecl要素

| `<classDecl>`
|   [ 宣言の要素 ([-@sec:decl])
|   ... ]
| `</classDecl>`

クラス宣言を表現する。

### constructorInitializerList要素

| `<constructorInitializerList>`
|   [ constructorInitializerList要素
|   ... ]
| `</constructorInitializerList>`

C++のmem-initializer-list(メンバー初期化子リスト)を表現する。

### constructorInitizlier要素

C++のmem-initializer(メンバー初期化子)を表現する。

非staticデータメンバーのmem-initializerに対応する
constructorInitizlier要素は次の形式に従う。

| `<constructorInitializer`
|   `member=` メンバー名 `>`
|   式の要素
| `</constructorInitializer>`

基本クラスのmem-initializerに対応する
constructorInitizlier要素は次の形式に従う。

| `<constructorInitializer`
|   `type=` データ型識別名 `>`
|   式の要素
| `</constructorInitializer>`
