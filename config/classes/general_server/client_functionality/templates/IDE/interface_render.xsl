<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" response:server-side-only="true" name="interface_render" version="1.0" extension-element-prefixes="dyn str regexp database server conversation request debug">
  <xsl:include xpath="/object:Server/repository:system_transforms[1]/interface_render"/>
  <xsl:include xpath="~DatabaseElement/interface_render"/>

  <xsl:template match="*" mode="children-IDETreeRecord">
    <xsl:param name="gs_interface_mode"/>
    <xsl:param name="gs_create_meta_context_attributes"/>
    <xsl:param name="gs_suppress_xml_id"/>
    <xsl:param name="gs_hardlink_output"/>
    <xsl:param name="gs_database_query_output"/>
    <xsl:param name="gs_interface_render_output"/>
    <xsl:param name="gs_top" select="false()"/>
    <xsl:param name="gs_child_interface" select="'list'"/>
    
    <xsl:apply-templates select="." mode="children">
      <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      <xsl:with-param name="gs_create_meta_context_attributes" select="$gs_create_meta_context_attributes"/>
      <xsl:with-param name="gs_suppress_xml_id" select="$gs_suppress_xml_id"/>
      <xsl:with-param name="gs_hardlink_output" select="$gs_hardlink_output"/>
      <xsl:with-param name="gs_database_query_output" select="$gs_database_query_output"/>
      <xsl:with-param name="gs_interface_render_output" select="$gs_interface_render_output"/>
      <xsl:with-param name="gs_top" select="$gs_top"/>
      <xsl:with-param name="gs_child_interface" select="$gs_child_interface"/>
    </xsl:apply-templates>
  
    <!-- ####################### record choices #######################
      only hardlink the parent in ONCE! for this children list 
    -->
    <xsl:variable name="gs_child_count_actual_fragment">
      <database:without-node-mask><xsl:value-of select="count(*)"/></database:without-node-mask>
    </xsl:variable>
    <xsl:variable name="gs_child_count_actual" select="number($gs_child_count_actual_fragment)"/>

    <xsl:if test="$gs_child_count_actual">
      <xsl:if test="$gs_user and position() = 1">
        <xsl:variable name="gs_parent_node" select=".."/>
        <xsl:variable name="gs_is_top" select="database:has-same-node($gs_parent_node, /) or database:has-same-node($gs_parent_node, /object:Server)"/>
        <!-- TODO: this does not work because the parent is a hardlink and database:has-same-node does not check for that -->
        <xsl:variable name="gs_already_expanded" select="database:has-same-node($gs_user/gs:persistent-server-states/gs:IDE-tree-expanded-objects/*, $gs_parent_node/hardlink::*)"/>
        <xsl:if test="not($gs_is_top) and not($gs_already_expanded)">
          <!-- database:hardlink-child select="$gs_parent_node" destination="$gs_user/gs:persistent-server-states/gs:IDE-tree-expanded-objects" description="record expanded tree node for user"/ -->
        </xsl:if>
      </xsl:if>
    </xsl:if>
  </xsl:template>
</xsl:stylesheet>