<xsl:stylesheet response:server-side-only="true" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:dyn="http://exslt.org/dynamic" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" name="utilities" version="1.0" extension-element-prefixes="dyn database server">
  <xsl:template match="*" mode="gs_construct_query_string">
    <!--
      create an URL query string from XML this=4&that=5
      TODO: URL encode!
    -->
    <xsl:apply-templates select="*" mode="gs_construct_query_string_pair"/>
  </xsl:template>

  <xsl:template match="*" mode="gs_construct_query_string_pair">
    <xsl:value-of select="local-name()"/>
    <xsl:text>=</xsl:text>
    <xsl:apply-templates select="text()"/>
    <xsl:if test="not(position() = last())">&amp;</xsl:if>
  </xsl:template>

  <xsl:template match="@*" mode="gs_attributes_to_xtransport_elements">
    <div class="gs_{local-name()}" xml:add-xml-id="no"><xsl:value-of select="."/></div>
    <xsl:value-of select="'&#10;'"/>
  </xsl:template>

  <xsl:template match="/|node()|@*" mode="gs_dash_to_camel">
    <xsl:param name="gs_string"/>
    <xsl:param name="gs_capitalise" selcet="false()"/>

    <xsl:choose>
      <xsl:when test="not($gs_string)"/>
      <xsl:when test="$gs_capitalise">
        <xsl:value-of select="translate(substring($gs_string, 1, 1), $gs_lowercase, $gs_uppercase)"/>
        <xsl:apply-templates select="." mode="gs_dash_to_camel">
          <xsl:with-param name="gs_string" select="substring($gs_string, 2)"/>
        </xsl:apply-templates>
      </xsl:when>
      <xsl:when test="not(contains($gs_string, '-'))">
        <xsl:value-of select="translate($gs_string, $gs_uppercase, $gs_lowercase)"/>
      </xsl:when>
      <xsl:otherwise>
        <!-- string contains a dash and we are not in capitalise mode -->
        <xsl:value-of select="translate(substring-before($gs_string, '-'), $gs_uppercase, $gs_lowercase)"/>
        <xsl:apply-templates select="." mode="gs_dash_to_camel">
          <xsl:with-param name="gs_capitalise" select="true()"/>
          <xsl:with-param name="gs_string" select="substring-after($gs_string, '-')"/>
        </xsl:apply-templates>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
</xsl:stylesheet>
