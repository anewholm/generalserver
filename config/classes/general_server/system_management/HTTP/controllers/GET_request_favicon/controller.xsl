<xsl:stylesheet response:server-side-only="true" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:dyn="http://exslt.org/dynamic" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" name="controller" controller="true" version="1.0" extension-element-prefixes="dyn">
  <xsl:include xpath="../basic_http_headers"/>

  <xsl:template match="object:Request/gs:HTTP" mode="basic_http_headers_code">404 Not Found</xsl:template>
  
  <xsl:template match="object:Request">
    <xsl:apply-templates select="gs:HTTP" mode="basic_http_headers"/>

    <xsl:text>Favicon should be requested from the resources server</xsl:text>
  </xsl:template>
</xsl:stylesheet>
