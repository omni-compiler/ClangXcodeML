# /XcodeMLtoCXX/src/下のファイル


## LibXMLUtil.h, LibXMLUtil.cpp

libxml を用いて XcodeML を容易に解析するためのユーティリティライブラリ。

## XMLString.h, XMLString.cpp

libxml の文字列(xmlChar\*)を C++で容易に扱うための
ラッパーを定義している部分。

## Stream.h, Stream.cpp

CXXCodeGen::Streamクラスを定義している部分。
CXXCodeGen::Streamは、C/C++プログラムを出力するのに便利なストリームのクラスである。

## StringTree.h, StringTree.cpp

CXXCodeGen::StringTreeクラスを定義している部分。
CXXCodeGen::StringTreeは、連接が高速にできる文字列のクラスである。

## SourceInfo.h

SourceInfo クラスを定義しているヘッダーファイル。
入力された XcodeML の構造すべてにアクセスできる XPath コンテキスト情報と、
そこから解析された XcodeML::Environment 情報(後述)・SymbolMap 情報(後述) を
束ねたデータ構造である。

## Symbol.h

SymbolMap 型を定義しているヘッダーファイル。
SymbolMap は、XcodeML の表す抽象構文木のある位置から見える名前と、
その名前が表す変数の型との対応関係に関する情報を保存している。
XcodeML における name 要素はデータ型に関する情報を必ずしも持たないため、
SymbolMap は C/C++ プログラムを出力する際に変数の型情報を得るのに使われて
いる。

## XcodeMlNns.h, XcodeMlNns.cpp

XcoedMl::Nnsクラスを定義している部分。
XcodeMl::Nnsは、(XcodeML/C++の定義する)NNSを表現する。

## XcodeMlType.h, XcodeMlType.cpp

XcodeMl::Type クラスを定義している部分。
XcodeMl::Type は、後述する XcodeMl::Environment と合わせて
XcodeML で定義されるデータ型を表現する。

## XcodeMlEnvironment.h, XcodeMlEnvironment.cpp

XcodeMl::Environment クラスを定義している部分。
XcodeMl::Environment は、データ型識別名と実際のデータ型との
対応関係に関する情報を保存している。

## XMLWalker.h

XMLWalker クラステンプレートを定義しているヘッダーファイル。
XML の各要素を処理する際、
要素の種類に合わせて別々の処理を行うことが必要になる場合がある。
XMLWalker はこれを実現する。
後述する SymbolAnalyzer、TypeAnalyzer、CodeBuilder で
XcodeML の各部分を処理するために使われている。

## AttrProc.h

AttrProc クラステンプレートを定義しているヘッダーファイル。
AttrProc を使うことで、与えられた XML の各要素に対し、
その属性(XML attribute)に応じた柔軟な処理を行うことができる。
後述する SymbolBuilder で、XcodeML の\<globalSymbols\>部の要素を処理する
ために使われている。

## TypeAnalyzer.h, TypeAnalyzer.cpp

XcodeML の\<typeTable\>部を解析して
データ型識別名と実際のデータ型との対応関係を管理する部分。

## NnsAnalyzer.h, NnsAnalyzer.cpp

XcodeML の\<nnsTable\>部を解析して
NNS識別名と実際のNNSとの対応関係を管理する部分。

## SymbolAnalyzer.h, SymbolAnalyzer.cpp

XcodeML の\<globalSymbols\>, \<symbols\>部を解析して
プログラム中の名前とそのデータ型との対応関係を管理する部分。

## SymbolBuilder.h, SymbolBuilder.cpp

XcodeML の\<globalSymbols\>, \<symbols\>部を解析して
C/C++プログラムにおける宣言(グローバル変数宣言、クラス・構造体定義など)部を
出力する部分。

## ClangClassHandler.h, ClangClassHandler.cpp

入力されたXML文書に含まれる\<clangStmt\>, \<clangDecl\>要素を解析して
C/C++プログラムを出力する部分。

## CodeBuilder.h, CodeBuilder.cpp

XcodeML の\<globalDeclarations\>部を解析して
C/C++プログラムを出力する部分。

## XcodeMLtoCXX.cpp

main 関数部分。
コマンドライン引数として与えられたファイル名が表す
XcodeML 文書を読み、
上記各 Walker を用いて C/C++プログラムを出力する。
