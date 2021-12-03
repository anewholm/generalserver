<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:xmlsecurity="http://general_server.org/xmlnamespaces/xmlsecurity/2006" xmlns:meta="http://general_server.org/xmlnamespaces/meta/2006" xmlns="http://www.w3.org/1999/xhtml" name="view_identifier" version="1.0" extension-element-prefixes="debug">
  <!-- DatabaseElement is the basic class for XML elements from the gs_interface_render
    anything that is a Class @element should be a DatabaseObject
    DatabaseElement should be locateable in the Database from their meta-info, specifically an @xml:id
  -->
  <xsl:template match="*" mode="gs_html_identifier_class">
    <!-- standard list template that can apply to everything including:
        XML elements
        IDX elements
        Class elements
    -->
    <xsl:param name="gs_interface_mode" select="@gs:interface-mode"/>
    <xsl:param name="gs_event_functions" select="@meta:event-functions"/>
    <xsl:param name="gs_display_class" select="@gs:display-class"/>
    <xsl:param name="gs_no_xml_id" select="false()"/> <!-- use class:*/@abstract=true to achieve this -->

    <!--
      .Object .Class__<main-class> => JavaScript object instanciation
      .CSS__<main-class>
      .CSS__<class-bases>
      REQUIRES: class_introspection.dxsl templates
    -->
    <xsl:variable name="gs_classes_fragment"><xsl:apply-templates select="." mode="gs_classes_string"/></xsl:variable>
    <xsl:variable name="gs_classes" select="string($gs_classes_fragment)"/>
    <xsl:text>Object </xsl:text>
    <xsl:if test="$gs_classes">
      <xsl:variable name="gs_base_classes_fragment"><xsl:apply-templates select="." mode="gs_base_classes_string"/></xsl:variable>
      <xsl:variable name="gs_base_classes" select="string($gs_base_classes_fragment)"/>
      <xsl:apply-templates select="." mode="gs_class_definitions">
        <xsl:with-param name="gs_class_definitions" select="$gs_classes"/>
      </xsl:apply-templates>
      <xsl:apply-templates select="." mode="gs_inherited_css_classes">
        <xsl:with-param name="gs_class_definitions" select="$gs_classes"/>
      </xsl:apply-templates>
      <xsl:apply-templates select="." mode="gs_inherited_css_classes">
        <xsl:with-param name="gs_class_definitions" select="$gs_base_classes"/>
      </xsl:apply-templates>
    </xsl:if>
    <xsl:if test="not($gs_classes)">
      <xsl:text>Class__DatabaseElement</xsl:text>
    </xsl:if>

    <!-- @gs:interface-mode => specific gs_view_render xsl:template @mode=@gs:interface-mode to use
      see dynamically generated gs_view_render xsl:template
      which translates @gs:interface-mode => xsl:template @mode=@gs:interface-mode
      @gs:interface-mode in an @mode NCName
      it MAY contain characters "_.-"
      @gs:interface-mode=list-partial => gs-interface-mode-list gs-interface-mode-list-partial
    -->
    <xsl:text> gs-interface-mode-</xsl:text>
    <xsl:value-of select="translate($gs_interface_mode, '.', '-')"/>
    <xsl:if test="not($gs_interface_mode)">default</xsl:if>
    <xsl:if test="contains($gs_interface_mode, '-')"> gs-interface-mode-<xsl:value-of select="substring-before($gs_interface_mode, '-')"/></xsl:if>

    <xsl:variable name="gs_namespace_prefix">
      <xsl:if test="@meta:fully-qualified-prefix"><xsl:value-of select="@meta:fully-qualified-prefix"/></xsl:if>
      <xsl:if test="not(@meta:fully-qualified-prefix)"><xsl:value-of select="substring-before(name(), ':')"/></xsl:if>
    </xsl:variable>
    <xsl:variable name="gs_has_children" select="not(@meta:child-count = 0)"/>

    <!-- HTML specific stuffs
      use @gs:display-class when the HTML output is the same but a different CSS is needed
    -->
    <xsl:if test="$gs_display_class"><xsl:text> </xsl:text><xsl:value-of select="$gs_display_class"/></xsl:if>
    <xsl:if test="$gs_event_functions"><xsl:text> </xsl:text><xsl:value-of select="$gs_event_functions"/></xsl:if>
    <xsl:if test="not($gs_interface_mode)">
      <xsl:text> </xsl:text><xsl:apply-templates select="." mode="gs_event_functions"/>
    </xsl:if>

    <!-- meta-data indicators -->
    <xsl:text> gs-name-</xsl:text>
    <xsl:value-of select="translate(translate(local-name(), $gs_uppercase, $gs_lowercase), '_', '-')"/>
    <xsl:if test="string(@name) and not(@name = local-name())">
      <xsl:text>-</xsl:text>
      <xsl:value-of select="translate(translate(@name, $gs_uppercase, $gs_lowercase), ' _:', '---')"/>
      <xsl:text> </xsl:text>
      <xsl:value-of select="translate(@name, ' _:', '---')"/>
    </xsl:if>
    <xsl:if test="@meta:is-hard-linked = 'true'"> gs-is-hard-linked</xsl:if>
    <xsl:if test="@meta:is-hard-link = 'true'"> gs-is-hard-link</xsl:if>
    <xsl:if test="@meta:is-registered = 'true'"> gs-is-registered</xsl:if>
    <xsl:if test="$gs_has_children"> gs-children-exist</xsl:if>
    <xsl:if test="not($gs_has_children)"> gs-children-none</xsl:if>
    <xsl:text> gs-local-name-</xsl:text><xsl:value-of select="local-name()"/>
    <xsl:if test="$gs_namespace_prefix"><xsl:text> namespace_</xsl:text><xsl:value-of select="$gs_namespace_prefix"/></xsl:if>
    <xsl:if test="not($gs_namespace_prefix)"><xsl:text> gs-no-namespace</xsl:text></xsl:if>
    <xsl:if test="@xml:id and not($gs_no_xml_id)">
      <xsl:text> gs-has-xml-id</xsl:text>
      <xsl:text> gs-xml-id-</xsl:text><xsl:value-of select="@xml:id"/>
    </xsl:if>
    
    <!-- html interaction classes -->
    <xsl:if test="@html:draggable"> gs-draggable</xsl:if>

    <xsl:text> </xsl:text>
    <xsl:apply-templates select="." mode="gs_extra_classes"/>
  </xsl:template>

  <xsl:template match="*" mode="gs_event_functions"/>
</xsl:stylesheet>