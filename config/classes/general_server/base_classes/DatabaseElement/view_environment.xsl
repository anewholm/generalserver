<xsl:stylesheet xmlns:xmlsecurity="http://general_server.org/xmlnamespaces/xmlsecurity/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" name="view_environment" version="1.0" extension-element-prefixes="debug">
  <xsl:template match="*" mode="environment" meta:interface-template="yes">
    <!-- environment display is for rendering in the html:head
      <script>
        /* GSProperties:
          <properties>
            <div class="gs-meta-data gs-namespace-prefix-gs gs-load-as-object">...</div>
            <div class="gs-meta-data gs-namespace-prefix-database gs-load-as-properties">...</div>
          </properties>
        */
      </script>
    -->
    <xsl:param name="gs_html_identifier_class"/>
    <xsl:param name="gs_interface_mode"/>

    <!-- DO NOT use html:script because it confuses the poor browser -->
    <script class="{$gs_html_identifier_class}">
      <xsl:text>/* GSProperties: </xsl:text>
      <properties>
        <xsl:apply-templates select="." mode="gs_meta_data_standard_groups">
          <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
        </xsl:apply-templates>
      </properties>
      <xsl:text> */</xsl:text>
    </script>
  </xsl:template>
</xsl:stylesheet>