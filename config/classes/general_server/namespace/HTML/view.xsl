<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns="http://www.w3.org/1999/xhtml" name="view" version="1.0" extension-element-prefixes="debug">
  <xsl:template match="*|@*" mode="gs_enforce_non_self_closing">
    <!-- we make this valid JavaScript anyway just in case -->
    <xsl:text>window.gs_enforce_non_self_closing</xsl:text>
  </xsl:template>
  
  <xsl:template match="html:*" meta:interface-template="yes">
    <!-- HTML can be placed in the data render directly -->
    <xsl:copy>
      <!-- TODO: copy the non @meta:* attributes only? -->
      <xsl:copy-of select="@*[not(self::class)]"/>
      <xsl:attribute name="class">
        <xsl:value-of select="@class"/>
        <xsl:if test="position() = 1"> gs-first</xsl:if>
        <xsl:if test="position() = last()"> gs-last</xsl:if>
      </xsl:attribute>

      <xsl:apply-templates mode="gs_view_render"/>
    </xsl:copy>
  </xsl:template>
</xsl:stylesheet>
