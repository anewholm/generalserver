<xsl:stylesheet response:server-side-only="true" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:request="http://general_server.org/xmlnamespaces/request/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:dyn="http://exslt.org/dynamic" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" name="controller" controller="true" version="1.0" extension-element-prefixes="dyn database debug">
  <xsl:output method="xml" encoding="UTF-8" omit-xml-declaration="yes" indent="yes"/>
  <!-- we DONT do these because it will place the DOCTYPE at the beginning before the HTTP headers
    doctype-public="-//W3C//DTD XHTML 1.1//EN"
    doctype-system="http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd" 
  -->

  <xsl:template match="object:Request">
    <debug:server-message output="SSL not currently supported"/>
  </xsl:template>
</xsl:stylesheet>
