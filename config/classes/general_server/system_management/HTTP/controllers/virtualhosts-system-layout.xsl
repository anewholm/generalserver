<xsl:stylesheet response:server-side-only="true" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns:str="http://exslt.org/strings" name="virtualhosts-system-layout" version="1.0" extension-element-prefixes="debug dyn str regexp database server request">
  <!-- Website, Database, LinkedServer objects -->
  <xsl:param name="gs_website_root" select="$gs_websites/object:Website[request:resource-match()][1]"/>
  <xsl:param name="gs_context_database" select="$gs_databases/object:Database[request:resource-match()][1]"/>
  <xsl:param name="gs_resource_server_object" select="$gs_linked_servers/object:LinkedServer[@name = $gs_website_root/@resource-server][1]"/>

  <!-- aliases -->
  <xsl:param name="db" select="$gs_context_database"/>
  <xsl:param name="website" select="$gs_website_root"/>
</xsl:stylesheet>
