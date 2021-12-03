<xsl:stylesheet response:server-side-only="true" xmlns:xxx="http://general_server.org/xmlnamespaces/dummyxsl/2006" xmlns:str="http://exslt.org/strings" xmlns:exsl="http://exslt.org/common" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns:flow="http://exslt.org/flow" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" name="system-layout" version="1.0" extension-element-prefixes="dyn database server str flow debug">
  <!-- ######################################## general_server useful static node variables ######################################## -->
  <!-- some of these cause security blocked node events because the xpath tries to read through the top level before arriving at a match -->
  <xsl:param name="gs_server" select="/object:Server[1]"/>
  <xsl:param name="gs_databases" select="$gs_server/repository:databases[1]"/>
  <xsl:param name="gs_websites" select="$gs_server/repository:websites[1]"/>
  <xsl:param name="gs_service_web" select="$gs_server/repository:services[1]/HTTP[1]"/>
  <xsl:param name="gs_linked_servers" select="$gs_server/repository:linked_servers[1]"/>
  <xsl:param name="gs_system_data" select="$gs_server/repository:system_data[1]"/>
  <xsl:param name="gs_system_entities" select="$gs_system_data/gs:entities[1]"/>
  <xsl:param name="gs_service_class" select="database:classes($gs_service)"/>
  <xsl:param name="gs_service_controllers" select="$gs_service_class/repository:controllers[1]"/>
  <xsl:param name="gs_service_transforms" select="$gs_service_controllers/repository:shared[1]"/>
</xsl:stylesheet>
