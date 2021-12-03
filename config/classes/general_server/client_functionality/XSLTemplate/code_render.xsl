<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:response="http://general_server.org/xmlnamespaces/response/2006" xmlns:xxx="http://general_server.org/xmlnamespaces/dummyxsl/2006" xmlns:conversation="http://general_server.org/xmlnamespaces/conversation/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:exsl="http://exslt.org/common" xmlns:class="http://general_server.org/xmlnamespaces/class/2006" xmlns:user="http://general_server.org/xmlnamespaces/user/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" response:server-side-only="true" name="code_render" version="1.0" extension-element-prefixes="dyn str regexp database server conversation request debug exsl">
  <xsl:template match="xsl:template" mode="gs_code_render">
    <xsl:if test="str:not(@response:server-side-only)">
        <xsl:apply-templates select="." mode="gs_inheritance_render_xsl"/>
    </xsl:if>
  </xsl:template>
</xsl:stylesheet>
