<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:xmlsecurity="http://general_server.org/xmlnamespaces/xmlsecurity/2006" xmlns="http://www.w3.org/1999/xhtml" name="view_list" version="1.0" extension-element-prefixes="debug">
  <xsl:template match="*" mode="list" meta:interface-template="yes">
    <!-- top level element contains required indicators
      no gs-toggle-click class because it is custom ajax enabled
      and then calls gs-toggle-click directly

      gs-xtransport the @meta:* (database:query etc.) also
      e.g. @interface-mode => <div class="gs-interface-mode-[@interface-mode]">...

      gs-interface-mode-* CSS class useful for display purposes
    -->
    <xsl:param name="gs_html_identifier_class"/>
    <xsl:param name="gs_interface_mode"/>
    <xsl:param name="gs_sub_interface"/> <!-- can be "partial" -->
    <xsl:param name="gs_event_functions"/>
    <!-- optional parent meta-data inheritance-merge
      using namespace-uri() instead of @meta:* because of an error in Chrome
      local meta-data takes precedence, e.g. @meta:child-count, @meta:xpath-to-node
    -->

    <!-- lists generated from interface_render/xsl:template @mode=children have:
      @meta:child-count the exact number of accessible children in the database without any node-mask
      <interface:Collection> the container for the children in THIS XML
      count(interface:Collection/*) count of children in THIS XML, not necessarily to be made css "visible"
      
      in-database => retrieved => visible
      
      JavaScript might use mergeNodes to AJAX add new children amongst EXISTING children
      in this case it would need to manually update the understanding of the children count:
      .gs-children-retrieved-patial => .gs-children-retrieved-all
    -->
    <xsl:variable name="gs_child_count" select="number(@meta:child-count)"/>
    <xsl:variable name="gs_children_retrieved" select="count(interface:Collection/*)"/>
    <xsl:variable name="gs_children_retrieved_class">
      <xsl:choose>
        <xsl:when test="not($gs_child_count)">gs-children-retrieved-none-exist</xsl:when>
        <xsl:when test="not($gs_children_retrieved)">gs-children-retrieved-none</xsl:when>
        <xsl:when test="$gs_child_count &gt; $gs_children_retrieved">gs-children-retrieved-patial</xsl:when>
        <xsl:otherwise>gs-children-retrieved-all</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    
    <li class="{$gs_html_identifier_class} {$gs_event_functions} {$gs_children_retrieved_class} f_click_select_children">
      <xsl:apply-templates select="." mode="gs_list_childControl">
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
        <xsl:with-param name="gs_children_retrieved_class" select="$gs_children_retrieved_class"/>
      </xsl:apply-templates>
      <xsl:apply-templates select="." mode="gs_list_details">
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
        <xsl:with-param name="gs_children_retrieved_class" select="$gs_children_retrieved_class"/>
      </xsl:apply-templates>
      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups">
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
        <xsl:with-param name="gs_children_retrieved_class" select="$gs_children_retrieved_class"/>
      </xsl:apply-templates>

      <!-- usually ajax children 
        interface:Collection to hold the children, and possibly already containing some or all of them
        interface:AJAXHTMLLoader to retrieve more children if any
      -->
      <xsl:apply-templates select="*" mode="gs_view_render">
        <!-- xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/ -->
      </xsl:apply-templates>
    </li>
  </xsl:template>

  <xsl:template match="*" mode="gs_list_childControl">
    <xsl:param name="gs_interface_mode"/>
    <xsl:param name="gs_children_retrieved_class"/>
    
    <div>
      <xsl:attribute name="class">
        <xsl:text>gs-list-child-control gs-toggle gs-clickable f_click_toggleCSSChildClassDOFromEventDisplayObject_children</xsl:text>
        <!-- it is only when all children are here from the database that we want to show the collapse option -->
        <xsl:if test="$gs_children_retrieved_class = 'gs-children-retrieved-all'"> expanded</xsl:if>
      </xsl:attribute>
      <xsl:text>&nbsp;</xsl:text>
    </div>
  </xsl:template>

  <xsl:template match="*" mode="gs_list_details">
    <!-- TODO: is $gs_namespace_prefix calc really required? -->
    <xsl:variable name="gs_namespace_prefix">
      <xsl:if test="@meta:fully-qualified-prefix"><xsl:value-of select="@meta:fully-qualified-prefix"/></xsl:if>
      <xsl:if test="not(@meta:fully-qualified-prefix)"><xsl:value-of select="substring-before(name(), ':')"/></xsl:if>
    </xsl:variable>

    <div class="details gs-clickable">
      <xsl:attribute name="style">
        <xsl:apply-templates select="@*" mode="gs_list_style"/>
      </xsl:attribute>

      <xsl:apply-templates select="." mode="gs_list_pre_additions"/>

      <xsl:if test="$gs_namespace_prefix">
        <span class="namespace_prefix"><xsl:value-of select="$gs_namespace_prefix"/><xsl:text>:</xsl:text></span>
      </xsl:if>
      <span class="name"><xsl:apply-templates select="." mode="gs_listname"/></span>
      <span class="child-count">
        <xsl:text>(</xsl:text>
        <xsl:value-of select="@meta:child-count"/>
        <xsl:text>)</xsl:text>
      </span>

      <span class="indicators">
        <xsl:if test="@meta:is-hard-link = 'true'"><img class="indicator-is-hard-link" src="{$gs_resource_server}/resources/shared/images/spacer.png"/></xsl:if>
        <xsl:if test="@meta:is-registered = 'true'"><img class="indicator-is-registered" src="{$gs_resource_server}/resources/shared/images/spacer.png"/></xsl:if>
        <xsl:if test="@gs:transient-area"><img class="indicator-is-transient-area" src="{$gs_resource_server}/resources/shared/images/spacer.png"/></xsl:if>
        <xsl:if test="@xml:id-policy-area"><img class="indicator-id-policy-area" src="{$gs_resource_server}/resources/shared/images/spacer.png"/></xsl:if>
      </span>

      <!-- span class="text"><xsl:apply-templates select="text()"/></span -->
      <div class="gs-xtransport xpath"><xsl:value-of select="name()"/></div>
      <ul class="attributes"><xsl:apply-templates select="@*" mode="gs_list_attributes"/></ul>

      <xsl:apply-templates select="." mode="gs_list_post_additions"/>
    </div>
  </xsl:template>

  <xsl:template match="*" mode="gs_list_pre_additions"/>
  <xsl:template match="*" mode="gs_list_post_additions"/>
  <xsl:template match="@*" mode="gs_list_style"/>

  <xsl:template match="@tree-icon" mode="gs_list_style">
    <xsl:text>background-image:url(</xsl:text>
    <xsl:value-of select="$gs_resource_server"/>
    <xsl:text>/resources/shared/images/icons/</xsl:text>
    <xsl:value-of select="."/>
    <xsl:text>.png);</xsl:text>
  </xsl:template>

  <xsl:template match="*" mode="listname" meta:interface-template="yes">
    <xsl:param name="gs_html_identifier_class"/>
    <xsl:param name="gs_interface_mode"/>
    
    <span class="{$gs_html_identifier_class}">
      <xsl:apply-templates select="." mode="gs_listname"/>
      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups">
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      </xsl:apply-templates>
    </span>
  </xsl:template>

  <xsl:template match="*" mode="gs_listname">
    <xsl:choose>
      <xsl:when test="@meta:list-name"><xsl:apply-templates select="@meta:list-name" mode="gs_field_attribute"/></xsl:when>
      <xsl:when test="@name"><xsl:apply-templates select="@name" mode="gs_field_attribute"/></xsl:when>
      <xsl:otherwise>
        <xsl:apply-templates select="." mode="gs_field_localname"/>
        <xsl:text> </xsl:text>
        <xsl:apply-templates select="@name" mode="gs_field_attribute"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="@*" mode="gs_list_attributes"/>
</xsl:stylesheet>
