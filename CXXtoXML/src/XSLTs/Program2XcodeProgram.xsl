<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output
    encoding="UTF-8"
    indent="yes"
    method="xml" />
  <xsl:template match="Program">
    <XcodeProgram>
      <xsl:apply-templates />
    </XcodeProgram>
  </xsl:template>

  <xsl:template match="enumType">
    <enumType>
      <xsl:apply-templates select="@*" />
      <symbols>
        <xsl:for-each select="symbols/clangDecl[@class='EnumConstant']" >
          <id>
            <xsl:apply-templates select="name" />
          </id>
        </xsl:for-each>
      </symbols>
    </enumType>
  </xsl:template>

  <xsl:template match="clangDecl[@class='TranslationUnit']">
    <globalDeclarations>
      <xsl:apply-templates />
    </globalDeclarations>
  </xsl:template>

  <xsl:template match="clangAST">
    <xsl:apply-templates />
  </xsl:template>

  <xsl:template match="clangDecl[@class='Function'
      or @class='CXXMethod'
      or @class='CXXConversion'
      or @class='CXXConstructor'
      or @class='CXXDestructor']">
    <xsl:choose>
      <xsl:when test="(@is_implicit='true') or (@is_implicit='1')" />
      <xsl:when test="clangStmt">
        <functionDefinition>
          <xsl:apply-templates select="@*" />
          <xsl:apply-templates select="name" />

          <xsl:apply-templates select="TypeLoc" />

          <xsl:if test="@class='CXXConstructor'">
            <constructorInitializerList>
              <xsl:apply-templates select="clangConstructorInitializer" />
            </constructorInitializerList>
          </xsl:if>

          <body>
            <xsl:apply-templates select="clangStmt" />
          </body>
        </functionDefinition>
      </xsl:when>
      <xsl:otherwise>
        <functionDecl>
          <xsl:apply-templates select="@*" />
          <xsl:apply-templates select="name" />
        </functionDecl>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="clangConstructorInitializer">
    <xsl:if test="@is_written = '1' or @is_written = 'true'">
      <constructorInitializer>
        <xsl:copy-of select="@*" /> <!-- including @member -->
        <xsl:apply-templates select="@xcodemlType" />
        <xsl:apply-templates select="clangStmt[1]" />
      </constructorInitializer>
    </xsl:if>
  </xsl:template>

  <xsl:template match="clangDecl[@class='Var']">
    <xsl:choose>
      <xsl:when test="(@is_implicit = 'true') or (@is_implicit = '1')"/>
      <xsl:otherwise>
        <varDecl>
          <xsl:apply-templates select="@*" />
          <xsl:apply-templates select="name" />
          <xsl:if test="@has_init = 1">
            <value>
              <xsl:apply-templates select="clangStmt" />
            </value>
          </xsl:if>
        </varDecl>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="clangDecl[@class='Record']" />

  <xsl:template match="clangDecl[@class='Field']">
    <varDecl>
      <xsl:apply-templates select="@*" />
      <xsl:apply-templates select="name" />
    </varDecl>
  </xsl:template>

  <xsl:template match="clangDecl[@class='Using']">
    <usingDecl>
      <xsl:apply-templates select="@*" />
      <xsl:apply-templates select="name"/>
    </usingDecl>
  </xsl:template>

  <xsl:template match="clangStmt[@class='IfStmt']">
    <ifStatement>
      <xsl:apply-templates select="@*" />
      <condition>
        <xsl:apply-templates select="*[1]" />
      </condition>
      <then>
        <xsl:apply-templates select="*[2]" />
      </then>
      <else>
        <xsl:apply-templates select="*[3]" />
      </else>
    </ifStatement>
  </xsl:template>

  <xsl:template match="clangStmt[@class='SwitchStmt']">
    <switchStatement>
      <value>
        <xsl:apply-templates select="*[1]" />
      </value>
      <body>
        <xsl:apply-templates select="*[position() &gt; 1]" />
      </body>
    </switchStatement>
  </xsl:template>

  <xsl:template match="clangStmt[@class='CaseStmt']">
    <caseLabel>
      <value>
        <xsl:apply-templates select="*[1]" />
      </value>
    </caseLabel>
    <xsl:apply-templates select="*[position() &gt; 1]" />
  </xsl:template>

  <xsl:template match="clangStmt[@class='DefaultStmt']">
    <defaultLabel/>
    <xsl:apply-templates />
  </xsl:template>

  <xsl:template match="clangStmt[@class='ReturnStmt']">
    <returnStatement>
      <xsl:apply-templates />
    </returnStatement>
  </xsl:template>

  <xsl:template match="clangStmt[@class='WhileStmt']">
    <whileStatement>
      <xsl:apply-templates select="@*" />
      <condition>
        <xsl:apply-templates select="*[1]" />
      </condition>
      <body>
        <xsl:apply-templates select="*[2]" />
      </body>
    </whileStatement>
  </xsl:template>

  <xsl:template match="clangStmt[@class='DoStmt']">
    <doStatement>
      <xsl:apply-templates select="@*" />
      <body>
        <xsl:apply-templates select="*[1]" />
      </body>
      <condition>
        <xsl:apply-templates select="*[2]" />
      </condition>
    </doStatement>
  </xsl:template>

  <xsl:template match="clangStmt[@class='ForStmt']">
    <forStatement>
      <init>
        <xsl:apply-templates select="*[@for_stmt_kind='init']" />
      </init>
      <condition>
        <xsl:apply-templates select="*[@for_stmt_kind='cond']" />
      </condition>
      <iter>
        <xsl:apply-templates select="*[@for_stmt_kind='iter']" />
      </iter>
      <body>
        <xsl:apply-templates select="*[@for_stmt_kind='body']" />
      </body>
    </forStatement>
  </xsl:template>

  <xsl:template match="clangStmt[@class='CompoundStmt']">
    <compoundStatement>
      <xsl:apply-templates select="@*" />
      <xsl:apply-templates />
    </compoundStatement>
  </xsl:template>

  <xsl:template match="clangStmt[@class='DeclStmt']">
    <xsl:apply-templates />
  </xsl:template>

  <xsl:template match="clangStmt[
    (@class='BinaryOperator' or @class='CompoundAssignOperator')
    and @binOpName]">
    <xsl:element name="{@binOpName}">
      <xsl:apply-templates select="@*" />
      <xsl:apply-templates select="*[1]" />
      <xsl:apply-templates select="*[2]" />
    </xsl:element>
  </xsl:template>

  <xsl:template match="clangStmt[@class='ArraySubscriptExpr']">
    <arrayRef>
      <xsl:apply-templates />
    </arrayRef>
  </xsl:template>

  <xsl:template match="clangStmt[@class='ConditionalOperator']">
    <xsl:element name="condExpr">
      <xsl:apply-templates select="@*" />
      <xsl:apply-templates select="*[1]" />
      <xsl:apply-templates select="*[2]" />
      <xsl:apply-templates select="*[3]" />
    </xsl:element>
  </xsl:template>

  <xsl:template match="clangStmt[@class='BinaryConditionalOperator']">
    <xsl:element name="condExpr">
      <xsl:apply-templates select="@*" />
      <xsl:apply-templates select="*[1]" />
      <xsl:apply-templates select="*[2]" />
    </xsl:element>
  </xsl:template>

  <xsl:template match="clangStmt[@class='CallExpr']">
    <functionCall>
      <xsl:apply-templates select="@*" />
      <function>
        <xsl:apply-templates select="*[1]" />
      </function>
      <arguments>
        <xsl:apply-templates select="*[position() &gt; 1]" />
      </arguments>
    </functionCall>
  </xsl:template>

  <xsl:template match="clangStmt[@class='CXXOperatorCallExpr']">
    <functionCall>
      <xsl:apply-templates select="@*" />
      <xsl:choose>

        <xsl:when test="clangStmt[1]/clangStmt[1]/@declkind = 'CXXMethod'">
          <memberFunction>
            <memberExpr>
              <xsl:apply-templates select="clangStmt[2]" />
              <name name_kind="operator">
                <xsl:value-of select="@xcodeml_operator_kind" />
              </name>
            </memberExpr>
          </memberFunction>
          <arguments>
            <xsl:apply-templates select="*[position() &gt; 2]" />
          </arguments>
        </xsl:when>

        <xsl:otherwise>
          <operator>
            <xsl:value-of select="@xcodeml_operator_kind" />
          </operator>
          <arguments>
            <xsl:apply-templates select="*[position() &gt; 1]" />
          </arguments>
        </xsl:otherwise>

      </xsl:choose>
    </functionCall>
  </xsl:template>

  <xsl:template match="clangStmt[@class='CXXNewExpr'
    and (@is_new_array='true' or @is_new_array='1')]">
    <newArrayExpr>
      <xsl:apply-templates select="@*" />
      <size>
        <xsl:apply-templates select="clangStmt[1]" />
      </size>
      <xsl:if test="clangStmt[2]">
        <arguments>
          <xsl:apply-templates
            select="clangStmt[@class='InitListExpr']/*" />
        </arguments>
      </xsl:if>
    </newArrayExpr>
  </xsl:template>

  <xsl:template match="clangStmt[@class='CXXNewExpr'
    and not(@is_new_array='true' or @is_new_array='1')]">
    <newExpr>
      <xsl:apply-templates select="@*" />
      <xsl:if test="clangStmt">
        <arguments>
          <xsl:choose>
            <xsl:when test="clangStmt[@class='CXXConstructExpr']">
              <!-- class types -->
              <!-- parse inside CXXConstructExpr -->
              <xsl:apply-templates
                select="clangStmt[@class='CXXConstructExpr']/*" />
            </xsl:when>
            <xsl:otherwise>
              <!-- scalar types -->
              <xsl:apply-templates select="clangStmt" />
            </xsl:otherwise>
          </xsl:choose>
        </arguments>
      </xsl:if>
    </newExpr>
  </xsl:template>

  <xsl:template match="clangStmt[@class='DeclRefExpr']">
    <Var>
      <xsl:if test="clangNestedNameSpecifier">
        <xsl:attribute name="nns">
          <xsl:value-of select="clangNestedNameSpecifier/@nns" />
        </xsl:attribute>
      </xsl:if>
      <xsl:value-of
        select="clangDeclarationNameInfo[@class='Identifier']" />
    </Var>
  </xsl:template>

  <xsl:template match="clangStmt[
    @class='UnaryOperator' and @unaryOpName]">
    <xsl:element name="{@unaryOpName}">
      <xsl:apply-templates select="*[1]|@*" />
    </xsl:element>
  </xsl:template>

  <xsl:template match="clangStmt[@class='InitListExpr']">
    <value>
      <xsl:apply-templates />
    </value>
  </xsl:template>

  <xsl:template match="clangStmt[@class='ImplicitCastExpr']">
    <implicitCastExpr>
      <xsl:apply-templates select="@*" />
      <xsl:apply-templates select="*[1]" />
    </implicitCastExpr>
  </xsl:template>

  <xsl:template match="clangStmt[@class='CStyleCastExpr']">
    <castExpr>
      <xsl:apply-templates select="@*" />
      <xsl:apply-templates select="*[2]" />
    </castExpr>
  </xsl:template>

  <xsl:template match="clangStmt[@class='CharacterLiteral']">
    <intConstant>
      <xsl:apply-templates select="@*" />
      <xsl:value-of select="@hexadecimalNotation" />
    </intConstant>
  </xsl:template>

  <xsl:template match="clangStmt[@class='IntegerLiteral']">
    <intConstant>
      <xsl:apply-templates select="@*" />
      <xsl:value-of select="@decimalNotation" />
    </intConstant>
  </xsl:template>

  <xsl:template match="clangStmt[@class='FloatingLiteral']">
    <floatConstant>
      <xsl:apply-templates select="@*" />
      <xsl:value-of select="@token" />
    </floatConstant>
  </xsl:template>

  <xsl:template match="clangStmt[@class='StringLiteral']">
    <stringConstant>
      <xsl:apply-templates select="@*" />
      <xsl:value-of select="@stringLiteral" />
    </stringConstant>
  </xsl:template>

  <xsl:template match="clangStmt[@class='CXXMemberCallExpr']">
    <functionCall>
      <memberFunction>
        <xsl:apply-templates select="clangStmt[1]" />
      </memberFunction>
      <arguments>
        <xsl:apply-templates select="clangStmt[position() &gt; 1]" />
      </arguments>
    </functionCall>
  </xsl:template>

  <xsl:template match="clangStmt[@class='CXXThisExpr']">
    <thisExpr />
  </xsl:template>

  <xsl:template match="clangStmt[@class='MemberExpr']">
    <xsl:variable
      name="is_anon"
      select="(@is_access_to_anon_record = '1')
              or (@is_access_to_anon_record = 'true')" />
    <xsl:variable
      name="is_arrow"
      select="(@is_arrow = '1')
              or (@is_arrow = 'true')" />
    <xsl:variable
      name="elemName"
      select="concat(substring('xcodemlAccessToAnonRecordExpr',
                               1 div $is_anon),
                     substring('memberExpr',
                               1 div (not($is_anon) and not($is_arrow))),
                     substring('memberRef',
                               1 div (not($is_anon) and $is_arrow)))"/>
    <!-- "if ($is_anon)
         then ('xcodemlAccessToAnonRecordExpr')
         else ('memberRef')" -->
    <xsl:element name="{$elemName}">
      <xsl:apply-templates select="@*" />
      <!-- lhs of member access (object) -->
      <xsl:choose>
        <xsl:when test="$is_anon" />
        <xsl:when test="$is_arrow">
          <xsl:apply-templates select="clangStmt" />
        </xsl:when>
        <xsl:otherwise>
          <xsl:apply-templates select="clangStmt" />
        </xsl:otherwise>
      </xsl:choose>

      <!-- rhs of member access (name) -->
      <xsl:apply-templates select="name" />
    </xsl:element>
  </xsl:template>

  <xsl:template match="sizeOfExpr">
    <sizeOfExpr>
      <xsl:apply-templates select="*[1]" />
    </sizeOfExpr>
  </xsl:template>

  <xsl:template match="name">
    <name>
      <xsl:copy-of select="../clangNestedNameSpecifier/@*" />
      <xsl:apply-templates select="@*" />
      <xsl:value-of select="." />
    </name>
  </xsl:template>

  <xsl:template match="@valueCategory">
    <xsl:attribute name="reference">
      <xsl:choose>
        <xsl:when test=".=lvalue">
          <xsl:text>lvalue</xsl:text>
        </xsl:when>
        <xsl:otherwise>
          <xsl:text>rvalue</xsl:text>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:attribute>
  </xsl:template>

  <xsl:template match="@xcodemlType">
    <xsl:attribute name="type">
      <xsl:value-of select="." />
    </xsl:attribute>
  </xsl:template>

  <xsl:template match="node()|@*">
    <xsl:copy>
      <xsl:apply-templates select="node()|@*"/>
    </xsl:copy>
  </xsl:template>
</xsl:stylesheet>
