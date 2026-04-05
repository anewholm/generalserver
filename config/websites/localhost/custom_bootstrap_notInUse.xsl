<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:dyn="http://exslt.org/dynamic" name="custom_bootstrap_notInUse" version="1.0" extension-element-prefixes="dyn">
  <xsl:output method="xml" encoding="UTF-8" omit-xml-declaration="yes"/>

  <xsl:include xpath="/object:Server/repository:services/repository:web-8776/repository:conversation/standard_bootstrap"/>

  <xsl:template match="request">
    <xsl:apply-templates select="*"/>
  </xsl:template>
</xsl:stylesheet>