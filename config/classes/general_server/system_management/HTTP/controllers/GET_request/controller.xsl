<xsl:stylesheet response:server-side-only="true" xmlns:meta="http://general_server.org/xmlnamespaces/meta/2006" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:request="http://general_server.org/xmlnamespaces/request/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:dyn="http://exslt.org/dynamic" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" name="controller" controller="true" version="1.0" extension-element-prefixes="dyn database debug">
  <xsl:output method="xml" encoding="UTF-8" omit-xml-declaration="yes" indent="yes"/>
  <!-- we DONT do these because it will place the DOCTYPE at the beginning before the HTTP headers
    doctype-public="-//W3C//DTD XHTML 1.1//EN"
    doctype-system="http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd" 
  -->

  <xsl:include xpath="../process_http_request"/>

  <!-- MI context specific HTTP handlers
    this MI INTERPRETS the request based on the type of the $gs_request_target[1]:
    XSLStylesheet, Response and AJAXHTMLLoader are all treated differently
    requests for other types of object are enveloped in an object:Response by the HTTP/HTTP handler

    use the /api/ or .xml extension for XML queries
    use the /direct/ for un-interpreted responses
    use extensions .js .css .jsapi for other media
    use .xsl, .dxsl
  -->
  <xsl:include xpath="~Response/HTTP"/>       <!-- object:Response interface style output -->
  <xsl:include xpath="~Repository/HTTP"/>     <!-- repository interface style output -->
  <xsl:include xpath="~XSLStylesheet/HTTP"/>  <!-- direct stylesheet analysis -->
  <xsl:include xpath="~AJAXHTMLLoader/HTTP"/> <!-- @is-client-ondemand AJAX -->

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
</xsl:stylesheet>
