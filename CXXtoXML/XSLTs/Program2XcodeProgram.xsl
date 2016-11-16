<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="xml" encoding="UTF-8"/>
  <xsl:template match="Program">
    <XcodeProgram>
      <xsl:apply-templates />
    </XcodeProgram>
  </xsl:template>

  <xsl:template match="Decl_TranslationUnit">
    <globalDeclarations>
      <xsl:apply-templates />
    </globalDeclarations>
  </xsl:template>

  <xsl:template match="clangAST">
    <xsl:apply-templates />
  </xsl:template>

  <xsl:template match="Decl_Function">
    <xsl:choose>
      <xsl:when test="Stmt_CompoundStmt">
        <functionDefinition>
          <name>
            <xsl:value-of select="DeclarationNameInfo_Identifier" />
          </name>
          <body>
            <xsl:apply-templates select="Stmt_CompoundStmt" />
          </body>
        </functionDefinition>
      </xsl:when>
      <xsl:when test="Stmt_CXXTryStmt">
        <functionDefinition>
          <name>
            <xsl:value-of select="DeclarationNameInfo_Identifier" />
          </name>
          <body>
            <xsl:apply-templates select="Stmt_CXXTryStmt" />
          </body>
        </functionDefinition>
      </xsl:when>
      <xsl:otherwise>
        <functionDecl>
          <name>
            <xsl:value-of select="DeclarationNameInfo_Identifier" />
          </name>
        </functionDecl>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="Stmt_IfStmt">
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

  <xsl:template match="node()|@*">
    <xsl:copy>
      <xsl:apply-templates select="node()|@*"/>
    </xsl:copy>
  </xsl:template>
</xsl:stylesheet>
