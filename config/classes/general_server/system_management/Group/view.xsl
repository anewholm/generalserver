<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" name="view" version="1.0">
  <xsl:template match="object:Group" mode="securityowner" meta:interface-template="yes">
    <xsl:param name="gs_html_identifier_class"/>

    <div class="{$gs_html_identifier_class}">
      <img src="{$gs_resource_server}/resources/shared/images/icons/{@tree-icon}.png"/>
      <div class="gs-name"><xsl:value-of select="@name"/></div>
      <div class="gs-security-relationship">group</div>
    </div>
  </xsl:template>
</xsl:stylesheet>