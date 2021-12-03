<xsl:stylesheet xmlns:conversation="http://general_server.org/xmlnamespaces/conversation/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:exsl="http://exslt.org/common" xmlns:class="http://general_server.org/xmlnamespaces/class/2006" xmlns:user="http://general_server.org/xmlnamespaces/user/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" response:server-side-only="true" name="interface_render" version="1.0" extension-element-prefixes="dyn str regexp database server conversation request debug exsl">
  <xsl:template match="object:Response" mode="gs_interface_render_extra_attributes">
    <!-- this will NOT get called if $gs_interface_render_output = 'placeholder' -->
    <xsl:attribute name="meta:xpath-to-node-client"><xsl:value-of select="conversation:xpath-to-node(., $gs_website_root)"/></xsl:attribute>
  </xsl:template>
</xsl:stylesheet>
