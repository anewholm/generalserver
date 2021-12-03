<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" name="view" version="1.0">
  <xsl:template match="object:MessageInterpretation" mode="default_content">
    <!-- default interface leads to render default_content -->
    <xsl:text>MessageInterpretation</xsl:text>
  </xsl:template>

  <xsl:template match="object:MessageInterpretation" mode="gs_listname">
    <xsl:value-of select="@name"/>
  </xsl:template>
</xsl:stylesheet>