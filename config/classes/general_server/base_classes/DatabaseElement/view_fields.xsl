<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:xmlsecurity="http://general_server.org/xmlnamespaces/xmlsecurity/2006" xmlns:meta="http://general_server.org/xmlnamespaces/meta/2006" xmlns="http://www.w3.org/1999/xhtml" name="view-fields" version="1.0" extension-element-prefixes="debug">
  <xsl:template match="@*" mode="gs_field_name_attribute">
    <span class="Field gs-field-name-attribute">
      <xsl:apply-templates select="."/>
    </span>
  </xsl:template>

  <xsl:template match="*" mode="gs_field_localname">
    <span class="Field gs-field-localname">
      <xsl:value-of select="translate(local-name(), '_', ' ')"/>
    </span>
  </xsl:template>

  <xsl:template match="@*" mode="gs_field_attribute">
    <span class="Field gs-field-attribute gs-name-{local-name()}">
      <xsl:apply-templates select="."/>
    </span>
  </xsl:template>

  <xsl:template match="@*" mode="gs_field_element">
    <span class="Field gs-field-element gs-name-{local-name()}">
      <xsl:apply-templates select="."/>
    </span>
  </xsl:template>
</xsl:stylesheet>