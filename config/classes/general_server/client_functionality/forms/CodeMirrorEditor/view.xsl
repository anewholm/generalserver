<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" name="view" version="1.0">
  <xsl:template match="xsd:attribute[@meta:editor-class = 'class:CodeMirrorEditor']" meta:output-order="10201" meta:interface-template="yes">
    <!-- CodeMirrorEditor is a html FORM component
      <xsd:attribute type="class:CodeMirrorEditor" ... />
      it will be rendered by the XSchema Class
      the $gs_current_external_value_node is the XML
    -->
    <xsl:param name="gs_html_identifier_class"/>
    <xsl:param name="gs_current_external_value_node" select="FORCE_SELECT_EMPTY_NODESET"/>

    <div class="{$gs_html_identifier_class} f_presubmit_updateFormElement f_appear_refresh f_resize_refresh">
      <textarea name="xml">
        <xsl:copy-of select="$gs_current_external_value_node"/>
      </textarea>

      <xsl:apply-templates select="." mode="gs_meta_data">
        <xsl:with-param name="gs_namespace_uri" select="'http://general_server.org/xmlnamespaces/general_server/2006'"/>
      </xsl:apply-templates>
    </div>
  </xsl:template>

  <xsl:template match="xsd:attribute[@meta:editor-class = 'class:CodeMirrorEditor']" mode="xpathajax" meta:interface-template="yes">
    <!-- CodeMirrorEditor is a html FORM component
      <xsd:attribute type="class:CodeMirrorEditor" ... />
      it will be rendered by the XSchema Class
      the $gs_value is NOT the XML. it is the xpath to the node to edit
      overriding the full because we need to pass parameters
    -->
    <xsl:param name="gs_html_identifier_class"/>
    <xsl:param name="gs_current_external_value_node" select="FORCE_SELECT_EMPTY_NODESET"/> <!-- not used here: xsd:documentation[@meta:type='destination'] used instead  -->

    <xsl:variable name="gs_xsd_schema" select="ancestor::xsd:schema[1]"/>
    <xsl:variable name="gs_destination_xpath" select="$gs_xsd_schema/xsd:annotation/xsd:documentation[@meta:type='xpath-to-destination']"/>

    <div class="{$gs_html_identifier_class} f_presubmit_updateFormElement f_appear_refresh f_resize_refresh">
      <!-- checks -->
      <xsl:if test="not($gs_xsd_schema)"><div class="gs_warning">cannot find owner xsd:schema element</div></xsl:if>
      <xsl:if test="not($gs_destination_xpath)"><div class="gs_warning">cannot find xsd:documentation[@meta:type='xpath-to-destination'] in xsd:schema</div></xsl:if>

      <xsl:if test="$gs_destination_xpath">
        <textarea name="xml">loading...</textarea>
        <xsl:apply-templates select="." mode="gs_meta_data">
          <xsl:with-param name="gs_namespace_uri" select="'http://general_server.org/xmlnamespaces/general_server/2006'"/>
        </xsl:apply-templates>
        <span class="gs-xtransport gs-ajax-xpath"><xsl:value-of select="$gs_destination_xpath"/></span>
      </xsl:if>
      
      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups"/>
    </div>
  </xsl:template>

  <xsl:template match="xsd:attribute[@meta:editor-class = 'class:CodeMirrorEditor']" mode="gs_context_menu_custom">
    <li class="f_click_setBreakpoint">set breakpoint</li>
    <li class="f_click_clearBreakpoint">clear breakpoint</li>
  </xsl:template>
</xsl:stylesheet>
