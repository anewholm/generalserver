<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" name="view" version="1.0">
  <xsl:template match="css:dependency" mode="html" meta:interface-template="yes">
    <!--
      note that also the stylesheet_transform will process <link @href in the same way as <img @src
      using <link @href is equivalent to using this <css:dependency @uri
    -->
    <xsl:variable name="gs_object_name">
      <xsl:choose>
        <xsl:when test="not(@object-name) or @object-name = '' or @object-name = 'inherit'">
          <xsl:text>CSS__</xsl:text><xsl:value-of select="local-name(ancestor::class:*[1])"/>
        </xsl:when>
        <xsl:otherwise><xsl:value-of select="@object-name"/></xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <link rel="stylesheet" xml:add-xml-id="no">
      <xsl:attribute name="href">
        <xsl:if test="starts-with(@uri, '/resources/')">
          <xsl:value-of select="$gs_resource_server"/>
        </xsl:if>
        <xsl:value-of select="@uri"/>
      </xsl:attribute>
      <xsl:if test="@meta:reason"><xsl:copy-of select="gs:reason"/></xsl:if>
      <xsl:if test="$gs_object_name"><xsl:attribute name="gs:object-name" select="{$gs_object_name}"/></xsl:if>
    </link>
  </xsl:template>
</xsl:stylesheet>
