<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" response:server-side-only="true" name="inheritance_render" version="1.0" extension-element-prefixes="flow database str debug request regexp repository">
  <!-- ALL tech inheritance handled by respective Class
    including class:* level templates, traversal and analysis

    xsl:template @match=class:* @mode=gs_render_inheritance_<tech>_<mode>
    e.g.
      xsl:template @match=class:* @mode=gs_render_inheritance_xschema_compiled
      xsl:template @match=class:* @mode=gs_render_inheritance_javascript
      xsl:template @match=class:* @mode=gs_render_inheritance_xsl_standalone
  -->
  <xsl:include xpath="~XSchema/inheritance_render"/>
  <xsl:include xpath="~XSLStylesheet/inheritance_render"/>
  <xsl:include xpath="~JavaScript/inheritance_render"/>
  <xsl:include xpath="~CSSStylesheet/inheritance_render"/>
</xsl:stylesheet>