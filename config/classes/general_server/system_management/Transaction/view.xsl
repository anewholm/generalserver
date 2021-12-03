<xsl:stylesheet xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" name="view" version="1.0">
  <xsl:template match="xmltransaction:*" mode="gs_context_menu_custom">
    <li class="f_click_run">run</li>
  </xsl:template>
</xsl:stylesheet>