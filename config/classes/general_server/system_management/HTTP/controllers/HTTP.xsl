<xsl:stylesheet response:server-side-only="true" xmlns:date="http://exslt.org/dates-and-times" xmlns:exsl="http://exslt.org/common" xmlns="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" name="HTTP" extension-element-prefixes="server debug" version="1.0">
  <xsl:include xpath="~XSLStylesheet/HTTP_processing"/>

  <xsl:template match="/|@*|node()" mode="gs_HTTP_render">
    <!-- xsl:included classes override this output.
      Javascript, CSSStylesheet and XSLStylesheet

      generic serving of an remaining un-managed elements, e.g. AJAX call for object:User
      with the standard system stylesheet Class__Response/envelope_loader
        interface:dynamic @data=$gs_request_target @...
      using the Class__XSLStylesheet HTTP processor
    -->
    <debug:server-message if="$gs_debug_url_steps" output="object:Request [{$gs_request/gs:message_type}] =&gt; {$gs_request/gs:url} =&gt; ~HTTP default Class__Response/envelope_loader for [{name($gs_request_target)}]"/>
    <xsl:variable name="gs_stylesheet" select="$gs_website_classes/self::class:Response/envelope_loader"/>
    <!-- checks -->
    <xsl:if test="not($gs_website_classes)"><debug:server-message output="$gs_website_classes is empty"/></xsl:if>
    <xsl:else-if test="not($gs_website_classes/self::class:Response)"><debug:server-message output="Response class not found in $gs_stylesheet_server_side_classes"/></xsl:else-if>
    <xsl:else-if test="not($gs_stylesheet)"><debug:server-message output="envelope_loader not found for HTTP XSLT"/></xsl:else-if>

    <xsl:if test="$gs_stylesheet"><xsl:apply-templates select="$gs_stylesheet" mode="gs_HTTP_render_client_server_switch"/></xsl:if>
  </xsl:template>
</xsl:stylesheet>
