<xsl:stylesheet response:server-side-only="true" xmlns="http://www.w3.org/1999/xhtml" xmlns:str="http://exslt.org/strings" xmlns:xxx="http://general_server.org/xmlnamespaces/dummyxsl/2006" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns:dyn="http://exslt.org/dynamic" name="controller" controller="true" version="1.0" extension-element-prefixes="server dyn">
  <!-- DOM XML to javascript variables requests
    window.Forum.model = '<>...</>';
    e.g. http://general_server.localhost:8776/$gs_website_classes/xsd:schema[not(@name)].jsapi?configuration-flags=LOG_REQUEST_DETAIL&property={$gs_object_name}.model
  -->
  <xsl:strip-space elements="*"/>

  <xsl:include xpath="../process_http_request"/>
  <!-- MI context specific HTTP handlers -->
  <!-- xsl:include xpath="~JavaScript/HTTP"/ -->

  <xsl:template match="/|@*|node()" mode="gs_HTTP_render">
    <!-- API: return config XPATH calls -->
    <xsl:apply-templates select="$gs_request/gs:HTTP" mode="basic_http_headers"/>
    <xsl:apply-templates select="$gs_request/gs:cookies" mode="process_session"/>
    <xsl:apply-templates select="." mode="basic_http_headers_content_type">
      <xsl:with-param name="gs_specific_extension" select="'js'"/>
    </xsl:apply-templates>

    <debug:NOT_CURRENTLY_USED because="have not escaped the &amp;apos; in the attributes"/>

    <xsl:apply-templates select="$gs_request_target" mode="gs_interface_render_jsapi"/>
  </xsl:template>

  <xsl:template match="/|@*|node()" mode="gs_interface_render_jsapi">
    <xsl:param name="gs_javascript_raw_property" select="$gs_query_string/@property"/>
    <xsl:param name="gs_class" select="ancestor-or-self::class:*[1]"/>

    <!-- convienience variables -->
    <xsl:variable name="gs_object_name" select="local-name($gs_class)"/>

    <xsl:variable name="gs_property">
      <xsl:if test="$gs_javascript_raw_property"><xsl:value-of select="str:dynamic($gs_javascript_raw_property)"/></xsl:if>
      <xsl:if test="not($gs_javascript_raw_property)">jsapi.results.<xsl:value-of select="local-name(.)"/></xsl:if>
    </xsl:variable>

    <xsl:value-of select="$gs_property"/>
    <xsl:text> = new JOO('</xsl:text>
    <!-- TODO: should we be using the API system of database:query? -->
    <xsl:apply-templates select="." mode="gs_interface_render">
      <xsl:with-param name="gs_create_meta_context_attributes" select="'none'"/>
      <xsl:with-param name="gs_hardlink_output" select="'follow'"/>
      <xsl:with-param name="gs_database_query_output" select="'follow'"/> <!-- aka softlinks -->
      <xsl:with-param name="gs_interface_render_output" select="'follow'"/>
    </xsl:apply-templates>
    <xsl:text>');</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template>
</xsl:stylesheet>
