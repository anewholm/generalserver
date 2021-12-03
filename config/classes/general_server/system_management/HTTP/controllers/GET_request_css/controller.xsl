<xsl:stylesheet response:server-side-only="true" xmlns="http://www.w3.org/1999/xhtml" xmlns:str="http://exslt.org/strings" xmlns:xxx="http://general_server.org/xmlnamespaces/dummyxsl/2006" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns:dyn="http://exslt.org/dynamic" name="controller" controller="true" version="1.0" extension-element-prefixes="server dyn">
  <xsl:output method="text" encoding="UTF-8" omit-xml-declaration="yes"/>

  <xsl:include xpath="../process_http_request"/>
  <!-- MI context specific HTTP handlers -->
  <xsl:include xpath="~CSSStylesheet/HTTP"/>

  <xsl:template match="object:Request/gs:HTTP" mode="basic_http_headers_client_cache">
    <xsl:if test="$gs_stage_live">
      <xsl:apply-templates select="." mode="basic_http_headers_client_cache_on"/>
    </xsl:if>
    <xsl:else>
      <xsl:apply-templates select="." mode="basic_http_headers_client_cache_off"/>
    </xsl:else>
  </xsl:template>  
</xsl:stylesheet>
