<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" name="view" version="1.0">
  <xsl:template match="repository:*" mode="editor" meta:interface-template="yes">
    <xsl:param name="gs_html_identifier_class"/>

    <ul class="{$gs_html_identifier_class}">
      <xsl:apply-templates select="*" mode="gs_view_render">
        <xsl:with-param name="gs_interface_mode" select="'list'"/>
        <xsl:with-param name="gs_event_functions" select="'f_click_selectchild'"/>
      </xsl:apply-templates>
    </ul>
  </xsl:template>
</xsl:stylesheet>