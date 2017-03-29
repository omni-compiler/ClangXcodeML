# /CXXtoXML/src/下のファイル

## XSLTs/

## CXXtoXML.{h,cpp}

main 関数部分。
与えられたコマンドラインから AST を構成し、
上記各 Visitor にAST を渡す部分。

## ClangOperator.{h,cpp}

オーバーロードされた演算子の種類をXcodeMLでの演算子名に変換するための
OverloadedOperatorKindToString関数を持つ。

## ClangUtil.{h,cpp}

## DeclarationsVisitor.{h,cpp}

clang の AST から XcodeML の \<globalDeclarations\> 部
および \<declarations\> 部を生成する部分。
この部分がCtoXcodeMLの中でもっとも大きな部分を占める。

## Hash.h

## InheritanceInfo.{h,cpp}

C++のクラス・構造体の継承関係の情報を扱う部分。

## Makefile

## NnsTableInfo.{h,cpp}

## TypeTableInfo.{h,cpp}

## XMLRAV.{h,cpp}

clang の libtooling ライブラリ内の
RecursiveASTvisitor.h を利用したクラスを実装している部分。
RAVBidirBridge をつかってclass XcodeMlVisitorBase との間で
双方向に橋渡しをしている。

## XMLVisitorBase.{h,cpp}

下記の各種の \*Visitor.cpp, \*Visitor.h の実装の基底クラスである
class XcodeMlVisitorBase を準備している部分。
意味的にはこのさらに上位に CRTPパターンで書かれた
RecursiveASTvisitor クラスを基底に持つのだが、
RecursiveASTvisitorクラスは大量のメソッドを持つため、
本当に各種の\*Visitorの基底クラスとして実装すると
コンパイル時間が何倍にもなるので、
pimpl イディオム相当の class RAVBidirBridge をつかって、
下記のXcodeMlRAV のほうに RecursiveASTvisitor の実装の部分を隠蔽している。
