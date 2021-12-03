<xsl:stylesheet response:server-side-only="true" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns:str="http://exslt.org/strings" name="virtualhosts-targets" version="1.0" extension-element-prefixes="debug dyn str regexp database server request">
  <!-- TODO: move all these in to paths.xml for proper cacheing, especially the classes info -->

  <!-- object target from URL
    ^ special is in force,      e.g. /^responseloader.xsl
    dynamic values are allowed, e.g. /^{$gs_user/@xml:id}
  -->
  <xsl:param name="gs_resource_server" select="$gs_resource_server_object/@uri"/>
  <xsl:param name="gs_dynamic_url" select="str:dynamic($gs_request/gs:url)"/>
  <xsl:param name="gs_AJAX" select="gs:HTTP/@X-Requested-With = 'XMLHttpRequest'"/>
  <xsl:param name="gs_request_use_indexes" select="str:boolean($gs_message_interpretation/@use-indexes, true())"/>
  <xsl:param name="gs_request_auto_target" select="str:boolean($gs_message_interpretation/@auto-target, true())"/>
  <xsl:param name="gs_request_throw_on_write" select="str:boolean($gs_message_interpretation/@throw-on-write, true())"/>
  <xsl:param name="gs_request_xpath" select="repository:filesystempath-to-XPath($gs_dynamic_url, $gs_request_use_indexes)"/>
  <xsl:param name="gs_request_target" select="repository:filesystempath-to-nodes($gs_dynamic_url, $gs_website_root, $gs_website_root/page_not_found, $gs_request_use_indexes, $gs_request_throw_on_write, $gs_request_auto_target)"/>
</xsl:stylesheet>
