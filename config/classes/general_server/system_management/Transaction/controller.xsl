<xsl:stylesheet xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" name="controller" controller="true" response:server-side-only="true" version="1.0">
  <xsl:template match="xmltransaction:*" mode="run">
    <!-- TODO: database:run-transaction / -->
  </xsl:template>

  <xsl:template match="xmltransaction:*" mode="rollback">
    <!-- TODO: database:rollback-transaction / -->
  </xsl:template>
</xsl:stylesheet>