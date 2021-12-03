<xsl:stylesheet xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" name="view" version="1.0" extension-element-prefixes="debug database">
  <xsl:template match="object:Person" mode="full_title">
    <xsl:text>Ms.</xsl:text>
    <xsl:value-of select="@test"/>
    <div xsl:if="@test = 'wibble'">Wibble mode!</div>
  </xsl:template>
</xsl:stylesheet>