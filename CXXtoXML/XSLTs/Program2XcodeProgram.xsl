<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="xml" encoding="UTF-8"/>
  <xsl:template match="Program">
    <XcodeProgram>
      <xsl:apply-templates />
    </XcodeProgram>
  </xsl:template>

  <xsl:template match="clangDecl[@class=&quot;TranslationUnit&quot;]">
    <globalDeclarations>
      <xsl:apply-templates />
    </globalDeclarations>
  </xsl:template>

  <xsl:template match="clangAST">
    <xsl:apply-templates />
  </xsl:template>

  <xsl:template match="clangDecl[@class=&quot;Function&quot;]">
    <xsl:choose>
      <xsl:when test="clangStmt">
        <functionDefinition>
          <name>
            <xsl:value-of select=
              "clangDeclarationNameInfo[@class=&quot;Identifier&quot;]" />
          </name>
          <body>
            <xsl:apply-templates select="clangStmt" />
          </body>
        </functionDefinition>
      </xsl:when>
      <xsl:otherwise>
        <functionDecl>
          <name>
            <xsl:value-of select=
              "clangDeclarationNameInfo[@class=&quot;Identifier&quot;]" />
          </name>
        </functionDecl>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="clangDecl[@class=&quot;Var&quot;]">
    <varDecl>
      <name>
        <xsl:attribute name="fullName">
          <xsl:value-of select="fullName" />
        </xsl:attribute>
      </name>
    </varDecl>
  </xsl:template>

  <xsl:template match="clangStmt[@class=&quot;IfStmt&quot;]">
    <ifStatement>
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

  <xsl:template match="clangStmt[@class=&quot;ReturnStmt&quot;]">
    <returnStmt>
      <xsl:apply-templates />
    </returnStmt>
  </xsl:template>

  <xsl:template match="node()|@*">
    <xsl:copy>
      <xsl:apply-templates select="node()|@*"/>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="clangStmt[@class=&quot;CompoundStmt&quot;]">
    <compoundStmt>
      <xsl:apply-templates />
    </compoundStmt>
  </xsl:template>
</xsl:stylesheet>
