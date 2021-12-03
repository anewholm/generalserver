<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" name="inheritance_render" response:server-side-only="true" version="1.0" extension-element-prefixes="flow database str debug request regexp repository">
  <!--
    THIS IS a NOT_CURRENTLY_USED placeholder
    database:class-xjavascript([string className|node class:*])

    JavaScript technology does not need compilation. objects have their prototype chain
      Manager.prototype = new Person();
    see the JavaScript Class
  -->
  <xsl:template match="class:*" mode="gs_inheritance_render_javascript_compiled">
    <debug:NOT_CURRENTLY_USED because="NEVER_USED JavaScript has its own client side multiple-inheritance implementation"/>
  </xsl:template>
</xsl:stylesheet>