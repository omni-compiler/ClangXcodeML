<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="xml" encoding="UTF-8"/>

  <xsl:template name="emit-id-list">
    <xsl:for-each select="clangDecl">
      <xsl:choose>
        <!-- language linkage specification -->
        <xsl:when test="@class = 'LinkageSpec'">
          <xsl:call-template name="emit-id-lists-in-externC" />
        </xsl:when>

        <!-- C++ class declaration -->
        <xsl:when test="@class = 'CXXRecord'">
          <id>
            <name><xsl:value-of select="fullName" /></name>
          </id>
          <xsl:call-template name="emit-id-lists-of-friends" />
        </xsl:when>

        <!-- otherwise, this declaration shall introduce a name -->
        <xsl:otherwise>
          <id>
            <xsl:attribute name="type">
              <xsl:value-of select="@xcodemlType" />
            </xsl:attribute>
            <name>
              <xsl:value-of select="fullName" />
            </name>
          </id>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="emit-id-lists-in-externC">
    <xsl:for-each select="clangDecl">
      <id>
        <name>
          <xsl:value-of select="fullName" />
        </name>
      </id>
    </xsl:for-each>

    <xsl:for-each
      select="
        clangDecl[@class='LinkageSpec']">
      <xsl:call-template name="emit-id-lists-in-externC" />
    </xsl:for-each>
  </xsl:template>

  <xsl:template name="emit-id-lists-of-friends">
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
    <xsl:for-each
      select="
        clangDecl[@class='Friend']/
        clangDecl[@class='Function']">
      <id>
        <name>
          <xsl:value-of select="fullName" />
        </name>
      </id>
    </xsl:for-each>
    <xsl:for-each
      select="
        clangDecl[@class='CXXRecord']">
      <xsl:call-template name="emit-id-lists-of-friends" />
    </xsl:for-each>
  </xsl:template>

  <xsl:template match="/Program">
    <Program>
      <xsl:apply-templates select="@*" />

      <xsl:apply-templates select="typeTable" />

      <globalSymbols>
        <xsl:for-each
          select="clangAST/clangDecl[@class='TranslationUnit']">
          <xsl:call-template name="emit-id-list"/>
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
            <xsl:attribute name="type">
              <xsl:value-of select="@xcodemlType" />
            </xsl:attribute>
            <name>
              <xsl:value-of select="fullName" />
            </name>
          </id>
        </xsl:for-each>
      </symbols>

      <xsl:apply-templates />
    </clangStmt>
  </xsl:template>

  <xsl:template match="clangDecl[@class='Function']">
    <clangDecl>
      <xsl:apply-templates select="@*" />

      <params>
        <xsl:for-each
          select="TypeLoc[@class='FunctionProto']
            /clangDecl[@class='ParmVar']">
          <id>
            <name>
              <xsl:value-of select="fullName" />
            </name>
          </id>
        </xsl:for-each>
      </params>

      <xsl:apply-templates />
    </clangDecl>
  </xsl:template>

  <xsl:template match="node()|@*">
    <xsl:copy>
      <xsl:apply-templates select="node()|@*"/>
    </xsl:copy>
  </xsl:template>

</xsl:stylesheet>
