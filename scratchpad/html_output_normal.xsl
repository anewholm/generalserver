<xsl:stylesheet xmlns="http://www.w3.org/1999/xhtml" xmlns:flow="http://exslt.org/flow" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:str="http://exslt.org/strings" xmlns:response-xml="http://general_server.org/xmlnamespaces/response/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" response-xml:server-side-only="true" name="html_output_normal" version="1.0" extension-element-prefixes="flow database str debug request response regexp repository" object:cpp-component="yes">
  <xsl:template match="html:img/@src|html:script/@src|html:link/@href" mode="gs_data_render">
    <xsl:attribute name="{name()}">
      <xsl:if test="starts-with(., '/resources/')">
        <xsl:value-of select="$gs_resource_server"/>
      </xsl:if>
      <xsl:value-of select="."/>
    </xsl:attribute>
  </xsl:template>
</xsl:stylesheet>