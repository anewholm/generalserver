<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" name="view" version="1.0">
  <xsl:template match="object:Manager" mode="full_title">
    <xsl:text>Dickhead.</xsl:text>
    (<xsl:value-of select="@test"/>)
  </xsl:template>

  <xsl:template match="object:Test"/>
</xsl:stylesheet>