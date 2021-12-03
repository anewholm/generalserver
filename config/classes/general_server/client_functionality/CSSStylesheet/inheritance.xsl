<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" name="inheritance_render" response:server-side-only="true" version="1.0" extension-element-prefixes="flow database str debug request regexp repository">
  <!--
    THIS IS a NOT_CURRENTLY_USED placeholder
    database:class-xcss([string className|node class:*])

    CSS technology does not need compilation. stylesheets have their independent name
    and they are chained stamped on classes:
      <div class="Class__Manager CSS__Manager CSS__Person CSS__User">
    see the CSSStylesheet Class
  -->
  <xsl:template match="class:*" mode="gs_inheritance_render_css">
    <debug:NOT_CURRENTLY_USED because="NEVER_USED CSS has its own client side multiple-inheritance implementation"/>
  </xsl:template>
</xsl:stylesheet>