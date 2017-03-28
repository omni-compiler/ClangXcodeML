% XcodeML/C++ 仕様書
  V1.3alpha J
% XcalableMP/Omni Compiler Project
%

---
linkReferences: True
---

改版履歴
XcodeML/C Version 0.91J

* 配列要素の参照のXML要素を変更。
* subArrayRef要素を変更。
* indexRange要素を追加。

XcodeML/C++ draft 0.1J

* C++対応ドラフト初版

XcodeML/C++ 1.0J

* C++対応初版。2015/10/15

XcodeML/C++ 1.1J

* 二章を修正(正規化を加筆、fullName属性について)。2016/3/25

XcodeML/C++ 1.2J

* 名前空間やテンプレートの表現方法について大幅改定作業を開始。2016/9/6


# はじめに {#sec:intro}
この仕様書は、プログラミング言語CおよびC++に対してXcalableMP拡張をほどこした言語を取り扱うための中間表現形式であるXcodeMLを記述する。
XcodeMLは、以下の特徴を持つ。

* CまたはC++の一つの翻訳単位(Translation Unit)のプログラムを入力にとり、各種の情報をXML形式で表現する。
  これにより、各種のプログラム変換処理を行いやすくするとともに、人間にとっても可読(human-readable)なフォーマットを持つ。
* ソースコードを意味的(semantic)に表現した抽象構文木構造を保持する。
  これにより、XcodeMLをソースコードに再度変換することができる。
  ここで、「意味的」としている理由は、プログラム変換処理が取り扱う必要のないソースコードの書き方の違いは、XcodeML上では表現しないからである。
* ソースコード上の名前空間の情報を、抽象構文木構造から独立した形で表現するとともに、抽象構文木内の名前それぞれについて、その名前がどの名前空間に所属するものであるかを付加する。
* ソースコード上のの型情報を、抽象構文木構造から独立した形で表現するとともに、抽象構文木内の式それぞれについて、その式が何型として扱われているかを付加する。
* ソースコード上の各種シンボルのスコープ範囲が明確となる構造を取る。
* C++のtemplateについて、ソースコード上の構文構造そのものを表現するとともに、その翻訳単位内で実際に用いられたテンプレート引数に基づいた展開後の結果も表現する。
  これにより、XcodeMLからC++コードへの変換を可能とするとともに、各種のプログラム変換処理においてテンプレート展開後の姿を対象にした処理も可能となるようにしている。

# 翻訳単位と`XcodeProgram`要素 {#sec:program}
ソースファイルに、`#include`指定されたファイルを再帰的にすべて展開したものを、翻訳単位と呼ぶ。
翻訳単位は`XcodeProgram`要素で表現される。下記の XML schema で定義される。


	  <xsd:element name="XcodeProgram">
		<xsd:complexType>
		  <xsd:sequence>
			<xsd:element minOccurs="0" maxOccurs="1" ref="nnsTable" />
			<xsd:element minOccurs="1" maxOccurs="1" ref="typeTable" />
			<xsd:element minOccurs="1" maxOccurs="1" ref="globalSymbols" />
			<xsd:element minOccurs="1" maxOccurs="1" ref="globalDeclarations" />
		  </xsd:sequence>
		  <xsd:attribute name="compiler-info" use="optional" />
		  <xsd:attribute name="version" use="optional" />
		  <xsd:attribute name="time" use="optional" />
		  <xsd:attribute name="language" use="optional" />
		  <xsd:attribute name="source" use="optional" />
		</xsd:complexType>
	  </xsd:element>



XcodeMLファイルのトップレベルのXML要素は、`XcodeProgram` 要素である。`XcodeProgram` 要素は以下の子要素を含む。

* `nnsTable` (C++のみ)　– 翻訳単位で利用されている名前空間の情報([-@sec:nns.attr]節)
* `typeTable`要素　– プログラムで利用されているデータ型の情報([-@sec:type]章)
* `globalSymbols`要素 – プログラムで利用されている大域変数の情報([-@sec:symb.global]節)
* `globalDeclarations` 要素 – 関数、変数宣言などの情報([-@sec:decl.global]節)

`XcodeProgram`要素は、属性として以下の情報を持つことができる。

* `compiler`-info　－　CtoC コンパイラの情報
* `version`　－　CtoC コンパイラのバージョン情報
* `time`　－　コンパイルされた日時
* `language`　－　ソース言語情報(Cの場合は "`C`"、C++の場合は "`C++`")
* `source`　－　ソース情報

## ソースコードの正規化 {#sec:program.norm}
XcodeMLの設計方針は、XcodeMLで表現されたプログラムを入力にとって各種の解析処理をおこなった上で処理結果をXcodeMLとして出力するという処理(以下、この種の処理を「XcodeML解析処理」と呼ぶことにする)に適した構造を持つ、ということを主眼においている。
このため、プログラムの意味が同一となるような記述方法が複数存在する場合、そのいずれかに「正規化」して扱うという設計方針をおいている。
具体的には次のとおりである。

* XcodeMLの木構造が演算子の結合性を反映するため、ソースコード上に明示的に書かれた丸カッコはXcodeMLでは明示的には表現しない。
  つまり、ソースコード上で`x`という式を表現するXcodeMLとその式をカッコでかこんだ `(x)` という式を表現するXcodeMLは同一のものとなる。
* 組み込み型に対する単項プラス演算子はXcodeMLでは表現しない。
  (C++の演算子オーバーロードを用いた式は演算子の一種とはみなされず関数呼び出しの一種として表現されるため、これについては単項プラス演算子も明示的に表現される)
* 宣言は`compoundStatement`要素の先頭でのみ取り扱う。
  すなわち、ソースコード上で複文の途中に新たな変数宣言が出現した場合には、その位置から複文末尾までの範囲を囲う`compoundStatement`を生成し、その先頭に配置する。
* データ型の宣言のうち、下記に述べる「複雑な型」に該当しないものについては、その宣言内容は`typeTable`で表現し、`globalDeclarations`要素の中には持ち込まない。
  これにより、XcodeML解析処理部が新たな型をXcodeMLに追加する際、そのソースコード上での配置を考える必要がない。
  これに対し、下記にのべるような「複雑な型」については、その宣言を包んでいる構文要素(主に`compoundStatement`要素)内で`localTypeTable`要素を用いて表現する方向で検討中である。
    * ローカルクラス
    * 入れ子のクラス(クラス内に別のクラス宣言を持つ場合、その両方ともが複雑な型とみなされる※要検討)
    * 無名クラス
    * テンプレート(展開前、すなわちテンプレートパラメータを持つ構文内での定義)
* C++のクラス(構造体、共用体を含む)のメソッドの定義は、クラス宣言内で書く方法と、クラス宣言の外で書く方法の二種類の定義方法がある。
  これについては、すべて「クラス宣言の外に書く方法」に相当する形のXcodeMLとして表現する。
  前述の「複雑な型」でなければ`globalDeclarations`要素に、「複雑な型」の場合にはその宣言を包んでいる構文要素の子要素の`declarations`に、それぞれ必要な定義が所属することになる。
* クラスのメンバや親クラスについてのアクセス指定子は、個々のメンバ・親クラスに対してそれぞれ指定されている形で扱う。
  すなわち、ソースコード上に実際にどこにアクセス指定子が書かれていたかはXcodeMLでは表現せず、各メンバにどのようなアクセス指定がかかっていたかのみを表現する。
* 名前空間を用いた名前については、次の節で解説する`nns`属性によって修飾子を表現する形で扱う。
    * 名前の定義については、構文的な`namespace｛｝`の入れ子構造はXcodeMLでは表現せず、個々の名前がどのようなnamespaceに所属すべきかのみを表現していることになる。
    * 名前の使用については、`using namespace`による名前空間のインポートをXcodeMLでは表現せず、全ての名前を修飾子つきの形で扱う。
* 演算子オーバーロードを用いた式については、XcodeMLでは演算子としては扱わず、 `operator` キーワードを明示指定した関数呼び出しに相当する形で扱う。
  同じ演算子に対するオーバーロード方法にはクラスメソッドの形のものとグローバル関数の形のものがあり、このどちらの呼び出しになっているかが明示される形になる。
  また、それぞれ`nns`属性がつくので、ADRによる名前空間検索の解決結果も反映される。

備考: [-@sec:expr.pointer]節に出現する「`pointerRef`と`varAddr`をまとめて`Var`として表現」する話なども正規化の一種として考えることができそうである。

これは「XcodeMLとしてはどちらの表現も可能」なので、ここで解説している正規化とは少し位置づけが違うようにも見えるが、上記で述べた「複雑な型」を表現するためには`globalDeclarations`だけで任意の型を表現できる必要があるので、
結局「正規化せずに全て`globalDeclarations`だけで扱う」表現と「正規化できるものは正規化して表現する」のどちらの表現も可能ということになる。
つまり、「まずは構文の木構造をそのまま反映しただけのXcodeML(正規化されていない)」と、「XcodeML解析処理に適した正規化がほどこされたあとのXcodeML」を考えて、前者の仕様と後者への正規化処理の仕様に分けて記述した方が厳密な議論ができるように思われる。
ただし、下記の`nns`の話やoperator呼び出しの種類、`decltype`による型などは、ソースコードには書かれていないがClangAST的には解決結果が保持されているので、これらは「正規化されていないXcodeML」の時点で情報を付与しておくべきである。
また、解決結果だけがあれば情報は足りているので、「正規化されていないXcodeML」がもともとのソースコード上の構文的な構造を完全に反映する必要があるわけではない。
このように考えると、「フェーズ1の正規化」と「フェーズ2の正規化」があり、フェーズ1の正規化は必ずおこなう(XcodeMLとして二種類の表現をするコースがそもそも準備されない)、という風に考える必要がある。

## `value`要素 {#sec:program.value}
`globalDeclarations`要素、`declarations`要素中で、初期化式を持つ変数宣言を表現する際の初期値の式を表現する。


| `<value>`
|   [ 式の要素([-@sec:expr]章) or　`value`要素
|   … ]
| `</value>`


属性: なし

備考:1.0版では`symbols`要素中でも用いることになっていたためこの節が[-@sec:program]章におかれたのだと考えられるが、
C\_Front実装でもCtoXcodeML実装でも`symbols`属性内では`value`要素を用いない(つまり初期化式は`globalDeclarations`要素や`declarations`要素の中で出現するのみである)ので、この節は[-@sec:program]章に置く必要がなく、[-@sec:decl]章以降に配置するのが適切である。

`{ }`で囲まれた式の並びは、`value`要素のネストで表現する。

例:

int型の初期値 `1` に対応する表現は次のとおりになる。

    <value>
      <intConstant type="int">1</intConstant>
    </value>

int型配列の初期値 `{ 1, 2 }` に対応する表現は次のとおりになる。

    <value>
      <value>
        <intConstant type="int">1</intConstant>
        <intConstant type="int">2</intConstant>
      </value>
    </value>

# `typeTable`要素とデータ型定義要素 {#sec:type}
`typeTable`要素は、翻訳単位([-@sec:program]章)に対して一つだけ存在し、翻訳単位で使われているすべてのデータ型についての情報を定義する。

| `<typeTable>`
|   [ データ型定義要素
|   … ]
| `</typeTable>`


属性(optional): なし

`typeTable`要素は、翻訳単位を表現する`XcodeProgram`要素([-@sec:program]章)の直接の子要素であり、データ型を定義するデータ型定義要素の列からなる。データ型定義要素には以下の要素がある。

* `basicType`要素([-@sec:type.basic]節)
* `pointerType`要素([-@sec:type.ptr]節)
* `functionType`要素([-@sec:type.func]節)
* `arrayType`要素([-@sec:type.array]節)
* `unionType`要素([-@sec:type.union]節)
* `structType`要素と`classType`要素([-@sec:type.struct]節)
* `enumType`要素([-@sec:type.enum]節)
* `typeInstance`要素([-@sec:temp.funcinstance]節)
* `classTemplate`要素([-@sec:temp.class]節)
* `aliasTemplate`要素([-@sec:temp.alias]節)

すべてのデータ型定義要素は、型識別名([-@sec:type.ident]節)を表す`type`要素をもつ。
データ型定義要素は、データ型定義要素属性([-@sec:type.attr]節)をもつことができる。

要検討:`decltype`対応

`decltype(式)`は式の型を表すが、式はスコープをもつので`typeTable`の中に移動することができない。

* 案1: 式の中のすべての名前に、スコープ名を付ける。`decltype(main:x + main:y)` など。→とても煩雑。scopenameを持たない`{ }`の中に出現した場合は？
* 案2: `typeTable`を翻訳単位に一つにするのではなく、スコープ毎にもつようにする。
* 案3: `decltype`が出現したスコープに限り、`typeTable`をもつ。

　Clang ASTでは「型推論の結果の型」をAST上に保持しているので、その仕組みに合わせて考えるのであれば、すべて「解決結果の型」を扱えばよいことになる。これも「正規化」の一種と考えて扱うのがよいかもしれない。

## データ型識別名 {#sec:type.ident}
プログラム内において、データ型はデータ型識別名で区別される。その名前は、次のいずれかである。

* 基本データ型
* C, C++の基本データ型(C++拡張)

    `void`, `char`, `short`, `int` , `long`, `long_long`, `unsigned_char`, `unsigned_short`, `unsigned`, `unsigned_long`, `unsigned_long_long`, `float`, `double`, `long_double`, `wchar_t`, `char16_t`, `char32_t`, `bool` (`_Bool`型)

* `_Complex`、`_Imaginary`に対応する型

    `float_complex`, `double_complex`, `long_double_complex`, `float_imaginary`, `double_imaginary`, `long_double_imaginary`

* GCCの組み込み型

    `__builtin_va_arg`

* 型の抽象(C++) —　テンプレートの型仮引数の型の名前

    `any_class`, `any_typename`

* 派生データ型とクラス

他のデータ型識別名とは異なる、翻訳単位内でユニークな英数字の並び。

### `typeName`要素 {#sec:type.typename}

| `<typeName/>`

属性(必須): `ref`

属性(optional): `access`

以下の属性をもつことができる。

* `ref`属性　—　データ型識別名を示す。
* `access`属性　—　`inheritedFrom`要素の子要素のときだけ使用する。`public`, `private`または`protecded`のいずれかの値をとる。

`typeName`要素は以下のように使用される。

* 型を引数とする関数の呼出しで
    * `sizeOfExpr`([-@sec:expr.unary]節)、`gccAlignOfExpr`([-@sec:expr.unary]節)、`builtin_op`([-@sec:other.builtinop]節)
* テンプレートの定義の型仮引数として([-@sec:temp]章)
* テンプレートのインスタンスの型実引数として([-@sec:temp.instance]章)
* 構造体とクラスの継承元([-@sec:type.class.inherit]項)

例:

式`sizeof(int)`は以下のように表現される。

    <sizeOfExpr>
    　　<typeName ref="int"/>
    </sizeOfExpr>

備考:`typeName`属性を`typeTable`以外でも用いるのであれば、この節は[-@sec:program]章に置くべきでは？
あるいは、そもそも中身の構造が違うのであればまったく別の要素として定義した方がよいのでは。

## データ型定義要素属性 {#sec:type.attr}
データ型定義要素は共通に以下の属性を持つことができる。これらをデータ型定義要素属性と呼ぶ。

* `is_const`　－　そのデータ型がconst修飾子をもつかどうか
* `is_volatile`　－　そのデータ型がvolatile修飾子をもつかどうか
* `is_restrict`　－　そのデータ型がrestrict修飾子をもつかどうか
* `is_static`　－　そのデータ型がstatic属性をもつかどうか
* `access`(C++)　－　アクセス指定子に対応。"`private`", "`protected`"または"`public`"
* `is_virtual`(C++)　—　そのメンバ関数がvirtual属性をもつかどうか。

"`is_`"　で始まる属性の値には、真を意味する`1`と`true`、および、偽を意味する`0`と`false`が許される。属性が省略されたとき、偽を意味する。

## `basicType`要素 {#sec:type.basic}
`basicType` 要素は、他のデータ型識別要素にデータ型定義要素属性を加えた、新しいデータ型定義要素を定義する。

    <basicType/>

属性(必須): `type`, `name`

属性(optional): `alignas`, データ型定義要素属性

以下の属性を持つ。

* `type`　－　この型に与えられたデータ型識別名
* `name`　－　この型の元になる型のデータ型識別名

備考: 旧仕様と実装の違い
本仕様は、旧仕様とは異なり、実装に合わせた。旧仕様では以下のように定義されていた。

> `basicType`要素は、C,C99の基本データ型を定義する。

実装では、データ型定義要素属性(constなど)を持たない基本データ型に対応するデータ型識別要素は定義されず、`type="int"` のようにデータ型識別名だけで表現されている。また、基本データ型以外の型(構造型など)に属性を付ける場合に、`basicType`要素を使用している。

例:

    struct {int x; int y;} s;
    struct s const * volatile p;

は次のXcodeMLに変換される。 `basicType`要素によって、"`struct s const`"を意味するデータ型識別名`B0` が定義されている。

	  <structType type="S0">
		<symbols>
		  <id type="int"><name="x"><./name></id>
		  <id type="int"><name="y"><./name></id>
		</symbols>
	  </structType>
	  <basicType type="B0" is_const="1" name="S0"/>
	  <pointerType type="P0" is_volatile="1" ref="B0"/>

## `pointerType`要素 {#sec:type.ptr}
`pointerType`要素はポインタ型またはリファレンス型を定義する。

| `<pointerType/>`

属性(必須): `type`, `ref`

属性(optional): `reference`, データ型定義要素属性

以下の属性を持つ。

* `type`　－　この型に与えられたデータ型識別名
* `ref`　－　このポインタが指すデータのデータ型識別名
* `reference`　－　
  属性が指定されたとき、この型がリファレンス型であることを表す。
  "`lvalue`"のときlvalueリファレンス型であることを意味し、
  "`rvalue`"のときrvalueリファレンス型であることを意味する。

`pointerType`要素は、子要素を持たない。

例:

"`int *`" に対応するデータ型定義は以下のようになる。

	<pointerType type="P0123" ref="int"/>

例:

    const int& x = 0;

は以下のXcodeMLに変換される。
`basicType`要素によって、"`const int`"を意味するデータ型識別名`B0`が定義されている。

    <basicType type="B0" is_const="1" name="int"/>
    <pointerType type="P0124" ref="B0" reference="lvalue" />

例:

以下の(lvalue)リファレンスの宣言があるとき、

    int& n_alias = n_org;

変数`n_alias`のデータ型識別要素は以下のようになる。

    <pointerType type="P0" ref="int" reference="lvalue"/>

以下のコンストラクタ(ムーブコンストラクタ)の定義の引数に現れたrvalueリファレンスについて、

    struct Array {
      int *p, len;
      Array( Array&& obj ) : p(obj.p), len(obj.len) {
        obj.p = nullptr;  obj.len = 0;
      }
    }

仮引数`obj`のデータ型識別要素は以下のようになる。
ここで"`S0`"は"`Array`"に対応するデータ型識別名として定義されている。

    <pointerType type="B2" ref="S0" reference="rvalue"/>

## `functionType`要素 {#sec:type.func}
`funtionType`要素は、関数型を定義する。

| `<functionType>`
| 　　[ `params`要素([-@sec:decl.params]節) ]
| `</functionType>`

属性(必須): `type`, `return_type`

属性(optional): `is_inline`

* `type`　－　この関数型に与えられたデータ型識別名
* `return_type`　－　この関数型が返すデータのデータ型識別名
* `is_inline`　－　この関数型がinline型であるかどうかの情報、`0` または `1`、`false` または `true` 省略時は`false`を意味する。

プロトタイプ宣言がある場合には、引数のXML要素に対応する`params`要素を含む。

例:

"`double foo(int a,int b)`" の`foo`に対するデータ型は以下のようになる。

    <functionType type="F0457" return_type="double">
        <params>
          <name type="int">a</name>
          <name type="int">b</name>
        </params>
    </fucntionType>

## `arrayType`要素 {#sec:type.array}
`arrayType`要素は、配列データ型を定義する。

| `<arrayType>`
|   [ `arraySize`要素]
| `</arrayType>`

属性(必須): `type`, `element_type`

属性(optional): `array_size`, データ型定義要素属性

`arrayType`要素は以下の属性を持つ。

* `type`　－　この配列型に与えられたデータ型識別名
* `element_type`　－　配列要素のデータ型識別名
* `array_size`　－　配列のサイズ(要素数)。`array_size`と子要素の`arraySize`を省略した場合は、サイズ未指定を意味する。`array_size`属性は子要素の`arraySize`と同時に指定することはできない。

以下の子要素を持つ。

* `arraySize`　－　配列のサイズ(要素数)を表す式。式要素ひとつを子要素に持つ。
  サイズを数値で表現できない場合や、可変長配列の場合に指定する。`arrayType`要素が`arraySize`要素を持つ場合、`array_size`属性の値は"`*`"とする。

例:
"`int a[10]`"の`a`に対する`type_entry`は以下のようになる。

    <arrayType type="A011" element_type="int" array_size="10"/>

## `unionType`要素 {#sec:type.union}
union(共用体)データ型は、`unionType`要素で定義する。

| `<unionType>`
|   `symbols`要素
| `</unionType>`

属性(必須): `type`

属性(optional): データ型定義要素属性

`unionType`要素は以下の属性を持つ。

* `type`　－　この共用体型のデータ型識別名

`unionType`要素は、メンバに対する識別子の情報である`symbols`要素を持つ。 構造体・共用体のタグ名がある場合には、スコープに対応するシンボルテーブルに定義されている。
メンバのビットフィールドは、`id`要素の `bit_field` 属性または `id`要素の子要素 である`bitField` 要素に記述する([-@sec:symb.id]節)。

## `structType`要素 {#sec:type.struct}
構造体を表現する。

| `<structType>`
|   `symbols`要素([-@sec:symb.local]節)
| `</structType>`

属性(必須): `type`

属性(optional):
  `lineno`,
  `file`,
  データ型定義要素属性

以下の子要素をもつ。

* `symbols`要素　－　メンバのリスト

以下の属性をもつ。

* `type`(必須)　－　この構造体に与えられたデータ型識別名

メンバのビットフィールドは、`id`要素の `bit_field` 属性または `id`要素の子要素 である`bitField`要素に記述する([-@sec:symb.id]節)。
構造体またはメンバの名前は、同じ`type`属性をもつ`id`要素で指定する。

例:

以下の構造体宣言

    struct {
      int x;
      int y : 8;
      int z : sizeof(int);
    };

に対する`structType`要素は以下のようになる。この構造体のデータ型識別名は`S0`と定義された。

      <stuctType type="S0">
        <symbols>
          <id type="int">
            <name>x</name>
          </id>
          <id type="int" bit_field="8">
            <name>y</name>
          </id>
          <id type="int" bit_field="*">
            <name>z</name>
            <bitField>
              <sizeOfExpr>
                <typeName ref="int"/>
              </sizeOfExpr>
            </bitField>
          </id>
         </symbols>
      </structType>



## `classType`要素(C++) {#sec:type.class}
クラスを表現する。

| `<classType>`
| 　 [ `inheritedFrom`要素([-@sec:type.class.inherit]) ]
|   `symbols`要素([-@sec:symb.local]節)
| `</classType>`

属性(必須): `type`

属性(optional):
  `lineno`,
  `file`,
  `inheritedFrom`,
  データ型定義要素属性

以下の子要素をもつ。

* `inheritedFrom`要素　－　基本クラス名のリスト
* `symbols`要素([-@sec:symb.local]節)　－　
  メンバー関数名およびメンバー変数名のリスト。
  定義されるクラスが基本クラスを持つ場合、
  そのpublicメンバー関数および変数も含む。

以下の属性をもつ。

* `type`(必須)　－　このクラスに与えられたデータ型識別名

メンバのビットフィールドは、`id`要素の `bit_field` 属性または `id`要素の子要素 である`bitField`要素に記述する([-@sec:symb.id]節)。
構造体またはメンバの名前は、同じ`type`属性をもつ`id`要素で指定する。typedef文またはusing文で指定された別名もまた、同じ`type`属性をもつ`id`要素で指定する。


### inheritedFrom要素(C++) {#sec:type.class.inherit}
基本クラスの並びを表現する。

| `<inheritedFrom>`
|   [ `typeName`要素([-@sec:type.typename]節)
|   ... ]
| `</inheritedFrom>`

属性なし

以下の子要素をもつ。

* `typeName`要素　－　
  基本クラスのデータ型識別名を示す。`
  access`属性により、
  `public`, `private`または`protected`の区別を指定できる。

## `enumType`要素 {#sec:type.enum}
enum型は、`enumType`要素で定義する。`type`要素で、メンバの識別子を指定する。

| `<enumType>`
|   [ `name`要素 ]
|   `symbols`要素
| `</enumType>`

属性(必須): `type`

属性(optional): データ型定義要素属性

次の子要素を持つ。

* `symbols`要素　－　メンバの識別子を定義する。メンバの値は`id`子要素の`value`子要素で表す。
* `name`要素(C++、オプショナル)　—　スコープ付き列挙型のときのスコープ名を定義する。

メンバの識別子は、スコープに対応するシンボルテーブルにクラス`moe`として定義されている。 enumのタグ名がある場合には、スコープに対応するシンボルテーブルに定義されている。

例:

"`enum { e1, e2, e3 = 10 } ee;`"の`ee`に対する`enumType`要素は以下のようになる。

      <enumType name="E0">
        <symbols>
          <id>
            <name>e1</name>
          </id>
          <id>
            <name>e2</name>
          </id>
          <id>
            <name>e3</name>
            <value><intConstant>10</intConstant></value>
          </id>
        </symbols>
      </enumType>

## `parameterPack`要素(C++) {#sec:type.parampack}
可変長引数を表現するための、仮引数の並びに対応する。

| `<parameterPack/>`

属性(必須): `type`, `element_type`

属性(optional): データ型定義要素属性

以下の属性を持つ。

* `type`　－　パックされた型に与えられたデータ型識別名
* `elem_type`　－　パックされる個々の型のデータ型識別名

`parameterPack`要素は、子要素を持たない。

例:

以下の関数テンプレートの定義において、

      template<typename T1, typename ... Types>
      T1 product(T1 val1, Types ... tail) {
        return val1 * product(tail...);
      }

"`typename ... Types`" に対応するデータ型定義は以下のようになる。

    <parameterPack type="K0" ref="typename"/>


# シンボルリスト {#sec:symb}

## `id`要素 {#sec:symb.id}
`id`要素は、変数名や配列名、関数名、struct/unionのメンバ名、 関数の引数、compound statementの局所変数名を定義する。

| `<id>`
|   `name`要素
|   [ `bitField`要素 ]
|   [ `alignAs`要素 ]
| `</id>`


属性(optional): `sclass`, `fspec`, `type`, `bit_field`, `align_as`, `is_gccThread`, `is_gccExtension`

`id`要素は次の属性を持つことができる。

* `sclass`属性　－　storage class をあらわし、 `auto`, `param`, `extern`, `extern_def`, `static`, `register`, `label`, `tagname`, `moe`, `typedef_name`, `template_param`(C++、テンプレートの型仮引数名), `namespace_name`(C++), , `alias_name`(C++、using文による別名)のいずれか。
* `is_inline`属性　－　関数の宣言がinline指定されていることを表す。
* `is_virtual`属性　－　メンバー関数がvirtualであることを表す。
* `is_explicit`属性　－　メンバー関数がexplicit指定されていることを表す。

    【要検討】storage class specifier以外のdecl-specifierである `friend`, `constexpr`もここで表現するか？

* `type`属性　－　識別子のデータ型識別名
* `bit_field`属性　－　`structType`、`unionType`と`classType`要素においてメンバのビットフィールドを数値で指定する。
* `is_thread_local`属性　－　thread\_local指定されていることを表す。
* `align_as`属性　－　`structType`、`unionType`と`classType`要素において、メンバのalignmentを数値またはデータ型識別名で指定する。
* `is_gccThread`属性　－　GCCの`__thread`キーワードが指定されているかどうかの情報、`0`または`1`、`false`または`true`。
* `is_gccExtension`属性

以下の子要素を持つことができる。

* `name`要素　－　識別子の名前は`name`要素で指定する。

    要検討: 実装時に再検討。何もかも`value`要素にするのがよいか？

* `bitField`要素　－　`unionType`と`classType`要素においてメンバのビットフィールドの値を`bit_field`属性の数値として指定できないとき使用する。`bitField`要素は式を子要素に持つ。`bitField`要素を使用するとき、`bit_field` 属性の値は、"`*`" とする。
* `alignAs`要素　—　`structType`、`unionType`と`classType`要素においてメンバのalignmentを`align_as`属性の数値として指定できないとき、`alignAs`要素の子要素として式の要素で指定する。

例:

"`int xyz;`"の変数`xyz`に対するシンボルテーブルエントリは以下のようになる。

      <id sclass="extern_def" type="int">
       <name>xyz</name>

      </id>

"`int foo()`"の関数`foo`に対するシンボルテーブルエントリは以下のようになる。なお、`F6f168`は、`foo`のデータ型に対するtype\_id。

      <id sclass="extern_def" type="F6f168">
       <name>foo</name>
      </id>

## `globalSymbols`要素 {#sec:symb.global}
大域のスコープを持つ識別子を定義する。

| `<globalSymbols>`
|   [ `id`要素([-@sec:symb.id]節)
|    … ]
| `</globalSymbols>`

属性なし

子要素として、大域のスコープを持つ識別子の`id`要素の並びを持つ。

## `symbols`要素 {#sec:symb.local}
局所スコープを持つ識別子を定義する。

| `<symbols>`
|   [ `id`要素([-@sec:symb.id]節)
|    … ]
| `</symbols>`

属性なし

子要素として、定義する識別子に対する`id`要素を持つ。

# `globalDeclarations`要素と`declarations`要素 {#sec:decl}

## `globalDeclarations`要素 {#sec:decl.global}
大域的な(翻訳単位全体をスコープとする)変数、関数などの宣言と定義を行う。

| `<globalDeclarations>`
|   [ {`varDecl`要素([-@sec:decl.var]節)　or
| `functionDecl`要素([-@sec:decl.func]節)　or
| `usingDecl`要素([-@sec:decl.using]節) or
| `functionDefinition`要素([-@sec:decl.fndef]節) or
| `functionTemplate`要素([-@sec:temp.func]節) or
| `text`要素([-@sec:stmt.text]節) }
|   … ]
| `</globalDeclarations>`

属性なし

以下の子要素を持つ。

* `functionDefinition`要素　－　関数の定義
* `varDecl`要素　－　変数の定義
* `functionDecl`要素　－　関数の宣言
* `text`要素　－　ディレクティブなど任意のテキストを表す

## `declarations`要素 {#sec:decl.local}
`compoundStatement`([-@sec:stmt.comp]節)、`classType`([-@sec:type.class]節)などをスコープとする変数、関数などの宣言と定義を行う。

| `<declarations>`
|   [ {`varDecl`要素([-@sec:decl.var]節)　or
| `functionDecl`要素([-@sec:decl.func]節)　or
| `usingDecl`要素([-@sec:decl.using]節) or
| `functionDefinition`要素([-@sec:decl.fndef]節) or
| `text`要素([-@sec:stmt.text]節) }
|   … ]
| `</declarations>`

属性なし

以下の子要素を持つ。

* `functionDefinition`要素　－　関数の定義
* `varDecl`要素　－　変数の定義
* `functionDecl`要素　－　関数の宣言
* `text`要素　－　ディレクティブなど任意のテキストを表す

## `functionDefinition`要素 {#sec:decl.fndef}
関数定義、メンバ関数の定義、コンストラクターの定義、デストラクターの定義、および、演算子オーバーロードの定義を行う。以下のいずれか一つの子要素を持つ。

| `<functionDefinition>`
|   `name`要素 or `operator`要素([-@sec:decl.op]) or `constructor`要素([-@sec:decl.ctor]) or `descructor`要素([-@sec:decl.dtor])
|   `symbols`要素([-@sec:symb.local]節)
|   `params`要素([-@sec:decl.params])
|   `body`要素
| `</functionDefinition>`


属性(optional): `is_gccExtension`

以下のいずれか一つの子要素を持つ。

* `name`要素　－　関数またはメンバ関数のときの、関数の名前
* `operator`要素　—　演算子オーバーロードのときの、演算子の名前
* `constructor`要素　—　構造体またはクラスのコンストラクタのとき
* `destructor`要素　—　構造体またはクラスのデストラクタのとき

加えて、以下の子要素をもつ。

* `symbols`要素　－　パラメータ(仮引数)のシンボルリスト。子要素はid要素の並び。
* `params`要素　－　パラメータ(仮引数)の並び
* `body`要素　－　関数本体。子要素として文(通常は`compoundStatement`)を含む。関数に局所的な変数などの宣言は、`body`要素の中に記述される。`body`要素内にGCCのネストされた関数を表す`functionDefinition`を含む場合がある。

以下の属性を持つ
* `is_gccExtension`属性

例:

関数の定義

    struct sss *foo(struct sss *arg1, int nnn)
    {
      ・・・(略)・・・

    }

に対し、以下の表現が対応する。

    <functionDefinition>
      <name>foo</name>
      <symbols>
        <id type="P1" sclass="param">
          <name>arg1</name>
        </id>
        <id type="int" sclass="param">
          <name>nnn</name>
        </id>
      </symbols>
      <params>
        <name type="P1">arg1</name>
        <name type="int">nnn</name>
      </params>
      <body>
        <compoundStatement>
          …(略)…
        </compoundStatement>
      </body>
    </functionDefinition>

### `operator`要素(C++) {#sec:decl.op}
`functionDefinition`要素の子要素。演算子オーバーロードを定義するとき、`name`要素の代わりに指定する。

| `<operator>`演算子名`</operator>`

属性なし

演算子名には、単項演算要素名([-@sec:expr.unary]節)、二項演算要素名([-@sec:expr.binary]節)などの[-@sec:expr]章で定義される演算子のXML要素の名前、または、ユーザ定義リテラルのアンダースコアで始まる名前を記述する。以下に例示する。

    <operator>plusExpr</operator>
    <operator>_my_op</operator>

### `constructor`要素(C++) {#sec:decl.ctor}
`functionDefinition`要素の子要素。そのメンバ関数がコンストラクタのとき、`name`要素の代わりに指定する。

| `<constructor>`
|    [ {    `name`要素
|       `value`要素([-@sec:program.value]節) }
|    … ]
| `</constructor>`


属性(optional): `is_explicit`

`name`要素と`value`要素の組は初期化構文に対応する。

要検討: コンストラクタのバリエーションに対応し切れていない。

### `destructor`要素(C++) {#sec:decl.dtor}
`functionDefinition`要素の子要素。そのメンバ関数がデストラクタであるとき、`name`要素の代わりに指定する。

| `<destructor/>`

### `params`要素 {#sec:decl.params}
関数の引数の並びを指定する。

| `<params>`
|   [ { `name`要素
|     [ `value`要素([-@sec:program.value]節) ] }
|   … ]
|   [ `ellipsis` ]
| `</params>`

属性なし

以下の子要素をもつことができる。

* `name`要素　－　引数の名前に対応する`name`要素を持つ。引数のデータ型の情報は、`name`要素の`type`属性名と同じ`type`属性名をもつデータ型定義要素([-@sec:type]章)で表現される。
* `value`要素　—　`params`が関数またはラムダ関数の仮引数並びで、直前の`name`要素に対応する仮引数がデフォルト実引数をもつとき、それを表現する。
* `ellipsis`　－　可変長引数を表す。`params`の最後の子要素に指定可能。

`params`要素内の`name`要素は、引数の順序で並んでいなくてはならない。

## `varDecl`要素 {#sec:decl.var}
変数の宣言を行う。

リファレンス型でない変数の宣言に対応する`varDecl`要素は次の形式に従う。

| `<varDecl>`
|   `name`要素
|   [ `value`要素([-@sec:program.value]節) ]
| `</varDecl>`

属性なし

変数宣言を行う識別子の名前を`name`要素で指定する。 以下の子要素を持つ。

* `name`要素　－　宣言する変数に対する`name`要素を持つ。
* `value`要素　－　初期値を持つ場合、`value`要素で指定する。配列・構造体の初期値の場合、`value`要素に複数の式を指定する。

例:

      int a[] = { 1, 2 };

      <varDecl>
        <name>a</name>
        <value>
          <intConstant type="int">1</intConstant>
          <intConstant type="int">2</intConstant>
        </value>
      </varDecl>

リファレンス型変数の宣言に対応する`varDecl`要素は次の形式に従う。

| `<varDecl>`
|   `name`要素
|   `value`要素([-@sec:program.value]節)
| `</varDecl>`

属性なし

変数宣言を行う識別子の名前を`name`要素で指定する。 以下の子要素を持つ。

* `name`要素　－　
  宣言する変数に対する`name`要素を持つ。
* `value`要素　－　
  初期化式を表現する。ここで、`value`要素は次の形式に従う。

    |   `<value>`
    |     `<addrOfExpr` `is_expedient=` `"1"` または `"true">`
    |       式の要素
    |     `</addrOfExpr>`
    |   `</value>`

    `value`要素の子要素である`addrOfExpr`要素の`is_expedient`属性値は"1"または"true"でなければならない。
    その他にデータ型定義要素属性をもってもよい。

例:

    int x = 0;
    int& rx = x;

    <varDecl>
      <name>x</name>
      <value>
        <intConstant type="int">0</intConstant>
      </value>
    </varDecl>
    <varDecl>
      <name>rx</name>
      <value>
        <addrOfExpr type="R1" is_expedient="true">
          <Var type="int">x</Var>
        </addrOfExpr>
      </value>
    </varDecl>

ここで`R1`は"`int&`"に対応するデータ型識別名として定義されている。

## `functionDecl`要素 {#sec:decl.func}
関数宣言を行う。

| `<functionDecl>`
|   `name`要素
| `</functionDecl>`

属性なし

以下の子要素を持つ

* `name`要素　－　関数名を指定する

## `usingDecl`要素(C++) {#sec:decl.using}
C++のusing宣言(using declaration)とusing指示(using directive)に対応する。

| `<usingDecl>`
|   `name`要素
| `</usingDecl>`


属性(optional): `lineno`, `file`, `namespace`

以下のようにusing文に対応する。

* using指示 "`using namespace 名前空間名`" の形のとき
    * `namespace`属性の値を`1`または`true`とする。
    * 名前空間名を`name`要素とする。名前空間名にはスコープ名と「`::`」が含まれることがある。
* using宣言 "`using 名前`" の形のとき
    * `namespace`属性を持たないか、値を`0`または`false`とする。
    * 名前を`name`要素とする。名前にはスコープ名と「`::`」が含まれることがある。
* 別名宣言 "`using 別名 = 型`"　の形のとき、`usingDecl`要素では表現されない。typedefと同様、データ型定義要素([-@sec:type]章)で表現される。

# 文の要素 {#sec:stmt}
Cの文の構文要素に対応するXML要素である。それぞれのXML要素には、文の元の行番号とファイル名を属性として付加することができる。

* `lineno`　－　文番号を値として持つ
* `file`　－　この文が含まれているファイル名


## `exprStatement`要素 {#sec:stmt.expr}
式で表現される文を表す。式の要素([-@sec:expr]章)を持つ。

| `<exprStatement>`
|   式の要素([-@sec:expr]章)
| `</exprStatement>`


属性(optional): `lineno`, `file`

## `compoundStatement`要素 {#sec:stmt.comp}
複文を表現する。

| `<compoundStatement>`
|   `symbols`要素([-@sec:symb.local]節)
|   `declarations`要素([-@sec:decl.local]節)
|   `body`要素
| `</compoundStatement>`


属性(optional): `lineno`, `file`

以下の子要素を持つ。

* `symbols`要素　－　このスコープの中で定義されているシンボルリスト
* `declarations`要素　－　このスコープの中で定義される宣言
* `body` 要素　－　複文本体。文の要素の並び。

## `ifStatement`要素 {#sec:stmt.if}
if文を表現する。

| `<ifStatement>`
|   `condition`要素
|   `then`要素
|   `else`要素
| `</ifStatement>`


属性(optional): `lineno`, `file`

以下の子要素を持つ。

* `condition` 要素　－　条件式を子要素として含む
* `then`要素　－　then部の文を子要素として含む
* `else`要素　－　else部の文を子要素として含む

## `whileStatment`要素 {#sec:stmt.while}
while文を表現する。

| `<whileStatement>`
|   `condition`要素
|   `body`要素
| `</whileStatement>`


属性(optional): `lineno`, `file`

以下の子要素を持つ

* `condition` 要素　－　条件式を子要素として含む
* `body` 要素　－　本体の文を子要素として含む

## `doStatement`要素 {#sec:stmt.do}
do文を表現する。

| `<doStatement>`
|   `body`要素
|   `condition`要素
| `</doStatement>`


属性(optional): `lineno`, `file`

以下の子要素を持つ。

* `body`要素　－　本体を表す、文の要素の並びを含む
* `condition`要素　－　条件式を表す式の要素を含む

## `forStatement`要素 {#sec:stmt.for}
for文(従来仕様)を表現する。

| `<forStatement>`
|   [ `init`要素 ]
|   [ `condition`要素 ]
|   [ `iter`要素 ]
|   `body`要素
| `</forStatement>`


属性(optional): `lineno`, `file`

以下の要素を持つ。

* `init`要素　－　初期化式または宣言文を要素として含む
* `condition`要素　－　条件式として式の要素を含む
* `iter`要素　－　繰り返し式として式の要素を含む
* `body`要素　－　for文の本体を表す、文の要素の並びを含む。

`init`要素は、for文の中の初期化式または宣言文を表現する。

| `<init>`
|   式の要素 or `symbols`要素
| `</init>`

属性なし

　`init`要素は、`forStatement`要素の中だけに現れる。初期化式を意味する式の要素を含むか、または、0個以上の局所変数の宣言を意味する`symbols`要素を含む。

## `rangeForStatement`要素(C++) {#sec:stmt.rangefor}
C++仕様のfor文

| `for` `(` for-range-declaration `:` expression `)` statement

を表現する。

| `<rangeForStatement>`
|   `id`要素
|   `range`要素
|   `body`要素
| `</rangeForStatement>`


属性(optional): `lineno`, `file`

以下の子要素を持つ。

* `id`要素([-@sec:symb.id]節)
* `range`要素　－　配列やコンテナを表す式の要素([-@sec:expr]章)を含む。
* `body`要素　－　for文の本体を表す、文の要素([-@sec:stmt]章)の並びを含む。

## `breakStatement`要素 {#sec:stmt.break}
break文を表現する。

| `<breakStatement/>`


属性(optional): `lineno`, `file`

## `continueStatement`要素 {#sec:stmt.cont}
continue文を表現する。

| `<continueStatement/>`


属性(optional): `lineno`, `file`

## `returnStatment`要素 {#sec:stmt.ret}
return文を表現する。

| `<returnStatement>`
|   [ 式の要素 ]
| `</returnStatement>`


属性(optional): `lineno`, `file`

returnする式を、子要素として持つことができる。

## `gotoStatement`要素 {#sec:stmt.goto}
goto文を表現する。

| `<gotoStatement>`
|   `name`要素 or 式の要素
| `</gotoStatement>`


属性(optional): `lineno`, `file`

子要素に`name`要素か式のいずれかを持つ。式はGCCにおいてジャンプ先として指定可能なアドレスの式を表す。

* `name`要素　－　ラベル名の名前を指定する。
* 式の要素　－　ジャンプ先のアドレス値を指定する。

## `tryStatement`要素(C++) {#sec:stmt.try}
try構文を表現する。

| `<tryStatement>`
|   `body`要素
| `</tryStatement>`


属性(optional): `lineno`, `file`

以下の子要素を持つ。

* `body`要素　－　本体を表す、文の要素([-@sec:stmt]章)の並びを含む

## `catchStatement`要素(C++) {#sec:stmt.catch}
catch構文を表現する。

| `<catchStatement>`
|   `params`要素([-@sec:decl.params]節)
|   `body`要素
| `</catchStatement>`


属性(optional): `lineno`, `file`

以下の子要素を持つ。

* `params`要素　—　内容は1つの`name`要素または1つの`ellipsis`でなければならない。補足する例外の型を示す。
* `body`要素　－　本体を表す、文の要素([-@sec:stmt]章)の並びを含む

## `statementLabel`要素 {#sec:stmt.label}
goto文のターゲットのラベルを表す。

| `<statementLabel>`
|   `name`要素
| `</statementLabel>`

属性なし

ラベル名を`name`要素として持つ。

* `name`要素　－　ラベル名の名前を指定する。

## `switchStatement`要素 {#sec:stmt.switch}
switch文を表現する。

| `<statementLabel>`
|   `value`要素
|   `body`要素
| `</statementLabel>`


属性(optional): `lineno`, `file`

以下の子要素を持つ。

* `value`要素　－　switchする値を表す式の要素([-@sec:program.value]章)
* `body`要素　－　switch文の本体を表す文の要素([-@sec:stmt]章)であり、多くの場合`compoundStatement`要素([-@sec:stmt.comp]節)となる。`caseLabel`要素([-@sec:stmt.label]節)と`gccRangedCaseLabel`要素([-@sec:stmt.gccrangecase]節)と`defaultLabel`要素([-@sec:stmt.default]節)を含むことができる。

## `caseLabel`要素 {#sec:stmt.case}
switch文のcase文を表す。`switch`要素の中の`body`要素の中の`compoundStatement`の中だけに現れることができる。

| `<caseLabel>`
|   `value`要素
| `</caseLabel>`


属性(optional): `lineno`, `file`

caseの値を子要素としてもつ。

* `value`要素　－　caseの値を指定する。

## `gccRangedCaseLabel`要素 {#sec:stmt.gccrangecase}
gcc拡張のcase文での範囲指定を表す。switch要素の中の`body`要素の中の`compoundStatement`の中だけに現れることができる。

| `<gccRangedCaseLabel>`
|   `value`要素
|   `value`要素
| `</gccRangedCaseLabel>`


属性(optional): `lineno`, `file`

caseの値を要素としてもつ。

* `value`要素　－　caseの値の下限値を指定する。
* `value`要素　－　caseの値の上限値を指定する。

## `defaultLabel`要素 {#sec:stmt.default}
switch文のdefaultラベルを表す。`switch`要素の中の`body`要素の中の`compoundStatement`の中だけに現れることができる。

| `<defaultLabel/>`


属性(optional): `lineno`, `file`

## `pragma`要素 {#sec:stmt.pragma}
`pragma`要素は#pragma文を表す。

| `<pragma>`文字列`</pragma>`


属性(optional): `lineno`, `file`

\#pragmaに指定する文字列を持つ。

## `text`要素 {#sec:stmt.text}
`text`要素は任意のテキストを表し、コンパイラに依存したディレクティブなどの情報を要素として持つために使用する。

| `<text>`文字列`</text>`


属性(optional): `lineno`, `file`

内容に任意の文字列を持つ。この要素は `globalDeclarasions` にも出現する。


# 式の要素 {#sec:expr}
式の構文要素に対応するXML要素である。式の要素には、本章に記述されたXML要素以外に、以下のものがある。

* `functionInstance`要素([-@sec:temp.funcinstance]節)

式の要素には、共通して以下の属性を付加できる。

* `type`属性　―　式のデータ型情報を取り出すことができる。
* (廃止予定)`lvalue`属性　―　式が左辺値であることを示す。

要検討:
`lvalue`属性は、式の要素の属性からテータ型定義要素の属性に移動したい。

## 定数の要素 {#sec:expr.constant}
定数は以下のXML要素によって表現する。

| `<intConstant>`10進数または16進数`</intConstant>`
| `<longlongConstant>`16進数 16進数`</longlongConstant>`
| `<floatConstant>`浮動小数点数`</floatConstant>`
| `<stringConstant>`文字列`</stringConstant>`
| `<moeConstant>`列挙型メンバ名`</moeConstant>`
| `<booleanConstant>`真偽値`</booleanConatant>`
| `<funcAddr>`関数名`</funcAddr>`

属性(必須): `type`

* `intConstant`要素　－　整数の値を持つ定数を表す。数値として、十進数もしくは、16進数(0xから始まる)を記述する。`type`属性には"`int`", "`long`", "`unsigned`", "`unsigned_long`", "`char`"と"`wchar_t`"が許される。C++ではこれらに加えて、"`char16_t`"と"`char32_t`"が許される。
備考:`char16_t`は必ず16ビット、`char32_t`は必ず32ビットだが、`wchar_t`は環境によって16ビットまたは32ビットであると定義されている。
* `longlongConstant`要素　－　32ビット16進数(0xから始まる)の2つの数字を空白で区切って記述する。`type`属性には"`long_long`"と"`unsigned_long_long`"が許される。
* `floatConstant`要素　－　floatまたはdoubleまたはlong doubleの値を持つ定数を表す。浮動小数点数のリテラルを記述する。`type`属性には"`float`", "`double`"と"`long_double`"が許される。
* `stringConstant`要素　－　内容にダブルクォーテーションで囲まない文字列を記述する。文字列中の特殊文字はXML(HTML)のルールに従ってクォートされる('<'は'`&lt;`'に置換されるなど)。`type`属性には、"`char`"と"`wchar_t`"が許される。C++ではこれらに加えて、"`char16_t`"と"`char32_t`"が許される。

    仕様変更:旧仕様では`type`属性を持たず、代わりに以下のように定義されている。

    > 属性に `is_wide=`[1|0|true|false]` (省略時`0`)を持ち、`1`または`true`のときwchar_t型の文字列を表す。

* `moeConstant`要素　－　enum型の定数を表す。内容にenum定数(列挙型のメンバの名前)を記述する。`type`属性は列挙型のタイプ名を記述する。
* `booleanConstant`要素　－　真理値リテラル。`false`または`true`。`type`属性は`bool`のみ許される。
* `funcAddr`要素　－　関数のアドレスを表す。内容に関数名を記述する。`type`属性は、原則としてその関数のインスタンスの型とするが、翻訳時に不明な場合には別の表現とする。(詳細は実装時に検討する。)

備考:`longlongConstant`だけ特別扱いするのは不自然。素直に10進数表記で表現する形にしたい。

## `Var`要素 {#sec:expr.var}

配列以外の変数への参照を表現する。

| `<Var>`変数名`</Var>`

属性(必須): `type`, `scope`

`scope`属性をつかって、局所変数を区別する。

* `scope`属性　－　"`local`", "`global`", "`param`"のいずれか

## `addrOfExpr` 要素 {#sec:expr.addrof}

式へのアドレス参照を表現する。

| `<addrOfExpr>`
|   式の要素
| `</addrOfExpr>`

属性(必須): `type`

## `arrayAddr`要素 {#sec:expr.arrayaddr}

配列型変数への参照を表現する。

| `<arrayAddr>`配列変数名`</arrayAddr>`

例:

`a`がint型の配列のとき、`a`の参照は、

    <arrayAddr type="A5" scope="local">a</arrayAddr>

と表現される。ここで`A5`は、`typeTable`の中で

    <arrayType type="A5" element_type="int" array_size="3"/>

などと宣言されている。

備考:

`a`が配列のとき、2015年10月現在のF\_Frontでは `&a` の参照を`a`の参照と同様`arrayAddr`で表現している。
これに関連してOmni XMPでは型の不一致によるエラーが出ている(バグレポート439)。

## `pointerRef`要素 {#sec:expr.pointer}
式(ポインタ型またはリファレンス型)の指示先を表現する。

| `<pointerRef>`
|   式の参照
| `</pointerRef>`

属性(必須): `type`

属性(optional): `is_expedient`

pointerRef要素は次の属性を持つことができる。

* `is_expedient`属性　－　
  "`1`"または"`true`"のとき、式がリファレンスであることを表す。
  "`0`"または"`false`"のとき、式がポインタであることを表す。
  省略時の値は"`false`"である。

例:

式 `*var1` (`var1`はint型へのポインタ)は以下のように表現される。

    <pointerRef  type="int">
      <Var type="P0" scope="local">var1</Var>
    </pointerRef>

要確認:

現状(2015年10月)のC\_Frontでは、`*(&var_name)` というパターンのとき

    <PointerRef><varAddr>var_name</varAddr></PointerRef>

でなく

    <Var>var_name</Var>

と表現している。なぜこのパターンに限って簡単化しているのか不明。

## `arrayRef`要素 {#sec:expr.array}
配列要素`a[i]`への参照を表現する。

| `<arrayRef>`
|   `arrayAddr`要素
|   式の要素
| `</arrayRef>`

属性(必須): `type`

例:

`int a[3];` と宣言されているとき、配列要素 `a[i]` の参照は、

    <arrayRef type="int">
      <arrayAddr type="A5"scope="local">a</arrayAddr>
      <Var type="int" scope="local">i</Var>
    </arrayRef>

のように表現される。配列要素のアドレス `&a[i]` の参照は、

    <addrOfExpr type="P232">
      <arrayRef type="int">
        <arrayAddr type="A5"scope="local">a</arrayAddr>
        <Var type="int" scope="local">i</Var>
      </arrayRef>
    </addrOfExpr>

のように表現される。ここで`P232`はint型へのポインタと宣言されている。後者は`arrayAddr`要素でないことに注意されたい。

## `memberRef`要素(C++拡張) {#sec:expr.memberref}

構造型、クラス、または共用型のオブジェクトがもつ配列以外のメンバへの参照を表現する。

| `<memberRef>`
| 　　式の要素
| `</memberRef>`

属性(必須): `type`, `member`

* `member`属性　－　
  参照するメンバをメンバ名で指定する。

`memberRef`要素は、子要素としてオブジェクトを表現する式の要素をもつ。

例:

オブジェクト`s`のint型メンバ`n`への参照 `s.n` について、以下のように表現する。

        <memberRef type="int" member="n">
            <varAddr type="P0" scope="local">s</varAddr>
        </memberRef>

## `memberArrayRef`要素(C++拡張) {#sec:expr.memberarrayref}

構造型、クラス、または共用型のオブジェクトがもつ配列型のメンバへの参照を表現する。

| `<memberArrayRef>`
| 　　式の要素
| `</memberArrayRef>`

属性(必須): `type`, `member`

* `member`属性　－　
  参照するメンバをメンバ名で指定する。

`memberArrayRef`要素は、子要素としてオブジェクトを表現する式の要素をもつ。

例:

オブジェクト`s`のint型配列メンバ`a`への参照 `s.a` について、以下のように表現する。

        <memberArrayRef type="A0" member="a">
          <varAddr type="P1" scope="local">s</varAddr>
        </memberArrayRef>

要検討:構造体まわりの現在のC\_Frontの変換仕様について

`arrayRef`要素([-@sec:expr.array]節)と`memberArrayRef`要素、`arrayAddr`要素([-@sec:expr.var]節)と`memberArrayAddr`要素は、それぞれ名前が似ているが意味の対称性がない。少なくとも名前を再考したい。他の点でも、今後構造体やクラスへの対応を考えると、整理しておきたいところ。

+-----------+-------------------------+-----------+------------------------------+
| C言語表現 | XcodeML表現             | C言語表現 | XcodeML表現                  |
+===========+=========================+===========+==============================+
| `v`       | - `Var v`               | `s.v`     | - `memberRef v`              |
|           |                         |           |     * `varAddr s`            |
+-----------+-------------------------+-----------+------------------------------+
| `&v`      | - `varAddr v`           | `&s.v`    | - `memberAddr v`             |
|           |                         |           |     * `varAddr s`            |
+-----------+-------------------------+-----------+------------------------------+
| `a`       | - `arrayAddr a`         | `s.a`     | - `memberArrayRef a`         |
|           |                         |           |     * `varAddr s`            |
+-----------+-------------------------+-----------+------------------------------+
| `&a`      | - `arrayAddr a`         | `s.a`     | - `memberArrayAddr a`        |
|           |                         |           |     * `varAddr s`            |
+-----------+-------------------------+-----------+------------------------------+
| `a[i]`    | - `arrayRef`            | `s.a[i]`  | - `pointerRef`               |
|           |     * `arrayAddr a`     |           |     * `plusExpr`             |
|           |     * `Var i`           |           |         + `memberArrayRef a` |
|           |                         |           |             - `varAddr s`    |
|           |                         |           |         + `Var i`            |
+-----------+-------------------------+-----------+------------------------------+
| `&a[i]`   | - `addrOfExpr`          | `&s.a[i]` | - `plusExpr`                 |
|           |     * `arrayRef`        |           |     * `memberArrayRef a`     |
|           |         + `arrayAddr a` |           |         + `varAddr s`        |
|           |         + `Var i`       |           |     * `Var i`                |
+-----------+-------------------------+-----------+------------------------------+

## メンバポインタの参照の要素(C++) {#sec:expr.memberpointer}
オブジェクト`s`のメンバへのポインタの参照`s.*p`を表現する。

| `<memberPointer>`
| 　　式の要素
| `</memberPointer>`

属性(必須): `type`, `name`

`name`属性に変数名を指定し、子要素で構造体のアドレスを表現する。

備考:

メンバの参照([-@sec:expr.memberpointer]節)では`member`属性にメンバ名を記述するのに対し、メンバポインタの参照(本節)では`name`属性に変数名を記述する。この仕様は実装を反映した。

例:

以下のプログラムで、(1)はメンバ変数へのポインタの宣言、(2)はメンバ関数へのポインタの宣言であり、それぞれメンバ変数、メンバ関数をポイントするよう初期化されている([-@sec:nns.attr]節の例参照)。(3)の右辺により`s1.foo`が引数`3`で呼び出され、左辺`s1.data`に代入される。

    struct S {
      int data;
      int foo(int n) { return n + 1; }
    };

    int S :: *d = &S :: data;        // (1)
    int (S :: *f)(int) = &S :: foo;        // (2)

    struct S s1;
    s1.*d = (s1.*f)(3);            // (3)

このとき、(3)の左辺は以下のように表現される。

      <memberPointerRef type="P4" name="d">
        <varAddr type="P3" scope="global">s1</varAddr>
      </memberPointerRef>

(3)の右辺は以下のように表現される。

      <functionCall type="int">
        <function>
          <memberPointerRef type="P4" name="f">
            <varAddr type="P3" scope="global">s1</varAddr>
          </memberPointerRef>
        </function>
        <arguments>
          <intConstant type="int">3</intConstant>
        </arguments>
      </functionCall>

## 複合リテラルの要素(新規) {#sec:expr.compval}
型`T`の複合リテラル `(T){ … }` および型Tの複合リテラルのアドレス `&(T){ … }` を表現する。

| `<compoundValue>` or `<compoundValueAddr>`
| 　　`value`要素
| `</compoundValue>` or `</compoundValueAddr>`

属性(必須): `type`

指示付きの初期化子(`(T){ [2]=1, .x=2 }` のような記述)に対応する表現は持たず、常に展開された表現に変換される(例参照)。

備考:

複合リテラルは、旧仕様書では`castExpr`要素で表現すると書かれているが、C\_Frontの動作と食い違っている。本節はC\_Frontの動作に合わせて書き起こした。

例:

以下のようなプログラムで、

    typedef struct { int x, y; } two_int_t;
    int n = 20;
    foo(&(two_int_t){ 1, n });
    goo((two_int_t){ .y=300 });

関数`foo`の引数は以下の表現となる。

    <compoundValueAddr type="P6">
      <value>
        <value>
          <intConstant type="int">1</intConstant>
          <Var type="int" scope="local">n</Var>
        </value>
      </value>
    </compoundValueAddr>

関数`goo`の引数は以下の表現となる。

    <compoundValue type="P6">
      <value>
        <value>
          <intConstant type="int">0</intConstant>
          <intConstant type="int">300</intConstant>
        </value>
      </value>
    </compoundValue>

## `thisExpr`要素(C++) {#sec:expr.this}
`thisExpr` 要素は、C++の this に対応する。

| `<thisExpr/>`

属性なし

## `assignExpr` 要素 {#sec:expr.assign}
`assignExpr` 要素は、2つの式の要素をsub要素に持ち、代入を表す。

| `<assignExpr>`
|   式の要素
|   式の要素
| `</assignExpr>`

属性(必須): `type`

属性(optional): `is_userDefined`

第1の式を左辺、第2の式を右辺とする代入文を表現する。

## 2項演算式の要素 {#sec:expr.binary}
二項演算式を表現する。被演算子の2つのXML要素を内容に指定する。

| `<`二項演算要素名`>`
|   式の要素
|   式の要素
| `</`二項演算要素名`>`

属性(必須): `type`

二項演算要素名には以下のものがある。

* 算術二項演算子
    * `plusExpr`　－　加算
    * `minusExpr`　－　減算
    * `mulExpr`　－　乗算
    * `divExpr`　－　除算
    * `modExpr`　－　剰余
    * `LshiftExpr`　－　左シフト
    * `RshiftExpr`　－　右シフト
    * `bitAndExpr`　－　ビット論理積
    * `bitOrExpr`　－　ビット論理和
    * `bitXorExpr`　－　ビット論理　排他和
* 代入演算子
    * `asgPlusExpr`　－　加算
    * `asgMinusExpr`　－　減算
    * `asgMulExpr`　－　乗算
    * `asgDivExpr`　－　除算
    * `asgModExpr`　－　剰余
    * `asgLshiftExpr`　－　左シフト
    * `asgRshiftExpr`　－　右シフト
    * `asgBitAndExpr`　－　ビット論理積
    * `asgBitOrExpr`　－　ビット論理和
    * `asgBitXorExpr`　－　ビット論理　排他和
* 論理二項演算子
    * `logEQExpr`　－　等価
    * `logNEQExpr`　－　非等価
    * `logGEExpr`　－　大なり、または同値
    * `logGTExpr`　－　大なり
    * `logLEExpr`　－　小なり、または等価
    * `logLTExpr`　－　小なり
    * `logAndExpr`　－　論理積
    * `logOrExpr`　－　論理和

## 単項演算式の要素 {#sec:expr.unary}
単項演算式を表現する。被演算子を内容に指定する。

| `<`単項演算要素名`>`
|   式の要素
| `</`単項演算要素名`>`

属性(必須): `type`

単項演算要素名には以下のものがある。

* 算術単項演算子
    * `unaryMinusExpr`　－　符号反転
    * `bitNotExpr`　－　ビット反転
* 論理単項演算子
    * `logNotExpr`　－　論理否定
* sizeof演算子
    * `sizeOfExpr`　－　子要素として式の要素または`typeName`要素を指定する。
* alignof演算子(C++)
    * `alignOfExpr`　－　子要素として式の要素または`typeName`要素を指定する。
* typeid演算子(C++)
    * `typeidExpr`　－　子要素として式の要素または`typeName`要素を指定する。
* GCC拡張の演算子
    * `gccAlignOfExpr`　－　GCCの\_\_alignof\_\_演算子を表す。子要素に式または`typeName`要素を指定する。
    * `gccLabelAddr`　－　GCCの\&\&単項演算子を表す。内容にラベル名を指定する。

## `functionCall`要素 {#sec:expr.call}
`functionCall`要素は関数呼び出しを表す。

| `<functionCall>`
|   `<function>`または`<memberRef>`または`<operator>`
|     式の要素
|   `</function>`または`</memberRef>`または`</operator>`
|   `arguments`要素([-@sec:expr.arguments]項)
| `</functionCall>`

属性(必須): `type`

`function`要素には呼び出す関数のアドレスを指定する。
`memberRef`　　　メンバ関数呼び出しの時のメンバアクセスの式を指定する。
`operator`　　　グローバル関数の形の演算子オーバーロードの呼び出しの場合の演算子名を指定する。
`arguments`要素には引数の並びを指定する。

### `arguments`要素 {#sec:expr.arguments}
実引数(actual argument)の0個以上の並びを表現する。

| `<arguments>`
|   [ 式の要素
|   ... ]
| `</arguments>`

## `commaExpr`要素 {#sec:expr.comma}
コンマ式(第1オペランドと第2オペランドを評価し、第2オペランドの値を返す式)を表す。

| `<commaExpr>`
|   式の要素
|   式の要素
| `</commaExpr>`

属性(必須): `type`

属性(optional): `is_userDefined`

## インクリメント・デクリメント要素(`postIncrExpr`, `postDecrExpr`, `preIncrExpr`, `preDecrExpr`) {#sec:expr.increment}
`postIncrExpr`要素、`postDecrExpr`要素は、CおよびC++のポストインクリメント、デクリメント式を表す。`preIncrExpr`要素、`preDecrExpr`要素は、CおよびC++のプレインクリメント、デクリメント式を表す。

| `<postIncrExpr>` or `<postDecrExpr>` or `<preIncrExpr>` or `<preDecrExpr>`
|   式の要素
| `</postIncrExpr>` or `</postDecrExpr>` or `</preIncrExpr>` or `</preDecrExpr >`

属性(必須): `type`

属性(optional): `is_userDefined`

## `castExpr`要素(廃止予定) {#sec:expr.cast}
`castExpr`要素は型変換の式(旧仕様)、または複合リテラルを表す。

| `<castExpr>`
|   式の要素 or `value`要素
| `</castExpr>`

属性(必須): `type`

属性(optional): `is_gccExtension`

以下の子要素を持つ。
* castされる式、または、複合リテラルのリテラル部

備考:

現在のC\_Frontでは、複合リテラルにこの表現は使われておらず、`compoundValue`要素または`compoundValueAddr`要素([-@sec:expr.compval]節)が使われている。キャストはC++仕様のstatic\_cast, const\_castまたはreinterpret\_castに変換して表現する方が、バリエーションの削減になるため望ましい。どちらの用途にも使われないのであれば、`castExpr`は廃止すべきと考える。

備考の備考:C++においてもCスタイルのキャストを書いた場合は static\_cast等とは違う意味になるので、この仕様は残さざるを得ないと考える。

## キャスト要素(`staticCast`, `dynamicCast`, `constCast`, `reinterpretCast`)(C++) {#sec:expr.cppcast}
順に、C++のstatic\_cast, dynamic\_cast, const\_castおよびreinterpret\_castを表現する。

| `<staticCast>` or `<dynamicCast>` or `<constCast>` or `<reinterpretCast>`
|   式の要素
| `</staticCast>` or `</dynamicCast>` or `</constCast>` or `</reinterpretCast>`

属性(必須): `type`

Cのcastは、`staticCast`または`constCast`または`reinterpretCast`で表現する。

## `implicitCastExpr`要素 {#sec:expr.implicitcast}

C++の標準型変換を表現する。

| `<implicitCastExpr>`
|   式の要素 or `value`要素
| `</implicitCastExpr>`

属性(必須): `type`

以下の子要素を持つ。

* キャストされる式、または、複合リテラルのリテラル部

## `condExpr`要素 {#sec:expr.cond}
三項演算 `x ? y : z` を表現する。

| `<condExpr>`
|   式の要素
|   [ 式の要素 ]
|   式の要素
| `</condExpr>`

属性(必須): `type`

第2オペランド(2番目の式)は省略されることがある(GNU拡張対応)。

要確認:

選択される式の`type`属性が異なるとき、`condExpr`要素の`type`属性はどうするべきか？
　→暗黙のキャストが入って同じ型にそろえられるので、その型を式全体の型と考えればよい(あるいは暗黙のキャストで対応できないほど型が食い違っている場合にはコンパイルエラーになるためそのようなソースコードは受理する必要がない)。

## `gccCompoundExpr`要素 {#sec:expr.gcccomp}
gcc拡張の複文式に対応する。

| `<gccCompoundExpr>`
|   `compoundStatement`要素
| `</gccCompoundExpr>`

属性(必須): `type`

* `compoundStatement`要素　－　複文式の内容を指定する。

## `newExpr`要素と`newExprArray`要素 {#sec:expr.new}
new演算子またはnew[]演算子から成る式を表現する。

| `<newExpr>`
|   `arguments`要素([-@sec:expr.arguments]項)
| `</newExpr>`

属性(必須): `type`

| `<newArrayExpr>`
|   式の要素([-@sec:expr]章)
| `</newArrrayExpr>`

属性(必須): `type`

第1と第2の書式は、それぞれnew演算子とnew[]演算子による領域確保を表現する。確保されるデータは、`type`属性の型をもつ。第1の書式の子要素は、コンストラクタに渡されるパラメタを表す。第2の書式の子要素は、確保する配列の長さを表す。

## `deleteExpr`要素と`deleteArrayExpr`要素 {#sec:expr.delete}
delete演算子またはdelete[]演算子から成る式を表現する。

| `<deleteExpr>`
|   式の要素
| `</deleteExpr>`

属性(必須): `type`

| `<deleteArrayExpr>`
|   式の要素
| `</deleteArrayExpr>`

属性(必須): `type`

第1と第2の書式は、それぞれdelete演算子とdelete[]演算子による領域解放を表現する。子要素は、解放する領域へのポインタである。

## `throwExpr`要素(C++) {#sec:expr.throw}
throw式を表現する。

| `<throwExpr>`
|  [ 式の要素 ]
| `</throwExpr>`


属性(optional): `lineno`, `file`

子要素として式の要素をもつ。式の要素は投げられる例外を表す。

## `lambdaExpr`要素 {#sec:expr.lambda}
C++のラムダ式を表現する。

| `<lambdaExpr>`
|   `captures`要素
|   `symbols`要素
|   `params`要素
|   `body`要素
| `</lambdaExpr>`

属性(必須): `type`

`symbols`要素、`params`要素([-@sec:decl.params]節)と`body`要素は、`functionDefinition`要素([-@sec:decl.fndef]節)の子要素と同様である。

### `captures`要素 {#sec:expr.captures}
`captures`要素は以下の表現である。

| `<captures>`
|   `<byReference>`
|     [ `name`要素
|     … ]
|   `</byReference>`
|   `<byValue>`
|     [ `name`要素
|     … ]
|   `</byValue>`
| `</captures>`


属性(optional): `default`, `is_mutable`

`captures`要素はオプショナルに以下の属性をもつ。

* `default`属性　－　"`by_reference`" のとき、スコープデフォルトが参照キャプチャ "`[&]`" であることを意味し、"`by_value`"のときデフォルトがコピーキャプチャ "`[=]`" であることを意味する。省略されたとき、キャプチャがないことを意味する。
* `is_mutable`属性　－　`1`または`true`のとき、mutable指定があることを意味する。`0`または`false`または省略されたとき、mutable指定がないことを意味する。
　子要素の`byReference`要素で指定された名前の変数は参照キャプチャされ、`byValue`要素で指定された名前の変数はコピーキャプチャされる。それ以外の変数は、`default`属性の指定に従う。

# `nnsTable`要素とNNS定義要素 {#sec:nns}
`nnsTable`要素は、翻訳単位([-@sec:program]章)に対して一つだけ存在し、
翻訳単位で使われているすべての名前修飾についての情報を定義する。

| `<nnsTable>`
|   [ NNS定義要素
|   … ]
| `</nnsTable>`

属性なし


`nnsTable`要素は、翻訳単位を表現する`XcodeProgram`要素([-@sec:program]章)の直接の子要素であり、
NNSを定義するNNS定義要素の列からなる。
NNS定義要素には以下の要素がある。

* `globalNNS`要素
* `namespaceNNS`要素
* `unnamedNamespaceNNS`要素
* `classNNS`要素
* `enumNNS`要素
* `typedefTypeNNS`要素
* `templateParamTypeNNS`要素
* `simpleTemplateIdNNS`要素

## `nns`属性 {#sec:nns.attr}

`nns`属性は、C++のスコープ解決演算子による修飾をおこなった形の「フルネーム」を指定するためのXML属性である。
ソースコード上での「名前」を表現する各要素について、適宜挿入される共通の構造である。

    nns="修飾子識別名"

下記の各要素に適宜挿入される。

* `nnsTable`要素に含まれるもの:
    * `napespaceNNS`要素
    * `classNNS`要素
* `typeTable`要素および`localTypeTable`に含まれるもの:
    * `name`要素
* `globalSymbols`要素および(`symbols`要素に含まれるもの:
    * `name`要素
* `globalDeclarations` 要素および`declarations`要素に含まれるもの:
    * `name`要素
    * `operator`要素
    * `Var`要素
    * `function`要素
    * `funcAddr`要素
    * `arrayRef`要素
    * `arrayAddr`要素
    * `memberRef`要素
    * `memberArrayRef`要素

どんな文字列も修飾子識別名として定義することができる。
ただし、以下の識別名は利用を予約されている。

* `global` -
  グローバル名前空間

例:

以下のプログラムで、

    namespace NS {
      int a;            // (1)
    }
    NS::a = 10;        // (2)

`namespace NS`の存在を表現するために、以下のような`nnsTable`が生成される。

    <nnsTable>
      <nestedNameSpecifier nns="Q0">
        <namespaceNNS nns="global">NS</namespace>
      </nestedNameSpecifier>
    </nnsTable>

これを用いて、(1)および(2)における`a`は、以下のように表現される。

    <name type="int" nns="Q0">a</name>

例:

以下のプログラムで、

    struct S {
        int data;
        int foo(int n) { return n + 1; }
    };

    int S :: *d = &S :: data;        // (1)
    int (S :: *f)(int) = &S :: foo;        // (2)
    struct S s1;
    int *p = &s1.data;            // (3)

 (1),(2)の`d`と`f`の名前は、それぞれ以下のように表現される。`MP1`は`S`のメンバーへのポインタ(int型を指すもの)の型であり、`MP2`は`S`のメンバ関数へのポインタ(intを引数にとりintを戻り値とする関数を指すもの)の型である。

・・・※ここは「その`typeTable`がどう表現されるか」を加筆すべきである。

    <name type="MP1" >d</name>
    <name type="MP2" >f</name>

(1)と(2)の右辺式は、それぞれ以下のように表現される。`S`は変数でないので`memberAddr`要素は用いられず、`data`変数のスコープと解釈する。ただし`S0`は`nnsTable`内で構造体`S`のスコープを表現するものとして定義されているとする。

    <varAddr type="P0" scope="global" nns="S0">data</varAddr>
    <varAddr type="P0" scope="global" nns="S0">foo</varAddr>

(3)の右辺式は、以下のように表現される。`s1`は変数名なので、`s1.data`は`memberAddr`要素で表現される。

    <memberAddr type="P5" member="data" nns="S">　…このnnsが必要か要検討
        <varAddr type="P4" scope="global">s1</varAddr>
    </memberAddr>

## `namespaceNNS`要素 {#sec:nns.namespace}

無名でない名前空間を表現する。

| `<namespaceNNS>`
|   `name`要素
|   `attributes`要素
| `</namespaceNNS>`

属性(必須): `nns`

属性(optional): `is_inline`

* `nns` -
  このNNSに与えられたNNS識別名
* `is_inline` -
  "`1`"または"`true`"のとき、inline名前空間であることを意味する。
  "`0`"または"`false`"のとき、inline名前空間ではないことを意味する。
  省略時の値は"`false`"である。

以下の子要素を持つ。

* `name`要素 -
  名前空間名
* `attributes`要素 -
  アトリビュートの並び

## `unnamedNamespaceNNS`要素 {#sec:nns.unnamednamespace}

無名名前空間を表現する。

| `<unnamedNamespaceNNS>`
|   `attributes`要素
| `</unnamedNamespaceNNS>`

属性(必須): `nns`

属性(optional): `is_inline`

* `nns` -
  このNNSに与えられたNNS識別名
* `is_inline` -
  "`1`"または"`true`"のとき、inline名前空間であることを意味する。
  "`0`"または"`false`"のとき、inline名前空間ではないことを意味する。
  省略時の値は"`false`"である。

以下の子要素を持つ。

* `attributes`要素 -
  アトリビュートの並び

## `classNNS`要素 {#sec:nns.class}

クラスを表現する。

| `<classNNS>`
|   `name`要素
| `</classNNS>`

属性(必須): `nns`, `type`

* `nns` -
  このNNSに与えられたNNS識別名
* `type` -
  typeTableにおいてこのクラス型に与えられたデータ型識別名

以下の子要素を持つ。

* `name`要素 -
  クラス名

## `enumNNS`要素 {#sec:nns.enum}

enumを表現する。

| `<enumNNS>`
|   `name`要素
| `</enumNNS>`

属性(必須): `nns`, `type`

* `nns` -
  このNNSに与えられたNNS識別名
* `type` -
  typeTableにおいてこのenum型に与えられたデータ型識別名

以下の子要素を持つ。

* `name`要素 -
  enum名

## `typedefTypeNNS`要素 {#sec:nns.typedef}

typedef名を表現する。

| `<typedefTypeNNS>`
|   `name`要素
| `</typedefTypeNNS>`

属性(必須): `nns`, `type`

* `nns` -
  このNNSに与えられたNNS識別名
* `type` -
  typeTableにおいてこの型と等価な型に与えられたデータ型識別名

以下の子要素を持つ。

* `name`要素 -
  typedef名

## `templateParamTypeNNS`要素 {#sec:nns.templateparam}

型テンプレート仮引数を表現する。

| `<templateParamTypeNNS>`
| `name`要素
| `<templateParamTypeNNS>`

属性(必須): `nns`

* `nns` -
  このNNSに与えられたNNS識別名

以下の子要素を持つ。

* `name`要素 -
  仮引数名

## `simpleTemplateIdNNS`要素 {#sec:nns.simpletemplateid}

実引数を与えられたクラステンプレートを表現する。

| `<simpleTemplateIdNNS>`
|   `<template>`
|     `name`要素
|   `</template>`
|   `<arguments>`
|     [ `name`要素
|     ... ]
|   `</arguments>`
| `</simpleTemplateIdNNS>`

属性(必須): `nns`

* `nns` -
  このNNSに与えられたNNS識別名

以下の子要素を持つ。

* `template`要素 -
  テンプレート名

* `arguments` -
  テンプレート実引数の並び

# テンプレート定義要素(C++) {#sec:temp}
テンプレート定義要素には、以下のものがある。

* `classTemplate`要素([-@sec:temp.class]節)　—　クラステンプレートを定義する。
* `funcitionTemplate`要素([-@sec:temp.func]節)　—　関数、メンバ関数、演算子オーバーロード、および、ユーザ定義リテラルのテンプレートを定義する。
* `aliasTemplate`要素([-@sec:temp.alias]節)　—　型のエイリアスのテンプレートを定義する。

これらのテンプレート定義要素は、共通して型仮引数を表現する`typeParams`要素([-@sec:temp.typeparams]節)をもつ。

## `typeParams`要素 {#sec:temp.typeparams}
テンプレートの型仮引数の並びを指定する。

| `<typeParams>`
|   [ { `typeName`要素([-@sec:type.typename]節)
|      [ `<value>`
|        `typeName`要素([-@sec:type.typename]節)
|      `</value>` ] }
|    … ]
| `</typeParams>`

属性なし

以下の子要素をもつことができる。

* `typeName`要素　－　型仮引数に対応するデータ型識別名を表現する。
* `value`要素　—　子要素として`typeName`要素をもつ。関数テンプレートまたはメンバ関数テンプレートにおいて、直前の`typeName`要素に対応する仮引数がデフォルト実引数をもつとき、それを表現する。

`typeName`要素は、引数の順序で並んでいなければならない。

## `classTemplate`要素 {#sec:temp.class}
データ型定義要素([-@sec:type]章)の一つ。クラスのテンプレートを以下のように表現する。

| `<classTemplate>`
|   `symbols`要素([-@sec:symb.local]節)
|   `typeParams`要素([-@sec:type.typeparams]節)
|   `classType`要素([-@sec:type.class]節)
| `</classTemplate>`


属性(optional): `lineno`, `file`

以下の子要素をもつ。

* `symbols`要素　—　型仮引数に関する`id`要素を子要素として持つ。
* `typeParams`要素　—　子要素として`typeName`要素の並びを持つ。
* `structType`要素または`classType`要素　—　構造体またはクラスの定義

例:

以下のプログラムは、

    template <typename T>
    struct pair { T val1, val2; };

以下のように表現される。

    <structTemplate>
      <symbols>
        <id type="S0" sclass="template_param">
          <name>T</name>
        </id>
      </symbols>
      <typeParams>
        <typeName ref="S0">
      </typeParams>
      <structType type="S1">
        <symbols>
          <id type="S0">
            <name>val1</name>
          </id>
          <id type="S0">
            <name>val2</name>
          </id>
        </symbols>
      </structType>
    </structTemplate>

ここで、データ型識別名`S0`は`typeTable`の中で以下のように定義されている。

    <basicType type="S0" name="any_typename"/>

## `functionTemplate`要素 {#sec:temp.func}
関数、メンバ関数、演算子オーバーロード、および、ユーザ定義リテラルのテンプレートを表現する。`globalDeclaration`要素([-@sec:decl.global]節)と`declaration`要素([-@sec:decl.local]節)の子要素。

| `<functionTemplate>`
|   `symbols`要素([-@sec:symb.local]節)
|   `typeParams`要素([-@sec:temp.typeparams]節)
|   `functionDefinition`要素([-@sec:decl.fndef]節)
| `</functionTemplate>`


属性(optional): `lineno`, `file`

以下の子要素をもつ。

* `symbols`要素　—　型仮引数に関する`id`要素を子要素として持つ。
* `typeParams`要素　—　子要素として`typeName`要素の並びを持つ。
* `functionDefinition`要素　—　関数の定義

例:

以下の関数テンプレートについて、

    template <class T>
    T square(const T& x) { return x * x; }

型仮引数`T`に対するデータ型識別名`X0`と、仮引数`x`の型`const T&`に対するデータ型識別名`X1`は、`typeTable`の中で以下のように定義される。

    <basicType type="X0" name="any_class"/>
    <basicType type="X1" is_const="1" is_lvalue="1" name="X0"/>

そして、関数テンプレートは以下のように定義される。

    <functionTemplate>
      <symbols>
        <id type="X0" sclass="template_param">
          <name>T</name>
        </id>
      </symbols>
      <typeParams>
        <typeName ref="X0">
      </typeParams>
      <functionDefinition>
        <name>square</name>
        <symbols>
          <id type="X1" sclass="param">
            <name>x</name>
          </id>
        </symbols>
        <typeParams>
          <name type="X1">x</name>
        </typeParams>
        <body>
          ・・・(略)・・・
        </body>
      </functionDefinition>
    </functionTemplate>

## `aliasTemplate`要素 {#sec:temp.alias}
データ型定義要素([-@sec:type]章)の一つ。エイリアステンプレートを表現する。

| `<aliasTemplate>`
|   `symbols`要素([-@sec:symb.local]節)
|   `typeParams`要素([-@sec:temp.typeparams]節)
| `</aliasTemplate>`

属性(必須): `type`, `name`

属性(optional): `lineno`, `file`

以下の属性を持つ。

* `type`　－　型の別名に与えられるデータ型識別名
* `name`　－　この型の元になる型のデータ型識別名

要検討:

属性名は`ref`がよいか`name`がよいか。

以下の子要素を持つ。

* `symbols`要素　—　型仮引数に関する`id`要素を子要素として持つ。
* `typeParams`要素　—　子要素として`typeName`要素の並びだけを持つことができる。

定義される別名は、この`type`属性値と同じ`type`属性値をもつ`id`要素で表現され、そのスコープのシンボルテーブル(`globalSymbols`または`symbols`)に登録される。

例:

以下の別名テンプレートについて、

    template <typename T>
    using myMap = std::map<int, T&>;

型仮引数`T`に対するデータ型識別名`X0`と、`std::map`の第2型引数`T&`に対するデータ型識別名`X0`は、`typeTable`の中で以下のように定義される。

    <basicType type="X0" name="any_typename"/>
    <basicType type="X1" is_lvalue="1" name="X0"/>

そして、別名テンプレートは以下のように定義される。ここで、`T0`は他で定義されている`std::map`のデータ型識別名であり、`std::map`の`id`要素の`type`属性と一致している。`T1`はこの`aliasTemplate`で定義されたデータ型識別名であり、`myMap`の`id`要素の`type`属性と一致している。

    <aliasTemplate type="T1" name="T0">
      <symbols>
        <id type="X0" sclass="template_param">
          <name>T</name>
        </id>
      </symbols>
      <typeParams>
        <typeName ref="X0">
      </typeParams>
    </aliasTemplate>

# テンプレートインスタンス要素(C++) {#sec:temp.instance}
テンプレートインスタンス要素には、以下のものがある。

* `typeInstance`要素([-@sec:temp.typeinstance]節)　—　構造型、クラス、および型の別名のテンプレートについて、型実引数を与えて具体化する。
* `funcitionInstance`要素([-@sec:temp.funcinstance]節)　—　関数、メンバ関数、演算子オーバーロード、および、ユーザ定義リテラルのテンプレートについて、型実引数を与えて具体化する。

これらのテンプレートインスタンス要素は、共通して型実引数を表現する`typeArtuments`要素([-@sec:temp.typearg]節)をもつ。

## `typeArguments`要素 {#sec:temp.typearg}
テンプレートのインスタンスの型実引数の並びを指定する。

| `<typeArguments>`
|   [ `typeName`要素([-@sec:type.typename]節)
|    … ]
| `</typeArguments>`

属性なし

以下の子要素をもつことができる。

* `typeName`要素　－　型実引数に対応するデータ型識別名を表現する。

`typeName`要素は、引数の順序で並んでいなければならない。

## `typeInstance`要素 {#sec:temp.typeinstance}
データ型定義要素([-@sec:type]章)の一つ。型のテンプレートのインスタンスを表現する。

| `<typeInstance>`
|   `typeArguments`要素([-@sec:temp.typearg]節)
| `</typeInstance>`


属性(optional): `type`, `ref`

以下の属性を持つ。

* `type`属性　—　`typeInstance`要素のデータ型識別名、すなわち表現されたインスタンスの型
* `ref`属性　—　対応するテンプレートのデータ型識別名

要検討:

属性名は`ref`がよいか`name`がよいか。

例:

構造型のテンプレート

    template <typename T1, typename T2, typename T3>
    struct Triple {
      ・・・(略)・・・
    };

のデータ型識別名を`TRI0`とするとき、そのインスタンス

    trile<int, int*, int**>

のデータ型識別名`TRI1`は、以下のように表現される。

    <typeInstance type="TRI1" ref="TRI0">
      <typeParameters>
        <typeName ref="int">
        <typeName ref="P0">
        <typeName ref="P1">
      </typeParameters>
    </typeInstance>

ここで、`P0`と`P1`は、以下のように定義されているデータ型識別名である。

    <pointerType type="P0" ref="int" />
    <pointerType type="P2" ref="int" />
    <pointerType type="P1" ref="P2" />

## `functionInstance`要素 {#sec:temp.funcinstance}
式の要素([-@sec:expr]章)の一つ。関数とメンバ関数のテンプレートのインスタンスを表現する。

| `<functionInstance>`
|   `typeArguments`要素([-@sec:temp.typearg]節)
|   `functionCall`要素([-@sec:expr.call]節)
| `</functionInstance>`

属性(必須): `type`, `name`

型実引数の並びは、スペースで区切られたデータ型識別名で表現する。

例:

`functionTemplate`要素([-@sec:temp.func]節)の例において、関数テンプレート

    template <class T>
    T square(const T& x) { return x * x; }

の`T`に対するデータ型識別名は以下の`X0`、仮引数`x`の型`const T&`に対するデータ型識別名は以下の`X1`である。

    <basicType type="X0" name="any_class"/>
    <basicType type="X1" is_const="1" is_lvalue="1" name="X0"/>

ここで、このテンプレート関数の参照

    square<int>(10)

は、以下のように表現される。

    <functionInstance>
      <typeArguments>
        <typeName ref="int"/>
      </typeArguments>
      <functionCall type="X0">
        <function>
          <funcAddr type="P0">square</funcAddr>
        </function>
        <arguments>
    　　   <intConstant type="init">10</intConstant>
        </arguments>
      </functionCall>
    <functionInstance>

ここで、`funcAddr`要素の`type`属性`P0`は、関数`square`へのポインタを意味するデータ型識別名である。


# XcalableMP固有の要素 {#sec:xmp}

備考:

C++対応版作成に当たって再検討していない。

## `coArrayType`要素 {#sec:xmp.coarray.type}
"`#pragma xmp coarray`" によって宣言された、Co-Array型を表す。 次の属性を持つ。

* `type`　－　派生データ型名。
* `element_type`　－　Co-Arrayの要素のデータ型名。データ型名に対応する型が`coArrayType`のときは、2次元以上のCo-Array型を表す。
* `array_size`　－　Co-Array次元を表す。

次の子要素を持つ。

* `arraySize`　－　Co-Array次元を表す。`arraySize`要素を持つときの `array_size` 属性の値は "`*`" とする。

例:

    int A[10];
    #pragma xmp coarray [*][2]::A

上記の変数`A`の型を表すXML要素は、次の`coArrayType` "`C2`"になる。

    <arrayType type="A1" element_type="int" array_size="10"/>
    <coArrayType type="C1" element_type="A1"/>
    <coArrayType type="C2" element_type="C1" array_size="2"/>

## `coArrayRef`要素 {#sec:xmp.coarray.ref}
Co-Array型の変数への参照を表す。
次の子要素を持つ。

* 1番目の式　－　Co-Array変数を表す式。
* 2番目以降の式　－　Co-Array次元を表す式。複数の次元を持つ場合は、複数の式を指定する。

## `subArrayRef`要素 {#sec:xmp.subarray.ref}
部分配列の参照を表す。
次の子要素を持つ。子要素を省略することはできない。

* 第一のXML要素として配列を表す式をもつ。
* 2番目以降の式　－　添字または添字3つ組を表す式。複数の次元を持つ場合は、複数の式を指定する。

## `indexRange`要素 {#sec:xmp.idx}
3つ組(triplet)を表す。
次の子要素を持つ。子要素を省略することはできない。

* `lowerBound`　－　下限のインデックスを表す。子要素に式を持つ。
* `upperBound`　－　上限のインデックスを表す。子要素に式を持つ。
* `step`　－　インデックスの刻み幅を表す。子要素に式を持つ。

# その他の要素・属性 {#sec:other}

## `is_gccExtension`属性 {#sec:other.isgcc}
`is_gccExtension`属性は、GCCの \_\_extension\_\_ キーワードをXML要素の先頭に付加するかどうかを定義し、値は `0` または `1` (`false`または`true`) である。
`is_gccExtension`属性は省略可能で、指定しないときは値 `0`を指定したときと同じ意味である。次のXML要素に `is_gccExtension` 属性を持つことができる。

* `id`
* `functionDefinition`
* `castExpr`
* `gccAsmDefinition`

例:

"`__extension__ typedef long long int64_t`" に対応する定義は次のようになる。

     <id type="long_long" sclass="typedef" is_gccExtension="1">
       <name>int64_t</name>
     </id>

## `gccAsm`要素、`gccAsmDefinition`要素、`gccAsmStatement`要素 {#sec:other.gcc.asm}
`gccAsm` 要素・`gccAsmDefinition`要素・`gccAsmStatement`要素は、GCCの asm/\_\_asm\_\_ キーワードを定義する。子要素として asm の引数の文字列を持つ。

* `gccAsm`　－　asm式を表す。次の子要素を持つ。
* `stringConstant` (1個)　－　アセンブラコードを表す。
* `gccAsmDefinition`　－　asm定義を表す。子要素は`gccAsm`と同じ。
* `gccAsmStatement`　－　asm文を表す。

次の属性を持つ。

* `is_volatile`　－　volatile が指定されているかどうかの情報、`0`または`1`、`false`または`true`。

次の子要素を持つ。

* `stringConstant` (1個)　－　アセンブラコードを表す。
* `gccAsmOperands` (2個)　－　1番目が出力オペランド、2番目が入力オペランドを表す。オペランドを省略する場合は、子要素を持たないタグを記述する。子要素に`gccAsmOperand`(複数)を持つ。
* `gccAsmClobbers` (0-1個)　－　クロバーを表す。子要素に0個以上の `stringConstant` を持つ。
* `gccAsmOperand`　－　入出力オペランドを表す。

次の属性を持つ。

* `match` (省略可) 　－　matching constraintの代わりに指定する識別子を表す("[識別子]" に対応)。
* `constraint` (省略不可) 　－　constraint/constraint modifierを表す。

次の子要素を持つ。

* 式 (1個)　－　入力または出力に指定する式を表す。

例:

      asm volatile (
           "661:\n"
           "\tmovl %0, %1\n662:\n"
           ".section .altinstructions,\"a\"\n"
           ".byte %c[feat]\n"
           ".previous\n"
           ".section .altinstr_replacement,\"ax\"\n"
           "663:\n"
           "\txchgl %0, %1\n"
           : "=r" (v), "=m" (*addr)
           : [feat] "i" (115), "0" (v), "m" (*addr));

           <gccAsmStatement is_volatile="1">
         <stringConstant><![CDATA[661:\n\tmovl .. (省略) ..]]></stringConstant>
         <gccAsmOperands>
           <gccAsmOperand constraint="=r">
             <Var>v</Var>
           </gccAsmOperand>
           <gccAsmOperand constraint="=m">
             <pointerRef><Var>addr</Var></pointerRef>
           </gccAsmOperand>
         </gccAsmOperands>
         <gccAsmOperands>
           <gccAsmOperand match="feat" constraint="i">
             <intConstant>115</intConstant>
           </gccAsmOperand>
           <gccAsmOperand constraint="m">
             <pointerRef><Var>addr</Var></pointerRef>
           </gccAsmOperand>
         </gccAsmOperands>
       </gccAsmStatement>

## `gccAttributes`要素 {#sec:other.gcc.attr}
`gccAttributes` 要素はGCCの \_\_attribute\_\_ キーワードを定義する。子要素として、\_\_attribute\_\_の引数の文字列を持つ。`gccAttributes` 要素は、`gccAttribute` 要素を子要素に複数持つ。

* 型を表すXML要素全てが `gccAttributes` 要素を子要素に持つ(0～1個)。
* `id` 要素が `gccAttributes` 要素を子要素に持つ(0～1個)。
* `functionDefinition` 要素が `gccAttributes` 要素を子要素に持つ(0～1個)。

例:

型を表すXML要素の子要素に、`gccAttributes` を設定する例

      typedef __attribute__((aligned(8))) int ia8_t;
      ia8_t __attribute__((aligned(16)) n;

     <typeTable>
        <basicType type="B0" name="int" align="8" size="4"/>
          <gccAttributes>
            <attribute>aligned(8)</attribute>
         </gccAttributes>
        </basicType>
        <basicType type="B1" name="int" align="16" size="4"/>
         <gccAttributes>
            <attribute>aligned(8)</attribute>
            <attribute>aligned(16)</attribute>
          </gccAttributes>
       </basicType>
      </typeTable>
     <globalSymbols>
       <id type="B0" sclass="typedef_name">
          <name>ia8_t</name>
        </id>
       <id type="B1">
          <name>n</name>
        </id>
      </globalSymbols>
     <globalDeclarations>
        <varDecl>
          <name>n</name>
        </varDecl>
      </globalDeclarations>

`id` 要素、`functionDefinition` 要素の子要素に、`gccAttributes` を設定する例

     void func(void);
     void func2(void) __attribute__(alias("func"));

      void __attribute__((noreturn)) func() {
         ...
      }


     <typeTable>
       <functionType type="F0">
         <params>
            <name type="void"/>
          </params>
        </functionType>
        <functionType type="F1">
          <params>
            <name type="void"/>
          </params>
        </functionType>
      </typeTable>
      <globalSymbols>
        <id type="F0" sclass="extern_def">
          <name>func</name>
        </id>
        <id type="F1" sclass="extern_def">
          <name>func2</name>
          <gccgccAttributes>
            <gccAttribute>alias("func")</gccAttribute>
          </gccgccAttributes>
        </id>
      </globalSymbols>
      <globalDeclarations>
        <functionDefinition>
          <name>func</name>
          <gccgccAttributes>
            <gccAttribute>noreturn</gccAttribute>
          </gccgccAttributes>
          <body>...</body>
        </functionDefinition>
      </globalDeclarations>

## `builtin_op`要素 {#sec:other.builtinop}
`builtin_op`要素はコンパイラ組み込みの関数呼び出しを表す。以下のXML要素をそれぞれ0～複数持つ。子要素の順番は関数引数の順番と一致していなければならない。

* 式　－　呼び出す関数の引数として、式を指定する。
* `typeName`　－　呼び出す関数の引数として、型名を指定する。
* `gccMemberDesignator`　－　呼び出す関数の引数として、構造体・共用体のメンバ指示子を指定する。属性に構造体・共用体の派生データ型名を示す ref、メンバ指示子の文字列を示す member を持つ。子要素に配列インデックスを表す式(0-1個)と、`gccMemberDesignator`要素(0-1個)を持つ。

## `is_gccSyntax`属性 {#sec:other.isgccsyntax}
`is_gccSyntax`属性はそのタグに対応する式、文、宣言がgcc拡張を使用しているかどうかを定義する。 値として`0` または `1` (`false`または`true`) を持つ。この属性は省略可能であり、省略された場合は値に`0`を指定した時と同じ意味になる。

## `is_modified`属性 {#sec:other.modified}
`is_modified`属性はそのタグに対応する式、文、宣言がコンパイルの過程で変形されたかどうかを定義する。値として`0` または `1` (`false`または`true`) を持つ。この属性は省略可能であり、省略された場合は値に`0`を指定した時と同じ意味になる。
次のXML要素に `is_gccSyntax` 属性、`is_modified` 属性を持つことができる。

* `varDecl`
* 文の要素
* 式の要素

# 未検討項目 {#sec:tbc}
以下の項目については、本ドキュメントで触れていない。

* 宣言
* asm ( … )
* 結合指定  `extern "C" double x;`
* クラス
* 部分特殊化
* final
* 純粋仮想関数、純粋指定子(=0)
* 例外処理
* noexceptキーワード
* 属性
* `[[ noreturn ]]` `[[ carries_dependency ]]` `[[ deprecated ]]`　


## コード例

例1:

      int a[10];
      int xyz;
      struct {    int x;   int y;} S;
      foo() {
        int *p;
        p =  &xyz;    /* 文1 */
        a[4] = S.y;    /* 文2 */
      }

文1:

      <exprStatement>
        <assignExpr type=" P6fc98">
       　　<ponterRef type=" P6fc98">
            <varAddr scope="local" type="P70768">p</varAddr>
           </pointerRef>
          <varAddr type=" P70828">xyz</varAddr>
        </assignExpr>
      </exprStatement>

もしくは、

      <exprStatement>
        <assignExpr type=" P6fc98">
      　　<Var scope="local" type=" P6fc98">p</Var>
           <varAddr type=" P70828">xyz</varAddr>
       </assignExpr>
      </exprStatement>


文2:

      <exprStatement>
       <assignExpr type="int">
          <pointerRef type="int">
            <plusExpr type=" P6fc98">
              <arrayAddr type=" P708e8">a</arrayAddr>
              <intConstant type="int">4</intConstant>
            </plusExpr>
         </pointerRef>
         <pointerRef type="int">
            <memberAddr type="P0dede" member="y">
               <varAddr type= "P70988">S</varAddr>
            </memberAddr>
        </pointerRef>
       </assignExpr>
      </exprStatement>

もしくは、

      <exprStatement>
       <assignExpr type="int">
          <pointerRef type="int">
            <plusExpr type=" P6fc98">
              <arrayAddr type=" P708e8">a</arrayAddr>
              <intConstant type="int">4</intConstant>
            </plusExpr>
         </pointerRef>
         <memberRef type="int" member="y">
            <varAddr type= "P70988">S</varAddr>
        </memberRef>
       </assignExpr>
      </exprStatement>


    <?xml version="1.0" encoding="ISO-8859-1"?>
    <XcodeProgram source="t3.c">
      <!--
        typedef struct complex {
            double real;
            double img;
        } complex_t;

        complex_t x;
        complex_t complex_add(complex_t x, double y);

        main()
        {
            complex_t z;

            x.real = 1.0;
            x.img = 2.0;

            z = complex_add(x,1.0);

            printf("z=(%f,%f)\n",z.real,z.img);

        }
        complex_t complex_add(complex_t x, double y)
        {
            x.real += y;
            return x;
        }
      -->
      <typeTable>
        <pointerType type="P0" ref="S0"/>
        <pointerType type="P1" ref="S0"/>
        <pointerType type="P2" ref="S0"/>
        <pointerType type="P3" ref="S0"/>
        <pointerType type="P4" ref="S0"/>
        <pointerType type="P5" ref="F0"/>
        <pointerType type="P6" is_restrict="1" ref="char"/>
        <pointerType type="P7" ref="F2"/>
        <structType type="S0">
          <symbols>
            <id type="double">
              <name>real</name>
            </id>
            <id type="double">
              <name>img</name>
            </id>
          </symbols>
        </structType>
        <functionType type="F0" return_type="S0">
          <params>
            <name type="S0">x</name>
            <name type="double">y</name>
          </params>
        </functionType>
        <functionType type="F1" return_type="int">
          <params/>
        </functionType>
        <functionType type="F2" return_type="int">
          <params/>
        </functionType>
        <functionType type="F3" return_type="S0">
          <params>
            <name type="S0">x</name>
            <name type="double">y</name>
          </params>
        </functionType>
      </typeTable>
      <globalSymbols>
        <id type="F0" sclass="extern_def">
          <name>complex_add</name>
        </id>
        <id type="S0" sclass="extern_def">
          <name>x</name>
        </id>
        <id type="F1" sclass="extern_def">
          <name>main</name>
        </id>
        <id type="F2" sclass="extern_def">
          <name>printf</name>
        </id>
        <id type="S0" sclass="typedef_name">
          <name>complex_t</name>
        </id>
        <id type="S0" sclass="tagname">
          <name>complex</name>
        </id>
      </globalSymbols>
      <globalDeclarations>
        <varDecl>
          <name>x</name>
        </varDecl>
        <funcDecl>
          <name>complex_add</name>
        </funcDecl>
        <functionDefinition>
          <name>main</name>
          <symbols>
            <id type="S0" sclass="auto">
              <name>z</name>
            </id>
          </symbols>
          <params/>
          <body>
            <compoundStatement>
              <symbols>
                <id type="S0" sclass="auto">
                  <name>z</name>
                </id>
              </symbols>
              <declarations>
                <varDecl>
                  <name>z</name>
                </varDecl>
              </declarations>
              <body>
                <exprStatement>
                  <assignExpr type="double">
                    <memberRef type="double" member="real">
                      <varAddr type="P0" scope="local">x</varAddr>
                    </memberRef>
                    <floatConstant type="double">1.0</floatConstant>
                  </assignExpr>
                </exprStatement>
                <exprStatement>
                  <assignExpr type="double">
                    <memberRef type="double" member="img">
                      <varAddr type="P1" scope="local">x</varAddr>
                    </memberRef>
                    <floatConstant type="double">2.0</floatConstant>
                  </assignExpr>
                </exprStatement>
                <exprStatement>
                  <assignExpr type="S0">
                    <Var type="S0" scope="local">z</Var>
                    <functionCall type="S0">
                      <function>
                        <funcAddr type="P5">complex_add</funcAddr>
                      </function>
                      <arguments>
                        <Var type="S0" scope="local">x</Var>
                        <floatConstant type="double">1.0</floatConstant>
                      </arguments>
                    </functionCall>
                  </assignExpr>
                </exprStatement>
                <exprStatement>
                  <functionCall type="int">
                    <function>
                      <funcAddr type="F2">printf</funcAddr>
                    </function>
                    <arguments>
                      <stringConstant>z=(%f,%f)\n</stringConstant>
                      <memberRef type="double" member="real">
                        <varAddr type="P2" scope="local">z</varAddr>
                      </memberRef>
                      <memberRef type="double" member="img">
                        <varAddr type="P3" scope="local">z</varAddr>
                      </memberRef>
                    </arguments>
                  </functionCall>
                </exprStatement>
              </body>
            </compoundStatement>
          </body>
        </functionDefinition>
        <functionDefinition>
          <name>complex_add</name>
          <symbols>
            <id type="S0" sclass="param">
              <name>x</name>
            </id>
            <id type="double" sclass="param">
              <name>y</name>
            </id>
          </symbols>
          <params>
            <name type="S0">x</name>
            <name type="double">y</name>
          </params>
          <body>
            <compoundStatement>
              <symbols>
                <id type="S0" sclass="param">
                  <name>x</name>
                </id>
                <id type="double" sclass="param">
                  <name>y</name>
                </id>
              </symbols>
              <declarations/>
              <body>
                <exprStatement>
                  <asgPlusExpr type="double">
                    <memberRef type="double" member="real">
                      <varAddr type="P4" scope="param">x</varAddr>
                    </memberRef>
                    <Var type="double" scope="param">y</Var>
                  </asgPlusExpr>
                </exprStatement>
                <returnStatement>
                  <Var type="S0" scope="param">x</Var>
                </returnStatement>
              </body>
            </compoundStatement>
          </body>
        </functionDefinition>
      </globalDeclarations>
    </XcodeProgram>
