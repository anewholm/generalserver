<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" name="view" version="1.0">
  <xsl:template match="interface:Forum" mode="default_content">
    <!-- default interface leads to render default_content -->
    <div class="details">
      <xsl:apply-templates select="gs:*"/>

      <ul class="children">
        <!-- Forum interface full shows the children objects in summary mode -->
        <xsl:apply-templates select="*" mode="summary"/>
      </ul>
    </div>
  </xsl:template>

  <xsl:template match="interface:Forum/*" mode="summary"/>
 
  <xsl:template match="interface:Forum/gs:title">
    <h1 class="{local-name()}"><xsl:value-of select="."/></h1>
  </xsl:template>

  <xsl:template match="interface:Forum/gs:description">
    <div class="{local-name()}"><xsl:value-of select="."/></div>
  </xsl:template>
</xsl:stylesheet>