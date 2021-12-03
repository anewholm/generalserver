<xsl:stylesheet xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:xxx="http://general_server.org/xmlnamespaces/dummyxsl/2006" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" extension-element-prefixes="flow database str debug request regexp repository" name="HTTP" response:server-side-only="true" version="1.0">
  <xsl:include xpath="view"/>

  <xsl:template match="css:stylesheet" mode="gs_HTTP_render">
    <debug:server-message if="$gs_debug_url_steps" output="object:Request [{$gs_request/gs:message_type}] =&gt; {$gs_request/gs:url} =&gt; {name()} gs_HTTP_render"/>

    <!-- HTTP headers -->
    <xsl:apply-templates select="$gs_request/gs:HTTP" mode="basic_http_headers"/>
    <xsl:apply-templates select="$gs_request/gs:cookies" mode="process_session"/>
    <xsl:apply-templates select="." mode="basic_http_headers_last_modified"/>
    <xsl:apply-templates select="." mode="basic_http_headers_content_type"/>

    <!-- always server-side transform
      this gs_HTTP_render is called on the first element in $gs_request_target
      and $gs_request_target contains all the required nodes
      in order to allow the Class to process the headers and include multiple responses in one output

      TODO: using a temporary sort of the included javascript files on @repository:name
        until front-end ordering is enabled that re-writes the directory in the correct order
        remember 05_* comes before 10_* because of the zero leader
    -->
    <xsl:apply-templates select="$gs_request_target" mode="standalone_contents">
      <xsl:sort select="@repository:name"/>
    </xsl:apply-templates>
  </xsl:template>
</xsl:stylesheet>
