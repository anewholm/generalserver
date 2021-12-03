<xsl:stylesheet xmlns="http://www.w3.org/1999/xhtml" xmlns:exsl="http://exslt.org/common" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:class="http://general_server.org/xmlnamespaces/class/2006" xmlns:user="http://general_server.org/xmlnamespaces/user/2006" name="HTTP_processing" response:server-side-only="true" version="1.0" extension-element-prefixes="dyn str regexp database server request flow debug repository user">
  <!-- HTTP rendering of XSLStylesheet nodes:
    (1) @mode=gs_HTTP_render a direct request for an XSLStylesheet node will conditionally:
      return the XML database:query data for a further request for the same XSLStylesheet node but with an ".xsl" extension for (2) below
      or carry out a complete server side transform and return HTML
    (2) @mode=gs_HTTP_render_direct_simple with an ".xsl" extension will complete the client-side transform:
      return the stylesheet directly for client transform
    (3) @mode=gs_HTTP_render_dynamic renders DSXL stylesheets
  -->

  <xsl:template match="xsl:stylesheet" mode="gs_HTTP_render_client_server_switch">
    <!-- (1) @mode=gs_HTTP_render a direct request for an XSLStylesheet node
      stylesheet processing options
      $gs_client_side_xslt is built from the FORCE_SERVER_XSLT option in the URL and subsequent object:Session attribute
      @meta:server-side-xslt is the stylesheet directive
      $gs_use_parent_context:
        on  if always a server-side transform      (@meta:server-side-xslt)
        off if mimicking the client-side transform (not @meta:server-side-xslt)
    -->
    <xsl:variable name="gs_output_headers" select="str:not(@meta:http-headers, true()) or (@meta:http-headers = 'initial request only' and $gs_is_first_request_on_connection)"/>
    <xsl:variable name="gs_use_parent_context" select="str:boolean(@meta:server-side-xslt)"/>

    <!--
      stylesheet connection control section NOT_CURRENTLY_USED:
      @meta:wait-for-further-requests and @meta:close-connection are useful for open connections
      GS currently uses only long-polling: every server message is a separate request
      and the client MUST re-start it's listening immediately after receiving a server message
    -->
    <xsl:if test="str:boolean(@meta:close-connection)"><conversation:close-connection setting="{@meta:close-connection}"/></xsl:if>
    <xsl:if test="str:boolean(@meta:wait-for-further-requests)"><conversation:wait-for-further-requests setting="{@meta:wait-for-further-requests}"/></xsl:if>

    <!--
      ask the server to transform the output if necessary
    -->
    <!-- xsl:variable name="gs_class" select="database:classes($gs_request_target)"/>
    <xsl:variable name="gs_server_side_transform" select="str:boolean($gs_class/@server-side-transform-only, false())"/ -->
    <xsl:variable name="gs_server_side_xslt" select="not($gs_client_can_XSLT) or str:boolean($gs_session/@force-server-XSLT) or str:boolean(@meta:server-side-xslt)"/>
    <xsl:if test="$gs_server_side_xslt">
      <debug:server-message if="$gs_debug_url_steps" output="object:Request [{$gs_request/gs:message_type}] =&gt; {$gs_request/gs:url} =&gt; server-side handling"/>
      <!-- TODO: response:set-post-transform stylesheet="."/ -->
    </xsl:if>

    <!--
      client-side data return portion
      defined by this stylesheet
      return a client side Processing Instruction to transform with this stylesheet
      will output HTML 4.01 at the client end for FireFox compatability
    -->
    <debug:server-message if="$gs_debug_url_steps or $gs_session/@force-server-XSLT" output="object:Request [{$gs_request/gs:message_type}] =&gt; {$gs_request/gs:url} =&gt; client-side handling"/>
    <xsl:if test="$gs_output_headers">
      <xsl:apply-templates select="$gs_request/gs:HTTP" mode="basic_http_headers"/>
      <xsl:apply-templates select="$gs_request/gs:cookies" mode="process_session"/>
      <xsl:apply-templates select="." mode="gs_main_stylesheet_header_xml"/>
    </xsl:if>

    <xsl:apply-templates select="." mode="gs_interface_render_stylesheet_data"/>
  </xsl:template>

  <!-- ###################################################################################################### -->
  <!-- ###################################################################################################### -->
  <!-- ###################################################################################################### -->
  <xsl:template match="xsl:stylesheet" mode="gs_HTTP_render_direct_simple">
    <!-- (2) @mode=gs_HTTP_render_direct_simple with an ".xsl" extension
        xmlns:javascript and xmlns:css processing
    -->
    <debug:server-message if="$gs_debug_url_steps" output="object:Request [{$gs_request/gs:message_type}] =&gt; {$gs_request/gs:url} =&gt; direct_simple handler [{name()}]"/>
    <xsl:choose>
      <xsl:when test="string(@meta:last-modified) and string($gs_request/gs:if-modified-since) and not(server:date-compare(@meta:last-modified, $gs_request/gs:if-modified-since) = '&gt;')">
        <!-- if not modified since x then 304 not changed, client side cache hit -->
        <debug:server-message if="$gs_debug_url_steps" output="object:Request [{$gs_request/gs:message_type}] last modified cache hit [{@meta:last-modified}] &lt; [{$gs_request/gs:if-modified-since}]"/>
        <xsl:apply-templates select="$gs_request/gs:HTTP" mode="basic_http_headers_not_modified"/>
      </xsl:when>

      <xsl:otherwise>
        <!-- if-modified-since x then re-serve the data: full output -->
        <debug:server-message if="$gs_debug_url_steps" output="object:Request [{$gs_request/gs:message_type}] last modified cache miss [{@meta:last-modified}] &lt; [{$gs_request/gs:if-modified-since}]"/>
        <xsl:apply-templates select="$gs_request/gs:HTTP" mode="basic_http_headers"/>
        <xsl:apply-templates select="$gs_request/gs:cookies" mode="process_session"/>

        <xsl:apply-templates select="." mode="basic_http_headers_last_modified"/>
        <xsl:apply-templates select="." mode="basic_http_headers_content_type"/>

        <xsl:apply-templates select="." mode="gs_code_render_direct_simple"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>


  <!-- ###################################################################################################### -->
  <!-- ###################################################################################################### -->
  <!-- ###################################################################################################### -->
  <xsl:template match="xsl:stylesheet" mode="gs_HTTP_render_dynamic">
    <!-- (3) renders DSXL stylesheets
      requires xsl:stylesheet[xsl:template[@mode='gs_generate_XSL']]
      DSXL is standalone: the input-node is always the entire readable system
    -->
    <debug:server-message if="$gs_debug_url_steps" output="object:Request [{$gs_request/gs:message_type}] =&gt; {$gs_request/gs:url} =&gt; dynamic (DXSL) handler [{name()}]"/>
    <xsl:apply-templates select="$gs_request/gs:HTTP" mode="basic_http_headers"/>
    <xsl:apply-templates select="$gs_request/gs:cookies" mode="process_session"/>

    <xsl:apply-templates select="." mode="basic_http_headers_last_modified"/>
    <xsl:apply-templates select="." mode="basic_http_headers_content_type"/>

    <!-- DSXL is standalone: the input-node is always the entire readable system -->
    <xsl:apply-templates select="." mode="gs_code_render_dynamic"/>
  </xsl:template>


  <!-- ###################################################################################################### -->
  <!-- ########################################### stylesheet data process (main) ################################ -->
  <!-- ###################################################################################################### -->
  <xsl:template match="xsl:stylesheet" mode="gs_interface_render_stylesheet_data">
    <!-- loader xsl:stylesheets contain statements that are now processed
        interface:dynamic ...
        database:query ...
        etc.
      as part the data portion for this stylesheet
      an included <?xsl-stylesheet statement will then request this same stylesheets xsl statements instead
    -->
    <debug:server-message if="$gs_debug_url_steps" output="object:Request [{$gs_request/gs:message_type}] =&gt; {$gs_request/gs:url} =&gt; gs_interface_render_stylesheet_data [{@name}] XML data output template with $gs_request_target [{name($gs_request_target)}]"/>
    <xsl:apply-templates select="*" mode="gs_interface_render_stylesheet_data_component"/>
  </xsl:template>

  <!-- interface render only the non-XSL statements in the xsl:stylesheet -->
  <xsl:template match="xsl:*" mode="gs_interface_render_stylesheet_data_component"/>
  <xsl:template match="*" mode="gs_interface_render_stylesheet_data_component">
    <xsl:apply-templates select="." mode="gs_interface_render"/>
  </xsl:template>

  <!-- ###################################################################################################### -->
  <!-- ########################################### header content-types and DECLs ########################### -->
  <!-- ###################################################################################################### -->
  <xsl:template match="xsl:stylesheet" mode="gs_main_stylesheet_header_xhtml">
    <xsl:apply-templates select="." mode="basic_http_headers_content_type">
      <xsl:with-param name="gs_specific_extension" select="(@meta:content-type | exsl:node-set('xhtml'))[1]"/>
    </xsl:apply-templates>

    <!-- we have to do the DTD manually
      because we are transforming directly in to a node, unlike client side
      and we want it half way down the output, not at the top
    -->
    <xsl:if test="not(xsl:output/@doctype-public = 'none') and not(@meta:doctype-public = 'none')">
      <xsl:apply-templates select="." mode="gs_doctype">
        <xsl:with-param name="gs_doctype_public" select="'-//W3C//DTD XHTML 1.0 Strict//EN'"/>
        <xsl:with-param name="gs_doctype_system" select="'http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd'"/>
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>

  <xsl:template match="xsl:stylesheet" mode="gs_main_stylesheet_header_html">
    <xsl:apply-templates select="." mode="basic_http_headers_content_type">
      <xsl:with-param name="gs_specific_extension" select="(@meta:content-type | exsl:node-set('html'))[1]"/>
    </xsl:apply-templates>

    <!-- we have to do the DTD manually
      because we are transforming directly in to a node, unlike client side
      and we want it half way down the output, not at the top
    -->
    <xsl:apply-templates select="xsl:output" mode="gs_doctype"/>
  </xsl:template>

  <xsl:template match="xsl:stylesheet" mode="gs_main_stylesheet_header_xml">
    <xsl:apply-templates select="." mode="basic_http_headers_last_modified"/>
    <xsl:apply-templates select="." mode="basic_http_headers_content_type">
      <!-- note that this is without extension and for the xsl namespace -->
      <xsl:with-param name="gs_specific_extension" select="(@meta:content-type | exsl:node-set('xml'))[1]"/>
    </xsl:apply-templates>

    <!--
      keep the stylesheet name simple to maintain client cacheing
      there are various ways to generate this stylesheet path
      remember <gs:server-redirect>
      and that XSL stylesheet requests must have an .xsl extension
        [BEST] use an xpath-to-node() function call based on the $gs_website_root for the relative URL
          because it is also hardlink aware when a base node is sent through...
          false() use-ids because we want relative xsl:include @href to work
        use the $request/url and add the @repository:name. NOTE: the served stylesheet might be different <gs:server-redirect>
        use the $gs_website_root_xpath. NOTE: this variable a bit poo
        use the @xml:id
          relative xsl:includes then will not work
          /a/b/load_dot_xsl => @href=other.xsl resolves to /a/b/other.xsl
          id('load_dot_xsl')      => @href=other.xsl resolves to /other.xsl
        [CURRENT] conditionally use the @xml:id
          if there are no xsl:includes / xsl:imports, use the @xml:id
          which is the case for the standard load_dot_xsl
    -->
    <xsl:variable name="gs_stylesheet_modifiers"><xsl:apply-templates select="$gs_request/gs:stylesheet_modifiers" mode="gs_construct_query_string"/></xsl:variable>
    <xsl:variable name="gs_explicit_stylesheet" select="$gs_query_string/@explicit-stylesheet"/>
    <xsl:variable name="gs_includes" select="xsl:include|xsl:import"/>
    <xsl:variable name="gs_non_DXSL_includes" select="$gs_includes[not(str:ends-with(@href,'.dxsl'))]"/>
    <xsl:variable name="gs_website_url_to_stylesheet">
      <xsl:if test="not($gs_explicit_stylesheet)">
        <!-- full URL because server-side XSLT will try to 
          Repository::factory_reader() based on this URL 
          however: this would mean that the HREF cannot be converted to XPath without the root
        -->
        <xsl:value-of select="$gs_request/gs:HTTP"/>
        <xsl:text>://</xsl:text>
        <xsl:value-of select="$gs_request/gs:host"/>
        <xsl:if test="$gs_request/gs:host/@port">:<xsl:value-of select="$gs_request/gs:host/@port"/></xsl:if>
        <xsl:text>/</xsl:text>
        <xsl:value-of select="database:xpath-to-node(., $gs_website_root)"/>
        <xsl:text>.xsl</xsl:text>
      </xsl:if>
      <xsl:if test="$gs_explicit_stylesheet"><xsl:value-of select="$gs_explicit_stylesheet"/></xsl:if>
      <xsl:if test="string($gs_stylesheet_modifiers)">&amp;<xsl:value-of select="$gs_stylesheet_modifiers"/></xsl:if>
    </xsl:variable>

    <xsl:apply-templates select="." mode="gs_xml_decl"/>
    
    <xsl:if test="str:boolean($gs_query_string/@data-only)">
      <!-- debug output only in ?data-only=yes mode -->
      <xsl:comment>
        <xsl:text>xml-stylesheet href="</xsl:text>
        <xsl:value-of select="$gs_website_url_to_stylesheet"/>
        <xsl:text>"</xsl:text>
      </xsl:comment>
    </xsl:if>
    <xsl:else>
      <xsl:apply-templates select="." mode="gs_xml_stylesheet_statement">
        <xsl:with-param name="gs_href" select="$gs_website_url_to_stylesheet"/>
      </xsl:apply-templates>
    </xsl:else>
  </xsl:template>

  <xsl:template match="*" mode="gs_xml_decl">
    <!-- response analyser will process this 
      TODO: however only the Get request has one at the moment
    -->
    <xsl:processing-instruction name="xml">
      <xsl:attribute name="version">1.0</xsl:attribute>
      <xsl:attribute name="encoding">utf-8</xsl:attribute>
    </xsl:processing-instruction>
    <!-- response:xml-decl/ -->
  </xsl:template>
  
  <xsl:template match="*" mode="gs_xml_stylesheet_statement">
    <!-- response analyser will process this -->
    <!-- xsl:processing-instruction name="xml-stylesheet">
      <xsl:attribute name="charset">utf-8</xsl:attribute>
      <xsl:attribute name="type">text/xsl</xsl:attribute>
      <xsl:attribute name="href"><xsl:value-of select="$gs_href"/></xsl:attribute>
    </xsl:processing-instruction -->
    <xsl:param name="gs_href"/>
    
    <response:xml-stylesheet href="{$gs_href}" response:append-classes="false"/>
  </xsl:template>
  
  <xsl:template match="xsl:output" mode="gs_doctype">
    <xsl:apply-templates select="/" mode="gs_doctype">
      <xsl:with-param name="gs_doctype_public" select="@doctype-public"/>
      <xsl:with-param name="gs_doctype_system" select="@doctype-system"/>
    </xsl:apply-templates>
  </xsl:template>
  
  <xsl:template match="/|*" mode="gs_doctype">
    <xsl:param name="gs_doctype_public"/>
    <xsl:param name="gs_doctype_system"/>
    
    <!-- TODO: replace this with an <xsl:doc-type> instruction -->
    <xsl:text disable-output-escaping="yes">&lt;!</xsl:text>
    <xsl:text>DOCTYPE html</xsl:text>
    <xsl:if test="$gs_doctype_public and not($gs_doctype_public = 'none')">
      <xsl:text> PUBLIC "</xsl:text><xsl:value-of select="$gs_doctype_public"/><xsl:text>"</xsl:text>
    </xsl:if>
    <xsl:if test="$gs_doctype_system and not($gs_doctype_system = 'none')">
      <xsl:text> "</xsl:text><xsl:value-of select="$gs_doctype_system"/><xsl:text>"</xsl:text>
    </xsl:if>
    <xsl:text disable-output-escaping="yes">&gt;</xsl:text>
  </xsl:template>
</xsl:stylesheet>
