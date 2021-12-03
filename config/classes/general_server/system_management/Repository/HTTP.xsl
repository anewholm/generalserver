<xsl:stylesheet response:server-side-only="true" xmlns:date="http://exslt.org/dates-and-times" xmlns:exsl="http://exslt.org/common" xmlns="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" name="HTTP" extension-element-prefixes="server debug" version="1.0">
  <xsl:template match="repository:*" mode="gs_HTTP_render">
    <!-- xsl:included classes override this output.
      Javascript, CSSStylesheet and XSLStylesheet

      generic serving of an remaining un-managed elements, e.g. interface:IDE
      with the standard system stylesheet Class__Response/response_loader
        interface:dynamic @data=$gs_request_target @...
      using the Class__XSLStylesheet HTTP processor
    -->
    <debug:server-message if="$gs_debug_url_steps" output="repository:* [{$gs_request/gs:message_type}] =&gt; {$gs_request/gs:url} =&gt; ~Repository using Class__Repository_response_loader for [{name($gs_request_target)}]"/>
    <xsl:apply-templates select="/config/classes/general_server/system_management/Repository/envelope_loader" mode="gs_HTTP_render"/>
  </xsl:template>
</xsl:stylesheet>
