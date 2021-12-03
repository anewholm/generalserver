<xsl:stylesheet response:server-side-only="true" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:dyn="http://exslt.org/dynamic" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" name="controller" controller="true" version="1.0" extension-element-prefixes="dyn server debug">
  <xsl:template match="object:Request">
    <debug:server-message output="server command: [{gs:url_parts/gs:part[3]}]"/>
    <command><server:execute command="{gs:url_parts/gs:part[3]}"/></command>
  </xsl:template>
</xsl:stylesheet>
