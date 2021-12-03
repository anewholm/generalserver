<xsl:stylesheet xmlns="http://www.w3.org/1999/xhtml" xmlns:exsl="http://exslt.org/common" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:class="http://general_server.org/xmlnamespaces/class/2006" xmlns:user="http://general_server.org/xmlnamespaces/user/2006" name="HTTP" response:server-side-only="true" version="1.0" extension-element-prefixes="dyn str regexp database server request flow debug repository user">
  <xsl:include xpath="HTTP_processing"/>

  <xsl:template match="xsl:stylesheet" mode="gs_HTTP_render">
    <xsl:apply-templates select="." mode="gs_HTTP_render_client_server_switch"/>
  </xsl:template>
</xsl:stylesheet>