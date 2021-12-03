<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" name="view" version="1.0">
  <xsl:template match="object:User" mode="securityowner" meta:interface-template="yes">
    <xsl:param name="gs_html_identifier_class"/>

    <div class="{$gs_html_identifier_class} details">
      <img src="{$gs_resource_server}/resources/shared/images/icons/{@tree-icon}.png"/>
      <div class="gs-name"><xsl:value-of select="@name"/></div>
      <div class="gs-security-relationship">owner</div>
    </div>
  </xsl:template>

  <xsl:template match="object:User" mode="default_content">
    <!-- default interface leads to render default_content -->
    <img src="{$gs_resource_server}/resources/shared/images/avatar_blank_gray.jpg"/>

    <div class="full-name">
      <xsl:apply-templates select="." mode="full_title"/>
      <xsl:value-of select="gs:name"/>
    </div>
  </xsl:template>

  <xsl:template match="object:User" mode="full_title">
    <xsl:value-of select="@name"/>
  </xsl:template>

  <xsl:template match="object:User" mode="controls" meta:interface-template="yes">
    <xsl:param name="gs_html_identifier_class"/>

    <div class="{$gs_html_identifier_class}">
      <div class="gs-float-left gs-clickable gs_dropdown gs-toggle-click-exclusive-global">
        <div class="gs-menu-dropdown-image"><xsl:apply-templates select="@name"/></div>
        <ul>
          <li class="f_click_profile">profile</li>
          <li class="f_click_logout">logout</li>
        </ul>
      </div>

      <!-- icon always present -->
      <div class="gs-float-right">
        <div class="gs-menu-dropdown-image">notifications (0)</div>
        <xsl:apply-templates select="repository:notifications" mode="controls"/>
      </div>

      <!-- icon present on demand. NOTE: this is the object:User repository:errors, not the session one -->
      <xsl:apply-templates select="repository:errors" mode="controls"/>

      <!-- icon present on demand. NOTE: this is the object:User repository:errors, not the session one -->
      <xsl:apply-templates select="gs:debug" mode="controls"/>
    </div>
  </xsl:template>

  <!-- ############################################### errors ############################################### -->
  <xsl:template match="object:User/repository:errors" mode="controls">
    <div class="gs-float-right f_mouseover_show gs-toggle-hover-exclusive-global">
      <div class="gs-menu-dropdown-image">errors (<xsl:value-of select="count(*)"/>)</div>
      <ul id="gs_errors">
        <xsl:apply-templates select="*" mode="controls"/>
      </ul>
    </div>
  </xsl:template>

  <xsl:template match="object:User/repository:errors/gs:error" mode="controls">
    <li>error</li>
  </xsl:template>

  <!-- ############################################### debug ############################################### -->
  <xsl:template match="gs:debug" mode="controls">
    <div class="gs-float-right gs-clickable gs_dropdown gs-toggle-click-exclusive-global">
      <div class="gs-menu-dropdown-image">debug (<xsl:value-of select="count(gs:breakpoints/*)"/>)</div>
      <xsl:apply-templates select="*" mode="controls"/>
    </div>
  </xsl:template>

  <xsl:template match="gs:debug/gs:breakpoints" mode="controls">
    <ul id="gs_breakpoints">
      <xsl:apply-templates select="*" mode="controls"/>
    </ul>
  </xsl:template>

  <xsl:template match="gs:breakpoints/*" mode="controls">
    <li><xsl:value-of select="local-name()"/> (<xsl:value-of select="@xml:id"/>)</li>
  </xsl:template>

  <!-- failsafe, because we might hit infinite recusrsion -->
  <xsl:template match="gs:breakpoints/*/*"/>
  <xsl:template match="gs:breakpoints/*/*"/>
  <xsl:template match="gs:breakpoints/*/*" mode="list">
    <xsl:param name="gs_event_functions"/>
  </xsl:template>
</xsl:stylesheet>