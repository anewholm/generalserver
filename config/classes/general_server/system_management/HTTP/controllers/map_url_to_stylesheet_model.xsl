<xsl:stylesheet response:server-side-only="true" xmlns:exsl="http://exslt.org/common" xmlns:class="http://general_server.org/xmlnamespaces/class/2006" xmlns:user="http://general_server.org/xmlnamespaces/user/2006" xmlns="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:request="http://general_server.org/xmlnamespaces/request/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:response="http://general_server.org/xmlnamespaces/response/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:dyn="http://exslt.org/dynamic" xmlns:regexp="http://exslt.org/regular-expressions" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns:flow="http://exslt.org/flow" xmlns:str="http://exslt.org/strings" name="map_url_to_stylesheet_model" version="1.0" extension-element-prefixes="dyn str regexp database server request flow debug repository user">
  <!-- ###################################################################################################### -->
  <!-- ############################### detect url type (xpath/filesystem) ################################### -->
  <!-- ###################################################################################################### -->
  <xsl:template match="object:Request/gs:url" mode="map_url_to_stylesheet_model">
    <debug:server-message if="$gs_debug_url_steps" output="object:Request [{$gs_request/gs:message_type}] =&gt; {$gs_request/gs:url} =&gt; map_url_to_stylesheet_model loaded and handling request"/>
    <xsl:apply-templates select="." mode="gs_detect_url_type"/>
  </xsl:template>

  <xsl:template match="object:Request/gs:url" mode="gs_detect_url_type">
    <!-- is the URL an xpath statement or a directory recursion -->
    <xsl:if test="$gs_website_root">
      <!--
        see if the path is a valid xpath location
        e.g.
          /admin/index                (first client data portion request, followed by the XSLStylesheet)
          /shared/ajax/javascript:*   (included handlers for JavaScript and CSSStylesheet will handle this)
          /admin/index_xsl.xsl        (.xsl simply indicates that we want to use the XSL MessageInterpretation)
        NOTE: if it IS a valid xpath, it may contain multiple nodes
        and, there may be a direct handler for them, e.g. css:* and javascript:*
        please xsl:include handlers above and the generic system will then use them
        no need for dynamic inclusion here
      -->
      <xsl:if test="$gs_request_target">
        <!-- potential MULTI-file output here (xpath)
          so we use the $gs_request_target to indicate the files
          and apply-templates only to the first one
          mime-type will failover to namespace, use the first node in the sequence
        -->
        <debug:server-message if="$gs_debug_url_steps" output="object:Request [{$gs_request/gs:message_type}] =&gt; {$gs_request/gs:url} =&gt; direct xpath $gs_request_target(s) [{count($gs_request_target)}] [{name($gs_request_target[1])}] found"/>
        <xsl:apply-templates select="$gs_request_target[1]" mode="gs_server_redirect"/>
      </xsl:if>

      <xsl:if test="not($gs_request_target)">
        <!-- location of xsl:include same as:
          C++ clearMyCachedStylesheetsRecursive()
          C++ translateIncludes() @href => @xpath
          LibXml2: xsl:include processing (of @xpath) during server side compilation
          map_url_to_stylesheet_model.xsl REST based client xsl:include requests
          AJAX client_stylesheet_transform.xsl xsl:include include directly in main xsl:stylesheet request
        -->
        <debug:server-message if="$gs_debug_url_steps" output="object:Request [{$gs_request/gs:message_type}] =&gt; {$gs_request/gs:url} =&gt; $gs_request_target not found, proceeding to gs_recurse_to_main_stylesheet"/>
        <debug:server-message output="object:Request [{$gs_request/gs:message_type}] =&gt; [{$gs_request/gs:url}] =&gt; [{$gs_request_xpath}] $gs_request_target not found, proceeding to gs_recurse_to_main_stylesheet"/>

        <!--
          otherwise try to follow the path translating from url to node local-name() || @name etc.
          we look for the stylesheet to get the info
          which requires adding the .xsl extension if it has not been stated, e.g. on the data portion
          these parts are then slowly fed, one-at-a-time in to $gs_website_root_xpath
            $gs_url_relative_path_extended ==> $gs_website_root_xpath
        -->
        <xsl:variable name="gs_url_relative_path_extended">
          <xsl:value-of select="$gs_request/gs:url"/>
          <xsl:if test="not($gs_request/gs:url_extension)">.xsl</xsl:if>
        </xsl:variable>

        <debug:server-message output="NOT_CURRENTLY_USED but sometimes triggers so we comment it" type="warning"/>
        <!--debug:NOT_CURRENTLY_USED because="$gs_request_target direct xpath being developed currently but this should still work"/>
        <xsl:apply-templates select="$gs_website_root" mode="gs_recurse_to_main_stylesheet">
          <xsl:with-param name="gs_url_relative_path" select="$gs_url_relative_path_extended"/>
        </xsl:apply-templates-->
      </xsl:if>
    </xsl:if>

    <xsl:if test="not($gs_website_root)">
      <debug:server-message if="$gs_debug_url_steps" output="object:Request [{$gs_request/gs:message_type}] =&gt; {$gs_request/gs:url} =&gt; website not found for [{request:resource-ID()}]" type="warning"/>
      <xsl:apply-templates select="$gs_websites/website_not_found" mode="process_main_stylesheet"/>
    </xsl:if>
  </xsl:template>


  <!-- ###################################################################################################### -->
  <!-- ############################### recursion to xsl:include for output ################################## -->
  <!-- ###################################################################################################### -->
  <xsl:template match="xsl:include">
    <!-- relative paths to style sheets -->
    <debug:NOT_CURRENTLY_USED/>
    <xsl:apply-templates select="../.." mode="recurse_to_sub_stylesheet">
      <xsl:with-param name="gs_url_relative_path" select="@href"/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="*" mode="recurse_to_sub_stylesheet">
    <xsl:param name="gs_url_relative_path"/>
    <debug:NOT_CURRENTLY_USED/>

    <!-- if we have no directories left then match on remainder -->
    <xsl:if test="not(contains($gs_url_relative_path, '/'))">
      <xsl:apply-templates select="*[local-name() = $gs_url_relative_path or @name = $gs_url_relative_path]/xsl:stylesheet"/>
    </xsl:if>

    <!-- otherwise, descend further using substrings (they return empty if strings are not found) -->
    <xsl:variable name="next" select="substring-before($gs_url_relative_path, '/')"/>
    <xsl:apply-templates select="*[local-name() = $next or @name = $gs_url_relative_path]|parent::*[$next = '..']" mode="recurse_to_sub_stylesheet">
      <xsl:with-param name="gs_url_relative_path" select="substring-after($gs_url_relative_path, '/')"/>
    </xsl:apply-templates>
  </xsl:template>

  <!-- ###################################################################################################### -->
  <!-- ########################################### server redirection ####################################### -->
  <!-- ###################################################################################################### -->
  <xsl:template match="/|@*|node()" mode="gs_server_redirect">
    <xsl:choose>
      <xsl:when test="gs:server-redirect and dyn:evaluate(gs:server-redirect/@test)">
        <!-- server redirect test passed: process another stylesheet -->
        <xsl:variable name="gs_server_redirect_location" select="(gs:server-redirect/@location | meta:server-redirect/@test)[1]"/>
        <xsl:variable name="gs_server_redirect_location_node" select="dyn:evaluate($gs_server_redirect_location)"/>
        <debug:server-message if="$gs_debug_url_steps" output="object:Request [{$gs_request/gs:message_type}] =&gt; {$gs_request/gs:url} =&gt; meta:server-redirect [{$gs_server_redirect_location}] =&gt; [{$gs_server_redirect_location_node/@repository:name}]" type="warning"/>
        <xsl:if test="$gs_server_redirect_location_node">
          <xsl:if test="database:has-same-node($gs_server_redirect_location_node, current())"><debug:server-message output="blocked cyclic server-redirect in {@xml:id}"/></xsl:if>
          <xsl:if test="not(database:has-same-node($gs_server_redirect_location_node, current()))">
            <xsl:apply-templates select="$gs_server_redirect_location_node" mode="gs_HTTP_render"/>
          </xsl:if>
        </xsl:if>
      </xsl:when>

      <xsl:otherwise>
        <!-- normal direct execution -->
        <debug:server-message if="$gs_debug_url_steps" output="object:Request [{$gs_request/gs:message_type}] =&gt; {$gs_request/gs:url} =&gt; no meta:server-redirect"/>
        <xsl:apply-templates select="." mode="gs_HTTP_render"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!-- ###################################################################################################### -->
  <!-- ###################################################################################################### -->
  <!-- ###################################################################################################### -->
  <xsl:template match="*" mode="gs_recurse_to_main_stylesheet">
    <!-- directory recursion and file examination may output shared, website, section (dir) and specific commands -->
    <xsl:param name="gs_url_relative_path"/>

    <debug:NOT_CURRENTLY_USED/>

    <!-- if we have no gs_url_relative_path then we have arrived! -->
    <xsl:if test="not($gs_url_relative_path)">
      <xsl:apply-templates select="." mode="gs_server_redirect"/>
    </xsl:if>

    <!-- otherwise, descend further using substrings (they return empty if strings are not found)
      we check:
        xpath section:    xsl:stylesheet
        local-name:       stylesheet
        name:             xsl:stylesheet
        [@repository:name: explorer.xsl] NOT_ANYMORE
        @name:            root
        parent:           ..
    -->
    <xsl:if test="$gs_url_relative_path">
      <xsl:variable name="next_name" select="str:substring-before($gs_url_relative_path, '/', $gs_url_relative_path)"/>
      <xsl:variable name="next_node_xpath" select="dyn:evaluate($next_name)" xsl:error-policy="continue"/>
      <xsl:variable name="next_node_name" select="*[local-name() = $next_name or name() = $next_name or @name = $next_name]"/>
      <xsl:variable name="next_node_parent" select="parent::*[$next_name = '..']"/>
      <xsl:variable name="next_node" select="($next_node_xpath | $next_node_name | $next_node_parent)[1]"/>
      <debug:server-message if="$gs_debug_url_steps" output="  -&gt; $next_name [{local-name($next_node)}/{$next_node/@xml:id}]"/>

      <!-- direct child find -->
      <xsl:if test="$next_node">
        <xsl:if test="count($next_node) &gt; 1">
          <xsl:text>404 - Ambiguous path</xsl:text>
          <debug:server-message output="404: Ambiguous path looking for [{$gs_url_relative_path}] at [{name(..)}/{name()}/]"/>
          <request:throw class="MultipleResourceFound" parameter1="{$gs_url_relative_path}"/>
        </xsl:if>
        <xsl:apply-templates select="$next_node" mode="gs_recurse_to_main_stylesheet">
          <xsl:with-param name="gs_url_relative_path" select="substring-after($gs_url_relative_path, '/')"/>
          <!-- TODO: need to make this follow the axis used above to get $next_node -->
        </xsl:apply-templates>
      </xsl:if>

      <!-- special index and all files in absence of gs_url_relative_path items -->
      <xsl:if test="not($next_node)">
        <xsl:choose>
          <xsl:when test="all">
            <xsl:apply-templates select="all" mode="gs_recurse_to_main_stylesheet"/>
          </xsl:when>

          <xsl:when test="index">
            <xsl:apply-templates select="index" mode="gs_recurse_to_main_stylesheet"/>
          </xsl:when>

          <xsl:otherwise>
            <xsl:text>404 - File not found on this server</xsl:text>
            <debug:server-message output="404: looking for [{$gs_url_relative_path}] at [{name(..)}/{name()}/]"/>
            <request:throw class="ResourceNotFound" parameter1="{$gs_url_relative_path}"/>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:if>
    </xsl:if>
  </xsl:template>
</xsl:stylesheet>
