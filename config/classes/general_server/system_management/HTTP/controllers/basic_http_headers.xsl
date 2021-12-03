<xsl:stylesheet response:server-side-only="true" xmlns:date="http://exslt.org/dates-and-times" xmlns:exsl="http://exslt.org/common" xmlns="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:response="http://general_server.org/xmlnamespaces/response/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" name="basic_http_headers" extension-element-prefixes="server response" version="1.0">
  <xsl:template match="object:Request/gs:HTTP" mode="basic_http_headers_not_modified">
    <response:set-header value="HTTP/1.0 304 Not Modified"/>
    <response:set-header header="Server" value="general_server/0.9 Alpha"/>
    <response:set-header header="X-Powered-by" value="general_server/0.9 Alpha"/>
    <!-- Wed, 29 Apr 2015 16:25:21 GMT -->
    <response:set-header header="Date" value="{date:date-format('', 'EEE, d MMM yyyy HH:mm:ss Z')}"/>
  </xsl:template>

  <xsl:template match="object:Request/gs:HTTP" mode="basic_http_headers_context"/>

  <xsl:template match="object:Request/gs:HTTP" mode="basic_http_headers">
    <!-- basic HTTP response headers
      HTTP/1.0 200 Ok
      Server: general_server/0.9 Alpha
      
      Unused header fields (TODO: will use some sort of auto-post-computed fill in system)
        Date: unknown\r
        Expires: unknown\r
        Content-length: ???\r ====> allows HTTP/1.1 and Connection: keep-alive
    -->
    <xsl:variable name="gs_header_line_basic">
      <xsl:apply-templates select="." mode="basic_http_headers_protocol"/><xsl:text>/</xsl:text>
      <xsl:apply-templates select="." mode="basic_http_headers_protocol_version"/><xsl:text> </xsl:text>
      <xsl:apply-templates select="." mode="basic_http_headers_code"/>
    </xsl:variable>
    <response:set-header value="{$gs_header_line_basic}"/>
    
    <xsl:variable name="gs_header_line_server">
      <xsl:apply-templates select="." mode="basic_http_headers_server_version"/>
    </xsl:variable>
    <response:set-header header="Server" value="{$gs_header_line_server}"/>

    <xsl:variable name="gs_header_line_powered">
      <xsl:apply-templates select="." mode="basic_http_headers_server_version"/>
    </xsl:variable>
    <response:set-header header="X-Powered-by" value="{$gs_header_line_powered}"/>

    <xsl:apply-templates select="." mode="basic_http_headers_client_cache"/>
    <xsl:apply-templates select="." mode="basic_http_headers_context"/> <!-- e.g. Location with 302 -->
    <xsl:apply-templates select="." mode="basic_http_headers_date"/>
    <xsl:apply-templates select="." mode="basic_http_headers_content_length"/>
    <!-- These are applied later by the url_handler target node processor -->
    <!-- xsl:apply-templates select="." mode="basic_http_headers_last_modified" / -->
  </xsl:template>

  <xsl:template match="object:Request/gs:HTTP" mode="basic_http_headers_client_cache_off">
    <!-- https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Cache-Control -->
    <response:set-header header="Cache-Control" value="no-store, no-cache, must-revalidate, post-check=0, pre-check=0"/>
    <response:set-header header="Pragma" value="no-cache"/>
  </xsl:template>

  <xsl:template match="object:Request/gs:HTTP" mode="basic_http_headers_client_cache_on">
    <!-- https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Cache-Control -->
    <response:set-header header="Cache-Control" value="public, max-age=31536000"/>
  </xsl:template>

  <xsl:template match="object:Request/gs:HTTP" mode="basic_http_headers_protocol">HTTP</xsl:template>
  <xsl:template match="object:Request/gs:HTTP" mode="basic_http_headers_protocol_version">1.0</xsl:template>
  <xsl:template match="object:Request/gs:HTTP" mode="basic_http_headers_code">200 OK</xsl:template>
  <xsl:template match="object:Request/gs:HTTP" mode="basic_http_headers_server_version">general_server/0.9 Alpha</xsl:template>
  <xsl:template match="object:Request/gs:HTTP" mode="basic_http_headers_client_cache"/>

  <xsl:template match="object:Request/gs:HTTP" mode="basic_http_headers_date">
    <!-- http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html#sec3.3.1
      RFC 822, updated by RFC 1123:
        Tue, 28 Apr 2015 11:35:16 GMT
      EXSLT: date:format-date() is not considered stable
        http://exslt.org/date/functions/format-date/date.format-date.html
        output: https://docs.oracle.com/javase/7/docs/api/java/text/SimpleDateFormat.html
        LibXML2 / EXSLT has not implemented it. see exslt/date.c
        Also, it doesn't allow Timezone specification. So we can't output in GMT anyway
      EXSLT: cannot do manual output because we require time-zone change for all output pieces
    -->
    <response:set-header header="Date" value="{server:date('now', '%HTTP')}"/>
  </xsl:template>

  <xsl:template match="object:Request/gs:HTTP" mode="basic_http_headers_content_length">
    <!-- content live replaced by the server after XSLT -->
    <response:set-header header="Content-Length" token="token-content-length"/>
  </xsl:template>

  <xsl:template match="/|@*|node()" mode="basic_http_headers_last_modified">
    <!-- called from the url_handler when the target node is understood -->
    <xsl:if test="@meta:last-modified">
      <response:set-header header="Last-Modified" value="{server:date(@meta:last-modified, '%HTTP')}"/>
    </xsl:if>
  </xsl:template>

  <xsl:template match="/|@*|node()" mode="basic_http_headers_content_type">
    <!-- called from the url_handler when the target node is understood -->
    <xsl:param name="gs_specific_extension"/>

    <xsl:variable name="gs_header_line">
      <xsl:apply-templates select="." mode="mime-type">
        <xsl:with-param name="gs_specific_extension" select="$gs_specific_extension"/>
      </xsl:apply-templates>
    </xsl:variable>
    <response:set-header header="Content-Type" value="{$gs_header_line}"/>
  </xsl:template>

  <xsl:template match="/|@*|node()" mode="mime-type">
    <xsl:param name="gs_specific_extension"/>
    <xsl:variable name="gs_mime_types" select="$gs_system_data/gs:mime_types[1]"/>

    <xsl:variable name="gs_mime_type">
      <xsl:if test="$gs_specific_extension">
        <xsl:value-of select="$gs_mime_types/*[local-name() = $gs_specific_extension]"/>
      </xsl:if>
      <xsl:if test="not($gs_specific_extension)">
        <xsl:if test="$gs_request/gs:extension">
          <xsl:variable name="gs_mime_type_extension" select="$gs_mime_types/*[local-name() = $gs_request/gs:extension]"/>
          <xsl:if test="$gs_mime_type_extension"><xsl:value-of select="$gs_mime_type_extension"/></xsl:if>
          <xsl:if test="not($gs_mime_type_extension)">
            <xsl:variable name="gs_mime_type_namespace" select="$gs_mime_types/*[local-name() = substring-before(name(), ':')]"/>
            <xsl:if test="$gs_mime_type_namespace"><xsl:value-of select="$gs_mime_type_namespace"/></xsl:if>
            <xsl:if test="not($gs_mime_type_namespace)"><xsl:value-of select="$gs_mime_types/gs:xml"/></xsl:if>
          </xsl:if>
        </xsl:if>
        <xsl:if test="not($gs_request/gs:extension)">
          <xsl:variable name="gs_namespace_prefix" select="substring-before(name(), ':')"/>
          <xsl:variable name="gs_mime_type_namespace" select="$gs_mime_types/*[local-name() = $gs_namespace_prefix]"/>
          <xsl:if test="$gs_mime_type_namespace"><xsl:value-of select="$gs_mime_type_namespace"/></xsl:if>
          <xsl:if test="not($gs_mime_type_namespace)"><xsl:value-of select="$gs_mime_types/gs:xml"/></xsl:if>
        </xsl:if>
      </xsl:if>
    </xsl:variable>

    <xsl:value-of select="$gs_mime_type"/>
    <!-- debug:debug output="$gs_specific_extension: [{$gs_specific_extension}] $gs_request/gs:extension [{$gs_request/gs:extension}] $gs_namespace_prefix: [{substring-before(name(), ':')}] $gs_mime_type: [{$gs_mime_type}]"/ -->
  </xsl:template>
</xsl:stylesheet>
