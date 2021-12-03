<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:xmlsecurity="http://general_server.org/xmlnamespaces/xmlsecurity/2006" xmlns:meta="http://general_server.org/xmlnamespaces/meta/2006" xmlns="http://www.w3.org/1999/xhtml" name="view_meta_transport" version="1.0" extension-element-prefixes="debug">
  <xsl:template match="*" mode="gs_meta_data_standard_groups" meta:debug="false">
    <xsl:param name="gs_interface_mode" select="'default'"/>

    <!-- @gs:* display-specific => j{data object} (including @gs:interface-mode, @gs:display-class) -->
    <xsl:apply-templates select="." mode="gs_meta_data">
      <xsl:with-param name="gs_namespace_uri" select="'http://general_server.org/xmlnamespaces/general_server/2006'"/>
      <xsl:with-param name="gs_group_name" select="'gs-data'"/>
      <xsl:with-param name="gs_display_specific" select="true()"/>
      <xsl:with-param name="gs_js_load_as" select="'properties'"/>
    </xsl:apply-templates>

    <!-- @html:* display-specific function-calls() => DisplayObject/*() (like DisplayObject->height(30px) etc.) -->
    <xsl:if test="$gs_interface_mode = 'default'">
      <xsl:apply-templates select="." mode="gs_meta_data">
        <xsl:with-param name="gs_namespace_uri" select="'http://www.w3.org/1999/xhtml'"/>
        <xsl:with-param name="gs_group_name" select="'html-jquery'"/>
        <xsl:with-param name="gs_display_specific" select="true()"/>
        <xsl:with-param name="gs_js_load_as" select="'function-calls'"/>
      </xsl:apply-templates>
    </xsl:if>

    <!-- @meta:* properties (like @meta:child-count, @meta:xpath-to-node, etc) -->
    <xsl:apply-templates select="." mode="gs_meta_data">
      <xsl:with-param name="gs_namespace_uri" select="'http://general_server.org/xmlnamespaces/meta/2006'"/>
      <xsl:with-param name="gs_group_name" select="'meta-data'"/>
      <xsl:with-param name="gs_display_specific" select="false()"/>
      <xsl:with-param name="gs_js_load_as" select="'properties'"/>
    </xsl:apply-templates>

    <!-- @xml:* properties (only @xml:id) -->
    <xsl:apply-templates select="." mode="gs_meta_data">
      <xsl:with-param name="gs_namespace_uri" select="'http://www.w3.org/XML/1998/namespace'"/>
      <xsl:with-param name="gs_group_name" select="'meta-data'"/>
      <xsl:with-param name="gs_display_specific" select="false()"/>
      <xsl:with-param name="gs_js_load_as" select="'properties'"/>
      <xsl:with-param name="gs_include_namespace" select="true()"/>
    </xsl:apply-templates>

    <!-- @xsl:* properties (only @xsl:literal) -->
    <xsl:apply-templates select="." mode="gs_meta_data">
      <xsl:with-param name="gs_namespace_uri" select="'http://www.w3.org/1999/XSL/Transform'"/>
      <xsl:with-param name="gs_group_name" select="'meta-data'"/>
      <xsl:with-param name="gs_display_specific" select="false()"/>
      <xsl:with-param name="gs_js_load_as" select="'properties'"/>
      <xsl:with-param name="gs_include_namespace" select="true()"/>
    </xsl:apply-templates>

    <!-- @* properties (like @name etc.) -->
    <xsl:apply-templates select="." mode="gs_meta_data">
      <!-- xsl:with-param name="gs_namespace_uri" select="''"/ -->
      <xsl:with-param name="gs_group_name" select="'data'"/>
      <xsl:with-param name="gs_display_specific" select="false()"/>
      <xsl:with-param name="gs_js_load_as" select="'properties'"/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="*" mode="gs_meta_data" meta:debug="false">
    <xsl:param name="gs_namespace_uri"/>  <!-- node-set value -->
    <!-- OR -->
    <xsl:param name="gs_meta_data" select="@*[namespace-uri() = $gs_namespace_uri]"/>  <!-- node-set value -->
    
    <xsl:param name="gs_group_name"/>  <!-- node-set value -->
    <xsl:param name="gs_js_load_as" select="'Options'"/>
    <xsl:param name="gs_display_specific" select="false()"/>
    <xsl:param name="gs_include_namespace" select="false()"/>

    <!-- no group if no @namespace:attributes -->
    <xsl:if test="$gs_meta_data">
      <xsl:variable name="gs_prefix" select="substring-before(name($gs_meta_data), ':')"/>

      <!-- we display:none in case the CSS is not included -->
      <div style="display:none;">
        <xsl:attribute name="class">
          <xsl:text>gs-meta-data</xsl:text>
          <xsl:if test="$gs_prefix"> gs-namespace-prefix-<xsl:value-of select="$gs_prefix"/></xsl:if>
          <xsl:if test="not($gs_prefix)"> gs-namespace-no-prefix</xsl:if>
          <xsl:if test="$gs_group_name"> gs-group-<xsl:value-of select="$gs_group_name"/></xsl:if>
          <xsl:if test="$gs_js_load_as"> gs-load-as-<xsl:value-of select="$gs_js_load_as"/></xsl:if>
          <xsl:if test="$gs_display_specific"> gs-display-specific</xsl:if>
          <xsl:if test="$gs_include_namespace"> gs-include-namespace</xsl:if>
        </xsl:attribute>

        <xsl:apply-templates select="$gs_meta_data" mode="gs_xtransport_values"/>
      </div>
    </xsl:if>
  </xsl:template>

  <xsl:template match="*|@*" mode="gs_xtransport_values" meta:debug="false">
    <!-- don't transport empty DIVs: they will render badly in HTML  -->
    <xsl:if test="string(.)"><div class="{local-name()}"><xsl:value-of select="."/></div></xsl:if>
  </xsl:template>

  <xsl:template match="text()" mode="gs_xtransport_values" meta:debug="false">
    <!-- don't transport empty DIVs: they will render badly in HTML  -->
    <xsl:if test="string(.)"><div class="gs-text"><xsl:value-of select="."/></div></xsl:if>
  </xsl:template>
</xsl:stylesheet>
