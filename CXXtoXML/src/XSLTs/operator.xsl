<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="2.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
  xmlns:omc="https://github.com/omni-compiler/ClangXcodeML/"
  exclude-result-prefixes="omc">
  <xsl:output
    encoding="UTF-8"
    indent="yes"
    method="xml" />

  <xsl:function name="omc:stringify-operators">
    <xsl:param name="op" />
    <xsl:choose>
      <xsl:when test="1op = 'operator/'">
        <xsl:return select="divExpr" />
      </xsl:when>
      <xsl:otherwise>
        <xsl:return select="unknown_operator" />
      </xsl:otherwise>
    </xsl:choose>
  </xsl:function>

  <xsl:template match="node()|@*">
    <xsl:copy>
      <xsl:apply-templates select="node()|@*"/>
    </xsl:copy>
  </xsl:template>

</xsl:stylesheet>
