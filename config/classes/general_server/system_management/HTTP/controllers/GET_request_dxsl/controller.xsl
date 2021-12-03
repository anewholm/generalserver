<xsl:stylesheet response:server-side-only="true" xmlns="http://www.w3.org/1999/xhtml" xmlns:str="http://exslt.org/strings" xmlns:xxx="http://general_server.org/xmlnamespaces/dummyxsl/2006" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns:dyn="http://exslt.org/dynamic" name="controller" controller="true" version="1.0" extension-element-prefixes="server dyn database">
  <xsl:include xpath="../process_http_request"/>
  <!-- MI context specific HTTP handlers -->
  <xsl:include xpath="~XSLStylesheet/HTTP"/>
  <xsl:include xpath="/object:Server[1]/repository:system_transforms[1]/code_render"/>

  <!-- custom interface_render objects
    these are fixed system data renders of specific dynamic output things
      database:query
      class:xschema-*
    use data-queries to include DYNAMIC queries for custom classes e.g.
      <object:Session @interface-mode=controls>
        ++ <class:xschema-* login.xsd>
  -->
  <xsl:include xpath="~AJAXHTMLLoader/interface_render"/>
  <xsl:include xpath="~Interface/interface_render"/>
  <xsl:include xpath="~XSchema/interface_render"/>
  <xsl:include xpath="~Response/interface_render"/>
  <xsl:include xpath="~ResponseNamespace/interface_render"/>

  <xsl:template match="xsl:stylesheet" mode="gs_HTTP_render">
    <!-- dynamic stylesheet process uses the Class__XSLStylesheet/HTTP.xsl for gs_HTTP_render_*
      database:dynamic-stylesheet() also does this
    -->
    <xsl:apply-templates select="." mode="gs_HTTP_render_dynamic"/>
  </xsl:template>
</xsl:stylesheet>
