<xsl:stylesheet xmlns:date="http://exslt.org/dates-and-times" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:flow="http://exslt.org/flow" xmlns:request="http://general_server.org/xmlnamespaces/request/2006" xmlns:conversation="http://general_server.org/xmlnamespaces/conversation/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:exsl="http://exslt.org/common" xmlns:class="http://general_server.org/xmlnamespaces/class/2006" xmlns:user="http://general_server.org/xmlnamespaces/user/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" response:server-side-only="true" name="interface_render" version="1.0" extension-element-prefixes="dyn str regexp database server conversation request debug exsl">
  <xsl:template match="database:validity-check" mode="gs_interface_render">
    <database:validity-check/>
  </xsl:template>

  <xsl:template match="interface:AJAXHTMLLoader" mode="gs_interface_render_extra_attributes">
    <!-- this will NOT get called if $gs_interface_render_output = 'placeholder' -->
    <xsl:attribute name="meta:xpath-to-node-client"><xsl:value-of select="conversation:xpath-to-node(., $gs_website_root)"/></xsl:attribute>
  </xsl:template>
  
  <xsl:template match="interface:AJAXHTMLLoader" mode="gs_interface_render">
    <xsl:param name="gs_database_query_output"/>
    <xsl:param name="gs_interface_render_output"/>
    <xsl:param name="gs_data_context" select="str:dynamic($gs_query_string/@data-context)"/>
    <xsl:param name="gs_data_context_id" select="str:dynamic($gs_query_string/@data-context-id)"/>
    <xsl:param name="gs_outer_interface_mode"/> <!-- for @interface-mode-condition -->
    <xsl:param name="gs_current" select="."/>
    <xsl:param name="gs_debug" select="str:boolean(str:dynamic(@debug)) or $gs_debug_interface_render"/>

    <xsl:variable name="gs_demand_mode" select="$gs_database_query_output = 'demand'"/>
    <xsl:variable name="gs_interface_mode" select="str:dynamic(@interface-mode)"/>
    <xsl:variable name="gs_query_identifier">
      <xsl:if test="@source"><xsl:value-of select="@source"/></xsl:if>
      <xsl:else><xsl:value-of select="database:xpath-to-node()"/></xsl:else>
    </xsl:variable>

    <debug:server-message if="$gs_debug" output="--------------------------------------------------------------- [{$gs_interface_mode}]"/>
    <debug:server-message if="$gs_debug" output="database:query [{$gs_query_identifier}] interface:AJAXHTMLLoader ({$gs_interface_render_output}/{$gs_database_query_output})"/>
    <xsl:if test="$gs_demand_mode">
      <!-- deliver the results of the query, not the query placeholder -->
      <xsl:if test="not($gs_database_query_output = 'ignore' or $gs_interface_render_output = 'ignore')">
        <xsl:if test="$gs_database_query_output = 'placeholder' or $gs_interface_render_output = 'placeholder'">
          <!-- placeholder means cancel the gs_interface_render override and use the normal output -->
          <debug:server-message if="$gs_debug" output="database:query [{$gs_query_identifier}] placeholder output (gs_interface_render_normal)"/>
          <xsl:apply-templates select="." mode="gs_interface_render_normal"/>
        </xsl:if>
        
        <xsl:else>
          <xsl:if test="$gs_demand_mode and ($gs_data_context or $gs_data_context_id)">
            <!-- demand with @data parameter -->
            <debug:server-message if="$gs_debug" output="database:query [{$gs_query_identifier}] demand @data-context parameter override = [{$gs_query_string/@data-context}/{$gs_query_string/@data-context-id}]"/>
            <xsl:apply-templates select="." mode="gs_interface_render_database_query">
              <!-- leave $gs_*_output to default to @values 
                $gs_database_query_output and $gs_interface_render_output sent from above can force this placeholder
                however, @database-query-output on THIS database:query controls only the sub-results of this query
              -->
              <xsl:with-param name="gs_data_context" select="$gs_data_context"/>
              <xsl:with-param name="gs_data_context_id" select="$gs_data_context_id"/>
              <xsl:with-param name="gs_current" select="$gs_current"/>
              <xsl:with-param name="gs_outer_interface_mode" select="$gs_outer_interface_mode"/>
              <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
            </xsl:apply-templates>
          </xsl:if>

          <xsl:else>
            <!-- normal database:query @attributes processing -->
            <debug:server-message if="$gs_debug and @data" output="database:query [{$gs_query_identifier}] @is-client-ondemand without $gs_data_context"/>
            <xsl:if test="not(@data)"><debug:server-message output="database:query [{$gs_query_identifier}] @is-client-ondemand $gs_data_context and @data not present" type="warning"/></xsl:if>
            
            <xsl:apply-templates select="." mode="gs_interface_render_database_query">
              <!-- leave $gs_*_output to default to @values 
                $gs_database_query_output and $gs_interface_render_output sent from above can force this placeholder
                however, @database-query-output on THIS database:query controls only the sub-results of this query
              -->
              <xsl:with-param name="gs_current" select="$gs_current"/>
              <xsl:with-param name="gs_outer_interface_mode" select="$gs_outer_interface_mode"/>
              <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
            </xsl:apply-templates>
          </xsl:else>
        </xsl:else>
      </xsl:if>
    </xsl:if>
    
    <xsl:else>
      <!-- AJAXHTMLLoader: send the interface:AJAXHTMLLoader placeholder in the client data! -->
      <debug:server-message if="$gs_debug" output="database:query [{$gs_query_identifier}] @is-client-ondemand direct output to client"/>
      <xsl:apply-templates select="." mode="gs_interface_render_normal">
        <xsl:with-param name="gs_database_query_output" select="$gs_database_query_output"/>
        <xsl:with-param name="gs_interface_render_output" select="$gs_interface_render_output"/>
        <xsl:with-param name="gs_current" select="$gs_current"/>
        <xsl:with-param name="gs_outer_interface_mode" select="$gs_outer_interface_mode"/>
      </xsl:apply-templates>
    </xsl:else>
  </xsl:template>

  <xsl:template match="database:query|database:wait-for-single-write-event" mode="gs_interface_render">
    <!-- provide generic support for any other interface_renderer to include in-built database:query @clauses
      TODO: avoid processing the xsl:stylesheet data directives when actually trying to output the stylesheet details
      $gs_database_query_output and $gs_interface_render_output sent from above can force this placeholder
      however, @database-query-output on THIS database:query controls only the sub-results of this query
    -->
    <xsl:param name="gs_database_query_output"/>
    <xsl:param name="gs_interface_render_output"/>
    <xsl:param name="gs_outer_interface_mode"/> <!-- for @interface-condition -->
    <xsl:param name="gs_interface_mode"/>      <!-- only for placeholder mode -->
    <xsl:param name="gs_current" select="."/>
    <xsl:param name="gs_debug" select="str:boolean(str:dynamic(@debug)) or $gs_debug_interface_render"/>

    <xsl:variable name="gs_query_identifier">
      <xsl:if test="@source"><xsl:value-of select="@source"/></xsl:if>
      <xsl:else><xsl:value-of select="database:xpath-to-node()"/></xsl:else>
    </xsl:variable>

    <xsl:if test="$gs_database_query_output = 'ignore' or $gs_interface_render_output = 'ignore'">
      <xsl:comment>database:query ignored</xsl:comment>
    </xsl:if>
    <xsl:else>
      <xsl:if test="$gs_database_query_output = 'placeholder' or $gs_interface_render_output = 'placeholder'">
        <!-- placeholder means cancel the gs_interface_render override and use the normal output -->
        <debug:server-message if="$gs_debug" output="database:query [{$gs_query_identifier}] placeholder output (gs_interface_render_normal)"/>
        <xsl:apply-templates select="." mode="gs_interface_render_normal"/>
      </xsl:if>
      <xsl:else>
        <xsl:apply-templates select="." mode="gs_interface_render_database_query">
          <!-- leave $gs_*_output to default to @values 
            $gs_database_query_output and $gs_interface_render_output sent from above can force this placeholder
            however, @database-query-output on THIS database:query controls only the sub-results of this query
          -->
          <xsl:with-param name="gs_current" select="$gs_current"/>
          <xsl:with-param name="gs_outer_interface_mode" select="$gs_outer_interface_mode"/>
        </xsl:apply-templates>
      </xsl:else>
    </xsl:else>
  </xsl:template>

  <xsl:template match="*" mode="gs_interface_render_database_query">
    <!-- provide generic support for any other interface_renderer to include in-built database:query @clauses
      xsl:apply-templates @select=. @mode=gs_interface_render_database_query
    -->
    <!-- database:query specific dynamic string inputs -->
    <xsl:param name="gs_data_container_name" select="str:dynamic(@data-container-name)"/>
    <xsl:param name="gs_data_context" select="str:dynamic(@data-context)"/>
    <xsl:param name="gs_data_context_id" select="str:dynamic(@data-context-id)"/>
    <xsl:param name="gs_data" select="str:dynamic(@data)"/>             <!-- REQUIRED: the data!  -->
    <xsl:param name="gs_data_id" select="str:dynamic(@data-id)"/>             <!-- REQUIRED: the data!  -->
    <xsl:param name="gs_child_doc" select="*"/>                         <!-- OR REQUIRED: the data!  -->
    <xsl:param name="gs_current" select="."/>
    <xsl:param name="gs_evaluate" select="str:dynamic(@evaluate)"/>
    <xsl:param name="gs_condition" select="str:dynamic(@condition)"/>
    <xsl:param name="gs_outer_interface_mode"/>                          <!-- for interface-mode-condition -->
    <xsl:param name="gs_outer_interface_mode_condition" select="str:dynamic(@interface-mode-condition)"/>
    <xsl:param name="gs_database" select="str:dynamic(@database)"/>
    <xsl:param name="gs_node_mask" select="str:dynamic(@node-mask)"/>
    <xsl:param name="gs_node_mask_type" select="str:dynamic(@node-mask-type)"/>
    <xsl:param name="gs_hardlink_policy" select="str:dynamic(@hardlink-policy)"/>
    <xsl:param name="gs_hardlink_output" select="str:dynamic(@hardlink-output)"/>
    <xsl:param name="gs_optional" select="str:dynamic(@optional)"/>
    <xsl:param name="gs_interface_mode" select="str:dynamic(@interface-mode)"/>
    <xsl:param name="gs_trace" select="str:dynamic(@trace)"/>
    <xsl:param name="gs_debug" select="str:boolean(str:dynamic(@debug)) or $gs_debug_interface_render"/>
    <xsl:param name="gs_create_meta_context_attributes" select="str:dynamic(@create-meta-context-attributes)"/>
    <xsl:param name="gs_suppress_xml_id" select="str:dynamic(@suppress-xmlid)"/>
    <xsl:param name="gs_copy_query_attributes" select="str:dynamic(@copy-query-attributes)"/>

    <!-- values sent from above should have forced this to display as placeholder 
      we now use the @values on this database:query to inform the results
    -->
    <xsl:variable name="gs_database_query_output" select="str:dynamic(@database-query-output)"/>
    <xsl:variable name="gs_interface_render_output" select="str:dynamic(@interface-render-output)"/>
    <xsl:variable name="gs_query_identifier">
      <xsl:if test="@source"><xsl:value-of select="@source"/></xsl:if>
      <xsl:else><xsl:value-of select="database:xpath-to-node()"/></xsl:else>
    </xsl:variable>
    
    <!-- debug:server-message output="processing sub-[{local-name()}]: [{@data-container-name}{@class-name}/{@xml:id}] =&gt; [{@data}]"/ -->
    <!-- database:query checks -->
    <debug:server-message if="$gs_debug" output="database:query [{$gs_query_identifier}] = [{$gs_data}] ({$gs_interface_render_output}/{$gs_database_query_output})"/>
    <xsl:if test="not(@data) and not($gs_child_doc)"><debug:server-message output="database:query [{$gs_query_identifier}] has no data attribute" type="warning"/></xsl:if>
    <xsl:if test="$gs_node_mask = 'false'"><debug:server-message output="database:query [{$gs_query_identifier}] [node-mask = false] did you mean false()" type="warning"/></xsl:if>
    <!-- xsl:if test="@data-container-name and $gs_data_container_name = ''"><debug:server-message output="database:query [{$gs_query_identifier}] name attribute [{@data-container-name}] evaluates to empty string"/></xsl:if -->

    <!-- other dnamics might use this conditionally so allow a no @data situation -->
    <xsl:if test="$gs_data = ''">
      <debug:server-message if="$gs_debug" output="database:query [{$gs_query_identifier}] no data"/>
    </xsl:if>
    <xsl:else>
      <!-- conditional returns for the data. useful in Classes that have varied data requirements. -->
      <debug:server-message if="$gs_debug and $gs_condition" output="database:query [{$gs_query_identifier}] checking @condition [{$gs_condition}]"/>
      <xsl:if test="not($gs_condition) or dyn:evaluate($gs_condition)">
        <debug:server-message if="$gs_debug and $gs_outer_interface_mode_condition" output="database:query [{$gs_query_identifier}] checking @interface-condition [{$gs_outer_interface_mode_condition}] with [{$gs_outer_interface_mode}]"/>
        <xsl:if test="not($gs_outer_interface_mode_condition) or $gs_outer_interface_mode = $gs_outer_interface_mode_condition">
          <!-- TODO: should not this be $gs_soflink_output -->
          <xsl:if test="$gs_data_container_name = ''">
            <xsl:apply-templates select="." mode="gs_interface_render_database_query_data">
              <xsl:with-param name="gs_data_container_name" select="$gs_data_container_name"/>
              <xsl:with-param name="gs_data_context" select="$gs_data_context"/>
              <xsl:with-param name="gs_data_context_id" select="$gs_data_context_id"/>
              <xsl:with-param name="gs_data" select="$gs_data"/>             <!-- REQUIRED: the data!  -->
              <xsl:with-param name="gs_data_id" select="$gs_data_id"/>             <!-- REQUIRED: the data!  -->
              <xsl:with-param name="gs_child_doc" select="$gs_child_doc"/>                         <!-- OR REQUIRED: the data!  -->
              <xsl:with-param name="gs_current" select="$gs_current"/>
              <xsl:with-param name="gs_evaluate" select="$gs_evaluate"/>
              <xsl:with-param name="gs_condition" select="$gs_condition"/>
              <xsl:with-param name="gs_outer_interface_mode_condition" select="$gs_outer_interface_mode_condition"/>
              <xsl:with-param name="gs_database" select="$gs_database"/>
              <xsl:with-param name="gs_node_mask" select="$gs_node_mask"/>
              <xsl:with-param name="gs_node_mask_type" select="$gs_node_mask_type"/>
              <xsl:with-param name="gs_hardlink_policy" select="$gs_hardlink_policy"/>
              <xsl:with-param name="gs_hardlink_output" select="$gs_hardlink_output"/>
              <xsl:with-param name="gs_database_query_output" select="$gs_database_query_output"/>
              <xsl:with-param name="gs_interface_render_output" select="$gs_interface_render_output"/>
              <xsl:with-param name="gs_optional" select="$gs_optional"/>
              <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
              <xsl:with-param name="gs_trace" select="$gs_trace"/>
              <xsl:with-param name="gs_debug" select="$gs_debug"/>
              <xsl:with-param name="gs_create_meta_context_attributes" select="$gs_create_meta_context_attributes"/>
              <xsl:with-param name="gs_suppress_xml_id" select="$gs_suppress_xml_id"/>
              <xsl:with-param name="gs_copy_query_attributes" select="$gs_copy_query_attributes"/>
            </xsl:apply-templates>
          </xsl:if>
          <xsl:else>
            <!-- @data-container-name=[namespace:]<name>
              e.g. xsd:app-info by default the Repository Class will handle the name with @html-container=ul
            -->
            <xsl:variable name="gs_data_container_name_spaced_fragment">
              <xsl:if test="not(contains($gs_data_container_name, ':'))">repository:</xsl:if>
              <xsl:value-of select="$gs_data_container_name"/>
            </xsl:variable>
            <xsl:element name="{$gs_data_container_name_spaced_fragment}">
              <xsl:apply-templates select="." mode="gs_interface_render_database_query_data">
                <xsl:with-param name="gs_data_container_name" select="$gs_data_container_name"/>
                <xsl:with-param name="gs_data_context" select="$gs_data_context"/>
                <xsl:with-param name="gs_data_context_id" select="$gs_data_context_id"/>
                <xsl:with-param name="gs_data" select="$gs_data"/>             <!-- REQUIRED: the data!  -->
                <xsl:with-param name="gs_data_id" select="$gs_data_id"/>             <!-- REQUIRED: the data!  -->
                <xsl:with-param name="gs_child_doc" select="$gs_child_doc"/>                         <!-- OR REQUIRED: the data!  -->
                <xsl:with-param name="gs_current" select="$gs_current"/>
                <xsl:with-param name="gs_evaluate" select="$gs_evaluate"/>
                <xsl:with-param name="gs_condition" select="$gs_condition"/>
                <xsl:with-param name="gs_outer_interface_mode_condition" select="$gs_outer_interface_mode_condition"/>
                <xsl:with-param name="gs_database" select="$gs_database"/>
                <xsl:with-param name="gs_node_mask" select="$gs_node_mask"/>
                <xsl:with-param name="gs_node_mask_type" select="$gs_node_mask_type"/>
                <xsl:with-param name="gs_hardlink_policy" select="$gs_hardlink_policy"/>
                <xsl:with-param name="gs_hardlink_output" select="$gs_hardlink_output"/>
                <xsl:with-param name="gs_database_query_output" select="$gs_database_query_output"/>
                <xsl:with-param name="gs_interface_render_output" select="$gs_interface_render_output"/>
                <xsl:with-param name="gs_optional" select="$gs_optional"/>
                <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
                <xsl:with-param name="gs_trace" select="$gs_trace"/>
                <xsl:with-param name="gs_debug" select="$gs_debug"/>
                <xsl:with-param name="gs_create_meta_context_attributes" select="$gs_create_meta_context_attributes"/>
                <xsl:with-param name="gs_suppress_xml_id" select="$gs_suppress_xml_id"/>
                <xsl:with-param name="gs_copy_query_attributes" select="$gs_copy_query_attributes"/>
              </xsl:apply-templates>
            </xsl:element>
          </xsl:else>
        </xsl:if>
      </xsl:if>
    </xsl:else>
  </xsl:template>

  <xsl:template match="*" mode="gs_interface_render_database_query_xpath">
    <!-- abstracted in to a template so that other users can use the same xpath evaluation logic
      this tmeplate outputs a text fragement
      that should be converted to a string([fragment]) before use
    -->
    <xsl:param name="gs_data" select="str:dynamic(@data)"/>             <!-- REQUIRED: the data!  -->
    <xsl:param name="gs_data_id" select="str:dynamic(@data-id)"/>             <!-- REQUIRED: the data!  -->
    <xsl:param name="gs_evaluate" select="str:dynamic(@evaluate)"/>
    <xsl:param name="gs_debug" select="str:boolean(str:dynamic(@debug)) or $gs_debug_interface_render"/>

    <xsl:if test="not($gs_data_id = '')">id('<xsl:value-of select="$gs_data_id"/>')</xsl:if>
    <xsl:if test="$gs_data_id = ''">
      <xsl:if test="$gs_data = ''">
        <debug:server-message if="$gs_debug" output="database:query [{$gs_query_identifier}] no xpath @data or @data-id"/>
      </xsl:if>
      <xsl:if test="not($gs_data = '')">
        <!-- need to make query relative with a current . whilst allowing descendant requests -->
        <xsl:variable name="gs_relative_query">
          <xsl:choose>
            <xsl:when test="$gs_data = '/'">.</xsl:when>
            <xsl:when test="starts-with($gs_data, '/')">.<xsl:value-of select="$gs_data"/></xsl:when>
            <xsl:otherwise><xsl:value-of select="$gs_data"/></xsl:otherwise>
          </xsl:choose>
        </xsl:variable>

        <!-- dyn:evaluate($query) ? -->
        <xsl:variable name="gs_final_query">
          <xsl:if test="str:boolean($gs_evaluate)"><xsl:value-of select="dyn:evaluate($gs_relative_query)"/></xsl:if>
          <xsl:if test="str:not($gs_evaluate)"><xsl:value-of select="$gs_relative_query"/></xsl:if>
        </xsl:variable>

        <debug:server-message if="$gs_debug and str:boolean($gs_evaluate)" output="database:query [{$gs_query_identifier}] evaluated query [{$gs_final_query}]"/>
        <xsl:value-of select="$gs_final_query"/>
      </xsl:if>
    </xsl:if>
  </xsl:template>

  <xsl:template match="*" mode="gs_interface_render_database_query_data">
    <!-- database:query specific dynamic string inputs -->
    <xsl:param name="gs_data_container_name" select="str:dynamic(@data-container-name)"/>
    <xsl:param name="gs_data_context" select="str:dynamic(@data-context)"/>
    <xsl:param name="gs_data_context_id" select="str:dynamic(@data-context-id)"/>
    <xsl:param name="gs_data" select="str:dynamic(@data)"/>             <!-- REQUIRED: the data!  -->
    <xsl:param name="gs_data_id" select="str:dynamic(@data-id)"/>             <!-- REQUIRED: the data!  -->
    <xsl:param name="gs_child_doc" select="*"/>                         <!-- OR REQUIRED: the data!  -->
    <xsl:param name="gs_current" select="."/>
    <xsl:param name="gs_evaluate" select="str:dynamic(@evaluate)"/>
    <xsl:param name="gs_condition" select="str:dynamic(@condition)"/>
    <xsl:param name="gs_outer_interface_mode_condition" select="str:dynamic(@interface-mode-condition)"/>
    <xsl:param name="gs_database" select="str:dynamic(@database)"/>
    <xsl:param name="gs_node_mask" select="str:dynamic(@node-mask)"/>
    <xsl:param name="gs_node_mask_type" select="str:dynamic(@node-mask-type)"/>
    <xsl:param name="gs_hardlink_policy" select="str:dynamic(@hardlink-policy)"/>
    <xsl:param name="gs_hardlink_output" select="str:dynamic(@hardlink-output)"/>
    <xsl:param name="gs_database_query_output" select="str:dynamic(@database-query-output)"/>
    <xsl:param name="gs_interface_render_output" select="str:dynamic(@interface-render-output)"/>
    <xsl:param name="gs_optional" select="str:dynamic(@optional)"/>
    <xsl:param name="gs_interface_mode" select="str:dynamic(@interface-mode)"/>
    <xsl:param name="gs_trace" select="str:dynamic(@trace)"/>
    <xsl:param name="gs_debug" select="str:boolean(str:dynamic(@debug)) or $gs_debug_interface_render"/>
    <xsl:param name="gs_create_meta_context_attributes" select="str:dynamic(@create-meta-context-attributes)"/>
    <xsl:param name="gs_suppress_xml_id" select="str:dynamic(@suppress-xmlid)"/>
    <xsl:param name="gs_copy_query_attributes" select="str:dynamic(@copy-query-attributes)"/>

    <xsl:variable name="gs_has_node_mask" select="$gs_node_mask and not($gs_node_mask = 'false()')"/>
    <xsl:variable name="gs_query_identifier">
      <xsl:if test="@source"><xsl:value-of select="@source"/></xsl:if>
      <xsl:else><xsl:value-of select="database:xpath-to-node()"/></xsl:else>
    </xsl:variable>
    
    <!-- resolve the xpath to use for the query -->
    <xsl:variable name="gs_xpath_fragment">
      <xsl:apply-templates select="." mode="gs_interface_render_database_query_xpath">
        <xsl:with-param name="gs_data" select="$gs_data"/>
        <xsl:with-param name="gs_data_id" select="$gs_data_id"/>
        <xsl:with-param name="gs_evaluate" select="$gs_evaluate"/>
        <xsl:with-param name="gs_debug" select="$gs_debug"/>
      </xsl:apply-templates>
    </xsl:variable>
    <xsl:variable name="gs_xpath" select="string($gs_xpath_fragment)"/>

    <!-- run select query!
      allowing manual database switching
      default to maybe empty child_doc if no data_select result
      i.e. if there are no results then return the child_doc which could be a warning message
    -->
    <xsl:variable name="gs_query_database" select="/"/>
    <xsl:variable name="gs_query_context" select="flow:first-set-not-empty(id($gs_data_context_id), dyn:map($gs_query_database, $gs_data_context), $gs_query_database)"/>
    <xsl:variable name="gs_data_select_data" select="dyn:map($gs_query_context, $gs_xpath)"/>
    <!-- NOTE: that both $gs_data_select_data $gs_child_doc can return multiple nodes -->
    <xsl:variable name="gs_data_select" select="$gs_data_select_data | $gs_child_doc[not($gs_data_select_data)]"/>
    
    <!-- checks -->
    <debug:server-message if="$gs_debug" output="database:query [{$gs_query_identifier}] returned [{count($gs_data_select)}/{count($gs_data_select_data)}/{count($gs_query_context)}] nodes, first is [{name($gs_data_select)}]"/>
    <xsl:if test="$gs_xpath and not($gs_data_select)">
      <xsl:if test="str:not($gs_optional)">
        <xsl:choose>
          <xsl:when test="$gs_query_context = /">
            <debug:server-message output="database:query [{$gs_query_identifier}] requested source node [{$gs_xpath}] in / not found" type="warning"/>
          </xsl:when>
          <xsl:when test="$gs_query_context">
            <debug:server-message output="database:query [{$gs_query_identifier}] requested source node [{$gs_xpath}] in query-context [{local-name($gs_query_context)} {$gs_query_context/@name} {$gs_query_context/@xml:id}] not found" type="warning"/>
          </xsl:when>
          <xsl:otherwise>
            <debug:server-message output="database:query [{$gs_query_identifier}] in query context [{$gs_data_context_id} {$gs_data_context}] empty" type="warning"/>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:if>
    </xsl:if>
    <!-- xsl:if test="$gs_stage_dev and not(self::database:wait-for-single-write-event) and not($gs_has_node_mask) and not($gs_data_select/..)">
      <debug:server-message output="database:query [{$gs_query_identifier}] is for the whole DOCUMENT without @node-mask" type="warning"/>
      <request:throw class="DataSizeAbort"/>
    </xsl:if -->

    <!-- default node-mask
      @node-mask="false()" will prevent the node-mask completely: use cautiously!
      if we have no @data, we are using a child-doc, then dont restrict it here: no node-mask
    -->
    <xsl:variable name="gs_node_mask_default">
      <xsl:choose>
        <xsl:when test="$gs_has_node_mask"><xsl:value-of select="$gs_node_mask"/></xsl:when>
        <xsl:when test="not($gs_node_mask) and not($gs_data) and $gs_interface_mode">database:class-xschema-node-mask(.,'<xsl:value-of select="$gs_interface_mode"/>')</xsl:when>
        <xsl:when test="not($gs_node_mask) and not($gs_data) and not($gs_interface_mode)">database:class-xschema-node-mask()</xsl:when>
      </xsl:choose>
    </xsl:variable>

    <!-- run data select -->
    <xsl:if test="$gs_data_select">
      <!-- the normal AJAX request will only report the interface:AJAXHTMLLoader request -->
      <xsl:variable name="gs_data_select_count"><xsl:if test="count($gs_data_select) &gt; 1">[<xsl:value-of select="count($gs_data_select)"/>]</xsl:if></xsl:variable>
      <debug:server-message indent="1" prefix="{local-name()}" output="[{$gs_query_identifier}] =&gt; {$gs_data_select_count}"/>
      
      <xsl:if test="$gs_interface_mode">
        <!-- ####################################### custom $gs_interface_mode #######################################################
          data output with custom interface-mode, map from current database:query to xsl:stylesheet level
        -->
        <debug:server-message if="$gs_debug" output="database:query [{$gs_query_identifier}] transform with [$gs_stylesheet_server_side_classes] ({$gs_interface_mode})] [{$gs_interface_render_output}/{$gs_database_query_output}] node-mask: [{$gs_node_mask_default}]"/>
        
        <xsl:if test="$gs_debug"><xsl:comment>transform [<xsl:value-of select="$gs_interface_mode"/>]</xsl:comment></xsl:if>
        <xsl:if test="self::database:wait-for-single-write-event">
          <conversation:clear-time-limit/>
          <database:wait-for-single-write-event select="$gs_data_select" transform="$gs_stylesheet_server_side_classes" interface-mode="{$gs_interface_mode}"/>
        </xsl:if>
        <xsl:else>
          <xsl:if test="$gs_trace and not($gs_trace = 'off')">
            <debug:server-message output="database:query [{$gs_query_identifier}] begin trace {name()} in [{../@data-container-name} ({@xml:id})] of [{$gs_xpath} ({$gs_node_mask_default}) ($gs_stylesheet_server_side_classes/{$gs_interface_mode})]"/>
            <debug:xslt-set-trace flags="{$gs_trace}"/>
          </xsl:if>
          
          <xsl:if test="$gs_stylesheet_server_side_classes">
            <database:transform select="$gs_data_select" stylesheet="$gs_stylesheet_server_side_classes" interface-mode="{$gs_interface_mode}" node-mask="{$gs_node_mask_default}" node-mask-type="{$gs_node_mask_type}" hardlink-policy="{$gs_hardlink_policy}" additional-params-node="." debug="{$gs_debug}">
              <database:with-param name="gs_data_select" select="$gs_data_select"/>
              <database:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
              <database:with-param name="gs_create_meta_context_attributes" select="$gs_create_meta_context_attributes"/>
              <database:with-param name="gs_suppress_xml_id" select="$gs_suppress_xml_id"/>
              <database:with-param name="gs_hardlink_output" select="$gs_hardlink_output"/>
              <database:with-param name="gs_database_query_output" select="$gs_database_query_output"/>
              <database:with-param name="gs_interface_render_output" select="$gs_interface_render_output"/>
              <database:with-param name="gs_debug" select="$gs_debug"/>
            </database:transform>
          </xsl:if>
          <xsl:else>
            <debug:server-message output="$gs_stylesheet_server_side_classes is empty at AJAXHTMLLoader/database:transform" type="warning"/>
          </xsl:else>
        </xsl:else>
      </xsl:if>

      <xsl:else>
        <!-- ####################################### direct gs_interface_render output, no transform ##################################### -->
        <debug:server-message if="$gs_debug" output="database:query [{$gs_query_identifier}] direct output render (gs_interface_render) [{$gs_interface_render_output}/{$gs_database_query_output}] with @gs:interface-mode [{$gs_interface_mode}]"/>
        <xsl:if test="string($gs_node_mask_default)">
          <debug:server-message if="$gs_debug" output="database:query [{$gs_query_identifier}] with node-mask [{$gs_node_mask_default}]"/>
          <database:with-node-mask hardlink-policy="{$gs_hardlink_policy}" select="$gs_data_select" node-mask="{$gs_node_mask_default}" node-mask-type="{$gs_node_mask_type}">
            <xsl:if test="str:boolean($gs_debug)"><xsl:comment>no transform, with node-mask [<xsl:value-of select="count($gs_data_select)"/>] nodes</xsl:comment></xsl:if>
            <xsl:if test="not($gs_data_select)"><debug:server-message output="database:query [{$gs_query_identifier}] node-mask is preventing access to select node" type="warning"/></xsl:if>
            <xsl:if test="$gs_trace and not($gs_trace = 'off')">
              <debug:server-message output="database:query [{$gs_query_identifier}] begin trace {name()} in [{../@data-container-name} ({@xml:id})] of [{$gs_xpath} ({$gs_node_mask_default}) (gs_interface_render)]"/>
              <debug:xslt-set-trace flags="{$gs_trace}"/>
            </xsl:if>
            <xsl:apply-templates select="$gs_data_select" mode="gs_interface_render">
              <xsl:with-param name="gs_data_select" select="$gs_data_select"/>
              <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
              <xsl:with-param name="gs_create_meta_context_attributes" select="$gs_create_meta_context_attributes"/>
              <xsl:with-param name="gs_suppress_xml_id" select="$gs_suppress_xml_id"/>
              <xsl:with-param name="gs_hardlink_output" select="$gs_hardlink_output"/>
              <xsl:with-param name="gs_database_query_output" select="$gs_database_query_output"/>
              <xsl:with-param name="gs_interface_render_output" select="$gs_interface_render_output"/>
              <xsl:with-param name="gs_debug" select="$gs_debug"/>
            </xsl:apply-templates>
          </database:with-node-mask>
        </xsl:if>

        <xsl:else>
          <debug:server-message if="$gs_debug" output="database:query [{$gs_query_identifier}] without node-mask"/>
          <database:without-node-mask hardlink-policy="{$gs_hardlink_policy}">
            <xsl:if test="str:boolean($gs_debug)"><xsl:comment>no transform, no node-mask</xsl:comment></xsl:if>
            <xsl:if test="$gs_trace and not($gs_trace = 'off')">
              <debug:server-message output="database:query [{$gs_query_identifier}] begin trace {name()} in [{../@data-container-name} ({@xml:id})] of [{$gs_xpath} ({$gs_node_mask_default}) (gs_interface_render)]"/>
              <debug:xslt-set-trace flags="{$gs_trace}"/>
            </xsl:if>

            <xsl:apply-templates select="$gs_data_select" mode="gs_interface_render">
              <xsl:with-param name="gs_data_select" select="$gs_data_select"/>
              <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
              <xsl:with-param name="gs_create_meta_context_attributes" select="$gs_create_meta_context_attributes"/>
              <xsl:with-param name="gs_suppress_xml_id" select="$gs_suppress_xml_id"/>
              <xsl:with-param name="gs_hardlink_output" select="$gs_hardlink_output"/>
              <xsl:with-param name="gs_database_query_output" select="$gs_database_query_output"/>
              <xsl:with-param name="gs_interface_render_output" select="$gs_interface_render_output"/>
              <xsl:with-param name="gs_debug" select="$gs_debug"/>
            </xsl:apply-templates>
          </database:without-node-mask>
        </xsl:else>
      </xsl:else>
    </xsl:if>


    <xsl:if test="$gs_trace and not($gs_trace = 'off')">
      <debug:server-message output="database:query [{$gs_query_identifier}] end trace on {name()} [{@xml:id}] in [{../@data-container-name}]"/>
      <debug:xslt-clear-trace/>
    </xsl:if>
  </xsl:template>
</xsl:stylesheet>
