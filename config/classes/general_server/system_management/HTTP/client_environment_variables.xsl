<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" name="client_environment_variables" version="1.0">
  <!-- useful client side variables for the standard object:Response/meta:environment
    <object:Response>
      <meta:environment>
        [environment objects]
      </meta:environment>

      <interface:HTMLWebpage>
        [page objects]
      </interface:HTMLWebpage>
    </object:Response>
  -->
  
  <!-- namespaces -->
  <xsl:variable name="NAMESPACE_XXX" select="'http://general_server.org/xmlnamespaces/dummyxsl/2006'"/>
  <xsl:variable name="NAMESPACE_GS" select="'http://general_server.org/xmlnamespaces/general_server/2006'"/>
  <xsl:variable name="NAMESPACE_EXSL" select="'http://exslt.org/common'"/>
  <xsl:variable name="NAMESPACE_CONVERSATION" select="'http://general_server.org/xmlnamespaces/conversation/2006'"/>
  <xsl:variable name="NAMESPACE_XSD" select="'http://www.w3.org/2001/XMLSchema'"/>
  <xsl:variable name="NAMESPACE_HTML" select="'http://www.w3.org/1999/xhtml'"/>
  <xsl:variable name="NAMESPACE_XSL" select="'http://www.w3.org/1999/XSL/Transform'"/>
  <xsl:variable name="NAMESPACE_DYN" select="'http://exslt.org/dynamic'"/>
  <xsl:variable name="NAMESPACE_DATABASE" select="'http://general_server.org/xmlnamespaces/database/2006'"/>
  <xsl:variable name="NAMESPACE_SERVER" select="'http://general_server.org/xmlnamespaces/server/2006'"/>
  <xsl:variable name="NAMESPACE_DEBUG" select="'http://general_server.org/xmlnamespaces/debug/2006'"/>
  <xsl:variable name="NAMESPACE_SERVICE" select="'http://general_server.org/xmlnamespaces/service/2006'"/>
  <xsl:variable name="NAMESPACE_REQUEST" select="'http://general_server.org/xmlnamespaces/request/2006'"/>
  <xsl:variable name="NAMESPACE_RESPONSE" select="'http://general_server.org/xmlnamespaces/response/2006'"/>
  <xsl:variable name="NAMESPACE_RX" select="'http://general_server.org/xmlnamespaces/rx/2006'"/>
  <xsl:variable name="NAMESPACE_REPOSITORY" select="'http://general_server.org/xmlnamespaces/repository/2006'"/>
  <xsl:variable name="NAMESPACE_OBJECT" select="'http://general_server.org/xmlnamespaces/object/2006'"/>
  <xsl:variable name="NAMESPACE_INTERFACE" select="'http://general_server.org/xmlnamespaces/interface/2006'"/>
  <xsl:variable name="NAMESPACE_JAVASCRIPT" select="'http://general_server.org/xmlnamespaces/javascript/2006'"/>
  <xsl:variable name="NAMESPACE_CLASS" select="'http://general_server.org/xmlnamespaces/class/2006'"/>
  <xsl:variable name="NAMESPACE_CSS" select="'http://general_server.org/xmlnamespaces/css/2006'"/>
  <xsl:variable name="NAMESPACE_XMLSECURITY" select="'http://general_server.org/xmlnamespaces/xmlsecurity/2006'"/>
  <xsl:variable name="NAMESPACE_XMLTRANSACTION" select="'http://general_server.org/xmlnamespaces/xmltransaction/2006'"/>
  <xsl:variable name="NAMESPACE_REGEXP" select="'http://exslt.org/regular-expressions'"/>
  <xsl:variable name="NAMESPACE_META" select="'http://general_server.org/xmlnamespaces/meta/2006'"/>
  <xsl:variable name="NAMESPACE_SESSION" select="'http://general_server.org/xmlnamespaces/session/2006'"/>
  <xsl:variable name="NAMESPACE_FLOW" select="'http://exslt.org/flow'"/>
  <xsl:variable name="NAMESPACE_STR" select="'http://exslt.org/strings'"/>
  
  <!-- $gs_response works on the server-side also 
    server-side: the server directly transforms the tmp-node/object:Response 
    clent-side:  the object:Response is the document root
  -->
  <xsl:param name="gs_response_client_side" select="/object:Response" database:path-check="ignore"/>
  <xsl:param name="gs_response_server_side" select="self::object:Response"/>
  <xsl:param name="gs_response" select="$gs_response_client_side|$gs_response_server_side"/>
  <xsl:param name="gs_environment" select="$gs_response/meta:environment"/>

  <!-- environment objects -->
  <xsl:param name="gs_user" select="$gs_environment/object:User"/>
  <xsl:param name="gs_request" select="$gs_environment/object:Request"/>
  <xsl:param name="gs_session" select="$gs_environment/object:Session"/>
  <xsl:param name="gs_website_root" select="$gs_environment/object:Website"/>
  <xsl:param name="gs_context_database" select="$gs_environment/object:Database"/>
  <xsl:param name="gs_resource_server_object" select="$gs_environment/object:LinkedServer"/>

  <!-- aliases -->
  <xsl:param name="db" select="$gs_context_database"/>
  <xsl:param name="website" select="$gs_website_root"/>

  <!-- useful (same as process_http_request.xsl) -->
  <xsl:param name="gs_newline" select="'&#10;'"/>
  <xsl:param name="gs_lowercase" select="'abcdefghijklmnopqrstuvwxyz'"/>
  <xsl:param name="gs_uppercase" select="'ABCDEFGHIJKLMNOPQRSTUVWXYZ'"/>

  <!-- environment elements (same as process_http_request.xsl) -->
  <xsl:param name="gs_request_id" select="string($gs_request/@id)"/>
  <xsl:param name="gs_query_string" select="$gs_request/gs:query-string"/>
  <xsl:param name="gs_form" select="$gs_request/gs:form"/>
  <xsl:param name="gs_configuration_flags" select="string($gs_query_string/@configuration-flags)"/>
  <xsl:param name="gs_is_logged_in" select="boolean($gs_user)"/>
  <xsl:param name="gs_isadministrator" select="boolean($gs_user/gs:groups/object:Group[xml:id='grp_1'])"/>
  <xsl:param name="gs_resource_server" select="$gs_resource_server_object/@uri"/>

  <xsl:param name="gs_request_url" select="$gs_request/gs:url"/>
  <xsl:param name="gs_request_target" select="$gs_response/gs:data/*"/>
</xsl:stylesheet>
