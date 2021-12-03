<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" name="view" version="1.0">
  <xsl:template match="interface:Menu//gs:menu-section">
    <xsl:if test="*">
      <xsl:if test="preceding-sibling::*">
        <li class="gs-menu-section"/>
      </xsl:if>
      <xsl:apply-templates select="*"/>
    </xsl:if>
  </xsl:template>

  <xsl:template match="interface:Menu//gs:menu-section">
    <xsl:if test="*">
      <xsl:if test="preceding-sibling::*">
        <li class="gs-menu-section"/>
      </xsl:if>
      <xsl:apply-templates select="*"/>
    </xsl:if>
  </xsl:template>
</xsl:stylesheet>