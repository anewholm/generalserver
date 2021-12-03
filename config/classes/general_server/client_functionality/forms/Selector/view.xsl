<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:meta="http://general_server.org/xmlnamespaces/meta/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" name="view" version="1.0">
  <xsl:template match="*" mode="selectoroption" meta:interface-template="yes">
    <xsl:param name="gs_html_identifier_class"/>

    <li class="{$gs_html_identifier_class}">
      <xsl:value-of select="local-name()"/>
    </li>
  </xsl:template>

  <xsl:template match="xsd:attribute[@meta:editor-class = 'class:Selector']" meta:output-order="10200" meta:interface-template="yes">
    <!-- @meta:selector-options=<xpath> -->
    <xsl:param name="gs_html_identifier_class"/>
    <xsl:param name="gs_current_external_value_node" select="FORCE_SELECT_EMPTY_NODESET"/>

    <xsl:variable name="gs_current_value" select="$gs_current_external_value_node/@*[name() = @name]"/>

    <div class="{$gs_html_identifier_class}">
      <input name="{@name}" value="{$gs_current_value}"/>
      <!-- default on-click on parent Class__Selector -->
      <database:query class="Object Class__AJAXHTMLLoader" is-client-ondemand="true" data-context-id="{@meta:selector-options-id}" data-context="{@meta:selector-options}" data="."/>
    </div>
  </xsl:template>
</xsl:stylesheet>
