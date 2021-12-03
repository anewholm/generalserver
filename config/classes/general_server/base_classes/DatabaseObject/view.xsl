<xsl:stylesheet xmlns:xmlsecurity="http://general_server.org/xmlnamespaces/xmlsecurity/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" name="view" version="1.0" extension-element-prefixes="debug">
  <xsl:template match="*" mode="gs_inherited_css_classes">
    <!-- output all the CSS__<definitions> prepended with CSS__ -->
    <xsl:param name="gs_class_definitions" select="."/>

    <xsl:variable name="gs_class_definition">
      <xsl:if test="contains($gs_class_definitions, ',')"><xsl:value-of select="normalize-space(substring-before($gs_class_definitions, ','))"/></xsl:if>
      <xsl:if test="not(contains($gs_class_definitions, ','))"><xsl:value-of select="normalize-space($gs_class_definitions)"/></xsl:if>
    </xsl:variable>

    <xsl:text> CSS__</xsl:text><xsl:value-of select="$gs_class_definition"/><xsl:text> </xsl:text>
    <xsl:if test="contains($gs_class_definitions, ',')">
      <xsl:apply-templates select="." mode="gs_inherited_css_classes">
        <xsl:with-param name="gs_class_definitions" select="substring-after($gs_class_definitions, ',')"/>
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>

  <xsl:template match="*" mode="gs_class_definitions">
    <!-- output all the Class__<definitions> prepended with Class__ -->
    <xsl:param name="gs_class_definitions"/>

    <xsl:variable name="gs_class_definition">
      <xsl:if test="contains($gs_class_definitions, ',')"><xsl:value-of select="normalize-space(substring-before($gs_class_definitions, ','))"/></xsl:if>
      <xsl:if test="not(contains($gs_class_definitions, ','))"><xsl:value-of select="normalize-space($gs_class_definitions)"/></xsl:if>
    </xsl:variable>

    <xsl:text> Class__</xsl:text><xsl:value-of select="$gs_class_definition"/><xsl:text> </xsl:text>
    <xsl:if test="contains($gs_class_definitions, ',')">
      <xsl:apply-templates select="." mode="gs_class_definitions">
        <xsl:with-param name="gs_class_definitions" select="substring-after($gs_class_definitions, ',')"/>
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>
</xsl:stylesheet>