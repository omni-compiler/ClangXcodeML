<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="xml" encoding="UTF-8"/>

  <xsl:template match="name">
    <xsl:choose>
      <xsl:when test="@name_kind = 'name'">
        <name>
          <xsl:copy-of select="../clangNestedNameSpecifier/@*"/>
          <xsl:copy-of select="@*"/>
          <xsl:value-of select="." />
        </name>
      </xsl:when>

      <xsl:when test="@name_kind = 'operator'">
        <operator>
          <xsl:copy-of select="../clangNestedNameSpecifier/@*"/>
          <xsl:copy-of select="@*"/>
          <xsl:value-of select="." />
        </operator>
      </xsl:when>

      <xsl:when test="@name_kind = 'constructor'">
        <constructor>
          <xsl:copy-of select="../clangNestedNameSpecifier/@*"/>
          <xsl:copy-of select="@*"/>
        </constructor>
      </xsl:when>

      <xsl:when test="@name_kind = 'destructor'">
        <destructor>
          <xsl:copy-of select="../clangNestedNameSpecifier/@*"/>
          <xsl:copy-of select="@*"/>
        </destructor>
      </xsl:when>

      <xsl:when test="@name_kind = 'conversion'">
        <conversion>
          <xsl:copy-of select="../clangNestedNameSpecifier/@*"/>
          <xsl:copy-of select="@*"/>
          <xsl:attribute name="type">
            <xsl:value-of
              select="../clangNestedNameSpecifier/@clang_name_type"/>
          </xsl:attribute>
        </conversion>
      </xsl:when>

      <xsl:otherwise>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="node()|@*">
    <xsl:copy>
      <xsl:apply-templates select="node()|@*"/>
    </xsl:copy>
  </xsl:template>
</xsl:stylesheet>
