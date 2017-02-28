<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="xml" encoding="UTF-8"/>

  <xsl:template match="/Program">
    <Program>
      <xsl:apply-templates select="@*" />

      <typeTable>
        <xsl:apply-templates
          select="clangAST/
            clangDecl[@class='TranslationUnit']/
            xcodemlTypeTable/*" />
      </typeTable>

      <globalSymbols>
        <xsl:for-each
          select="clangAST/clangDecl[@class='TranslationUnit']/clangDecl">
          <id>
            <name>
              <xsl:value-of select="fullName" />
            </name>
          </id>
        </xsl:for-each>
      </globalSymbols>

      <xsl:apply-templates select="clangAST" />

    </Program>
  </xsl:template>

  <xsl:template match="clangStmt[@class='CompoundStmt']">
    <clangStmt>
      <xsl:apply-templates select="@*" />

      <symbols>
        <xsl:for-each select="clangStmt[@class='DeclStmt']/*">
          <id>
            <name>
              <xsl:value-of select="fullName" />
            </name>
          </id>
        </xsl:for-each>
      </symbols>

      <xsl:apply-templates />
    </clangStmt>
  </xsl:template>

  <xsl:template match="node()|@*">
    <xsl:copy>
      <xsl:apply-templates select="node()|@*"/>
    </xsl:copy>
  </xsl:template>

</xsl:stylesheet>
