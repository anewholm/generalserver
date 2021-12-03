<xsl:stylesheet xmlns:xxx="http://general_server.org/xmlnamespaces/dummyxsl/2006" xmlns:date="http://exslt.org/dates-and-times" xmlns:exsl="http://exslt.org/common" xmlns="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:conversation="http://general_server.org/xmlnamespaces/conversation/2006" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" name="inheritance_render" response:server-side-only="true" extension-element-prefixes="server conversation" version="1.0">
  <xsl:namespace-alias result-prefix="xsl" stylesheet-prefix="xxx"/>

  <xsl:template match="xsl:template" mode="gs_inheritance_render_xsl">
    <!-- GS server side carries out dynamic inheritance CompilationContext:
        auto-morphs @match during CompilationContext read phase
        then dynamic @match={} afterwards
      we copy that process here for the client side templates
        database:derived-template-match(@match)
        str:dynamic(@match)
    -->
    <xsl:param name="gs_class" select="../ancestor::class:*[1]"/>
    <xsl:param name="gs_priority" select="database:base-class-count($gs_class)"/>

    <xsl:copy>
      <xsl:attribute name="match">
        <xsl:value-of select="str:dynamic(database:derived-template-match(@match))"/>
      </xsl:attribute>
      <xsl:copy-of select="@mode"/>   <!-- optional: @mode or worker @mode -->
      <xsl:copy-of select="@name"/>   <!-- optional: un-advised!! -->
      <!-- xsl:attribute name="priority"><xsl:value-of select="$gs_priority"/></xsl:attribute -->
      <!-- xsl:attribute name="meta:xpath-to-node"><xsl:value-of select="conversation:xpath-to-node()"/></xsl:attribute -->
      <xsl:attribute name="meta:base-class-count"><xsl:value-of select="$gs_priority"/></xsl:attribute>
      
      <!-- copy in the params before the comment otherwise we would get an error -->
      <xsl:copy-of select="xsl:param"/>

      <!-- xsl:if test="str:not(@meta:debug) and not(xsl:attribute)">
        <xxx:comment>
          <xsl:text>Class__</xsl:text><xsl:value-of select="local-name($gs_class)"/><xsl:text> xsl:template</xsl:text>
          <xsl:text> @elements=</xsl:text><xsl:value-of select="$gs_class/@elements"/>
          <xsl:text> @original-match=</xsl:text><xsl:value-of select="@match"/>
          <xsl:text> @mode=</xsl:text><xsl:value-of select="@mode"/>
          <xsl:text> @xml:id=</xsl:text><xsl:value-of select="@xml:id"/>
          <xsl:text> called on [</xsl:text><xxx:value-of select="name()"/><xsl:text>]</xsl:text>
        </xxx:comment>

        <xsl:if test="not(starts-with(@match, $gs_class/@elements))">
          <xsl:comment>NON_CLASS_ELEMENT_TEMPLATE: this template does not share its class:*/@elements</xsl:comment>
        </xsl:if>
      </xsl:if-->

      <xsl:apply-templates select="node()[not(self::xsl:param)]" mode="gs_code_render"/>
    </xsl:copy>
  </xsl:template>
</xsl:stylesheet>
