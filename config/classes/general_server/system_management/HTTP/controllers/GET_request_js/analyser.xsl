<xsl:stylesheet response:server-side-only="true" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:regexp="http://exslt.org/regular-expressions" xmlns:response="http://general_server.org/xmlnamespaces/response/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:request="http://general_server.org/xmlnamespaces/request/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:dyn="http://exslt.org/dynamic" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" name="analyser" version="1.0" extension-element-prefixes="dyn database debug">
  <!-- input is a valid parsed XML document
    THIS IS NOT the main document, so ~Class and id() lookups will fail
    database:stuff() will still work
    
    <?xml processing instruction are copied through
    [response:<string tokens>] will be processed also afterwards by the server
    e.g. Content-Length: [response:token-content-length] NOT processed here
  -->
  <!-- TODO: these are direct injected with LibXML, would be nicer to declare them... -->
  <!-- xsl:param name="gs_user"   / -->
  <!-- xsl:param name="gs_request"/ -->
  <!-- xsl:param name="gs_session"/ -->
  <!-- xsl:param name="gs_service"/ -->
  <!-- xsl:param name="gs_message_interpretation"/ -->
  <!-- xsl:param name="gs_primary_stylesheet"/ -->

  <xsl:output method="text" encoding="UTF-8" omit-xml-declaration="yes" indent="no"/>

  <xsl:param name="gs_stage_live" select="/*[1]/@stage = 'live'"/>

  <xsl:template match="/">
    <!-- TODO: this works on the headers as well at the moment!!!! -->
    <xsl:if test="$gs_stage_live">
      <!-- first pass -->
      <xsl:variable name="gs_js_no_comments" select="'^\s*[/][/].*$'"/>
      <xsl:variable name="gs_js_no_STAGEDEVlines" select="'^.*[/][/]STAGE-DEV.*$'"/>
      <xsl:variable name="gs_first_pass" select="regexp:replace(., concat($gs_js_no_comments, '|', $gs_js_no_STAGEDEVlines), 'gmn')"/>

      <!-- second pass -->
      <xsl:variable name="gs_js_no_blanklines" select="'^\s*$'"/>
      <xsl:value-of select="regexp:replace($gs_first_pass, $gs_js_no_blanklines, 'gmn')"/>
    </xsl:if>
    <xsl:else>
      <xsl:apply-templates/>
    </xsl:else>
  </xsl:template>
  
  <xsl:template match="response:type"/>
</xsl:stylesheet>
