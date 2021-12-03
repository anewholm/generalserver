<xsl:stylesheet xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" name="view" version="1.0">
  <xsl:template match="interface:QualifiedName" meta:interface-template="yes">
    <!-- {[1]/@name} :: {[2]/@name} :: {[3]/@name} ... -->
    <xsl:param name="gs_html_identifier_class"/>

    <xsl:variable name="gs_delimiter">
      <xsl:if test="@delimiter"><xsl:value-of select="@delimiter"/></xsl:if>
      <xsl:if test="not(@delimiter)"> :: </xsl:if>
    </xsl:variable>

    <div class="{$gs_html_identifier_class}">
      <xsl:apply-templates mode="gs_delimit">
        <xsl:with-param name="gs_delimiter" select="$gs_delimiter"/>
      </xsl:apply-templates>
      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups"/>
    </div>
  </xsl:template>

  <xsl:template match="*" mode="gs_delimit">
    <!-- utility template: where should it go? -->
    <xsl:param name="gs_delimiter" select="', '"/>
    <xsl:if test="@name">
      <xsl:value-of select="@name"/>
      <xsl:if test="not(last())"><xsl:value-of select="$gs_delimiter"/></xsl:if>
    </xsl:if>
  </xsl:template>
</xsl:stylesheet>