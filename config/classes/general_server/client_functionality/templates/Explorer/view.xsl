<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" name="view" version="1.0">
  <xsl:template match="interface:Explorer" mode="gs_event_functions">
    <xsl:text>f_ready_setFrameHeights f_resize_setFrameHeights</xsl:text>
  </xsl:template>
</xsl:stylesheet>