# 逆変換ツールの各ソースコードについて


## LibXMLUtil.h, LibXMLUtil.cpp

libxml を用いて XcodeML を容易に解析するためのユーティリティライブラリ。

## XMLString.h, XMLString.cpp

libxml の文字列(`xmlChar*`)を C++で容易に扱うための
ラッパーを定義している部分。

## Stream.h, Stream.cpp

`CXXCodeGen::Stream`クラスを定義している部分。
`CXXCodeGen::Stream`は、C/C++プログラムを出力するのに便利なストリームのクラスである。

## StringTree.h, StringTree.cpp

`CXXCodeGen::StringTree`クラスを定義している部分。
`CXXCodeGen::StringTree`は、文字列の高速な連接機能を提供する。

## SourceInfo.h, SourceInfo.cpp

`SourceInfo`クラスを定義している部分。
入力された XML の構造すべてにアクセスできる XPath コンテキスト情報と、
そこから解析された`XcodeMl::TypeTable`情報(後述)・`XcodeMl::NnsTable`情報(後述) を
束ねたデータ構造である。

## Util.h

便利なテンプレートを集めたヘッダーファイル。

## XcodeMlNns.h, XcodeMlNns.cpp

`XcoedMl::Nns`クラスを定義している部分。
`XcodeMl::Nns`は、(XcodeML/C++の定義する)NNSを表現する。

## XcodeMlName.h, XcodeMlName.cpp

XcodeMl::Nameクラスを定義している部分。
XcodeMl::Nameは、識別子、変換関数ID、コンストラクター名など、
C++が定義する各種の名前を表現する。

## XcodeMlOperator.h, XcodeMlOperator.cpp

XcodeMLが定義する演算子名とC++の演算子記号の対応関係を取り扱う部分。

## XcodeMlType.h, XcodeMlType.cpp

`XcodeMl::Type`クラスを定義している部分。
`XcodeMl::Type`は、後述する`XcodeMl::TypeTable`と合わせて
XcodeML で定義されるデータ型を表現する。

## XcodeMlTypeTable.h, XcodeMlTypeTable.cpp

`XcodeMl::TypeTable`クラスを定義している部分。
`XcodeMl::TypeTable`は、データ型識別名と実際のデータ型との
対応関係に関する情報を保存している。

## XcodeMlUtil.h, XcodeMlUtil.cpp

XcodeMLの定義する構造に従ったXMLを解析するために便利な関数を集めたファイル。

## XMLWalker.h

`XMLWalker`クラステンプレートを定義しているヘッダーファイル。
XML の各要素を処理する際、
要素の種類に合わせて別々の処理を行うことが必要になる場合がある。
`XMLWalker`はこれを実現する。
後述する `NnsAnalyzer`、`TypeAnalyzer`、`CodeBuilder` で
XML の各部分を処理するために使われている。

## AttrProc.h

`AttrProc`クラステンプレートを定義しているヘッダーファイル。
`AttrProc`を使うことで、与えられた XML の各要素に対し、
その属性(XML attribute)に応じた柔軟な処理を行うことができる。
後述する`ClangDeclHandler`、`ClangNestedNameSpecHandler`、`ClangStmtHandler`、
`ClangTypeLocHandler`で、
XMLの各要素を処理する ために使われている。

## TypeAnalyzer.h, TypeAnalyzer.cpp

XML の`<xcodemlTypeTable>`部を解析して
データ型識別名と実際のデータ型との対応関係を管理する部分。

## NnsAnalyzer.h, NnsAnalyzer.cpp

XML の`<xcodemlNnsTable>`部を解析して
NNS識別名と実際のNNSとの対応関係を管理する部分。

## ClangDeclHandler.h, ClangDeclHandler.cpp

入力されたXML文書に含まれる`<clangDecl>`要素を解析して
C/C++プログラムを出力する部分。

## ClangNestedNameSpecHandler.h, ClangNestedNameSpecHandler.cpp

入力されたXML文書に含まれる\<clangNestedNameSpecifier\>要素を解析して
C/C++プログラムを出力する部分。

## ClangStmtHandler.h, ClangStmtHandler.cpp

入力されたXML文書に含まれる\<clangStmt\>要素を解析して
C/C++プログラムを出力する部分。

## ClangTypeLocHandler.h, ClangTypeLocHandler.cpp

入力されたXML文書に含まれる\<clangTypeLoc\>要素を解析して
C/C++プログラムを出力する部分。

## CodeBuilder.h, CodeBuilder.cpp

XML の各要素を解析して
その要素名に応じて適切な C/C++プログラムを出力する部分。

## XcodeMLtoCXX.cpp

`main`関数部分。
コマンドライン引数として与えられたファイル名が表す
XML 文書を読み、
上記各 Walker、Handler を用いて C/C++プログラムを出力する。
