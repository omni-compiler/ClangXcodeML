% ClangXMLの要素
% XcalableMP/Omni Compiler Project
%

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
