<xsl:stylesheet response:server-side-only="true" xmlns:date="http://exslt.org/dates-and-times" xmlns:exsl="http://exslt.org/common" xmlns:flow="http://exslt.org/flow" xmlns:meta="http://general_server.org/xmlnamespaces/meta/2006" xmlns:session="http://general_server.org/xmlnamespaces/session/2006" xmlns:response="http://general_server.org/xmlnamespaces/response/2006" xmlns="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:css="http://general_server.org/xmlnamespaces/css/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:user="http://general_server.org/xmlnamespaces/user/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:dyn="http://exslt.org/dynamic" name="controller" controller="true" version="1.0" extension-element-prefixes="dyn debug conversation flow session database">
  <xsl:output method="xml" encoding="utf-8" omit-xml-declaration="yes" indent="yes" cdata-section-elements="html:script javascript:raw css:raw javascript:method javascript:init javascript:static-method javascript:global-init"/>

  <xsl:include xpath="../process_http_request"/>
  <xsl:include xpath="help"/>

  <!-- custom interface_render objects
    these are fixed system data renders of specific dynamic output things
      database:query
      class:xschema-*
    use data-queries to include DYNAMIC queries for custom classes e.g.
      <object:Session @interface-mode=controls>
        ++ <class:xschema-* login.xsd>
  -->
  <xsl:include xpath="~AJAXHTMLLoader/interface_render"/>
  <xsl:include xpath="~Interface/interface_render"/>
  <xsl:include xpath="~XSchema/interface_render"/>
  <xsl:include xpath="~Response/interface_render"/>
  <xsl:include xpath="~ResponseNamespace/interface_render"/>

  <xsl:template match="/|@*|node()" mode="gs_HTTP_render">
    <!-- uses meta:query API xpath commands -> interface_render.xsl -> gs_interface_render -> XML -->
    <xsl:variable name="gs_data_query" select="$gs_request/database:query"/>

    <!-- login -->
    <xsl:if test="string($gs_data_query/@username)">
      <xsl:if test="str:boolean($gs_data_query/@session-login)">
        <conversation:set-security-context username="{$gs_data_query/@username}" password="{$gs_data_query/@password}"/>
      </xsl:if>
      <xsl:else>
        <database:set-security-context username="{$gs_data_query/@username}" password="{$gs_data_query/@password}"/>
      </xsl:else>
    </xsl:if>
    <xsl:variable name="gs_user" select="conversation:user-node()"/>
    <xsl:if test="$gs_user"><debug:server-message output="  Logged In as [{$gs_user/@name}]"/></xsl:if>
    
    <xsl:choose>
      <xsl:when test="starts-with($gs_request/gs:url, 'help')">
        <!-- /api/help pages: server-side XSLT -->
        <xsl:apply-templates select="$gs_request/gs:HTTP" mode="basic_http_headers"/>
        <xsl:apply-templates select="$gs_request/gs:cookies" mode="process_session"/>
        <xsl:apply-templates select="." mode="basic_http_headers_content_type">
          <xsl:with-param name="gs_specific_extension" select="'html'"/>
        </xsl:apply-templates>

        <xsl:apply-templates select="/config/classes/general_server/system_management/HTTP/controllers/GET_request_api/controller" mode="gs_API_help"/>
      </xsl:when>
      
      <xsl:otherwise>
        <!-- XML response HTTP headers -->
        <xsl:apply-templates select="$gs_request/gs:HTTP" mode="basic_http_headers"/>
        <xsl:apply-templates select="$gs_request/gs:cookies" mode="process_session"/>
        <xsl:apply-templates select="." mode="basic_http_headers_content_type"/>

        <!-- this is a non conditional client side explicit-stylesheet -->
        <xsl:apply-templates select="." mode="gs_xml_decl"/>
        <xsl:if test="$gs_data_query/@explicit-stylesheet">
          <xsl:apply-templates select="." mode="gs_xml_stylesheet_statement">
            <xsl:with-param name="gs_href" select="$gs_data_query/@explicit-stylesheet"/>
          </xsl:apply-templates>
        </xsl:if>

        <xsl:choose interface:name="schema">
          <!-- checks -->
          <xsl:when test="starts-with($gs_request/gs:url, 'server/')">
            <object:Response><xsl:comment>********** WARNING: /api/server/.* looks like a server request. however that must be a POST, not a GET</xsl:comment></object:Response>
          </xsl:when>
          <xsl:when test="$gs_data_query/@hardlink-output = 'placeholder' and $gs_data_query/@node-mask and not($gs_data_query/@node-mask = 'false()')">
            <object:Response><xsl:comment>********** WARNING: node-masks will generally prevent the placeholders from appearing.</xsl:comment></object:Response>
          </xsl:when>
          
          <!-- schemas -->
          <xsl:when test="$gs_data_query/@schema = 'query'" interface:option="query">
            <xsl:apply-templates select="." mode="gs_render_API_data">
              <xsl:with-param name="gs_data_query" select="$gs_data_query"/>
            </xsl:apply-templates>
          </xsl:when>
          
          <xsl:when test="$gs_data_query/@schema = 'data'" interface:option="data" interface:default="yes">
            <gs:data source="API" controller="{$gs_data_query/@controller}">
              <xsl:apply-templates select="." mode="gs_render_API_data">
                <xsl:with-param name="gs_data_query" select="$gs_data_query"/>
              </xsl:apply-templates>
            </gs:data>
          </xsl:when>

          <xsl:otherwise interface:option="default">
            <object:Response response:include-all="on">
              <xsl:comment>see /api/help for the API form</xsl:comment>
              
              <xsl:apply-templates select="/config/classes/general_server/system_management/Response/interfaces/webpage-response/meta:environment" mode="gs_interface_render"/>

              <!-- use the meta:data container to conform to the main styelsheet model output -->
              <gs:data source="API" data="{$gs_data_query/@data}" controller="{$gs_data_query/@controller}">
                <xsl:apply-templates select="." mode="gs_render_API_data">
                  <xsl:with-param name="gs_data_query" select="$gs_data_query"/>
                </xsl:apply-templates>
              </gs:data>
            </object:Response>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
  
  <xsl:template match="*" mode="gs_render_API_data">
    <!-- interpret the database:query
      TODO: ~AJAXHTMLLoader/interface_render will run gs_interface_render on the results!!!
      including any sub-database:query results
    -->
    <xsl:param name="gs_data_query" select="$gs_request/database:query"/>
    
    <xsl:choose interface:name="controller">
      <xsl:when test="$gs_data_query/@controller = 'apply-templates'" interface:option="apply-templates">
        <xsl:apply-templates select="dyn:evaluate($gs_data_query/@data)"/>
      </xsl:when>
      <xsl:when test="$gs_data_query/@controller = 'copy-of'" interface:option="copy-of">
        <xsl:copy-of select="dyn:evaluate($gs_data_query/@data)"/>
      </xsl:when>
      <xsl:when test="$gs_data_query/@controller = 'database-query'" interface:option="database-query">
        <!-- TODO: not compelete -->
      </xsl:when>
      <xsl:otherwise interface:option="default"> 
        <!-- interface-render -->
        <xsl:apply-templates select="$gs_data_query" mode="gs_interface_render"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>
</xsl:stylesheet>
