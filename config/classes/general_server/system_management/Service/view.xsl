<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" name="view" version="1.0">
  <xsl:template match="object:Service" mode="default_content">
    <!-- default interface leads to render default_content -->
    <xsl:text>Servicey</xsl:text>
  </xsl:template>

  <xsl:template match="object:Service/@port" mode="gs_list_attributes">
    <li class="attribute_{local-name()}"><xsl:value-of select="name()"/> (<xsl:value-of select="."/>)</li>
  </xsl:template>

  <xsl:template match="object:Service" mode="gs_context_menu_custom">
    <li class="f_click_stop">stop</li>
  </xsl:template>
</xsl:stylesheet>