<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:conversation="http://general_server.org/xmlnamespaces/conversation/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:exsl="http://exslt.org/common" xmlns:class="http://general_server.org/xmlnamespaces/class/2006" xmlns:user="http://general_server.org/xmlnamespaces/user/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" response:server-side-only="true" name="interface_render" version="1.0" extension-element-prefixes="dyn str regexp database server conversation request debug exsl">
  <xsl:template match="interface:dynamic" mode="gs_interface_render">
    <!-- interface:dynamic is a restricted database:query
      it belongs to HTTP website delivery
      requires $gs_website_root
      uses a default transform
        e.g. interface:dynamic @interface-mode=children
      @transform      => $gs_stylesheet_server_side_classes
      @interface-mode => database:query @interface-mode
      
      a blank @interface will cause a default gs_interface_render
      TODO: move this to ~HTTP?
    -->
    <xsl:param name="gs_outer_interface_mode"/> <!-- for @interface-mode-condition -->
    <xsl:param name="gs_debug" select="str:boolean(str:dynamic(@debug)) or $gs_debug_interface_render"/>
    
    <xsl:variable name="gs_interface_mode" select="str:dynamic(@interface-mode)"/>

    <debug:server-message if="$gs_debug" output="interface:dynamic [{@xml:id}] using interface-mode ({$gs_interface_mode})"/>
    <xsl:apply-templates select="." mode="gs_interface_render_database_query">
      <xsl:with-param name="gs_outer_interface_mode" select="$gs_outer_interface_mode"/>
      <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      <xsl:with-param name="gs_debug" select="$gs_debug"/>
    </xsl:apply-templates>
  </xsl:template>
</xsl:stylesheet>