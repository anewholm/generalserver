<xsl:stylesheet xmlns:meta="http://general_server.org/xmlnamespaces/meta/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:request="http://general_server.org/xmlnamespaces/request/2006" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:regexp="http://exslt.org/regular-expressions" name="view" extension-element-prefixes="server request regexp debug" version="1.0">
  <xsl:template match="meta:*" meta:interface-template="yes">
    <!-- meta data needs to be directly accessed, it is not automatically view rendered -->
  </xsl:template>
</xsl:stylesheet>