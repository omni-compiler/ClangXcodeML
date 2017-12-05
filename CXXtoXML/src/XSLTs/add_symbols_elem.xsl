<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output
    encoding="UTF-8"
    indent="yes"
    method="xml" />

  <xsl:template match="clangStmt[@class='CompoundStmt']">
    <clangStmt>
      <xsl:apply-templates select="@*" />

      <symbols>
        <xsl:for-each select="clangStmt[@class='DeclStmt']/*">
          <id>
            <xsl:attribute name="type">
              <xsl:value-of select="@xcodemlType" />
            </xsl:attribute>

            <xsl:attribute name="sclass">
              <xsl:choose>
                <xsl:when test="@class = 'Var'">
                  <xsl:text>auto</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:text>__unknown__</xsl:text>
                </xsl:otherwise>
              </xsl:choose>
            </xsl:attribute>

            <xsl:copy-of select="name"/>
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
