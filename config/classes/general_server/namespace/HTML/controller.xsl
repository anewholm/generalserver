<xsl:stylesheet xmlns:xxx="http://general_server.org/xmlnamespaces/dummyxsl/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:str="http://exslt.org/strings" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" name="controller" controller="true" response:server-side-only="true" extension-element-prefixes="str" version="1.0">
  <xsl:template match="*|@*" mode="gs_enforce_non_self_closing">
    <xsl:text>gs_enforce_non_self_closing</xsl:text>
  </xsl:template>
</xsl:stylesheet>
