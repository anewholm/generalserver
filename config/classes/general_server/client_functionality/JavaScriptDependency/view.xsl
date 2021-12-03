<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" name="view" version="1.0">
  <xsl:include xpath="~HTML/view"/>
  
  <xsl:template match="javascript:dependency" mode="html" meta:interface-template="yes">
    <!-- no @xml:id on these as they are re-used and will cause repeat @xml:id issues on the client -->
    <xsl:param name="gs_html_identifier_class"/>

    <xsl:variable name="gs_object_name">
      <xsl:choose>
        <xsl:when test="not(@object-name) or @object-name = '' or @object-name = 'inherit'">
          <xsl:value-of select="local-name(ancestor::class:*[1])"/>
        </xsl:when>
        <xsl:otherwise><xsl:value-of select="@object-name"/></xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <script type="text/javascript" class="{$gs_html_identifier_class} details" xml:add-xml-id="no">
      <xsl:attribute name="src">
        <xsl:if test="starts-with(@uri, '/resources/')">
          <xsl:value-of select="$gs_resource_server"/>
        </xsl:if>
        <xsl:value-of select="@uri"/>
      </xsl:attribute>
      <xsl:if test="$gs_object_name"><xsl:attribute name="gs:object-name"><xsl:value-of select="$gs_object_name"/></xsl:attribute></xsl:if>

      <xsl:apply-templates select="." mode="gs_enforce_non_self_closing"/>
    </script>
  </xsl:template>
</xsl:stylesheet>
