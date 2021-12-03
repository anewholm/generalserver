<xsl:stylesheet response:server-side-only="true" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" name="process_session" version="1.0" extension-element-prefixes="dyn database server debug response">
  <!-- request: session vars, NOTE: $session has been provided by the server -->
  <xsl:template match="object:Request/gs:cookies" mode="process_session">
    <!-- always set the cookie -->
    <xsl:if test="not($gs_session)"><debug:server-message output="session missing" type="warning"/></xsl:if>
    <xsl:if test="not($gs_session/@guid)"><debug:server-message output="GSSESSID blank" type="warning"/></xsl:if>
    <response:set-header header="Set-Cookie" value="GSSESSID={$gs_session/@guid}; path=/"/>
  </xsl:template>
</xsl:stylesheet>
