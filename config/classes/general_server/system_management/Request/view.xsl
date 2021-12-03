<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" name="view" version="1.0">
  <xsl:template match="object:Request" mode="default_content">
    <!-- default interface leads to render default_content -->
    <xsl:text>Styley</xsl:text>
  </xsl:template>

  <xsl:template match="object:Request" mode="gs_listname">
    <xsl:if test="@name"><xsl:value-of select="@name"/></xsl:if>
    <xsl:if test="not(@name)">request</xsl:if>
  </xsl:template>

  <xsl:template match="object:Request/@url" mode="gs_list_attributes">
    <li class="attribute_{local-name()}"><xsl:value-of select="name()"/> (<xsl:value-of select="."/>)</li>
  </xsl:template>

  <xsl:template match="object:Request/@method[.='POST']" mode="gs_list_attributes">
    <li class="attribute_{local-name()} attribute_value_post"><xsl:value-of select="name()"/> (<xsl:value-of select="."/>)</li>
  </xsl:template>

  <xsl:template match="xsl:template/@match|xsl:output/@method|gs:HTTP/@type|object:Session/@id">
    <!-- TODO: bigger custom list mode main template for these also... -->
    <li class="attribute_{local-name()}"><xsl:value-of select="name()"/> (<xsl:value-of select="."/>)</li>
  </xsl:template>

  <!-- using default list templates from idx_object.xsl -->
</xsl:stylesheet>