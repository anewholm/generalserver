<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xxx="http://general_server.org/xmlnamespaces/dummyxsl/2006" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" response:server-side-only="true" name="controller" controller="true" version="1.0">
  <xsl:include xpath="~HTML/controller"/>
  
  <xsl:template match="css:stylesheet" mode="dependency">
    <!--
      a html link to this stylesheet
      output example:
        <link rel="text/css" class="Object Class__CSSDependency gs-interface-mode-dependency gs-xml-id-idx_2344" />
      no @xml:id on these as they are re-used and will cause repeat @xml:id issues on the client
    -->
    <xsl:param name="gs_html_identifier_class"/>

    <xsl:if test="css:*">
      <xsl:variable name="gs_class" select="ancestor::class:*[1]"/>
      <xsl:variable name="gs_href" select="database:xpath-to-node(., $gs_website_root)"/>

      <link type="text/css" rel="stylesheet" href="/{$gs_href}.css?{local-name($gs_class)}&amp;CSSStylesheet">
        <xsl:attribute name="class">
          <xsl:value-of select="$gs_html_identifier_class"/>
          <xsl:text>DatabaseObject Class__CSSDependency CSS__CSSDependency gs-has-xml-id gs-xml-id-</xsl:text>
          <xsl:value-of select="@xml:id"/>
          <xsl:if test="$gs_class"> gs-class-css-<xsl:value-of select="local-name($gs_class)"/></xsl:if>
          <xsl:text> gs-interface-mode-dependency</xsl:text>
          <xsl:text> details</xsl:text>
        </xsl:attribute>
      </link>
    </xsl:if>
  </xsl:template>
</xsl:stylesheet>
