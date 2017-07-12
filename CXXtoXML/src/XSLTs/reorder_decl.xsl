<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output
    encoding="UTF-8"
    indent="yes"
    method="xml" />

  <xsl:template
    match="clangStmt[@class='ForStmt'
    and (clangStmt[1]/@class) = 'DeclStmt']">
    <clangStmt class="CompoundStmt">
      <xsl:copy-of select="*[1]"/>
      <xsl:copy>
        <xsl:copy-of select="@*"/>

        <xsl:copy-of select="*[position() > 1]"/>
      </xsl:copy>
    </clangStmt>
  </xsl:template>

  <xsl:template match="node()|@*">
    <xsl:copy>
      <xsl:apply-templates select="node()|@*"/>
    </xsl:copy>
  </xsl:template>
</xsl:stylesheet>
