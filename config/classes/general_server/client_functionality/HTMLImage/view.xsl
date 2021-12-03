<xsl:stylesheet xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" name="view" version="1.0">
  <!-- xsl:template match="html:img/@src|html:script/@src|html:link/@href">
    <xsl:copy>
      <xsl:if test="starts-with(., '/resources/')">
        <xsl:value-of select="$gs_resource_server"/>
      </xsl:if>
      <xsl:value-of select="."/>
    </xsl:copy>
  </xsl:template -->
</xsl:stylesheet>