<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output
    encoding="UTF-8"
    indent="yes"
    method="xml" />

  <xsl:template name="emit-id-list-in-namespace">
    <xsl:for-each select="clangDecl">
      <xsl:choose>
        <!-- language linkage specification -->
        <xsl:when test="@class = 'LinkageSpec'">
          <xsl:call-template name="emit-id-list-in-namespace" />
        </xsl:when>

        <!-- C++ class declaration -->
        <xsl:when test="@class = 'CXXRecord'">
          <id sclass="class_name">
            <xsl:attribute name="type">
              <xsl:value-of select="@xcodemlType" />
            </xsl:attribute>
            <xsl:copy-of select="name" />
          </id>
          <xsl:call-template name="emit-id-lists-in-class" />
        </xsl:when>

        <!-- typedef declaration -->
        <xsl:when test="@class = 'Typedef'">
          <id sclass="typedef_name">
            <xsl:copy-of select="@*" />
            <xsl:attribute name="type">
              <xsl:value-of select="@xcodemlTypedefType" />
            </xsl:attribute>
            <xsl:copy-of select="name" />
          </id>
        </xsl:when>

        <!--
             otherwise, emit an <id> node if this <clangDecl>
             node has name and type
        -->
        <xsl:when
            test="
            (name)
            and @xcodemlType">
          <id>
            <xsl:attribute name="type">
              <xsl:value-of select="@xcodemlType" />
            </xsl:attribute>

            <xsl:attribute name="sclass">
              <xsl:choose>
                <xsl:when test="@class = 'Function'">
                  <xsl:text>extern_def</xsl:text>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:text>__unknown__</xsl:text>
                </xsl:otherwise>
              </xsl:choose>
            </xsl:attribute>

            <xsl:copy-of select="name" />
          </id>
        </xsl:when>
      </xsl:choose>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="emit-id-lists-in-class">
    <!--
         A function defined in a friend declaration belongs
         to the namespace that encloses the class.

         Example:
          ```
          namespace NS {
            class A {
              class B {
                friend void func() { } // belongs to NS
              };
            };
          }
          ```
     -->
    <xsl:for-each select="clangDecl">
      <xsl:choose>
        <!-- friend function declaration or definition -->
        <xsl:when
            test="(@class = 'Friend')
                  and (clangDecl[@class='Function'])">
          <id sclass="__friend__">
            <xsl:attribute name="type">
              <xsl:value-of
                select="clangDecl[@class='Function']/@xcodemlType" />
            </xsl:attribute>
            <name>
              <xsl:value-of
                select="clangDecl[@class='Function']/name" />
            </name>
          </id>
        </xsl:when>

        <!-- child class declaration -->
        <xsl:when test="@class = 'CXXRecord'">
          <!--
          <id>
            <name><xsl:value-of select="name" /></name>
          </id>
          -->
          <xsl:call-template name="emit-id-lists-in-class" />
        </xsl:when>

        <!-- otherwise, emit nothing -->
        <xsl:otherwise />
      </xsl:choose>
    </xsl:for-each>
  </xsl:template>

  <xsl:template match="/Program">
    <Program>
      <xsl:apply-templates select="@*" />

      <typeTable>
        <xsl:apply-templates
            select="
            clangDecl[@class='TranslationUnit']/
            xcodemlTypeTable/*"/>
      </typeTable>

      <nnsTable>
        <xsl:apply-templates
            select="
            clangDecl[@class='TranslationUnit']/
            xcodemlNnsTable/*"/>
      </nnsTable>

      <globalSymbols>
        <xsl:for-each
          select="clangDecl[@class='TranslationUnit']">
          <xsl:call-template name="emit-id-list-in-namespace"/>
        </xsl:for-each>
      </globalSymbols>

      <xsl:apply-templates select="clangDecl" />

    </Program>
  </xsl:template>

  <xsl:template match="xcodemlTypeTable" />

  <xsl:template match="xcodemlNnsTable" />

  <xsl:template match="node()|@*">
    <xsl:copy>
      <xsl:apply-templates select="node()|@*"/>
    </xsl:copy>
  </xsl:template>

</xsl:stylesheet>
