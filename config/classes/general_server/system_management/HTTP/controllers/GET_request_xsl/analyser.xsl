<xsl:stylesheet response:server-side-only="true" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:response="http://general_server.org/xmlnamespaces/response/2006" xmlns:meta="http://general_server.org/xmlnamespaces/meta/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:request="http://general_server.org/xmlnamespaces/request/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:dyn="http://exslt.org/dynamic" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" name="analyser" version="1.0" extension-element-prefixes="dyn database debug">
  <!-- input is a valid parsed XML document
    THIS IS NOT the main document, so ~Class and id() lookups will fail
    database:stuff() will still work
    <?xml processing instruction are copied through
  -->
  <!-- TODO: these are direct injected with LibXML, would be nicer to declare them... -->
  <!-- xsl:param name="gs_user"   / -->
  <!-- xsl:param name="gs_request"/ -->
  <!-- xsl:param name="gs_session"/ -->
  <!-- xsl:param name="gs_service"/ -->
  <!-- xsl:param name="gs_message_interpretation"/ -->
  <!-- xsl:param name="gs_primary_stylesheet"/ -->

  <xsl:include xpath="../system-layout"/>
  <xsl:include xpath="../request-layout"/>
  <xsl:include xpath="../virtualhosts-system-layout"/>   <!-- system-layout relative to this request -->
  <xsl:include xpath="../virtualhosts-stage"/>           <!-- dev, eval, live: depends on $gs_website_root also -->
  <xsl:include xpath="../virtualhosts-system-layout"/>
  
  <xsl:output method="xml" encoding="UTF-8" omit-xml-declaration="yes" indent="yes"/>

  <xsl:template match="/">
    <xsl:apply-templates select="xsl:stylesheet" mode="gs_checks"/>
    <xsl:copy-of select="node()"/>
  </xsl:template>

  <xsl:template match="xsl:stylesheet" mode="gs_checks">
    <xsl:if test="$gs_stage_dev">
      <xsl:if test="xsl:stylesheet//*/@xsl:*"><debug:server-message output="@xsl:* attributes still in the output doc" type="warning"/></xsl:if>
      <xsl:if test="xsl:stylesheet//xsl:else"><debug:server-message output="xsl:else still in the output doc" type="warning"/></xsl:if>
      <xsl:if test="xsl:stylesheet//xsl:*/@*[contains(., 'database:')]"><debug:server-message output="database namespace still in the output doc" type="warning"/></xsl:if>
      <xsl:if test="xsl:stylesheet//database:*"><debug:server-message output="database namespace still in the output doc" type="warning"/></xsl:if>
      <xsl:if test="xsl:stylesheet//debug:*"><debug:server-message output="debug namespace still in the output doc" type="warning"/></xsl:if>
    </xsl:if>
  </xsl:template>    
</xsl:stylesheet>
