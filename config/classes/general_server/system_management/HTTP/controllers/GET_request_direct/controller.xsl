<xsl:stylesheet response:server-side-only="true" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:request="http://general_server.org/xmlnamespaces/request/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:dyn="http://exslt.org/dynamic" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" name="controller" controller="true" version="1.0" extension-element-prefixes="dyn">
  <xsl:include xpath="../process_http_request"/>

  <!-- all HTTP handled by the ~HTTP/HTTP which wraps direct content in the Response envelope -->
  <!-- no interface_render plugins: output is direct -->
</xsl:stylesheet>
