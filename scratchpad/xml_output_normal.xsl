<xsl:stylesheet xmlns="http://www.w3.org/1999/xhtml" xmlns:flow="http://exslt.org/flow" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:str="http://exslt.org/strings" xmlns:response-xml="http://general_server.org/xmlnamespaces/response/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" response-xml:server-side-only="true" name="xml_output_normal" version="1.0" extension-element-prefixes="flow database str debug request response regexp repository" object:cpp-component="yes">
  <!-- standard recursive, overridable node output -->

  <!-- xsl:strip-space elements="*" / -->

  <xsl:template match="*" mode="xml_output_normal_copy">
    <!-- pseudo mode using xml_output_normal parameters that emulate an xsl:copy-of -->
    <xsl:apply-templates select="." mode="xml_output_normal">
      <xsl:with-param name="gs_create_meta_context_attributes" select="'none'"/>
    </xsl:apply-templates>
  </xsl:template>

  <!-- TODO: should we allow dynamic attribute texts? -->
  <!-- xsl:template match="@*" mode="xml_output_normal">
    <xsl:copy-of select="."/>
  </xsl:template -->

  <!-- the built-in text() template outputs the text -->
  <!-- xsl:template match="text()" mode="xml_output_normal">
    <xsl:value-of select="."/>
  </xsl:template -->

  <xsl:template match="*" mode="xml_output_normal">
    <!-- copy all node types, and apply-templates on all relevant axies -->
    <xsl:param name="gs_display_mode"/>
    <xsl:param name="gs_create_meta_context_attributes"/>
    <xsl:param name="gs_suppress_xml_id"/>
    <xsl:param name="gs_additional_attributes"/>
    <xsl:param name="gs_hardlink_output"/>
    <xsl:param name="gs_softlink_first"/>
    <xsl:param name="gs_param1"/>
    <xsl:param name="gs_param2"/>
    <xsl:param name="gs_param3"/>

    <xsl:copy>
      <!-- copy attributes, meta attributes and additional attributes -->
      <xsl:if test="$gs_additional_attributes">
        <xsl:apply-templates select="$gs_additional_attributes" mode="xml_output_normal">
          <xsl:with-param name="gs_display_mode" select="$gs_display_mode"/>
          <xsl:with-param name="gs_create_meta_context_attributes" select="$gs_create_meta_context_attributes"/>
          <xsl:with-param name="gs_suppress_xml_id" select="$gs_suppress_xml_id"/>
          <xsl:with-param name="gs_additional_attributes" select="$gs_additional_attributes"/>
          <xsl:with-param name="gs_hardlink_output" select="$gs_hardlink_output"/>
          <xsl:with-param name="gs_softlink_first" select="$gs_softlink_first"/>
          <xsl:with-param name="gs_param1" select="$gs_param1"/>
          <xsl:with-param name="gs_param2" select="$gs_param2"/>
          <xsl:with-param name="gs_param3" select="$gs_param3"/>
        </xsl:apply-templates>
      </xsl:if>

      <xsl:apply-templates select="@*[not(.=(../@gs:child_count|../@gs:position|../@xml:id))]" mode="xml_output_normal">
        <xsl:with-param name="gs_display_mode" select="$gs_display_mode"/>
        <xsl:with-param name="gs_create_meta_context_attributes" select="$gs_create_meta_context_attributes"/>
        <xsl:with-param name="gs_suppress_xml_id" select="$gs_suppress_xml_id"/>
        <xsl:with-param name="gs_additional_attributes" select="$gs_additional_attributes"/>
        <xsl:with-param name="gs_hardlink_output" select="$gs_hardlink_output"/>
        <xsl:with-param name="gs_softlink_first" select="$gs_softlink_first"/>
        <xsl:with-param name="gs_param1" select="$gs_param1"/>
        <xsl:with-param name="gs_param2" select="$gs_param2"/>
        <xsl:with-param name="gs_param3" select="$gs_param3"/>
      </xsl:apply-templates>

      <xsl:if test="not($gs_suppress_xml_id)">
        <xsl:apply-templates select="@xml:id" mode="xml_output_normal"/>
      </xsl:if>

      <xsl:if test="not($gs_create_meta_context_attributes = 'none')">
        <xsl:apply-templates select="." mode="xml_output_create_class_attributes"/>
      </xsl:if>

      <xsl:if test="not($gs_create_meta_context_attributes = 'none' or $gs_create_meta_context_attributes = 'class-only')">
        <xsl:apply-templates select="." mode="xml_output_create_meta_context_attributes"/>
      </xsl:if>

      <!-- child elements and text() nodes and extra elements -->
      <xsl:apply-templates select="self::*" mode="xml_output_normal_extra_elements">
        <xsl:with-param name="gs_display_mode" select="$gs_display_mode"/>
        <xsl:with-param name="gs_create_meta_context_attributes" select="$gs_create_meta_context_attributes"/>
        <xsl:with-param name="gs_suppress_xml_id" select="$gs_suppress_xml_id"/>
        <xsl:with-param name="gs_additional_attributes" select="$gs_additional_attributes"/>
        <xsl:with-param name="gs_hardlink_output" select="$gs_hardlink_output"/>
        <xsl:with-param name="gs_softlink_first" select="$gs_softlink_first"/>
        <xsl:with-param name="gs_param1" select="$gs_param1"/>
        <xsl:with-param name="gs_param2" select="$gs_param2"/>
        <xsl:with-param name="gs_param3" select="$gs_param3"/>
      </xsl:apply-templates>

      <xsl:apply-templates select="self::*" mode="xml_output_normal_children">
        <xsl:with-param name="gs_display_mode" select="$gs_display_mode"/>
        <xsl:with-param name="gs_create_meta_context_attributes" select="$gs_create_meta_context_attributes"/>
        <xsl:with-param name="gs_suppress_xml_id" select="$gs_suppress_xml_id"/>
        <xsl:with-param name="gs_additional_attributes" select="$gs_additional_attributes"/>
        <xsl:with-param name="gs_hardlink_output" select="$gs_hardlink_output"/>
        <xsl:with-param name="gs_softlink_first" select="$gs_softlink_first"/>
        <xsl:with-param name="gs_param1" select="$gs_param1"/>
        <xsl:with-param name="gs_param2" select="$gs_param2"/>
        <xsl:with-param name="gs_param3" select="$gs_param3"/>
      </xsl:apply-templates>
    </xsl:copy>
  </xsl:template>

  <!-- normal attribute suppression -->
  <!-- xsl:template match="@gs:display-mode" mode="xml_output_normal"/ -->
  <xsl:template match="@object:cpp-component" mode="xml_output_normal"/>
  <xsl:template match="@repository:type" mode="xml_output_normal"/>
  <xsl:template match="@xml:add-xml-id" mode="xml_output_normal"/>

  <xsl:template match="*" mode="xml_output_normal_extra_elements"/>

  <xsl:template match="*" mode="xml_output_create_meta_context_attributes">
    <xsl:if test="not(@gs:child_count)">
      <xsl:attribute name="gs:child_count">
        <xsl:value-of select="count(*)"/>
      </xsl:attribute>
    </xsl:if>
    <xsl:if test="not(@gs:position)">
      <xsl:attribute name="gs:position">
        <xsl:value-of select="count(preceding-sibling::*)"/>
      </xsl:attribute>
    </xsl:if>
  </xsl:template>

  <xsl:template match="node()" mode="xml_output_normal_children">
    <!-- self to child traversal -->
    <xsl:param name="gs_display_mode"/>
    <xsl:param name="gs_create_meta_context_attributes"/>
    <xsl:param name="gs_suppress_xml_id"/>
    <xsl:param name="gs_additional_attributes"/>
    <xsl:param name="gs_softlink_first"/>
    <xsl:param name="gs_hardlink_output"/>
    <xsl:param name="gs_param1"/>
    <xsl:param name="gs_param2"/>
    <xsl:param name="gs_param3"/>

    <!-- delegate the navigation to children in order to allow overriding -->
    <xsl:apply-templates select="node()" mode="xml_output_normal">
      <xsl:with-param name="gs_display_mode" select="$gs_display_mode"/>
      <xsl:with-param name="gs_create_meta_context_attributes" select="$gs_create_meta_context_attributes"/>
      <xsl:with-param name="gs_suppress_xml_id" select="$gs_suppress_xml_id"/>
      <xsl:with-param name="gs_additional_attributes" select="$gs_additional_attributes"/>
      <xsl:with-param name="gs_hardlink_output" select="$gs_hardlink_output"/>
      <xsl:with-param name="gs_softlink_first" select="$gs_softlink_first"/>
      <xsl:with-param name="gs_param1" select="$gs_param1"/>
      <xsl:with-param name="gs_param2" select="$gs_param2"/>
      <xsl:with-param name="gs_param3" select="$gs_param3"/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="text()" mode="xml_output_normal_children">
    <!-- output the bare value of all text nodes -->
    <xsl:value-of select="."/>
  </xsl:template>
</xsl:stylesheet>