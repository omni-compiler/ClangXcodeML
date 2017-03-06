<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="xml" encoding="UTF-8"/>
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
            <name>
              <xsl:apply-templates select="fullName/@*" />
              <xsl:value-of select="fullName" />
            </name>
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

  <xsl:template match="clangDecl[@class='Function']">
    <xsl:choose>
      <xsl:when test="clangStmt">
        <functionDefinition>
          <xsl:apply-templates select="@*" />
          <name>
            <xsl:value-of select=
              "clangDeclarationNameInfo[@class='Identifier']" />
          </name>

          <xsl:apply-templates select="params" />

          <body>
            <xsl:apply-templates select="clangStmt" />
          </body>
        </functionDefinition>
      </xsl:when>
      <xsl:otherwise>
        <functionDecl>
          <xsl:apply-templates select="@*" />
          <name>
            <xsl:value-of select=
              "clangDeclarationNameInfo[@class='Identifier']" />
          </name>
        </functionDecl>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="clangDecl[@class='Var']">
    <varDecl>
      <xsl:apply-templates select="@*" />
      <name>
        <xsl:value-of select="fullName" />
      </name>
      <xsl:if test="@has_init = 1">
        <value>
          <xsl:apply-templates select="clangStmt" />
        </value>
      </xsl:if>
    </varDecl>
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

  <xsl:template match="clangStmt[@class='CompoundStmt']">
    <compoundStatement>
      <xsl:apply-templates select="@*" />
      <xsl:apply-templates />
    </compoundStatement>
  </xsl:template>

  <xsl:template match="clangStmt[@class='DeclStmt']">
    <xsl:apply-templates select="*[1]" />
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

  <xsl:template match="clangDeclarationNameInfo[@class='Identifier']">
    <Var>
      <xsl:value-of select="." />
    </Var>
  </xsl:template>

  <xsl:template match="clangStmt[
    @class='UnaryOperator' and @unaryOpName]">
    <xsl:element name="{@unaryOpName}">
      <xsl:apply-templates select="*[1]|@*" />
    </xsl:element>
  </xsl:template>

  <xsl:template match="clangStmt[@class='ImplicitCastExpr']">
    <implicitCastExpr>
      <xsl:apply-templates select="@*" />
      <xsl:apply-templates select="*[1]" />
    </implicitCastExpr>
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

  <xsl:template match="clangStmt[@class='StringLiteral']">
    <stringConstant>
      <xsl:apply-templates select="@*" />
      <xsl:value-of select="@stringLiteral" />
    </stringConstant>
  </xsl:template>

  <xsl:template match="clangStmt[@class='CXXThisExpr']">
    <thisExpr />
  </xsl:template>

  <xsl:template match="clangStmt[@class='MemberExpr']">
    <memberRef>
      <xsl:apply-templates select="@*" />
      <xsl:attribute name="member">
        <xsl:value-of select="clangDeclarationNameInfo[@class='Identifier']" />
      </xsl:attribute>
      <xsl:apply-templates select="*[2]" />
    </memberRef>
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
