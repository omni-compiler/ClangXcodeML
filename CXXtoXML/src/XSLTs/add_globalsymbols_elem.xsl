<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0"
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
  <xsl:output method="xml" encoding="UTF-8"/>

  <xsl:template name="emit-id-list-in-namespace">
    <xsl:for-each select="clangDecl">
      <xsl:choose>
        <!-- language linkage specification -->
        <xsl:when test="@class = 'LinkageSpec'">
          <xsl:call-template name="emit-id-list-in-namespace" />
        </xsl:when>

        <!-- C++ class declaration -->
        <xsl:when test="@class = 'CXXRecord'">
          <id>
            <name><xsl:value-of select="fullName" /></name>
          </id>
          <xsl:call-template name="emit-id-lists-in-class" />
        </xsl:when>

        <!-- typedef declaration -->
        <xsl:when test="@class = 'Typedef'">
          <id sclass="typedef_name">
            <xsl:attribute name="type">
              <xsl:value-of select="@xcodemlTypedefType" />
            </xsl:attribute>
            <name>
              <xsl:value-of select="fullName" />
            </name>
          </id>
        </xsl:when>

        <!--
             otherwise, emit an <id> node if this <clangDecl>
             node has name and type
        -->
        <xsl:when
            test="
            (fullName)
            and @xcodemlType">
          <id>
            <xsl:attribute name="type">
              <xsl:value-of select="@xcodemlType" />
            </xsl:attribute>
            <name>
              <xsl:value-of select="fullName" />
            </name>
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
    <xsl:for-each
        select="clangDecl[position() &gt; 1]">
        <!--
             Temporarily ignore first node
             Example:
                 class A {
                   int x;
                   void f() { }
                 };

                 <clangDecl class="CXXRecord" xcodemlType="Class0">
                   <fullName>A</fullName>
                   <clangDecl class="CXXRecord" is_implicit="1" xcodemlType="Class0"> // ignore this node
                     <fullName>A::A</fullName>
                   </clangDecl>
                   <clangDecl class="Field" xcodemlType="int">
                     <fullName>A::x</fullName>
                     // ...
                   </clangDecl>
                   <clangDecl class="CXXMethod" xcodemlType="Function1">
                     <fullName>A::f</fullName>
                     // ...
                   </clangDecl>
                 </clangDecl>
        -->
      <xsl:choose>
        <!-- friend function declaration or definition -->
        <xsl:when
            test="(@class = 'Friend')
                  and (clangDecl[@class='Function'])">
          <id>
            <name>
              <xsl:attribute name="type">
                <xsl:value-of
                  select="clangDecl[@class='Function']/@xcodemlType" />
              </xsl:attribute>
              <xsl:value-of
                select="clangDecl[@class='Function']/fullName" />
            </name>
          </id>
        </xsl:when>

        <!-- child class declaration -->
        <xsl:when test="@class = 'CXXRecord'">
          <id>
            <name><xsl:value-of select="fullName" /></name>
          </id>
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

      <xsl:apply-templates select="typeTable" />

      <globalSymbols>
        <xsl:for-each
          select="clangAST/clangDecl[@class='TranslationUnit']">
          <xsl:call-template name="emit-id-list-in-namespace"/>
        </xsl:for-each>
      </globalSymbols>

      <xsl:apply-templates select="clangAST" />

    </Program>
  </xsl:template>

  <xsl:template match="node()|@*">
    <xsl:copy>
      <xsl:apply-templates select="node()|@*"/>
    </xsl:copy>
  </xsl:template>

</xsl:stylesheet>
