<xsl:stylesheet xmlns:xxx="http://general_server.org/xmlnamespaces/dummyxsl/2006" xmlns:str="http://exslt.org/strings" xmlns:exsl="http://exslt.org/common" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns:flow="http://exslt.org/flow" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" name="request-layout" version="1.0" extension-element-prefixes="dyn database server str flow debug">
  <xsl:param name="gs_query_string" select="$gs_request/gs:query-string"/>
  <xsl:param name="gs_form" select="$gs_request/gs:form"/>
  <xsl:param name="gs_request_id" select="string($gs_request/@id)"/>
  <xsl:param name="gs_configuration_flags" select="concat($gs_query_string/@configuration-flags, $gs_query_string/@flags)"/>
</xsl:stylesheet>
