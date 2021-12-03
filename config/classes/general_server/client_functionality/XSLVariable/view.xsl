<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" name="view" version="1.0">
  <xsl:template match="xsl:variable|xsl:param" mode="controller" meta:interface-template="yes">
    <xsl:param name="gs_html_identifier_class"/>
    
    <div class="{$gs_html_identifier_class} gs-xsd-value gs-extended-attributes">
      <input name="{@name}" value="{@select}"/>
      <span class="gs-xtransport gs-xsd-field-message"><xsl:value-of select="@name"/></span>
      <span class="gs-xtransport gs-xsd-field-message-type">H</span>
      <span class="gs-xtransport gs-required">[a-zA-Z][-a-zA-Z0-9_.]{3}</span>
    </div>
  </xsl:template>
</xsl:stylesheet>