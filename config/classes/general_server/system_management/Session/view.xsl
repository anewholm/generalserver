<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" name="view" version="1.0">
  <xsl:template match="object:Session" mode="controls" meta:interface-template="yes">
    <xsl:param name="gs_html_identifier_class"/>

    <div class="{$gs_html_identifier_class} details">
      <xsl:apply-templates select="object:User" mode="gs_view_render">
        <xsl:with-param name="gs_interface_mode" select="'controls'"/>
      </xsl:apply-templates>
      <xsl:apply-templates select="xsd:schema" mode="gs_view_render"/>

      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups"/>
    </div>
  </xsl:template>
</xsl:stylesheet>