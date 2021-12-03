<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" name="view" version="1.0">
  <xsl:template match="interface:Overlay" mode="gs_extra_classes">
    <!-- conveinience template
      TODO: use Javascript object setting()s instead
    -->
    <xsl:text>gs-item-</xsl:text><xsl:apply-templates select="." mode="gs_extra_classes_overlay_transition"/>
    <xsl:text> gs-exclusivity-</xsl:text><xsl:apply-templates select="." mode="gs_extra_classes_overlay_exclusivity"/>
    <xsl:text> gs-position-</xsl:text><xsl:apply-templates select="." mode="gs_extra_classes_overlay_position"/>
  </xsl:template>

  <xsl:template match="*" mode="gs_extra_classes_overlay_transition"/>
  <xsl:template match="*" mode="gs_extra_classes_overlay_exclusivity"/>
  <xsl:template match="*" mode="gs_extra_classes_overlay_position"/>
</xsl:stylesheet>