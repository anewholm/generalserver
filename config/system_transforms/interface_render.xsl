<xsl:stylesheet xmlns="http://www.w3.org/1999/xhtml" xmlns:conversation="http://general_server.org/xmlnamespaces/conversation/2006" xmlns:meta="http://general_server.org/xmlnamespaces/meta/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:exsl="http://exslt.org/common" xmlns:flow="http://exslt.org/flow" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:str="http://exslt.org/strings" xmlns:response="http://general_server.org/xmlnamespaces/response/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" extension-element-prefixes="session conversation database debug" response:server-side-only="true" name="interface_render" version="1.0">
  <!-- gs_interface_render - recursive, overridable node output
    all part of the @mode process:
      gs_HTTP_render => gs_interface_render => gs_view_render
      with gs_inheritance_render
    uses interface-render.xsl in various classes for custom gs_interface_render items, e.g.
      <database:query .../>
      <database:wait-* .../>
      <class:xschema-add  @destination .../>
      <class:xschema-edit @data .../>
    $parameters include:
      FIRST element effect:
        $gs_interface_mode                 => @gs:interface-mode
      WHOLE tree effect:
        $gs_create_meta_context_attributes => ALL dynamic on each element: @meta:child-count, @meta:xpath-to-node, ...
        $gs_hardlink_output                => if = non-default 'placeholder' good for editing the XML, i.e. meta:hardlink @target-id
        $gs_database_query_output          => if = non-default 'placeholder' good for editing the XML, i.e. database:query
        $gs_interface_render_output        => if = non-default 'placeholder' good for editing the XML, e.g. xsd:add
        $gs_suppress_xml_id                =>
    does not output
      @repository:*
      can also suppress @xml:id
  -->
  <xsl:template match="*" mode="gs_interface_render_copy" meta:interface-template="yes">
    <!-- pseudo mode using gs_interface_render parameters that emulate an xsl:copy-of
      no meta:hardlink or database:query processing
    -->
    <xsl:apply-templates select="." mode="gs_interface_render">
      <!-- everything off! -->
      <xsl:with-param name="gs_create_meta_context_attributes" select="'none'"/>
      <xsl:with-param name="gs_hardlink_output" select="'placeholder'"/>
      <xsl:with-param name="gs_database_query_output" select="'placeholder'"/>
      <xsl:with-param name="gs_interface_render_output" select="'placeholder'"/>
    </xsl:apply-templates>
  </xsl:template>

  <!-- attribute output: allow non-XSL data dynamic attributes {@example} -->
  <xsl:template match="@*" mode="gs_interface_render" meta:interface-template="yes">
    <xsl:param name="gs_interface_mode"/>
    <xsl:param name="gs_interface_render_output"/>
    <xsl:param name="gs_truncate_ajax" select="1024"/>
    
    <xsl:if test="response:can-have-attribute()">
      <debug:server-message if="$gs_debug_interface_render" output="  @{name()}"/>
      <xsl:if test="$gs_interface_render_output = 'placeholder'"><xsl:copy-of select="."/></xsl:if>
      <xsl:else>
        <xsl:variable name="gs_value" select="string(.)"/>
        <xsl:if test="contains($gs_value, '{') and not(parent::html:script) and not(parent::javascript:*)">
          <!-- cannot use xsl:copy here because it will copy the entire attribute with value
            we may experience some issues with namespaced nodes and dynamic values
            e.g. @xlink:thing="{whatever}"
          -->
          <xsl:variable name="gs_closest_xsl" select="ancestor-or-self::xsl:*[1]"/>
          <xsl:variable name="gs_is_embedded_xsl_stylesheet_interface" select="$gs_closest_xsl[self::xsl:stylesheet]"/>
          <!-- getting xmlns:ns_1 namespaces when using @namespace-uri -->
          <xsl:attribute name="{name()}">
            <xsl:if test="$gs_closest_xsl and not($gs_is_embedded_xsl_stylesheet_interface)">
              <xsl:value-of select="$gs_value"/>
            </xsl:if>
            <xsl:else>
              <xsl:apply-templates select=".." mode="gs_interface_render_str_dynamic">
                <xsl:with-param name="gs_str" select="$gs_value"/>
              </xsl:apply-templates>
            </xsl:else>
          </xsl:attribute>
        </xsl:if>
        <xsl:else><xsl:copy-of select="."/></xsl:else>
      </xsl:else>
    </xsl:if>
    <xsl:else>
      <debug:server-message if="$gs_debug_interface_render" output="  @{name()} REJECTED"/>
    </xsl:else>
  </xsl:template>

  <xsl:template match="/|*" mode="gs_interface_render_str_dynamic" meta:interface-template="yes">
    <!-- apply str:dynamic on the parent element, rather than the attribute
      RECURSIVE str:dynamic() 100 times
      TODO: recursion happens all in the context of this node. however, that still supports spreadsheet still cell referencing
      NOTE: can cause errors if a node {} is requested in an unexpected context
      e.g. <interface:AJAXHTMLLoader @interface-mode={$gs_not_available}
    -->
    <xsl:param name="gs_str"/>
    <xsl:param name="gs_truncate_ajax" select="1024"/>

    <debug:server-message if="$gs_debug_interface_render" output="gs_view_render_data T[{substring($gs_str, 0, 10)}] [{$gs_truncate_ajax}] gs_interface_render_str_dynamic"/>
    
    <xsl:if test="self::html:pre">
      <xsl:value-of select="$gs_str"/>
    </xsl:if>
    <xsl:else>
      <xsl:variable name="gs_original_length" select="string-length($gs_str)"/>
      <debug:server-message if="$gs_debug_interface_render" output="HERE1, not getting to HERE2"/>
      <xsl:variable name="gs_str_dynamic" select="str:dynamic($gs_str, '', 100)"/>
      <debug:server-message if="$gs_debug_interface_render" output="HERE2"/>
      <xsl:variable name="gs_dynamic_length" select="string-length($gs_str_dynamic)"/>
      
      <xsl:if test="$gs_dynamic_length &gt; $gs_truncate_ajax">
        <!-- @meta:xpath-to-node points to the text here -->
        <debug:server-message if="$gs_debug_interface_render" output="gs_view_render_data T[{substring(., 0, 10)}] object:Truncate"/>
        <object:Truncate original-length="{$gs_original_length}" dynamic-length="{$gs_dynamic_length}" truncated-length="{$gs_truncate_ajax}" meta:xpath-to-node="{conversation:xpath-to-node()}">
          <xsl:value-of select="substring($gs_str_dynamic, 0, 1024)"/>
        </object:Truncate>
      </xsl:if>
      <xsl:else>
        <xsl:value-of select="$gs_str_dynamic"/>
      </xsl:else>
    </xsl:else>
  </xsl:template>

  <!-- comment output
    the built-in comment template ignores them
    TODO: ignore them on live?
  -->
  <!-- xsl:template match="comment()" mode="gs_interface_render">
    <xsl:copy-of select="."/>
  </xsl:template -->

  <!-- the built-in text() handler just outputs text content -->
  <xsl:template match="text()" mode="gs_interface_render" meta:interface-template="yes">
    <xsl:param name="gs_interface_render_output"/>
    <xsl:param name="gs_truncate_ajax" select="1024"/>

    <xsl:choose>
      <xsl:when test="$gs_interface_render_output = 'placeholder' or parent::html:style or parent::html:script or parent::javascript:* or ../@dynamic-string = 'off'">
        <debug:server-message if="$gs_debug_interface_render" output="gs_view_render_data T[{substring(., 0, 10)}] placeholder"/>
        <xsl:copy-of select="."/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:variable name="gs_str" select="."/>
        <debug:server-message if="$gs_debug_interface_render" output="gs_view_render_data T[{substring(., 0, 10)}] [{string-length(.)}] dynamic"/>
        
        <!-- we do not want spurious carrige return nodes in the output 
          generally we should be xsl:apply-templates select="*" to prevent this
          but sometimes we want to output text
        -->
        <xsl:if test="string-length($gs_str)">
          <xsl:apply-templates select=".." mode="gs_interface_render_str_dynamic">
            <xsl:with-param name="gs_str" select="$gs_str"/>
            <xsl:with-param name="gs_truncate_ajax" select="$gs_truncate_ajax"/>
          </xsl:apply-templates>
        </xsl:if>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!-- the built-in ignores processing-instruction() -->
  <!-- xsl:template match="processing-instruction()" mode="gs_interface_render"/ -->
  
  <xsl:template match="*" mode="gs_interface_render" meta:interface-template="yes">
    <!-- TOP LEVEL ONLY: $gs_interface_mode -->
    <xsl:param name="gs_interface_mode"/>
    <xsl:param name="gs_first" select="true()"/>
    
    <!-- WHOLE TREE: $gs_create_meta_context_attributes, $etc:
      @meta:child-count, @meta:position
      @meta:fully-qualified-prefix
      @meta:is-hard-linked, @meta:is-hard-link, @meta:is-registered
      @meta:xpath-to-node
    -->
    <xsl:param name="gs_create_meta_context_attributes"/>
    <xsl:param name="gs_suppress_xml_id"/>
    <xsl:param name="gs_hardlink_output"/>
    <xsl:param name="gs_database_query_output"/> <!-- aka softlinks -->
    <xsl:param name="gs_interface_render_output"/>
    <xsl:param name="gs_truncate_ajax" select="1024"/> <!-- TODO: gs_truncate_ajax -->
    
    <xsl:apply-templates select="." mode="gs_interface_render_normal">
      <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      <xsl:with-param name="gs_first" select="$gs_first"/>
      <xsl:with-param name="gs_create_meta_context_attributes" select="$gs_create_meta_context_attributes"/>
      <xsl:with-param name="gs_suppress_xml_id" select="$gs_suppress_xml_id"/>
      <xsl:with-param name="gs_hardlink_output" select="$gs_hardlink_output"/>
      <xsl:with-param name="gs_database_query_output" select="$gs_database_query_output"/> <!-- aka softlinks -->
      <xsl:with-param name="gs_interface_render_output" select="$gs_interface_render_output"/>
      <xsl:with-param name="gs_truncate_ajax" select="$gs_truncate_ajax"/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="*" mode="gs_interface_render_normal">
    <xsl:param name="gs_interface_mode"/>
    <xsl:param name="gs_first" select="true()"/>
    <xsl:param name="gs_create_meta_context_attributes"/>
    <xsl:param name="gs_suppress_xml_id"/>
    <xsl:param name="gs_hardlink_output"/>
    <xsl:param name="gs_database_query_output"/> <!-- aka softlinks -->
    <xsl:param name="gs_interface_render_output"/>
    <xsl:param name="gs_truncate_ajax" select="1024"/>

    <xsl:param name="gs_is_hardlink" select="database:is-hard-link()"/>

    <!-- debug -->
    <xsl:if test="$gs_debug_interface_render">
      <xsl:choose>
        <xsl:when test="database:is-node-element()">
          <debug:server-message output="gs_view_render_data [{name()}/{@xml:id}]"/>
        </xsl:when>
        <xsl:when test="database:is-node-attribute()">
          <debug:server-message output="gs_view_render_data @[{name()}/{../@xml:id}/{.}]"/>
        </xsl:when>
        <xsl:when test="database:is-node-text()">
          <debug:server-message output="gs_view_render_data T[{substring(., 0, 10)}]"/>
        </xsl:when>
        <xsl:otherwise>
          <debug:server-message output="gs_view_render_data ?[...]"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:if>

    <!-- assess size of output operation given the node-mask -->
    <xsl:variable name="gs_max_output_size" select="3000"/>
    <xsl:variable name="gs_output_size">
      <!-- database:with-node-mask>
        <xsl:value-of select="count(descendant-or-self::*)"/>
      </database:with-node-mask -->
    </xsl:variable>
    
    <xsl:if test="$gs_output_size &gt; $gs_max_output_size">
      <debug:server-message output="more than {$gs_max_output_size} nodes in output [{$gs_output_size}]. disregarding" type="error"/>
    </xsl:if>
    <xsl:else>
      <!-- hardlink output handling: follow / placeholder -->
      <xsl:choose>
        <xsl:when test="$gs_is_hardlink and $gs_hardlink_output = 'ignore'">
          <debug:server-message if="$gs_debug_interface_render" output="  $gs_hardlink_output=ignore"/>
        </xsl:when>
        <xsl:when test="str:boolean(@meta:system-node)">
          <xsl:comment>@meta:system-node [<xsl:value-of select="name()"/>] ignored</xsl:comment>
          <debug:server-message if="$gs_debug_interface_render" output="  @meta:system-node=yes"/>
        </xsl:when>
        <xsl:when test="@meta:xml-output = 'indirect'">
          <debug:server-message if="$gs_debug_interface_render" output="  @meta:xml-output=indirect"/>
        </xsl:when>
        <xsl:when test="$gs_is_hardlink and not($gs_first) and $gs_hardlink_output = 'placeholder'">
          <!-- gs:query-string/gs:xpath-to-target-nodes/* are hardlinked in but we want to display the top node at least -->
          <database:hardlink-info/>
          <debug:server-message if="$gs_debug_interface_render" output="  $gs_hardlink_output=placeholder"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:variable name="gs_class_definitions" select="database:classes()"/>
          <xsl:variable name="gs_base_class_definitions" select="database:base-classes-all()"/>
          <xsl:variable name="gs_all_classes" select="$gs_class_definitions|$gs_base_class_definitions"/>

          <debug:server-message if="$gs_debug_interface_render" output="  normal rendering..."/>
          
          <!-- data-queries: classes can require replacements for their data elements during output
            allows each class to apply DYNAMIC data-queries for it's data element response
            called during gs_view_render_data data output for each element
            Class xsl:include interface_render.xsl is inappropriate because it must be xsl:include
              and has more difficulty using database:query
            @interface-render-output=placeholder will suppress this
          -->
          <xsl:variable name="gs_data_queries_replace_result_fragment">
            <gs:queries-fragment>
              <database:without-node-mask hardlink-policy="suppress-exact-repeat">
                <xsl:variable name="gs_data_queries_replace" select="$gs_all_classes/repository:data-queries/*[@meta:data-query-type = 'replace']"/>
                <xsl:if test="$gs_data_queries_replace">
                  <xsl:if test="$gs_interface_render_output = 'placeholder'">
                    <debug:server-message if="$gs_debug_interface_render" output="[{count($gs_data_queries_replace)}] data queries ignored for [{name()}/{@xml:id}] in interface [{$gs_interface_mode}] because in placeholder mode"/>
                  </xsl:if>

                  <xsl:if test="not($gs_interface_render_output = 'placeholder')">
                    <debug:server-message if="$gs_debug_interface_render" output="testing [{count($gs_data_queries_replace)}] data queries for [{name()}/{@xml:id}] in interface [{$gs_interface_mode}]"/>
                    <xsl:apply-templates select="$gs_data_queries_replace" mode="gs_interface_render">
                      <xsl:with-param name="gs_outer_interface_mode" select="$gs_interface_mode"/>
                      <xsl:with-param name="gs_current" select="."/>
                      <xsl:with-param name="gs_class_definitions" select="$gs_class_definitions"/>
                      <xsl:with-param name="gs_base_class_definitions" select="$gs_base_class_definitions"/>
                    </xsl:apply-templates>
                  </xsl:if>
                </xsl:if>
              </database:without-node-mask>
            </gs:queries-fragment>
          </xsl:variable>
          <xsl:variable name="gs_data_queries_result" select="exsl:node-set($gs_data_queries_replace_result_fragment)/gs:queries-fragment/*"/>

          <!-- TODO: this data-fragment is actually just one XML_DOCUMENT_NODE... -->
          <xsl:if test="$gs_data_queries_result">
            <debug:server-message if="$gs_debug_interface_render" output="rendering [{name()}/{@xml:id}] replacement data-query"/>
            <xsl:copy-of select="$gs_data_queries_result"/>
          </xsl:if>
          <xsl:else>
            <!-- begin direct copy rendering of the node -->
            <xsl:apply-templates select="." mode="gs_interface_render_output_node">
              <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
              <xsl:with-param name="gs_create_meta_context_attributes" select="$gs_create_meta_context_attributes"/>
              <xsl:with-param name="gs_suppress_xml_id" select="$gs_suppress_xml_id"/>
              <xsl:with-param name="gs_hardlink_output" select="$gs_hardlink_output"/>
              <xsl:with-param name="gs_database_query_output" select="$gs_database_query_output"/> <!-- aka softlinks -->
              <xsl:with-param name="gs_interface_render_output" select="$gs_interface_render_output"/>
              <xsl:with-param name="gs_is_hardlink" select="$gs_is_hardlink"/>
              <xsl:with-param name="gs_class_definitions" select="$gs_class_definitions"/>
              <xsl:with-param name="gs_base_class_definitions" select="$gs_base_class_definitions"/>
              <xsl:with-param name="gs_all_classes" select="$gs_all_classes"/>
              <xsl:with-param name="gs_truncate_ajax" select="$gs_truncate_ajax"/>
            </xsl:apply-templates>
          </xsl:else>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:else>
  </xsl:template>

  <xsl:template match="*" mode="gs_interface_render_output_node">
    <xsl:param name="gs_interface_mode"/>
    <xsl:param name="gs_create_meta_context_attributes"/>
    <xsl:param name="gs_suppress_xml_id"/>
    <xsl:param name="gs_hardlink_output"/>
    <xsl:param name="gs_database_query_output"/> <!-- aka softlinks -->
    <xsl:param name="gs_interface_render_output"/>
    <xsl:param name="gs_is_hardlink" select="database:is-hard-link()"/>
    <xsl:param name="gs_class_definitions" select="database:classes()"/>
    <xsl:param name="gs_base_class_definitions" select="database:base-classes-all()"/>
    <xsl:param name="gs_all_classes" select="$gs_class_definitions|$gs_base_class_definitions"/>
    <xsl:param name="gs_truncate_ajax" select="1024"/>

    <xsl:copy>
      <xsl:apply-templates select="." mode="gs_interface_render_adorn_node">
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
        <xsl:with-param name="gs_create_meta_context_attributes" select="$gs_create_meta_context_attributes"/>
        <xsl:with-param name="gs_suppress_xml_id" select="$gs_suppress_xml_id"/>
        <xsl:with-param name="gs_hardlink_output" select="$gs_hardlink_output"/>
        <xsl:with-param name="gs_database_query_output" select="$gs_database_query_output"/> <!-- aka softlinks -->
        <xsl:with-param name="gs_interface_render_output" select="$gs_interface_render_output"/>
        <xsl:with-param name="gs_is_hardlink" select="$gs_is_hardlink"/>
        <xsl:with-param name="gs_class_definitions" select="$gs_class_definitions"/>
        <xsl:with-param name="gs_base_class_definitions" select="$gs_base_class_definitions"/>
        <xsl:with-param name="gs_all_classes" select="$gs_all_classes"/>
        <xsl:with-param name="gs_truncate_ajax" select="$gs_truncate_ajax"/>
      </xsl:apply-templates>

      <!-- will render all child::node() nodes, includes text(), but not @*
        $gs_interface_mode is only relevant to the top level nodes
      -->
      <xsl:apply-templates select="." mode="gs_interface_render_child_nodes">
        <xsl:with-param name="gs_create_meta_context_attributes" select="$gs_create_meta_context_attributes"/>
        <xsl:with-param name="gs_suppress_xml_id" select="$gs_suppress_xml_id"/>
        <xsl:with-param name="gs_hardlink_output" select="$gs_hardlink_output"/>
        <xsl:with-param name="gs_database_query_output" select="$gs_database_query_output"/>
        <xsl:with-param name="gs_interface_render_output" select="$gs_interface_render_output"/>
        <xsl:with-param name="gs_truncate_ajax" select="$gs_truncate_ajax"/>
      </xsl:apply-templates>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="*" mode="gs_interface_render_adorn_node" with-all-params="all" meta:interface-template="yes">
    <xsl:param name="gs_interface_mode"/>
    <xsl:param name="gs_display_class"/>
    <xsl:param name="gs_create_meta_context_attributes"/>
    <xsl:param name="gs_suppress_xml_id"/>
    <xsl:param name="gs_hardlink_output"/>
    <xsl:param name="gs_database_query_output"/> <!-- aka softlinks -->
    <xsl:param name="gs_interface_render_output"/>
    <xsl:param name="gs_is_hardlink" select="database:is-hard-link()"/>
    <xsl:param name="gs_class_definitions" select="database:classes()"/>
    <xsl:param name="gs_base_class_definitions" select="database:base-classes-all()"/>
    <xsl:param name="gs_all_classes" select="$gs_class_definitions|$gs_base_class_definitions"/>
    <xsl:param name="gs_truncate_ajax" select="1024"/>

    <xsl:apply-templates select="." mode="gs_interface_render_output_attributes">
      <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      <xsl:with-param name="gs_display_class" select="$gs_display_class"/>
      <xsl:with-param name="gs_create_meta_context_attributes" select="$gs_create_meta_context_attributes"/>
      <xsl:with-param name="gs_suppress_xml_id" select="$gs_suppress_xml_id"/>
      <xsl:with-param name="gs_hardlink_output" select="$gs_hardlink_output"/>
      <xsl:with-param name="gs_database_query_output" select="$gs_database_query_output"/>
      <xsl:with-param name="gs_interface_render_output" select="$gs_interface_render_output"/>
      <xsl:with-param name="gs_is_hardlink" select="$gs_is_hardlink"/>
      <xsl:with-param name="gs_class_definitions" select="$gs_class_definitions"/>
      <xsl:with-param name="gs_base_class_definitions" select="$gs_base_class_definitions"/>
      <xsl:with-param name="gs_all_classes" select="$gs_all_classes"/>
      <xsl:with-param name="gs_truncate_ajax" select="$gs_truncate_ajax"/>
    </xsl:apply-templates>

    <xsl:apply-templates select="." mode="gs_interface_render_output_elements">
      <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      <xsl:with-param name="gs_display_class" select="$gs_display_class"/>
      <xsl:with-param name="gs_create_meta_context_attributes" select="$gs_create_meta_context_attributes"/>
      <xsl:with-param name="gs_suppress_xml_id" select="$gs_suppress_xml_id"/>
      <xsl:with-param name="gs_hardlink_output" select="$gs_hardlink_output"/>
      <xsl:with-param name="gs_database_query_output" select="$gs_database_query_output"/>
      <xsl:with-param name="gs_interface_render_output" select="$gs_interface_render_output"/>
      <xsl:with-param name="gs_is_hardlink" select="$gs_is_hardlink"/>
      <xsl:with-param name="gs_class_definitions" select="$gs_class_definitions"/>
      <xsl:with-param name="gs_base_class_definitions" select="$gs_base_class_definitions"/>
      <xsl:with-param name="gs_all_classes" select="$gs_all_classes"/>
      <xsl:with-param name="gs_truncate_ajax" select="$gs_truncate_ajax"/>
    </xsl:apply-templates>
  </xsl:template>
  
  <xsl:template match="*" mode="gs_interface_render_output_attributes">
    <xsl:param name="gs_interface_mode"/>
    <xsl:param name="gs_display_class"/>
    <xsl:param name="gs_create_meta_context_attributes"/>
    <xsl:param name="gs_suppress_xml_id"/>
    <xsl:param name="gs_hardlink_output"/>
    <xsl:param name="gs_database_query_output"/> <!-- aka softlinks -->
    <xsl:param name="gs_interface_render_output"/>
    <xsl:param name="gs_is_hardlink" select="database:is-hard-link()"/>
    <xsl:param name="gs_class_definitions" select="database:classes()"/>
    <xsl:param name="gs_base_class_definitions" select="database:base-classes-all()"/>
    <xsl:param name="gs_all_classes" select="$gs_class_definitions|$gs_base_class_definitions"/>
    <xsl:param name="gs_truncate_ajax" select="1024"/>

    <debug:server-message if="$gs_debug_interface_render" output="  gs_interface_render_output_attributes"/>
    
    <!-- first attribute call to avoid conflicts: no response:can-have-attribute() necessary -->
    <xsl:if test="not($gs_interface_render_output = 'placeholder')">
      <xsl:apply-templates select="." mode="gs_interface_render_extra_attributes"/>
    </xsl:if>
    
    <!-- if the Class is using the default display template, e.g. Frame, then it can't set the debug -->
    <xsl:if test="$gs_interface_mode and response:can-have-attribute('gs:interface-mode')"><xsl:attribute name="gs:interface-mode"><xsl:value-of select="$gs_interface_mode"/></xsl:attribute></xsl:if>
    <xsl:if test="$gs_display_class and response:can-have-attribute('gs:display-class')"><xsl:attribute name="gs:display-class"><xsl:value-of select="$gs_display_class"/></xsl:attribute></xsl:if>

    <!-- @* response:can-have-attribute() each attribute -->
    <xsl:apply-templates select="@*" mode="gs_interface_render">
      <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      <xsl:with-param name="gs_interface_render_output" select="$gs_interface_render_output"/>
      <xsl:with-param name="gs_truncate_ajax" select="$gs_truncate_ajax"/>
    </xsl:apply-templates>

    <xsl:if test="str:not($gs_suppress_xml_id, true())">
      <xsl:apply-templates select="@xml:id" mode="gs_interface_render"/>
    </xsl:if>

    <!-- meta:classes, meta:class-bases, meta:is-registered -->
    <xsl:if test="not($gs_create_meta_context_attributes = 'none' or $gs_create_meta_context_attributes = 'class-only')">
      <xsl:apply-templates select="." mode="gs_interface_render_create_meta_context_attributes">
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
        <xsl:with-param name="gs_class_definitions" select="$gs_class_definitions"/>
        <xsl:with-param name="gs_interface_render_output" select="$gs_interface_render_output"/>
        <xsl:with-param name="gs_truncate_ajax" select="$gs_truncate_ajax"/>
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>
  
  <xsl:template match="*" mode="gs_interface_render_output_elements">
    <xsl:param name="gs_interface_mode"/>
    <xsl:param name="gs_create_meta_context_attributes"/>
    <xsl:param name="gs_suppress_xml_id"/>
    <xsl:param name="gs_hardlink_output"/>
    <xsl:param name="gs_database_query_output"/> <!-- aka softlinks -->
    <xsl:param name="gs_interface_render_output"/>
    <xsl:param name="gs_is_hardlink" select="database:is-hard-link()"/>
    <xsl:param name="gs_class_definitions" select="database:classes()"/>
    <xsl:param name="gs_base_class_definitions" select="database:base-classes-all()"/>
    <xsl:param name="gs_all_classes" select="$gs_class_definitions|$gs_base_class_definitions"/>
    <xsl:param name="gs_truncate_ajax" select="1024"/>

    <debug:server-message if="$gs_debug_interface_render" output="  gs_interface_render_output_elements"/>

    <!-- data-queries: classes can require additional data for their data elements during output
      allows each class to apply DYNAMIC data-queries for it's data element response
      called during gs_view_render_data data output for each element
      Class xsl:include interface_render.xsl is inappropriate because it must be xsl:include
        and has more difficulty using database:query
      @interface-render-output=placeholder will suppress this
    -->
    <database:without-node-mask hardlink-policy="suppress-exact-repeat">
      <xsl:variable name="gs_data_queries_include" select="$gs_all_classes/repository:data-queries/*[not(@meta:data-query-type)]"/>

      <xsl:if test="$gs_data_queries_include">
        <xsl:if test="$gs_interface_render_output = 'placeholder'">
          <debug:server-message if="$gs_debug_interface_render" output="[{count($gs_data_queries_include)}] data queries ignored for [{name()}/{@xml:id}] in interface [{$gs_interface_mode}] because in placeholder mode"/>
        </xsl:if>

        <xsl:if test="not($gs_interface_render_output = 'placeholder')">
          <debug:server-message if="$gs_debug_interface_render" output="testing [{count($gs_data_queries_include)}] data queries for [{name()}/{@xml:id}] in interface [{$gs_interface_mode}]"/>
          <xsl:apply-templates select="$gs_data_queries_include" mode="gs_interface_render">
            <xsl:with-param name="gs_outer_interface_mode" select="$gs_interface_mode"/>
            <xsl:with-param name="gs_current" select="."/>
            <xsl:with-param name="gs_class_definitions" select="$gs_class_definitions"/>
            <xsl:with-param name="gs_base_class_definitions" select="$gs_base_class_definitions"/>
            <xsl:with-param name="gs_truncate_ajax" select="$gs_truncate_ajax"/>
          </xsl:apply-templates>
        </xsl:if>
      </xsl:if>
    </database:without-node-mask>

    <!-- many transforms use gs_interface_render templates
      allow them to place extra elements as children of the data elements
      this is similar to using data-queries
    -->
    <xsl:apply-templates select="." mode="gs_interface_render_extra_child_nodes">
      <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      <xsl:with-param name="gs_class_definitions" select="$gs_class_definitions"/>
      <xsl:with-param name="gs_base_class_definitions" select="$gs_base_class_definitions"/>
      <xsl:with-param name="gs_truncate_ajax" select="$gs_truncate_ajax"/>
    </xsl:apply-templates>

    <xsl:if test="$gs_hardlink_output = 'placeholder'">
      <database:deviation-info/>
    </xsl:if>
  </xsl:template>
  
  <!-- normal void extensions -->
  <xsl:template match="*" mode="gs_interface_render_extra_attributes"/>
  <xsl:template match="*" mode="gs_interface_render_extra_child_nodes"/>

  <!-- normal suppression -->
  <xsl:template match="@repository:*" mode="gs_interface_render"/>
  <!-- xsl:template match="@gs:interface-mode" mode="gs_interface_render"/ -->
  <xsl:template match="@object:cpp-component" mode="gs_interface_render"/>
  <xsl:template match="@xml:add-xml-id" mode="gs_interface_render"/>

  <xsl:template match="*" mode="gs_interface_render_create_meta_context_attributes">
    <xsl:variable name="gs_is_transient" select="database:is-transient()"/>
  
    <xsl:if test="response:can-have-attribute('meta:child-count')">
      <xsl:attribute name="meta:child-count">
        <database:without-node-mask>
          <xsl:value-of select="count(*)"/>
        </database:without-node-mask>
      </xsl:attribute>
    </xsl:if>
    <!-- xsl:if test="response:can-have-attribute('meta:position')"><xsl:attribute name="meta:position"><xsl:value-of select="count(preceding-sibling::*)"/></xsl:attribute></xsl:if -->
    <xsl:if test="response:can-have-attribute('meta:fully-qualified-prefix')"><xsl:attribute name="meta:fully-qualified-prefix"><xsl:value-of select="substring-before(database:fully-qualified-name(.), ':')"/></xsl:attribute></xsl:if>
    <xsl:if test="database:has-descendant-deviants() and response:can-have-attribute('meta:has-descendant-deviants')"><xsl:attribute name="meta:has-descendant-deviants">true</xsl:attribute></xsl:if>
    <xsl:if test="database:is-registered()  and response:can-have-attribute('meta:is-registered')"><xsl:attribute name="meta:is-registered">true</xsl:attribute></xsl:if>
    <xsl:if test="database:is-soft-link()   and response:can-have-attribute('meta:is-soft-link')"><xsl:attribute name="meta:is-soft-link">true</xsl:attribute></xsl:if>
    <xsl:if test="database:is-hard-linked() and response:can-have-attribute('meta:is-hard-linked')"><xsl:attribute name="meta:is-hard-linked">true</xsl:attribute></xsl:if>
    <xsl:if test="database:is-hard-link()   and response:can-have-attribute('meta:is-hard-link')"><xsl:attribute name="meta:is-hard-link">true</xsl:attribute></xsl:if>
    <xsl:if test="database:is-deviant()     and response:can-have-attribute('meta:is-deviant')"><xsl:attribute name="meta:is-deviant">true</xsl:attribute></xsl:if>
    <xsl:if test="database:has-deviants()   and response:can-have-attribute('meta:has-deviants')"><xsl:attribute name="meta:has-deviants">true</xsl:attribute></xsl:if>
    <xsl:if test="$gs_is_transient          and response:can-have-attribute('meta:is-transient')"><xsl:attribute name="meta:is-transient">true</xsl:attribute></xsl:if>

    <xsl:if test="$gs_is_transient">
      <xsl:if test="$gs_stage_dev and response:can-have-attribute('meta:no-xpath-to-node')">
        <xsl:attribute name="meta:no-xpath-to-node">(because transient)</xsl:attribute>
      </xsl:if>
    </xsl:if>
    <xsl:else>
      <!-- the parent_route aware name enabled absolute database xpath to the node 
        we do not present this if the node is transient because it may well be deleted
        e.g. the meta:environment/object:Conversation
        
        conversation:xpath-to-node will switch between 
          session:xpath-to-node  e.g. !5 when there is a session available
          database:xpath-to-node e.g. /object:Server/...
        but also use database:xpath-to-node when it is the document node so that document listeners don't need session
        or a ~Class node
        
        in some environments, e.g. Test, no conversation is available
      -->
      <xsl:if test="response:can-have-attribute('meta:xpath-to-node')">
        <xsl:attribute name="meta:xpath-to-node">
          <xsl:variable name="gs_session_xpath_available" value="function-available('conversation:xpath-to-node')"/>
          <xsl:variable name="gs_suppress_session_xpath" value="@conversation:identity = 'database:xpath-to-node'"/>
          <xsl:variable name="gs_high_level_node" value="database:has-same-node(., /|/object:Server|/object:Server/*)"/>
          <xsl:variable name="gs_is_class_namespace" value="namespace-uri() = 'http://general_server.org/xmlnamespaces/class/2006'"/>
          <xsl:if test="not($gs_session_xpath_available) or $gs_suppress_session_xpath or $gs_high_level_node or $gs_is_class_namespace">
            <database:xpath-to-node/>
          </xsl:if>
          <xsl:else>
            <conversation:xpath-to-node/>
          </xsl:else>
        </xsl:attribute>
      </xsl:if>
    </xsl:else>
    
    <xsl:if test="$gs_stage_dev">
      <xsl:if test="response:can-have-attribute('meta:parent-route')">
        <xsl:attribute name="meta:parent-route">
          <database:without-node-mask>
            <xsl:apply-templates select="parent-route::*" mode="gs_debug_parent_route"/>
          </database:without-node-mask>
        </xsl:attribute>
      </xsl:if>
    </xsl:if>
  </xsl:template>

  <xsl:template match="*" mode="gs_debug_parent_route">
    <xsl:if test="not(position() = 1)"><xsl:text> =&gt; </xsl:text></xsl:if>
    <xsl:value-of select="name()"/>
    <xsl:if test="database:has-descendant-deviants()">*</xsl:if>
  </xsl:template>
  
  <xsl:template match="*" mode="gs_interface_render_child_nodes">
    <!-- self to child::node() traversal. inludes: text() -->
    <xsl:param name="gs_interface_mode"/>
    <xsl:param name="gs_create_meta_context_attributes"/>
    <xsl:param name="gs_suppress_xml_id"/>
    <xsl:param name="gs_hardlink_output"/>
    <xsl:param name="gs_database_query_output"/>
    <xsl:param name="gs_interface_render_output"/>
    <xsl:param name="gs_truncate_ajax" select="1024"/>

    <debug:server-message if="$gs_debug_interface_render" output="  gs_interface_render_child_nodes ({count(child::node())})"/>

    <xsl:apply-templates select="child::node()" mode="gs_interface_render">
      <xsl:with-param name="gs_create_meta_context_attributes" select="$gs_create_meta_context_attributes"/>
      <xsl:with-param name="gs_suppress_xml_id" select="$gs_suppress_xml_id"/>
      <xsl:with-param name="gs_hardlink_output" select="$gs_hardlink_output"/>
      <xsl:with-param name="gs_database_query_output" select="$gs_database_query_output"/>
      <xsl:with-param name="gs_interface_render_output" select="$gs_interface_render_output"/>
      <xsl:with-param name="gs_truncate_ajax" select="$gs_truncate_ajax"/>
    </xsl:apply-templates>
  </xsl:template>

  <!-- #################################### attribute re-serialisation for text output #################################### -->
  <xsl:template match="@*" mode="gs_serialised_value">
    <xsl:value-of select="str:dynamic(.)"/>
  </xsl:template>
  
  <xsl:template match="@*[starts-with(local-name(), 'xpath-') or starts-with(local-name(), 'class')]" mode="gs_serialised_value">
    <xsl:apply-templates select="." mode="gs_dynamic_xpath_to_absolute"/>
  </xsl:template>
  
  <xsl:template match="*|@*" mode="gs_dynamic_xpath_to_absolute">
    <!-- this attribute contains xpath to node(s)
      the current path may use context $gs_variables
        e.g. $gs_query_string/...
      so we find it, and then convert it to absolute xpath
    -->
    <xsl:variable name="gs_dynamic_value" select="str:dynamic(.)"/>
    
    <xsl:if test="contains($gs_dynamic_value, '$')">
      <xsl:variable name="gs_node" select="dyn:evaluate($gs_dynamic_value)"/>

      <xsl:if test="count($gs_node) &gt; 1"><debug:server-message output="multiple nodes in xsd:add field @{name()}=[{.}]" type="warning"/></xsl:if>
      <xsl:if test="not($gs_node)"><debug:server-message output="node not found in xsd:add field @{name()}=[{.}]" type="warning"/></xsl:if>

      <xsl:variable name="gs_xpath" select="conversation:xpath-to-node($gs_node)"/>
      <xsl:value-of select="$gs_xpath"/>
    </xsl:if>
    <xsl:else>
      <xsl:value-of select="$gs_dynamic_value"/>
    </xsl:else>
  </xsl:template>  
</xsl:stylesheet>
