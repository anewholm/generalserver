<xsl:stylesheet xmlns="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" name="databases" meta:handler-for="/config/databases" version="1.0">
  <database:query data-container-name="databases" interface-mode="summary" node-mask=".|*" data="/config/databases"/>

  <xsl:template match="gs:root">
    <h1>databases</h1>
    <ul><xsl:apply-templates select="gs:data/gs:databases"/></ul>
  </xsl:template>

  <xsl:template match="object:Database">
    <li><xsl:value-of select="@name"/></li>
  </xsl:template>
</xsl:stylesheet>
