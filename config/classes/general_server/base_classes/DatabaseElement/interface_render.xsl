<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:flow="http://exslt.org/flow" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" response:server-side-only="true" name="interface_render" extension-element-prefixes="dyn database server str flow debug" version="1.0">
  <xsl:include xpath="/object:Server/repository:system_transforms[1]/interface_render"/>

  <xsl:template match="*">
    <!-- default interface render stylesheet with no mode (@interface-mode) -->
    <xsl:apply-templates select="." mode="gs_interface_render"/>
  </xsl:template>
  
  <xsl:template match="*" mode="*">
    <!-- interface render stylesheet with mode not found (@interface-mode) 
      this is a new interface option @mode=* (can only be an asterix)
      which catches all xsl:apply-templates mode=x that are not resolved
      
      Altered LibXML will throw an error if this template is not available
      because the default is to output the whole sub-tree text
    -->
    <xsl:param name="gs_database_query_output"/>
    <xsl:param name="gs_interface_render_output"/>
    <xsl:param name="gs_debug"/>
    
    <xsl:variable name="gs_mode" select="flow:current-mode-name()"/>
    
    <debug:server-message if="$gs_debug_interface_render" output="mode=* caught mode [{$gs_mode}] on [{name()}] ({$gs_database_query_output}/{$gs_interface_render_output})"/>
    <xsl:apply-templates select="." mode="gs_interface_render">
      <xsl:with-param name="gs_interface_mode" select="$gs_mode"/>
      <xsl:with-param name="gs_database_query_output" select="$gs_database_query_output"/>
      <xsl:with-param name="gs_interface_render_output" select="$gs_interface_render_output"/>
      <xsl:with-param name="gs_debug" select="$gs_debug"/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="*" mode="list">
    <!-- @mode=list is the container version for @mode=children
      ensure that a @node-mask=. to present only this item 
      a Collection and AJAXHTMLLoader will be included if there are children
    -->
    <xsl:copy>
      <xsl:apply-templates select="." mode="gs_interface_render_adorn_node">
        <xsl:with-param name="gs_interface_mode" select="'list'"/>
      </xsl:apply-templates>
      
      <xsl:apply-templates select="." mode="children">
        <xsl:with-param name="gs_child_interface_mode" select="'list'"/>
      </xsl:apply-templates>
    </xsl:copy>
  </xsl:template>
  
  <xsl:template match="*" mode="children">
    <!-- CHILDREN-interface of context node
      auto-load the children of expanded nodes: $gs_user/gs:persistent-server-states/gs:IDE-tree-expanded-objects/*/*
      include all the extended attributes using gs_interface_render as normal would do
      these NODE params have been sent through from the database:query processor
      BUT: currently we do not declare them because of C++ issues
      that NODE params are sent through as already declared global params
    -->
    <!-- xsl:param name="gs_create_meta_context_attributes"/>
    <xsl:param name="gs_interface_mode"/ -->
    <xsl:param name="gs_create_meta_context_attributes"/>
    <xsl:param name="gs_suppress_xml_id"/>
    <xsl:param name="gs_hardlink_output"/>
    <xsl:param name="gs_database_query_output"/>
    <xsl:param name="gs_interface_render_output"/>
    
    <!-- template specific parameters -->
    <xsl:param name="gs_child_interface_mode" select="'list'"/>
    <xsl:param name="gs_grouping_threshold" select="16"/> <!-- 0 indicates no grouping -->

    <xsl:variable name="gs_children_to_consider" select="*[str:not(@meta:system-node) and not(@meta:xml-output = 'indirect')]"/>
    <xsl:variable name="gs_child_count_without_node_mask_fragment">
      <database:without-node-mask><xsl:value-of select="count(*[str:not(@meta:system-node) and not(@meta:xml-output = 'indirect')])"/></database:without-node-mask>
    </xsl:variable>
    
    <xsl:variable name="gs_child_count_without_node_mask" select="number($gs_child_count_without_node_mask_fragment)"/>
    <xsl:variable name="gs_child_count_not_currently_masked" select="count($gs_children_to_consider)"/>
    <xsl:variable name="gs_some_children_masked" select="not($gs_child_count_without_node_mask = $gs_child_count_not_currently_masked)"/>
    <xsl:variable name="gs_should_group" select="$gs_grouping_threshold &gt; 0 and $gs_child_count_not_currently_masked &gt; $gs_grouping_threshold"/>
    
    <!-- ####################### data render children ####################### 
      active @node-mask=.|* may prevent the children being visible
      <interface:Collection
        @meta:child-count (actually the context nodes children)
        @meta:child-count-visible
        @meta:for (context-node xpath)
      >
        <...>
      </interface:Collection>
      [<interface:AJAXHTMLLoader ... />] if this is only a partial request for this level
    -->
    <xsl:if test="$gs_child_count_without_node_mask">
      <xsl:if test="$gs_child_count_not_currently_masked">
        <xsl:if test="$gs_should_group and false()">
          <TODO>node results grouping</TODO>
        </xsl:if>
        <xsl:else>
          <interface:Collection name="children" meta:for="{database:xpath-to-node()}" gs:display-class="children" meta:context-menu="off">
            <!-- TODO: return to node-mask including the $gs_user settings
              <database:with-node-mask hardlink-policy="suppress-exact-repeat" select="." node-mask=".|$gs_user/gs:persistent-server-states/gs:IDE-tree-expanded-objects/*/*">
            -->
            <xsl:apply-templates select="$gs_children_to_consider" mode="gs_children_nodes">
              <xsl:with-param name="gs_interface_mode" select="$gs_child_interface_mode"/>
              <xsl:with-param name="gs_create_meta_context_attributes" select="$gs_create_meta_context_attributes"/>
              <xsl:with-param name="gs_interface_render_output" select="'placeholder'"/>
              <xsl:with-param name="gs_grouping_threshold" select="$gs_grouping_threshold"/> <!-- 0 indicates no grouping -->
            </xsl:apply-templates>
          </interface:Collection>
        </xsl:else>
      </xsl:if>
      
      <!-- ####################### further requests ####################### 
        @gs:data-context="{xpath}" goes to the client
        so that the context of this GENERIC @data=. literal interface:AJAXHTMLLoader can be understood
      -->
      <xsl:if test="$gs_some_children_masked">
        <interface:AJAXHTMLLoader gs:data-context="{conversation:xpath-to-node()}" name="children" interface-mode="children-IDETreeRecord" interface-child-interface-mode="{$gs_child_interface_mode}" data="." node-mask=".|*" optional="true" interface-render-output="placeholder" meta:xpath-to-node="{conversation:xpath-to-command-node()}" meta:xpath-to-node-client="{conversation:xpath-to-command-node($gs_website_root)}"/> 
      </xsl:if>
    </xsl:if>
  </xsl:template>
  
  <xsl:template match="*" mode="gs_children_nodes">
    <xsl:param name="gs_interface_mode" select="'list'"/>
    <xsl:param name="gs_create_meta_context_attributes"/>
    <xsl:param name="gs_interface_render_output"/>
    <xsl:param name="gs_grouping_threshold" select="16"/> <!-- 0 indicates no grouping -->
    
    <xsl:copy>
      <!-- standard gs_interface_render for this xsl:copy element 
        no data children are rendered, although programmatic elements may be added to this node
      -->
      <xsl:apply-templates select="." mode="gs_interface_render_adorn_node">
        <xsl:with-param name="gs_create_meta_context_attributes" select="$gs_create_meta_context_attributes"/>
        <xsl:with-param name="gs_interface_render_output" select="$gs_interface_render_output"/>
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      </xsl:apply-templates>
      
      <!-- now render the children interface:
        for this context node 
        according to the node-mask, 
          e.g. .|* will cause an AJAXHTMLLoader to be included
          e.g. ancestors::* will cause a node path to be rendered and a partial load AJAXHTMLLoader to be included
      -->
      <xsl:apply-templates select="." mode="children">
        <xsl:with-param name="gs_create_meta_context_attributes" select="$gs_create_meta_context_attributes"/>
        <xsl:with-param name="gs_interface_render_output" select="$gs_interface_render_output"/>
        <xsl:with-param name="gs_child_interface_mode" select="$gs_interface_mode"/>
        <xsl:with-param name="gs_grouping_threshold" select="$gs_grouping_threshold"/> <!-- 0 indicates no grouping -->
      </xsl:apply-templates>
    </xsl:copy>
  </xsl:template>
</xsl:stylesheet>
