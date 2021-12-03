<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" name="view" version="1.0">
  <xsl:template match="*" mode="gs_client_side_xsl_warnings">
    <xsl:apply-templates select="*" mode="gs_client_side_xsl_warnings"/>
  </xsl:template>

  <xsl:template match="html:*" mode="gs_client_side_xsl_warnings">
    <xsl:apply-templates select="@*[contains(.,'{')]" mode="gs_client_side_xsl_warnings"/>
  </xsl:template>

  <xsl:template match="@*" mode="gs_client_side_xsl_warnings">
    <xsl:if test="contains(., 'database:') or contains(., 'str:')">
      <div class="gs-warning">
	attribute dynamic value @<xsl:value-of select="name()"/>=<xsl:value-of select="."/> is not advised
	it will cause FireFox to critically fail and Chrome will silently not compile it
      </div>
    </xsl:if>
  </xsl:template>

  <xsl:template match="xsl:value-of" mode="gs_client_side_xsl_warnings">
    <xsl:if test="contains(@select, 'database:') or contains(@select, 'str:')">
      <div class="gs-warning">
	value-of <![CDATA[<xsl:value-of select="]]><xsl:value-of select="@select"/><![CDATA[" />]]> is not advised
	it will cause FireFox to critically fail and Chrome will silently not compile it
      </div>
    </xsl:if>
  </xsl:template>
</xsl:stylesheet>