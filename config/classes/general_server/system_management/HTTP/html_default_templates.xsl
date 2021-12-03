<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" name="html_default_templates" version="1.0">
  <xsl:template match="fake-template-source-warning">
    <script>alert('if you are seeing this it is because the browser has rendered the XSL and is now processing the XSL as HTML. it will also request all the html:links in the XSL templates with variables and throw server errors.\n\ntry using view-source:url');</script>
  </xsl:template>

  <xsl:template match="/">
    <xsl:param name="gs_interface_mode"/>

    <xsl:apply-templates select="*" mode="gs_view_render">
      <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="*">
    <xsl:param name="gs_interface_mode"/>

    <xsl:if test="@gs:interface-mode">
      <div class="gs-warning">
        modeless template called on [<xsl:value-of select="name()"/>]
        with @gs:interface-mode [<xsl:value-of select="@gs:interface-mode"/>]
        should have been caught by @gs:interface-mode xsl:templates
        are they not included?
      </div>
    </xsl:if>

    <xsl:if test="not(@gs:interface-mode)">
      <div class="gs-warning">
        element [<xsl:value-of select="name()"/>] called without mode.
        this means it is not inherited from DatabaseElement which has a modeless match template for everything:
          <b>xsl:template @match=*</b>
        template order may be compromised
      </div>
    </xsl:if>
  </xsl:template>

  <!-- we don't do this because we want to render text from interfaces and it would not show -->
  <!-- xsl:template match="text()" mode="gs_view_render"/ -->
  
  <!-- ######################## the standard built-in template rules for reference ########################
    these are commented here for the programmers reference
  <xsl:template match="text()|@*">
    <xsl:value-of select="."/>
  </xsl:template>

  <xsl:template match="processing-instruction()|comment( )"/>

  <xsl:template match="*|/">
    <xsl:apply-templates/>
  </xsl:template -->
</xsl:stylesheet>
