# /XcodeMLtoCXX/src/下のファイル


## AttrProc.h

AttrProcクラステンプレートを定義する。
AttrProcは、XML要素を読みながら何らかの処理を行うが、その際要素の属性に応じて処理を切りかえる。

## ClangClassHandler.{h,cpp}

入力されたXML文書に含まれるclangStmt、clangDecl要素を処理してC/C++プログラムを出力する。

## CodeBuilder.{h,cpp}

入力されたXML文書に含まれる要素のうち、clangStmt、clangDecl以外の要素を処理してC/C++プログラムを出力する。

## LibXMLUtil.{h,cpp}

libxmlに関わるユーティリティライブラリ。

## Makefile

(Makefile)

## NnsAnalyzer.{h,cpp}

入力されたXML文書に含まれるnnsTable要素を読み、NNS識別名からNNSへのマップを構築する。

## Stream.{h,cpp}

CXXCodeGen::Streamを定義する。
CXXCodeGen::Streamは、C/C++プログラムを出力するのに便利なストリームのクラスである。

## StringTree.{h,cpp}

CXXCodeGen::StringTreeを定義する。
CXXCodeGen::StringTreeは、連接が高速にできる文字列のクラスである。

## SourceInfo.h

SourceInfoを定義する。
SourceInfoは、入力されたXML文書を読んでC/C++プログラムを出力する際に必要となる情報を保持する。

## Symbol.h

SymbolMapを定義する。
SymbolMapは、プログラムのある場所から可視である全ての識別子とその型(データ型識別名)に関する情報を保持する。

## SymbolAnalyzer.{h,cpp}

入力されたXML文書に含まれるglobalSymbols要素を読み、識別子からデータ型へのマップを構築する。

## SymbolBuilder.{h,cpp}

入力されたXML文書に含まれるglobalSymbols要素を読み、(特にtypedef宣言や構造体定義などの)C/C++プログラムを出力する。

## TypeAnalyzer.{h,cpp}

入力されたXML文書に含まれるtypeTable要素を読み、データ型識別名から型(XcodeMl::Type)へのマップを構築する。

## XMLString.{h,cpp}

XMLStringを定義する。
XMLStringは、libxmlの文字列(xmlChar)をC++で扱うためのラッパークラスである。

## XMLWalker.h

XMLWalkerクラステンプレートを定義する。
XMLWalkerは、XML要素を再帰的に読みながら何らかの処理を行うが、その際要素名に応じて処理を切りかえる。

## XcodeMLtoCXX.cpp

main関数を定義する。

## XcodeMlEnvironment.{h,cpp}

XcodeMl::Environmentを定義する。
XcodeMl::Environmentは、データ型識別名から型(XcodeMl::Type)へのマップを表現する。

## XcodeMlNns.{h,cpp}

XcoedMl::Nnsを定義する。
XcodeMl::Nnsは、(XcodeML/C++の定義する)NNSを表現する。

## XcodeMlType.{h,cpp}

XcodeMl::Typeを定義する。
XcodeMl::Typeは、C++の型を表現する。
