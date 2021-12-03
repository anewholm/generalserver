<xsl:stylesheet xmlns:xmlsecurity="http://general_server.org/xmlnamespaces/xmlsecurity/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" name="view_link" version="1.0" extension-element-prefixes="debug">
  <xsl:template match="*" mode="link" meta:interface-template="yes">
    <xsl:param name="gs_html_identifier_class"/>
    <xsl:param name="gs_interface_mode"/>

    <li class="{$gs_html_identifier_class} gs-toggle-hover-exclusive-global">
      <a class="details" href="/{@meta:xpath-to-node-client}" target="_top">
        <xsl:choose>
          <xsl:when test="string(@meta:menu-item)"><xsl:value-of select="@meta:menu-item"/></xsl:when>
          <xsl:when test="string(@name)"><xsl:value-of select="@name"/></xsl:when>
          <xsl:otherwise><xsl:value-of select="local-name()"/></xsl:otherwise>
        </xsl:choose>
      </a>

      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups">
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      </xsl:apply-templates>

      <!-- sub-menus -->
      <xsl:if test="*">
        <ul>
          <xsl:apply-templates select="*" mode="gs_view_render">
            <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
          </xsl:apply-templates>
        </ul>
      </xsl:if>
    </li>
  </xsl:template>
</xsl:stylesheet>
