<xsl:stylesheet response:server-side-only="true" xmlns:meta="http://general_server.org/xmlnamespaces/meta/2006" xmlns:exsl="http://exslt.org/common" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:request="http://general_server.org/xmlnamespaces/request/2006" xmlns:flow="http://exslt.org/flow" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:dyn="http://exslt.org/dynamic" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" name="controller" controller="true" version="1.0" extension-element-prefixes="dyn database debug flow server request exsl response">
  <xsl:include xpath="../process_http_request"/>

  <!-- the POST interface for data making changes to the:
      Database (data changes, movements with transforms)
      Session (login / data)
      Server (admin)
    the API can achieve the same things through xpath commands -> interface_render.xsl
    even HTMLWebpage.load() -> /system/ajax/load.xsl -> interface_render.xsl
    ALL POSTs come here and must be data change requests

    Immediate redirection using 302 code to avoid repeat form submission
    http://en.wikipedia.org/wiki/HTTP_302
    TODO: AJAX context? need to return a direct response... different MI?

    usage:
      1) Class__Database dom-method calls: everything on the gs:form
      2) XSD schema form POSTS. usually for a Class add / update but also for things like login
      3) Class xsl:template javascript direct function calls
    settings come from the:
      incoming gs:form (dynamic settings), e.g. meta:select, gs:new_name
      AND optionally xsd:schema/xsd:app-info indicated by gs:form/gs:gs_xsd_xpath
    Class__XShema will auto-inject gs:form/gs:gs_xsd_xpath for the relevant xshema being displayed
  -->
    <!-- header output is normally configured by the stylesheet but here we can process it now -->
  <xsl:template match="object:Request/gs:HTTP">
    <xsl:apply-templates select="." mode="basic_http_headers"/>
  </xsl:template>

  <xsl:template match="object:Request/gs:HTTP" mode="basic_http_headers_code">
    <xsl:if test="$gs_AJAX">200 Ok</xsl:if>
    <xsl:else>302 Found</xsl:else>
  </xsl:template>

  <xsl:template match="object:Request/gs:HTTP" mode="basic_http_headers_context">
    <xsl:if test="not($gs_AJAX)">
      <xsl:variable name="gs_header_line">
        <xsl:apply-templates select=".." mode="full_url"/>
      </xsl:variable>
      <response:set-header header="Location" value="{$gs_header_line}"/>
    </xsl:if>
  </xsl:template>

  <xsl:template match="object:Request/gs:url">
    <!-- POST does not care about the URL or $gs_request_target
      so we override early
      output happens from the gs:form
    -->
  </xsl:template>

  <xsl:template match="object:Request/gs:form">
    <!-- POST communications MUST contain:
      xsd:schemas are often entirely dynamically generated and thus the xsd:schema element does not exist in the DB
      so an xsd:annotation is required instead <meta:xpath-to-xsd-data-processing>:
        xsd:annotation @meta:type=data-processing
          xsd:app-info @meta:type=processing @meta:dom-method
    -->
    <xsl:apply-templates select="@*" mode="gs_hardlink_xpath_attributes"/>
    <debug:server-message output-node="."/>

    <xsl:variable name="gs_xsd_data_processing" select="dyn:evaluate(@meta:xpath-to-xsd-data-processing)"/>
    <xsl:variable name="gs_xsd_processing" select="$gs_xsd_data_processing/xsd:app-info[@meta:type='processing']"/>
    <xsl:variable name="gs_DOM_method" select="($gs_xsd_processing/@meta:dom-method | @meta:dom-method)[1]"/>
    <xsl:variable name="gs_xsd_destination_string" select="$gs_xsd_processing/@meta:xpath-to-destination|$gs_xsd_processing/@meta:destination"/>
    <xsl:variable name="gs_form_destination_string" select="@meta:xpath-to-destination|@meta:destination"/>

    <!-- common form parameters in html:form @meta:* or on the incoming $gs_request/gs:form 
      $gs_form_select and [$gs_form_destination] primary nodes may be:
        1) hardlinked under $gs_xsd_processing/select
        2) dynamically provided in $gs_xsd_processing/@meta:[xpath-to-]select
        3) dynamically provided in current()/@meta:[xpath-to-]select
    -->
    <xsl:variable name="gs_form_select" select="flow:first-set-not-empty($gs_xsd_processing/select/*,      dyn:evaluate($gs_xsd_processing/@meta:xpath-to-select|$gs_xsd_processing/@meta:select), dyn:evaluate(@meta:xpath-to-select|@meta:select))"/>
    <xsl:variable name="gs_form_destination" select="flow:first-set-not-empty($gs_xsd_processing/destination/*, dyn:evaluate($gs_xsd_destination_string), dyn:evaluate($gs_form_destination_string))"/>
    
    <!-- text node inputs -->
    <xsl:variable name="gs_form_select_name" select="string(($gs_xsd_processing/@meta:select-name | meta:select-name)[1])"/>
    <xsl:variable name="gs_description" select="string(($gs_xsd_processing/@meta:description | meta:description)[1])"/>
    <xsl:variable name="gs_interface_mode" select="string(($gs_xsd_processing/@meta:interface-mode | @meta:interface-mode)[1])"/>

    <!-- info and warnings -->
    <xsl:if test="not(@meta:xpath-to-xsd-data-processing = 'none' or $gs_xsd_data_processing)"><debug:server-message output="$gs_xsd_data_processing [{@meta:xpath-to-xsd-data-processing}] did not resolve" output-node="$gs_request/gs:form" type="warning"/></xsl:if>
    <xsl:if test="not(@meta:xpath-to-xsd-data-processing = 'none' or $gs_xsd_processing)"><debug:server-message output="$gs_xsd_processing for [{$gs_xsd_data_processing/@name}] not found" output-node="$gs_request/gs:form" type="warning"/></xsl:if>
    <xsl:if test="not($gs_DOM_method)"><debug:server-message output="$gs_xsd_processing/@meta:dom-method not found for [{$gs_xsd_data_processing/@name}] did you mean meta:dom-method=none?" output-node="$gs_request/gs:form" type="error"/></xsl:if>
    <debug:server-message output="processing [{name($gs_form_select)}/{name($gs_form_destination)}] with xsd_data_processing [{$gs_xsd_data_processing/@name}] as [{$gs_DOM_method}] ..."/>

    <object:Response stage="{$gs_stage}" xml:id-policy-area="ignore">
      <!-- not including the meta:environment here because this is a 302 redirect response -->
      <xsl:apply-templates select="$gs_DOM_method" mode="gs_dom_method_switch">
        <xsl:with-param name="gs_form" select="."/>
        <xsl:with-param name="gs_xsd_data_processing" select="$gs_xsd_data_processing"/>
        <xsl:with-param name="gs_xsd_processing" select="$gs_xsd_processing"/>
        <xsl:with-param name="gs_form_select" select="$gs_form_select"/>
        <xsl:with-param name="gs_form_destination" select="$gs_form_destination"/>
        <xsl:with-param name="gs_form_select_name" select="$gs_form_select_name"/>
        <xsl:with-param name="gs_description" select="$gs_description"/>
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      </xsl:apply-templates>
    </object:Response>
  </xsl:template>
  
  <xsl:template match="@meta:dom-method[.='none']" mode="gs_dom_method_switch"/>

  <xsl:template match="@meta:dom-method[.='class-command']" mode="gs_dom_method_switch">
    <xsl:param name="gs_form"/>
    <xsl:param name="gs_xsd_data_processing"/>
    <xsl:param name="gs_xsd_processing"/>
    <xsl:param name="gs_form_select"/>
    <xsl:param name="gs_form_destination"/>
    <xsl:param name="gs_form_select_name"/>
    <xsl:param name="gs_description"/>
    <xsl:param name="gs_interface_mode"/>
    
    <!-- using the @additional-params-node here
      because the parameters are flexible for the chosen xsl:template
      the javascript function arguments (generated from the <xsl:params>) needs to send through the right names
      TODO: better search for the xsl:template in xsl:includes
    -->
    <xsl:choose>
      <xsl:when test="not($gs_form_select)"><debug:server-message output="$gs_form_select not found for class-command" type="warning"/></xsl:when>
      <xsl:when test="not($gs_interface_mode)"><debug:server-message output="$gs_interface_mode not found for class-command" type="warning"/></xsl:when>
      <xsl:when test="not($gs_stylesheet_server_side_classes/xsl:include) and not($gs_stylesheet_server_side_classes/xsl:template)"><debug:server-message output="$gs_stylesheet_server_side_classes is empty" type="warning"/></xsl:when>
      <xsl:when test="not($gs_stylesheet_server_side_classes/xsl:include) and not($gs_stylesheet_server_side_classes/xsl:template[@mode = $gs_interface_mode])"><debug:server-message output="$gs_stylesheet_server_side_classes/xsl:template @mode={$gs_interface_mode} not found for class-command" type="warning"/></xsl:when>
      <xsl:otherwise>
        <xsl:variable name="gs_class" select="database:classes($gs_form_select)"/>
        <debug:server-message output="processing class-command on [{name($gs_form_select)}] class [{local-name($gs_class)}] with stylesheet [$gs_stylesheet_server_side_classes / {$gs_interface_mode}] ..."/>
        <gs:commands_output>
          <database:transform stylesheet="$gs_stylesheet_server_side_classes" select="$gs_form_select" interface-mode="{$gs_interface_mode}" additional-params-node="."/>
        </gs:commands_output>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="@meta:dom-method[.='set-attribute']" mode="gs_dom_method_switch">
    <xsl:param name="gs_xsd_data_processing"/>
    <xsl:param name="gs_xsd_processing"/>
    <xsl:param name="gs_form_select"/>
    <xsl:param name="gs_form_destination"/>
    <xsl:param name="gs_form_select_name"/>
    <xsl:param name="gs_description"/>
    <xsl:param name="gs_interface_mode"/>
    
    <!-- these $gs_xsd_processing settings POINT to the values themselves IN this form data child node-set -->
    <xsl:variable name="gs_form_new_value" select="flow:first-set-not-empty(dyn:evaluate($gs_xsd_processing/@meta:new_value), $gs_form/meta:new_value)"/>

    <xsl:choose>
      <xsl:when test="not($gs_form_select)"><debug:server-message output="$gs_form_select not found for form [{$gs_xsd_data_processing/@name}]" type="warning" output-node="$gs_request/gs:form"/></xsl:when>
      <xsl:when test="not($gs_form_select_name)"><debug:server-message output="$gs_form_select_name not found for form [{$gs_xsd_data_processing/@name}]" output-node="$gs_request/gs:form" type="warning"/></xsl:when>
      <xsl:when test="$gs_interface_mode"><debug:server-message output="$gs_interface_mode [{$gs_interface_mode}] invalid for [{.}]" output-node="$gs_request/gs:form" type="warning"/></xsl:when>
      <xsl:otherwise>
        <database:set-attribute select="$gs_form_select" name="{$gs_form_select_name}" value="{$gs_form_new_value}" description="{$gs_description}"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="@meta:dom-method[.='set-value']" mode="gs_dom_method_switch">
    <xsl:param name="gs_xsd_data_processing"/>
    <xsl:param name="gs_xsd_processing"/>
    <xsl:param name="gs_form_select"/>
    <xsl:param name="gs_form_destination"/>
    <xsl:param name="gs_form_select_name"/>
    <xsl:param name="gs_description"/>
    <xsl:param name="gs_interface_mode"/>
    
    <!-- these $gs_xsd_processing settings POINT to the values themselves IN this form data child node-set -->
    <xsl:variable name="gs_form_new_value" select="flow:first-set-not-empty(dyn:evaluate($gs_xsd_processing/@meta:new_value), $gs_form/gs:new_value)"/>

    <xsl:choose>
      <xsl:when test="not($gs_form_select)"><debug:server-message output="$gs_form_select not found for form [{$gs_xsd_data_processing/@name}]" type="warning" output-node="$gs_request/gs:form"/></xsl:when>
      <xsl:when test="$gs_interface_mode"><debug:server-message output="$gs_interface_mode [{$gs_interface_mode}] invalid for [{.}]" output-node="$gs_request/gs:form" type="warning"/></xsl:when>
      <xsl:otherwise>
        <database:set-value select="$gs_form_select" value="{$gs_form_new_value}" description="{$gs_description}"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="@meta:dom-method[.='remove-node']" mode="gs_dom_method_switch">
    <xsl:param name="gs_xsd_data_processing"/>
    <xsl:param name="gs_xsd_processing"/>
    <xsl:param name="gs_form_select"/>
    <xsl:param name="gs_form_destination"/>
    <xsl:param name="gs_form_select_name"/>
    <xsl:param name="gs_description"/>
    <xsl:param name="gs_interface_mode"/>

    <debug:server-message output="remove-node [{name($gs_form_select)}]"/>

    <xsl:choose>
      <xsl:when test="not($gs_form_select)"><debug:server-message output="$gs_form_select not found for form [{$gs_xsd_data_processing/@name}]" output-node="$gs_request/gs:form" type="warning"/></xsl:when>
      <xsl:when test="$gs_interface_mode"><debug:server-message output="$gs_interface_mode [{$gs_interface_mode}] invalid for [{.}]" output-node="$gs_request/gs:form" type="warning"/></xsl:when>
      <xsl:otherwise>
        <database:remove-node select="$gs_form_select" description="{$gs_description}"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="@meta:dom-method[.='move-child']" mode="gs_dom_method_switch">
    <xsl:param name="gs_xsd_data_processing"/>
    <xsl:param name="gs_xsd_processing"/>
    <xsl:param name="gs_form_select"/>
    <xsl:param name="gs_form_destination"/>
    <xsl:param name="gs_form_select_name"/>
    <xsl:param name="gs_description"/>
    <xsl:param name="gs_interface_mode"/>

    <debug:server-message output="move-child [{name($gs_form_select)}] with [$gs_stylesheet_server_side_classes] mode [{$gs_interface_mode}] to destination [{name($gs_form_destination)}]"/>

    <xsl:choose>
      <xsl:when test="not($gs_form_select)"><debug:server-message output="$gs_form_select not found for form [{$gs_xsd_data_processing/@name}]" output-node="$gs_request/gs:form" type="warning"/></xsl:when>
      <xsl:when test="not($gs_form_destination)"><debug:server-message output="$gs_form_destination not found for form [{$gs_xsd_data_processing/@xml:id}]" output-node="$gs_request/gs:form" type="warning"/></xsl:when>
      <xsl:otherwise>
        <database:move-child select="$gs_form_select" destination="$gs_form_destination" transform="$gs_stylesheet_server_side_classes" interface-mode="{$gs_interface_mode}" description="{$gs_description}"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="@meta:dom-method[.='copy-child']" mode="gs_dom_method_switch">
    <xsl:param name="gs_xsd_data_processing"/>
    <xsl:param name="gs_xsd_processing"/>
    <xsl:param name="gs_form_select"/>
    <xsl:param name="gs_form_destination"/>
    <xsl:param name="gs_form_select_name"/>
    <xsl:param name="gs_description"/>
    <xsl:param name="gs_interface_mode"/>

    <debug:server-message output="copy-child [{name($gs_form_select)}] with [$gs_stylesheet_server_side_classes] mode [{$gs_interface_mode}] to destination [{name($gs_form_destination)}]"/>

    <xsl:choose>
      <xsl:when test="not($gs_form_select)"><debug:server-message output="$gs_form_select not found for form [{$gs_xsd_data_processing/@name}]" output-node="$gs_request/gs:form" type="warning"/></xsl:when>
      <xsl:when test="not($gs_form_destination)"><debug:server-message output="$gs_form_destination not found for form [{$gs_xsd_data_processing/@name}]" output-node="$gs_request/gs:form" type="warning"/></xsl:when>
      <xsl:otherwise>
        <database:copy-child select="$gs_form_select" destination="$gs_form_destination" transform="$gs_stylesheet_server_side_classes" interface-mode="{$gs_interface_mode}" description="{$gs_description}"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="@meta:dom-method[.='hardlink-child']" mode="gs_dom_method_switch">
    <xsl:param name="gs_xsd_data_processing"/>
    <xsl:param name="gs_xsd_processing"/>
    <xsl:param name="gs_form_select"/>
    <xsl:param name="gs_form_destination"/>
    <xsl:param name="gs_form_select_name"/>
    <xsl:param name="gs_description"/>
    <xsl:param name="gs_interface_mode"/>

    <debug:server-message output="hardlink-child [{name($gs_form_select)}] to destination [{name($gs_form_destination)}]"/>

    <xsl:choose>
      <xsl:when test="not($gs_form_select)"><debug:server-message output="$gs_form_select not found for form [{$gs_xsd_data_processing/@name}]" output-node="$gs_request/gs:form" type="warning"/></xsl:when>
      <xsl:when test="not($gs_form_destination)"><debug:server-message output="$gs_form_destination not found for form [{$gs_xsd_data_processing/@name}]" output-node="$gs_request/gs:form" type="warning"/></xsl:when>
      <xsl:when test="$gs_interface_mode"><debug:server-message output="$gs_interface_mode [{$gs_interface_mode}] invalid for [{.}]" output-node="$gs_request/gs:form" type="warning"/></xsl:when>
      <xsl:otherwise>
        <database:hardlink-child select="$gs_form_select" destination="$gs_form_destination" description="{$gs_description}"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="@meta:dom-method[.='softlink-child']" mode="gs_dom_method_switch">
    <xsl:param name="gs_xsd_data_processing"/>
    <xsl:param name="gs_xsd_processing"/>
    <xsl:param name="gs_form_select"/>
    <xsl:param name="gs_form_destination"/>
    <xsl:param name="gs_form_select_name"/>
    <xsl:param name="gs_description"/>
    <xsl:param name="gs_interface_mode"/>

    <debug:server-message output="softlink-child [{name($gs_form_select)}] to destination [{name($gs_form_destination)}]"/>

    <xsl:choose>
      <xsl:when test="not($gs_form_select)"><debug:server-message output="$gs_form_select not found for form [{$gs_xsd_data_processing/@name}]" output-node="$gs_request/gs:form" type="warning"/></xsl:when>
      <xsl:when test="not($gs_form_destination)"><debug:server-message output="$gs_form_destination not found for form [{$gs_xsd_data_processing/@name}]" output-node="$gs_request/gs:form" type="warning"/></xsl:when>
      <xsl:when test="$gs_interface_mode"><debug:server-message output="$gs_interface_mode [{$gs_interface_mode}] invalid for [{.}]" output-node="$gs_request/gs:form" type="warning"/></xsl:when>
      <xsl:otherwise>
        <database:softlink-child select="$gs_form_select" destination="$gs_form_destination" description="{$gs_description}"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="@meta:dom-method[.='replace-node']" mode="gs_dom_method_switch">
    <xsl:param name="gs_xsd_data_processing"/>
    <xsl:param name="gs_xsd_processing"/>
    <xsl:param name="gs_form_select"/>
    <xsl:param name="gs_form_destination"/>
    <xsl:param name="gs_form_select_name"/>
    <xsl:param name="gs_description"/>
    <xsl:param name="gs_interface_mode"/>

    <!--
        1) xml    -> replaces destination (no select allowed)
        2) select -> replaces destination (with optional transform)
    -->
    <xsl:variable name="gs_form_xml" select="flow:first-set-not-empty(dyn:evaluate($gs_xsd_processing/@meta:xml), $gs_form/gs:xml)"/>
    <xsl:variable name="gs_form_xml_mode" select="boolean(string($gs_form_xml))"/>
    <xsl:if test="$gs_form_xml_mode"><debug:server-message output="replace-node [{name($gs_form_destination)}] with xml"/></xsl:if>
    <xsl:else><debug:server-message output="replace-node [{name($gs_form_destination)}] with [{name($gs_form_select)}]"/></xsl:else>

    <xsl:choose>
      <xsl:when test="$gs_form_xml_mode      and not($gs_form_destination)"><debug:server-message output="xml mode: $gs_form_destination not found for form [{$gs_xsd_data_processing/@name}]" output-node="$gs_request/gs:form" type="warning"/></xsl:when>
      <xsl:when test="$gs_form_xml_mode      and $gs_form_select">          <debug:server-message output="xml mode: $gs_form_select not allowed for form [{$gs_xsd_data_processing/@name}]" output-node="$gs_request/gs:form" type="warning"/></xsl:when>
      <xsl:when test="not($gs_form_xml_mode) and not($gs_form_select)">     <debug:server-message output="node mode: $gs_form_select not found for form [{$gs_xsd_data_processing/@name}]" output-node="$gs_request/gs:form" type="warning"/></xsl:when>
      <xsl:when test="not($gs_form_xml_mode) and not($gs_form_destination)"><debug:server-message output="node mode: $gs_form_destination [{$gs_xsd_processing/@destination}] not found for form [{$gs_xsd_data_processing/@name}]" output-node="$gs_request/gs:form" type="warning"/></xsl:when>
      <xsl:when test="$gs_interface_mode"><debug:server-message output="$gs_interface_mode [{$gs_interface_mode}] invalid for [{.}]" output-node="$gs_request/gs:form" type="warning"/></xsl:when>
      <xsl:otherwise>
        <database:replace-node select="$gs_form_select" xml="{$gs_form_xml}" destination="$gs_form_destination" description="{$gs_description}"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="@meta:dom-method[.='merge-node']" mode="gs_dom_method_switch">
    <xsl:param name="gs_xsd_data_processing"/>
    <xsl:param name="gs_xsd_processing"/>
    <xsl:param name="gs_form_select"/>
    <xsl:param name="gs_form_destination"/>
    <xsl:param name="gs_form_select_name"/>
    <xsl:param name="gs_description"/>
    <xsl:param name="gs_interface_mode"/>

    <!--
        1) xml    -> replaces destination (no select allowed)
        2) select -> replaces destination (with optional transform)
    -->
    <xsl:variable name="gs_form_xml" select="flow:first-set-not-empty(dyn:evaluate($gs_xsd_processing/@meta:xml), $gs_form/gs:xml)"/>
    <xsl:variable name="gs_form_xml_mode" select="boolean(string($gs_form_xml))"/>
    <xsl:if test="$gs_form_xml_mode"><debug:server-message output="merge-node [{name($gs_form_destination)}] with xml"/></xsl:if>
    <xsl:else><debug:server-message output="merge-node [{name($gs_form_destination)}] with [{name($gs_form_select)}]"/></xsl:else>

    <xsl:choose>
      <xsl:when test="$gs_form_xml_mode      and not($gs_form_destination)"><debug:server-message output="xml mode: $gs_form_destination not found for form [{$gs_xsd_data_processing/@name}]" output-node="$gs_request/gs:form" type="warning"/></xsl:when>
      <xsl:when test="$gs_form_xml_mode      and $gs_form_select">          <debug:server-message output="xml mode: $gs_form_select not allowed for form [{$gs_xsd_data_processing/@name}]" output-node="$gs_request/gs:form" type="warning"/></xsl:when>
      <xsl:when test="not($gs_form_xml_mode) and not($gs_form_select)">     <debug:server-message output="node mode: $gs_form_select not found for form [{$gs_xsd_data_processing/@name}]" output-node="$gs_request/gs:form" type="warning"/></xsl:when>
      <xsl:when test="not($gs_form_xml_mode) and not($gs_form_destination)"><debug:server-message output="node mode: $gs_form_destination [{$gs_xsd_processing/@destination}] not found for form [{$gs_xsd_data_processing/@name}]" output-node="$gs_request/gs:form" type="warning"/></xsl:when>
      <xsl:when test="$gs_interface_mode"><debug:server-message output="$gs_interface_mode [{$gs_interface_mode}] invalid for [{.}]" output-node="$gs_request/gs:form" type="warning"/></xsl:when>
      <xsl:otherwise>
        <database:merge-node select="$gs_form_select" xml="{$gs_form_xml}" destination="$gs_form_destination" description="{$gs_description}"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="@meta:dom-method" mode="gs_dom_method_switch">
    <xsl:param name="gs_xsd_data_processing"/>
    <xsl:param name="gs_xsd_processing"/>
    <xsl:param name="gs_form_select"/>
    <xsl:param name="gs_form_destination"/>
    <xsl:param name="gs_form_select_name"/>
    <xsl:param name="gs_description"/>
    <xsl:param name="gs_interface_mode"/>
    
    <debug:server-message output="meta:dom-method [{.}] not valid for form [{$gs_xsd_data_processing/@name}]" output-node="$gs_request/gs:form" type="warning"/>
  </xsl:template>

  <xsl:template match="object:Request" mode="full_url">
    <!-- http://general_server.localhost:8776/this/is/the?things=this -->
    <xsl:value-of select="gs:host/@protocol"/>
    <xsl:if test="not(gs:host/@protocol)">http</xsl:if>
    <xsl:text>://</xsl:text>
    
    <xsl:value-of select="gs:host"/>
    <xsl:if test="gs:host/@port">:<xsl:value-of select="gs:host/@port"/></xsl:if>
    
    <xsl:text>/</xsl:text>
    <xsl:value-of select="gs:url"/>
    <xsl:apply-templates select="gs:query-string" mode="full_url"/>
  </xsl:template>

  <xsl:template match="object:Request/gs:query-string" mode="full_url">
    <xsl:if test="*"><xsl:text>?</xsl:text></xsl:if>
    <xsl:apply-templates select="*" mode="full_url"/>
  </xsl:template>

  <xsl:template match="object:Request/gs:query-string/*" mode="full_url">
    <xsl:if test="not(position() = 1)">&amp;</xsl:if>
    <xsl:value-of select="local-name()"/>=<xsl:value-of select="."/>
  </xsl:template>

  <xsl:template match="object:Request" mode="gs_request_tests"/>
</xsl:stylesheet>
