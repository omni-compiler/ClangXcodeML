ClangXcodeML拡張
================

## 独自属性

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
