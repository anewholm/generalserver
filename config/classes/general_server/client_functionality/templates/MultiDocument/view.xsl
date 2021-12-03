<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" name="view" version="1.0" extension-element-prefixes="debug">
  <xsl:template match="interface:MultiDocument" mode="default_content">
    <!-- default interface leads to render default_content -->
    <xsl:apply-templates select="interface:*" mode="gs_view_render"/>

    <!-- jQuery tabs works on the ul:first -->
    <ul class="gs-jquery-nodrag gs-sort-axis-x">
      <xsl:apply-templates select="gs:tab" mode="gs_full_tab"/>
    </ul>

    <!-- jQuery tabs matches the ul:first href@ -->
    <xsl:apply-templates select="gs:tab" mode="gs_full_tab_container"/>
  </xsl:template>

  <xsl:template match="gs:tab-template" mode="gs_full_tab">
  </xsl:template>

  <xsl:template match="gs:tab" mode="gs_full_tab">
    <xsl:variable name="gs_tab_id"><xsl:if test="@id"><xsl:value-of select="@id"/></xsl:if><xsl:if test="not(@id)"><xsl:value-of select="translate(@title, ' ', '-')"/></xsl:if></xsl:variable>
    <li id="tab-{$gs_tab_id}-tab"><a href="#tab-{$gs_tab_id}"><xsl:value-of select="@title"/></a></li>
  </xsl:template>

  <xsl:template match="gs:tab" mode="gs_full_tab_container">
    <xsl:variable name="gs_tab_id"><xsl:if test="@id"><xsl:value-of select="@id"/></xsl:if><xsl:if test="not(@id)"><xsl:value-of select="translate(@title, ' ', '-')"/></xsl:if></xsl:variable>

    <div id="tab-{$gs_tab_id}" class="gs-tab-container">
      <xsl:apply-templates select="." mode="gs_full_tab_content"/>
    </div>
  </xsl:template>

  <xsl:template match="gs:tab" mode="gs_full_tab_content">
    <!-- copy the content, e.g. an interface:MultiView -->
    <xsl:apply-templates mode="gs_view_render"/>
  </xsl:template>
</xsl:stylesheet>