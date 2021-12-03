<xsl:stylesheet xmlns:xmlsecurity="http://general_server.org/xmlnamespaces/xmlsecurity/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" name="view_default" version="1.0" extension-element-prefixes="debug">
  <xsl:template match="*" mode="gs_extra_classes"/>

  <xsl:template match="*" meta:output-order="1000" meta:interface-template="yes">
    <!-- standard default HTML implementation
      @meta:output-order=1000 ensures we trump the ~HTTP/default_templates
      <container DIV> (OR class:*/@meta:html-container)
        <@mode=default_content>
          <title/>
          => gs_view_render
        </default_content>
      </container DIV>
    -->
    <xsl:param name="gs_html_identifier_class"/>
    <xsl:param name="gs_interface_mode"/>

    <xsl:variable name="gs_container_tag_fragment">
      <xsl:apply-templates select="." mode="gs_html_container"/>
    </xsl:variable>
    <xsl:variable name="gs_container_tag_defaulted_fragment">
      <xsl:value-of select="$gs_container_tag_fragment"/>
      <xsl:if test="not(string($gs_container_tag_fragment))">div</xsl:if>
    </xsl:variable>
    <xsl:variable name="gs_container_tag" select="string($gs_container_tag_defaulted_fragment)"/>

    <xsl:element name="{$gs_container_tag}">
      <xsl:attribute name="class"><xsl:value-of select="$gs_html_identifier_class"/></xsl:attribute>

      <xsl:apply-templates select="." mode="default_content">
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      </xsl:apply-templates>

      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups">
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      </xsl:apply-templates>
    </xsl:element>
  </xsl:template>

  <xsl:template match="*" mode="default_content">
    <!-- default interface leads to render default_content -->
    <xsl:param name="gs_interface_mode"/>

    <xsl:apply-templates select="@title|@title-1|@title-2|@title-3|@title-4|@title-5" mode="full_title"/>
    <xsl:apply-templates mode="gs_view_render"/>
  </xsl:template>

  <xsl:template match="@title" mode="full_title">
    <div class="gs-title"><xsl:value-of select="."/></div>
  </xsl:template>

  <xsl:template match="@title-1" mode="full_title">
    <h1><xsl:value-of select="."/></h1>
  </xsl:template>

  <xsl:template match="@title-2" mode="full_title">
    <h2><xsl:value-of select="."/></h2>
  </xsl:template>

  <xsl:template match="@title-3" mode="full_title">
    <h3><xsl:value-of select="."/></h3>
  </xsl:template>

  <xsl:template match="@title-4" mode="full_title">
    <h4><xsl:value-of select="."/></h4>
  </xsl:template>

  <xsl:template match="@title-5" mode="full_title">
    <h5><xsl:value-of select="."/></h5>
  </xsl:template>
</xsl:stylesheet>