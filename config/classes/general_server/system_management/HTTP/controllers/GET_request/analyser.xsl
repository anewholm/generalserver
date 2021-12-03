<xsl:stylesheet response:server-side-only="true" xmlns:meta="http://general_server.org/xmlnamespaces/meta/2006" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:response="http://general_server.org/xmlnamespaces/response/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:request="http://general_server.org/xmlnamespaces/request/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:dyn="http://exslt.org/dynamic" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" name="analyser" version="1.0" extension-element-prefixes="dyn database debug">
  <!-- input is a valid parsed XML document
    THIS IS NOT the main document, so ~Class and id() lookups will fail
    database:stuff() will still work
    
    <?xml processing instruction are copied through
    [response:<string tokens>] will be processed also afterwards by the server
    e.g. Content-Length: [response:token-content-length] NOT processed here

    response:* is NOT an element-extension-prefix because it must be xsl:copy-of in to this analyser
    <response:remove> removed here
    <response:* is processed here
  -->
  <!-- TODO: these are direct injected with LibXML, would be nicer to declare them... -->
  <!-- xsl:param name="gs_user"   / -->
  <!-- xsl:param name="gs_request"/ -->
  <!-- xsl:param name="gs_session"/ -->
  <!-- xsl:param name="gs_service"/ -->
  <!-- xsl:param name="gs_message_interpretation"/ -->
  <!-- xsl:param name="gs_primary_stylesheet"/ -->

  <xsl:output method="xml" encoding="UTF-8" omit-xml-declaration="yes" indent="yes"/>

  <!-- TODO: make the $gs_response configurable with a @class-elements-parent=<xpath> -->
  <xsl:param name="gs_response" select="descendant::object:Response[1]"/>
  <xsl:param name="gs_response_data" select="$gs_response/gs:data[1]"/>
  <xsl:param name="gs_data_classes_and_bases" select="database:classes($gs_response/descendant-or-self::*, true())"/>
  
  <xsl:template match="/">
    <xsl:apply-templates/>
  </xsl:template>
  
  <xsl:template match="processing-instruction()">
    <xsl:copy-of select="."/>
  </xsl:template>
  
  <xsl:template match="*">
    <xsl:copy>
      <xsl:apply-templates select="@*"/>
      <xsl:apply-templates/>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="@*">
    <xsl:copy-of select="."/>
  </xsl:template>

  <!-- #################################### response processing #################################### -->
  <xsl:template match="@response:*">
    <xsl:attribute name="{name()}">
      <xsl:value-of select="str:dynamic(.)"/>
    </xsl:attribute>
  </xsl:template>

  <xsl:template match="@response:classes-and-bases">
    <!-- Javascript framework and classes are also needed
      JS and CSS responses will xsl:sort the code by inheritance-order
      true() = bases also 
    -->
      
    <xsl:attribute name="response:classes-and-bases">
      <xsl:if test=". = '[from-data]' or . = ''">
        <!-- THIS IS NOT the main document: ~Class and id('Class__IDE') will fail -->
        <xsl:text>~Class|</xsl:text>
        <xsl:value-of select="str:list($gs_data_classes_and_bases, '|', 'xpath()')"/>
      </xsl:if>
      <xsl:else><xsl:value-of select="str:dynamic(.)"/></xsl:else>
    </xsl:attribute>
  </xsl:template>

  <xsl:template match="response:remove">
    <!-- remove here so that top level output is formatted with new lines -->
    <xsl:apply-templates/>
  </xsl:template>
  
  <xsl:template match="response:token">
    <xsl:value-of select="str:dynamic(@select)"/>
  </xsl:template>

  <xsl:template match="response:comment">
    <xsl:comment>
      <xsl:value-of select="str:dynamic(@select)"/>
      <xsl:apply-templates/>
    </xsl:comment>
  </xsl:template>

  <xsl:template match="response:xml-decl">
    <xsl:processing-instruction name="xml">
      <xsl:attribute name="version">1.0</xsl:attribute>
      <xsl:attribute name="encoding">utf-8</xsl:attribute>
    </xsl:processing-instruction>
  </xsl:template>
  
  <xsl:template match="response:xml-stylesheet">
    <xsl:param name="gs_href" select="str:dynamic(@href)"/>
    
    <xsl:if test="not($gs_response)"><xsl:comment>$gs_response not valid in response:xml-stylesheet analysis</xsl:comment></xsl:if>
    <xsl:else-if test="not($gs_response_data)"><xsl:comment>$gs_response_data not valid in response:xml-stylesheet analysis</xsl:comment></xsl:else-if>
    
    <xsl:processing-instruction name="xml-stylesheet">
      <xsl:attribute name="type">text/xsl</xsl:attribute>
      <xsl:attribute name="charset">utf-8</xsl:attribute>
      <xsl:attribute name="href">
        <xsl:value-of select="$gs_href"/>
        <xsl:if test="contains($gs_href, '?')">&amp;</xsl:if>
        <xsl:else>?</xsl:else>
        
        <xsl:text>auto-add=true</xsl:text>
        
        <!-- OFF at the moment because we want to cache the stylesheet on the client -->
        <xsl:if test="str:boolean(@response:append-classes, true())">
          <xsl:text>&amp;classes=</xsl:text>
          <xsl:value-of select="str:list($gs_data_classes_and_bases, '|', 'xpath()')"/>
        </xsl:if>
      </xsl:attribute>
    </xsl:processing-instruction>
  </xsl:template>
</xsl:stylesheet>
