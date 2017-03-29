# 正変換ツールの各ソースコードについて

## XMLVisitorBase.h, XMLVisitorBase.cpp

DeclarationsVisitor.cpp, DeclarationsVisitor.h の実装の基底クラスである
class XcodeMlVisitorBase を準備している部分。
意味的にはこのさらに上位に CRTPパターンで書かれた
RecursiveASTvisitor クラスを基底に持つのだが、
RecursiveASTvisitorクラスは大量のメソッドを持つため、
本当にDeclarationsVisitorの基底クラスとして実装すると
コンパイル時間が何倍にもなるので、
pimpl イディオム相当の class RAVBidirBridge をつかって、
下記のXcodeMlRAV のほうに RecursiveASTvisitor の実装の部分を隠蔽している。

## XMLRAV.h, XMLRAV.cpp

clang の libtooling ライブラリ内の
RecursiveASTvisitor.h を利用したクラスを実装している部分。
RAVBidirBridge をつかってclass XcodeMlVisitorBase との間で
双方向に橋渡しをしている。

## DeclarationsVisitor.h, DeclarationsVisitor.cpp

clang の AST から XcodeML の \<globalDeclarations\> 部
および \<declarations\> 部を生成する部分。
この部分がCXXtoXMLの中でもっとも大きな部分を占める。

## Hash.h

clang AST の QualType を C++ 標準ライブラリの std::hash で用いるための
テンプレートを定義する。

## TypeTableInfo.h, TypeTableInfo.cpp

clang AST の QualType で示された値 (型の種別情報) とXcodeML のデータ型識別名との対応関係を管理する部分。

## InheritanceInfo.h, InheritanceInfo.cpp

C++のクラスの継承関係の情報を扱う部分。

## NnsTableInfo.h, NnsTableInfo.cpp

Clang AST の NestedNameSpecifier で示された値 (nested-name-specifierの種別情報) と XcodeML のNNS識別名との対応関係を管理する部分。

## ClangOperator.h, ClangOperator.cpp

Clang で定義された演算子の種類をXcodeMLでの演算子名に変換するための
ユーティリティ関数を定義する。

## ClangUtil.h, ClangUtil.cpp

Clang で定義された各種のオブジェクトを CXXtoXML で用いるための
ユーティリティ関数を定義する。

## CXXtoXML.h, CXXtoXML.cpp

main 関数部分。
与えられたコマンドラインから AST を構成し、
DeclarationsVisitor にAST を渡す部分。
