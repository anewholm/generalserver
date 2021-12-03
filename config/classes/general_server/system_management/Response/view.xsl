<xsl:stylesheet xmlns:meta="http://general_server.org/xmlnamespaces/meta/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" name="view" extension-element-prefixes="server response" version="1.0">
  <xsl:template match="object:Response" meta:interface-template="yes">
    <!-- object:Response just do the normal apply-templates gs_view_render
      it houses some meta:environment information, which the meta:* gs_view_render will ignore
      and then the usual objects in meta:data, e.g. interface:HTMLWebpage
    -->
    <xsl:if test="not($gs_response)"><xsl:comment>no $gs_response</xsl:comment></xsl:if>
    <xsl:if test="not($gs_environment)"><xsl:comment>no $gs_environment</xsl:comment></xsl:if>
    <xsl:if test="not($gs_website_root)"><xsl:comment>no $gs_website_root</xsl:comment></xsl:if>
    <xsl:if test="not($gs_context_database)"><xsl:comment>no $gs_context_database</xsl:comment></xsl:if>
    <xsl:if test="not($gs_resource_server_object)"><xsl:comment>no $gs_resource_server_object</xsl:comment></xsl:if>
    <xsl:if test="not($gs_resource_server)"><xsl:comment>no $gs_resource_server</xsl:comment></xsl:if>
    <xsl:if test="not($gs_request_target)"><xsl:comment>no $gs_request_target</xsl:comment></xsl:if>

    <xsl:if test="@server-side-xslt = 'true'">
      <!-- server side XSLT manual HTTP HTML headers 
        @server-side-xslt is appended to the data object:Response
        when the server is intending a server-side-xslt as a response
        which will require also HTTP headers including the HTML Content-Type 
      -->
      <response:set-header value="HTTP/1.0 200 OK"/>
      <response:set-header header="Server" value="general_server/0.9 Alpha"/>
      <response:set-header header="X-Powered-by" value="general_server/0.9 Alpha"/>
      <response:set-header header="Date" value="Wed, 29 Apr 2015 16:25:21 GMT"/>
      <response:set-header header="Content-Type" value="text/html"/>
      <response:set-header header="Content-Length" token="token-content-length"/>
    </xsl:if>
    
    <xsl:apply-templates select="*" mode="gs_view_render"/>
  </xsl:template>

  <xsl:template match="object:Response/gs:data">
    <xsl:apply-templates select="*" mode="gs_view_render">
      <xsl:with-param name="gs_interface_mode" select="parent::object:Response/@gs:force-data-interface"/>
    </xsl:apply-templates>
  </xsl:template>
</xsl:stylesheet>
