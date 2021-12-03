<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" name="controller" controller="true" response:server-side-only="true" version="1.0">
  <xsl:template match="object:User" mode="logout">
    <debug:server-message output="logout"/>
    <conversation:clear-security-context/>
  </xsl:template>
</xsl:stylesheet>