<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" name="view" version="1.0" extension-element-prefixes="debug">
  <xsl:template match="interface:MultiView" mode="default_content">
    <!-- default interface leads to render default_content -->
    <xsl:apply-templates select="gs:view" mode="gs_full_view_container"/>

    <!-- render the usual VerticalMenu and any other interfaces at the top -->
    <xsl:apply-templates select="interface:*" mode="gs_view_render"/>
    <xsl:if test="not(interface:*)">
      <ul class="Object Class__SubMenu CSS__SubMenu CSS__Overlay CSS__VerticalMenu CSS__Menu gs-draggable">
        <xsl:apply-templates select="gs:view" mode="gs_full_view"/>
      </ul>
    </xsl:if>
  </xsl:template>

  <xsl:template match="interface:MultiView/interface:SubMenu" mode="default_content">
    <xsl:apply-templates select="../gs:view" mode="gs_full_view"/>
    <xsl:apply-templates mode="gs_view_render"/>
  </xsl:template>

  <xsl:template match="interface:MultiView" mode="editor" meta:interface-template="yes">
    <xsl:param name="gs_html_identifier_class"/>
    <xsl:apply-templates select=".">
      <xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="gs:view" mode="gs_full_view">
    <xsl:variable name="gs_view_id"><xsl:if test="@id"><xsl:value-of select="@id"/></xsl:if><xsl:if test="not(@id)"><xsl:value-of select="translate(@title, ' ', '_')"/></xsl:if></xsl:variable>
    <li>
      <xsl:attribute name="class">
        <xsl:text> f_click_changeView</xsl:text>
        <xsl:text> view-</xsl:text><xsl:value-of select="$gs_view_id"/>
        <xsl:if test="@disabled"> disabled_<xsl:value-of select="@disabled"/></xsl:if>
        <xsl:if test="position() = 1">
          <xsl:text> first</xsl:text>
          <xsl:text> selected</xsl:text> <!-- default to first view visible -->
        </xsl:if>
      </xsl:attribute>

      <xsl:value-of select="@title"/>

      <!-- TODO: this is temporarily visible until the contentchanged missing event on body reappears and enabled toggling again -->
      <ul><xsl:apply-templates select="gs:sub-control" mode="gs_full_view_sub_control"/></ul>
    </li>
  </xsl:template>

  <xsl:template match="gs:sub-control" mode="gs_full_view_sub_control">
    <xsl:variable name="gs_control_id"><xsl:if test="@id"><xsl:value-of select="@id"/></xsl:if><xsl:if test="not(@id)"><xsl:value-of select="translate(@title, ' ', '-')"/></xsl:if></xsl:variable>
    <li>
      <xsl:attribute name="class">
        <xsl:text>f_click_</xsl:text><xsl:value-of select="$gs_control_id"/>
        <xsl:if test="position() = 1"> first</xsl:if>
      </xsl:attribute>

      <xsl:value-of select="@title"/>
    </li>
  </xsl:template>

  <xsl:template match="gs:view" mode="gs_full_view_container">
    <xsl:variable name="gs_view_id"><xsl:if test="@id"><xsl:value-of select="@id"/></xsl:if><xsl:if test="not(@id)"><xsl:value-of select="translate(@title, ' ', '-')"/></xsl:if></xsl:variable>

    <div>
      <xsl:attribute name="class">
        <xsl:text>gs-view-container</xsl:text>
        <xsl:text> view-</xsl:text><xsl:value-of select="$gs_view_id"/><xsl:text>-container</xsl:text>
        <xsl:if test="position() = 1">
          <xsl:text> first</xsl:text>
          <xsl:text> selected</xsl:text> <!-- default to first view visible -->
        </xsl:if>
      </xsl:attribute>
      <xsl:attribute name="style">
        <xsl:if test="not(position() = 1)">display:none;</xsl:if>
      </xsl:attribute>

      <xsl:apply-templates mode="gs_full_view_content"/>
    </div>
  </xsl:template>

  <xsl:template match="gs:view/gs:sub-control" mode="gs_full_view_content">
    <!-- do not render the controls in the content area -->
  </xsl:template>

  <xsl:template match="*" mode="gs_full_view_content">
    <!-- normal direct render of the immediate content -->
    <xsl:apply-templates select="." mode="gs_view_render"/>
  </xsl:template>
</xsl:stylesheet>