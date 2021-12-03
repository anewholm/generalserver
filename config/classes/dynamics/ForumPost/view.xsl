<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" name="view" version="1.0">
  <xsl:template match="interface:ForumPost" mode="summary">
    <xsl:param name="gs_html_identifier_class"/>

    <li class="{$gs_html_identifier_class}">
      <div class="details">
        <div><xsl:value-of select="gs:title"/></div>
        <div><xsl:value-of select="gs:body"/></div>
      </div>
    </li>
  </xsl:template>
</xsl:stylesheet>