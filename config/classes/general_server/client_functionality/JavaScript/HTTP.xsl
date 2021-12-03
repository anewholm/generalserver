<xsl:stylesheet xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:xxx="http://general_server.org/xmlnamespaces/dummyxsl/2006" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" name="HTTP" extension-element-prefixes="flow database str debug request regexp repository" response:server-side-only="true" version="1.0">
  <xsl:include xpath="view"/>
  <xsl:include xpath="~XSLTemplate/view"/> <!-- controller templates -->
  <xsl:include xpath="~Debugger/view"/>

  <xsl:template match="javascript:code" mode="gs_HTTP_render">
    <!-- ~JavaScript framework uses this call directly -->
    <debug:server-message if="$gs_debug_url_steps" output="object:Request [{$gs_request/gs:message_type}] =&gt; {$gs_request/gs:url} =&gt; {name()} gs_HTTP_render javascript:code"/>

    <!-- HTTP headers -->
    <xsl:apply-templates select="$gs_request/gs:HTTP" mode="basic_http_headers"/>
    <xsl:apply-templates select="$gs_request/gs:cookies" mode="process_session"/>
    <xsl:apply-templates select="." mode="basic_http_headers_last_modified"/>
    <xsl:apply-templates select="." mode="basic_http_headers_content_type"/>

    <!-- gs_HTTP_render is called on the first element in $gs_request_target(s) -->
    <xsl:apply-templates select="$gs_request_target" mode="standalone_contents">
      <xsl:sort select="@repository:name"/>
    </xsl:apply-templates>
  </xsl:template>
  
  <xsl:template match="class:*" mode="gs_HTTP_render">
    <!-- class's are being requested instead of direct JavaScript
      this style allows better analysis of the class hierarchy and ordered response
      and also for classes WITHOUT javascript:object to receive the gs_default_object_code
    -->
    <debug:server-message if="$gs_debug_url_steps" output="object:Request [{$gs_request/gs:message_type}] =&gt; {$gs_request/gs:url} =&gt; {name()} gs_HTTP_render class:* (js)"/>
    
    <!-- the default javascript:object for classes that don't have one 
      e.g. HTTP will receive a default javascript:object so that $(HTTP) can be used etc.
    -->
    <xsl:variable name="gs_default_object_code" select="~JavaScript/default-object-code"/>

    <!-- HTTP headers -->
    <xsl:apply-templates select="$gs_request/gs:HTTP" mode="basic_http_headers"/>
    <xsl:apply-templates select="$gs_request/gs:cookies" mode="process_session"/>
    <xsl:apply-templates select="$gs_default_object_code" mode="basic_http_headers_last_modified"/>
    <xsl:apply-templates select="$gs_default_object_code" mode="basic_http_headers_content_type"/>

    <!-- gs_HTTP_render is called on the first element in $gs_request_target(s) -->
    <xsl:apply-templates select="$gs_request_target/javascript:code" mode="standalone_contents">
      <!-- classes in the JavaScript class have priority but NOT the actual ~JavaScript class (JS) -->
      <xsl:sort select="count(../ancestor::class:JavaScript[1])" order="descending"/>
      <!-- simple cached count of the deep inherited class count -->
      <xsl:sort select="database:base-class-count(..)" data-type="number"/>
      <!-- low level objects with static and global init should take precedence, e.g. Event.registerEvent(...) -->
      <xsl:sort select="count(javascript:object/javascript:static-method|javascript:object/javascript:global-init)" order="descending"/>
    </xsl:apply-templates>

    <!-- missing javascript:object's, e.g. HTTP -->
    <xsl:apply-templates select="$gs_request_target[not(javascript:code/javascript:object)]" mode="gs_blank_standalone_contents">
      <xsl:with-param name="gs_default_object_code" select="$gs_default_object_code"/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="class:*" mode="gs_blank_standalone_contents">
    <!-- missing javascript:object's, e.g. HTTP -->
    <xsl:param name="gs_default_object_code"/>
    <xsl:apply-templates select="$gs_default_object_code" mode="standalone_contents">
      <xsl:with-param name="gs_class" select="."/>
    </xsl:apply-templates>
  </xsl:template>
</xsl:stylesheet>
