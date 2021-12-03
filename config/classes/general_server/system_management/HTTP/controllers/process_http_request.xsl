<xsl:stylesheet response:server-side-only="true" xmlns:xxx="http://general_server.org/xmlnamespaces/dummyxsl/2006" xmlns:str="http://exslt.org/strings" xmlns:exsl="http://exslt.org/common" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns:flow="http://exslt.org/flow" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" name="process_http_request" version="1.0" extension-element-prefixes="dyn database server str flow debug">
  <!--
    ######################################## inputted parameters from Request.cpp ########################################
    many of these variables have equivalent client-side variables in: client_stylesheet_variables.xml
    which are copied in to main client transform sheets
  -->
  <xsl:param name="gs_is_first_request_on_connection"/>
  <!-- TODO: these are direct injected with LibXML, would be nicer to declare them... -->
  <!-- xsl:param name="gs_user"   / -->
  <!-- xsl:param name="gs_request"/ -->
  <!-- xsl:param name="gs_session"/ -->
  <!-- xsl:param name="gs_service"/ -->
  <!-- xsl:param name="gs_message_interpretation"/ -->
  <!-- xsl:param name="gs_primary_stylesheet"/ -->

  <xsl:namespace-alias result-prefix="xsl" stylesheet-prefix="xxx"/>

  <xsl:include xpath="system-layout"/>
  <xsl:include xpath="constants"/>
  <xsl:include xpath="request-layout"/>
  <xsl:include xpath="user-context"/>
  <xsl:include xpath="default_templates"/>

  <!-- bootstrap (dependent on previous includes above) -->
  <xsl:include xpath="virtualhosts-system-layout"/>   <!-- system-layout relative to this request -->
  <xsl:include xpath="virtualhosts-stage"/>           <!-- dev, eval, live: depends on $gs_website_root also -->
  <xsl:include xpath="virtualhosts-targets"/>         <!-- node requested, resource server, type requested, etc. -->
  <xsl:include xpath="virtualhosts-classes"/>         <!-- PERFORMANCE: $gs_classes-* -->
  <xsl:include xpath="standard_bootstrap"/>           <!-- templates for standard bootstrap -->
  <xsl:include xpath="xpath_attribute_to_hardlink"/>  <!-- query-string interpretation -->
  <!-- HTTP.xsl standard controller will output the object within an object:Response/interface:HTMLWebpage
    EACH MI has its own custom loaded HTTP handlers, for example:
      .xsl => xsl:stylesheet has its own database:querys that get enveloped in an object:Response/interface:HTMLWebpage
      .css => ~CSSStylesheet/HTTP standalone_contents
      .js  => ~JavaScript/HTTP    standalone_contents
    the basic GET_request MI loads only:
      ~HTTP/HTTP       => ~Response/HTTP
      ~Response/HTTP   => ~XSLStylesheet/HTTP
      ~Repository/HTTP => ~Repository/HTTP
  -->
  <xsl:include xpath="~HTTP/repository:controllers/HTTP"/>

  <!-- inheritance rendering for inherited items, e.g. xsd:add -->
  <xsl:include xpath="~XSchema/inheritance_render"/>
  <xsl:include xpath="~XSLStylesheet/inheritance_render"/>
  <xsl:include xpath="~CSSStylesheet/inheritance_render"/>
  <xsl:include xpath="~JavaScript/inheritance_render"/>

  <!-- data output recursion
    Optional plugins (include in the MI):
      ~AJAXHTMLLoader/interface_render (database:query, etc.)
      ~XSchema/interface_render    (xschema:add/edit/render/...)
      ~Class/interface_render    (xschema:add/edit/render/...)
      ...
  -->
  <xsl:include xpath="/object:Server[1]/repository:system_transforms[1]/interface_render"/>

  <!-- ######################################## debug switches ########################################
    the XML library switches will have been applied at the C++ layer already and are here for reference
  -->
  <xsl:param name="gs_debug_url_steps" select="contains($gs_configuration_flags, 'LOG_REQUEST_DETAIL')"/>
  <xsl:param name="gs_debug_interface_render" select="contains($gs_configuration_flags, 'LOG_DATA_RENDER')"/>
  <xsl:param name="gs_debug_parent_route" select="contains($gs_configuration_flags, 'XML_DEBUG_PARENT_ROUTE')"/>
  <xsl:param name="gs_debug_xpath_expr" select="contains($gs_configuration_flags, 'XML_DEBUG_XPATH_EXPR')"/>
  <xsl:param name="gs_debug_xslt_all" select="contains($gs_configuration_flags, 'XSLT_TRACE_ALL')"/>

  <!-- ######################################## request processing ######################################## -->
  <xsl:template match="Request">
    server namespace resolution issues: [<xsl:value-of select="namespace-uri()"/>]
    <debug:GDB-break/>
  </xsl:template>

  <xsl:template match="object:Request">
    <xsl:variable name="gs_AJAX_string"><xsl:if test="$gs_AJAX">(AJAX)</xsl:if></xsl:variable>
    <debug:server-message prefix="" output="-------------------------------------- [{$gs_request/gs:message_type}]"/>
    <debug:server-message prefix="" output="[{$gs_dynamic_url}] =&gt; [{count($gs_request_target)}] {$gs_AJAX_string}"/>
    <debug:server-message if="$gs_debug_url_steps" output="object:Request [{$gs_request/gs:message_type}] =&gt; [{$gs_dynamic_url}]"/>
    
    <!-- these are also set by the server, but it only gets the first flag! -->
    <debug:server-message if="$gs_configuration_flags" output="setting configuration flags [{$gs_configuration_flags}]"/>
    <debug:xslt-set-trace flags="{$gs_configuration_flags}"/>
    <debug:xml-set-debug flags="{$gs_configuration_flags}"/>

    <!-- some system checks -->
    <xsl:if test="$gs_debug_url_steps">
      <debug:server-message output="$gs_website_root: [{$gs_website_root/@name}]"/>
      <debug:server-message output="$gs_context_database: [{$gs_context_database/@name}]"/>
      <debug:server-message output="$gs_resource_server_object: [{$gs_resource_server_object/@name}]"/>
      <debug:server-message output="$gs_resource_server: [{$gs_resource_server}]"/>
      <debug:server-message output="$gs_website_classes: [{count($gs_website_classes)}/{count($gs_website_classes)}/{count($gs_all_class_stylesheets)}/{count($gs_client_side_class_stylesheets)}]"/>
      <debug:server-message output="$gs_request_xpath: [{$gs_request_xpath}]"/>
      <xsl:apply-templates select="." mode="gs_request_tests"/>
    </xsl:if>

    <xsl:variable name="gs_custom_bootstrap" select="$gs_website_root/repository:custom_bootstrap[1]"/>
    <xsl:if test="$gs_custom_bootstrap">
      <debug:server-message output="object:Request [{$gs_request/gs:message_type}] =&gt; NOT_USED custom bootstrap [{$gs_custom_bootstrap/@xml:id}]" type="warning"/>
      <debug:NOT_CURRENTLY_USED because="custom bootstrap not being considered currently"/>
      <!-- database:transform xmlns:database="http://general_server.org/xmlnamespaces/database/2006" stylesheet="$gs_custom_bootstrap"/ -->
    </xsl:if>

    <xsl:else>
      <xsl:apply-templates select="*"/>
    </xsl:else>
  </xsl:template>

  <xsl:template match="object:Request" mode="gs_request_tests">
    <xsl:if test="$gs_request_target"><debug:server-message output="$gs_request_target: [{name($gs_request_target)}] ({count($gs_request_target)})"/></xsl:if>
    <xsl:else><debug:server-message output="$gs_request_target [{$gs_request_xpath}] not found!"/></xsl:else>
  </xsl:template>
</xsl:stylesheet>
