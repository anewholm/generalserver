<xsl:stylesheet xmlns:date="http://exslt.org/dates-and-times" xmlns:exsl="http://exslt.org/common" xmlns="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" response:server-side-only="true" name="code_render" extension-element-prefixes="server database debug" version="1.0">
  <xsl:template match="xsl:include|xsl:import" mode="gs_code_render">
    <!-- xsl:include can be expanded during gs_interface_render
        DXSL MUST be server-side resolved when carrying out a server-side XSLT because the xsl:include process cannot run the DXSL generate
        ?server-side-includes=yes (AJAX uses server-side-includes=yes) clients sometimes can't do it, e.g. Google Chrome
        xsl:include @meta:server-side-include=yes stamped on the xsl:include element
      TODO: processing uses the xsl:include @href. what about @xpath etc.
      NOTE: .dxsl extension mime-type syntax still resolves to the correct @xpath location
      that is why we have not used a new augmented syntax !<dxsl location> or {<dxsl location>}
      DXSL is an APPLICATION level concept. the XML library xsl:include does not understand it
      note that the gs_server_side_includes can be forced by parameter
    -->
    <xsl:param name="gs_DXSL_extension" select="str:ends-with(@href, '.dxsl')"/>
    <xsl:param name="gs_XSL_extension" select="str:ends-with(@href, '.xsl')"/>
    <xsl:param name="gs_is_DXSL" select="$gs_DXSL_extension"/>
    <xsl:param name="gs_server_side_includes" select="str:boolean($gs_query_string/@gs:server-side-includes) or $gs_DXSL_extension or str:boolean(@meta:server-side-include)"/>

    <xsl:if test="@href and not($gs_XSL_extension) and not($gs_DXSL_extension)">
      <debug:server-message type="warning" output="xsl:include presumably for an XSL stylesheet needs to end with .xsl or .dxsl to trigger the appropriate MessageInterpretation"/>
    </xsl:if>

    <xsl:if test="not($gs_server_side_includes)">
      <!-- normal xsl:include output -->
      <xsl:copy-of select="."/>
    </xsl:if>

    <xsl:if test="$gs_server_side_includes">
      <!-- location of xsl:include same as:
        C++ clearMyCachedStylesheetsRecursive()
        C++ translateIncludes() @href => @xpath
        LibXml2: xsl:include processing (of @xpath) during server side compilation
        map_url_to_stylesheet_model.xsl REST based client xsl:include requests
        AJAX client_stylesheet_transform.xsl xsl:include include directly in main xsl:stylesheet request
      -->
      <xsl:variable name="gs_xpath_to_include">
        <xsl:choose>
          <xsl:when test="@xpath"><xsl:value-of select="@xpath"/></xsl:when>
          <xsl:when test="@href"><xsl:value-of select="repository:filesystempath-to-XPath(@href)"/></xsl:when>
          <xsl:otherwise><debug:server-message output="xsl:include without @xpath or @href" type="warning"/></xsl:otherwise>
        </xsl:choose>
      </xsl:variable>

      <xsl:variable name="gs_included_stylesheet" select="dyn:map(../.., $gs_xpath_to_include)" xsl:error-policy="continue"/>
      <xsl:if test="$gs_included_stylesheet">
        <xsl:if test="$gs_is_DXSL">
          <!-- transformative -->
          <xsl:comment>[<xsl:value-of select="@href"/>] DXSL</xsl:comment>
          <database:transform select="$gs_included_stylesheet" stylesheet="$gs_included_stylesheet" interface-mode="gs_generate_XSL"/>
          <xsl:comment>/[<xsl:value-of select="@href"/>]</xsl:comment>
        </xsl:if>
        <xsl:if test="not($gs_is_DXSL)">
          <!-- static recursive -->
          <xsl:comment>[<xsl:value-of select="@href"/>] has been auto-expanded</xsl:comment>
          <xsl:apply-templates select="$gs_included_stylesheet/xsl:*" mode="gs_interface_render"/>
          <xsl:comment>/[<xsl:value-of select="@href"/>]</xsl:comment>
        </xsl:if>
      </xsl:if>
      <xsl:if test="not($gs_included_stylesheet)">
        <debug:server-message output="include not found [{$gs_xpath_to_include}] server-side" type="error"/>
      </xsl:if>
    </xsl:if>
  </xsl:template>
</xsl:stylesheet>