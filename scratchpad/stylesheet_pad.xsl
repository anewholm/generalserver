<xsl:stylesheet xmlns:database="http://general_server.org/xmlnamespaces/database/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:exsl="http://exslt.org/common" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:response="http://general_server.org/xmlnamespaces/response/2006" xmlns:session="http://general_server.org/xmlnamespaces/session/2006" xmlns:conversation="http://general_server.org/xmlnamespaces/conversation/2006" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:service="http://general_server.org/xmlnamespaces/service/2006" xmlns:request="http://general_server.org/xmlnamespaces/request/2006" xmlns:rx="http://general_server.org/xmlnamespaces/rx/2006" xmlns:javascript="http://general_server.org/xmlnamespaces/javascript/2006" xmlns:class="http://general_server.org/xmlnamespaces/class/2006" xmlns:css="http://general_server.org/xmlnamespaces/css/2006" xmlns:xmlsecurity="http://general_server.org/xmlnamespaces/xmlsecurity/2006" xmlns:xmltransaction="http://general_server.org/xmlnamespaces/xmltransaction/2006" xmlns:regexp="http://exslt.org/regular-expressions" xmlns:meta="http://general_server.org/xmlnamespaces/meta/2006" version="1.0" exclude-result-prefixes="#default repository gs database debug conversation html rx class xmlsecurity xmltransaction xsl object interface xsd service javascript css" auto-add="true" response:include-all="on"><!--recommended xsl:output (firefox requires a HTMLDocument object from @method to work)--><xsl:output name="HTML" method="html" version="4.01" encoding="UTF-8" doctype-public="-//W3C//DTD HTML 4.01//EN" doctype-system="http://www.w3.org/TR/html4/strict.dtd"/><!--[/~Class/interface_translators.dxsl] DXSL--><xsl:template match="*" mode="gs_view_render"><!--
        xsl:param defaults to a blank string
        further xsl:param declerations will receive the string and not default to the @select
        set the default to a blank node set with @meta:default-type=node-set
        note the BUG in LibXml2 that interprets the namespace-alias as an invalid param if this pre-xsl-statement is not here
      --><xsl:param name="gs_interface_mode"/><xsl:param name="gs_current_external_value_node" select="FORCE_SELECT_EMPTY_NODESET"/><xsl:param name="gs_title" select="xsd:annotation/xsd:*[@meta:type='title']"/><xsl:param name="gs_submit" select="xsd:annotation/xsd:*[@meta:type='submit-caption']"/><xsl:param name="gs_description" select="xsd:annotation/xsd:*[@meta:type='description']"/><xsl:param name="gs_event_functions" select="''"/><xsl:param name="gs_current_value_node" select="xsd:annotation/xsd:*[@meta:type='edit-data']/*"/><xsl:param name="gs_field_model" select="ancestor::xsd:schema[1]/xsd:annotation[@meta:type='data-queries']/xsd:app-info[@meta:type='field-model']/xsd:schema"/><xsl:param name="gs_field_types" select="ancestor::xsd:schema[1]/xsd:annotation[@meta:type='data-queries']/xsd:app-info[@meta:type='field-types']/xsd:schema"/><xsl:param name="gs_sub_interface" select="''"/><!--main and complex default interface--><xsl:variable name="gs_default_interface_fragment"><xsl:value-of select="$gs_interface_mode"/><xsl:if test="not($gs_interface_mode)"><xsl:value-of select="@gs:interface-mode"/></xsl:if></xsl:variable><xsl:variable name="gs_default_interface" select="string($gs_default_interface_fragment)"/><xsl:variable name="gs_main_interface_fragment"><xsl:value-of select="substring-before($gs_default_interface, '-')"/><xsl:if test="not(contains($gs_default_interface, '-'))"><xsl:value-of select="$gs_default_interface"/></xsl:if></xsl:variable><xsl:variable name="gs_main_interface" select="string($gs_main_interface_fragment)"/><!--classes analysis and extended attributes--><xsl:variable name="gs_html_identifier_class"><xsl:apply-templates select="." mode="gs_html_identifier_class"><xsl:with-param name="gs_interface_mode" select="$gs_default_interface"/></xsl:apply-templates></xsl:variable><!--begin @gs:interface-mode choose--><xsl:choose><xsl:when test="$gs_main_interface='editor'"><xsl:apply-templates select="." mode="editor"><xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/><xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/><xsl:with-param name="gs_field_model" select="$gs_field_model"/><xsl:with-param name="gs_field_types" select="$gs_field_types"/></xsl:apply-templates></xsl:when><xsl:when test="$gs_main_interface='html'"><xsl:apply-templates select="." mode="html"><xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/><xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/></xsl:apply-templates></xsl:when><xsl:when test="$gs_main_interface='xpathajax'"><xsl:apply-templates select="." mode="xpathajax"><xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/><xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/><xsl:with-param name="gs_current_external_value_node" select="$gs_current_external_value_node"/></xsl:apply-templates></xsl:when><xsl:when test="$gs_main_interface='new'"><xsl:apply-templates select="." mode="new"><xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/><xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/></xsl:apply-templates></xsl:when><xsl:when test="$gs_main_interface='set'"><xsl:apply-templates select="." mode="set"><xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/><xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/></xsl:apply-templates></xsl:when><xsl:when test="$gs_main_interface='selectoroption'"><xsl:apply-templates select="." mode="selectoroption"><xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/><xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/></xsl:apply-templates></xsl:when><xsl:when test="$gs_main_interface='eventhandler'"><xsl:apply-templates select="." mode="eventhandler"><xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/><xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/></xsl:apply-templates></xsl:when><xsl:when test="$gs_main_interface='view'"><xsl:apply-templates select="." mode="view"><xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/><xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/></xsl:apply-templates></xsl:when><xsl:when test="$gs_main_interface='controller'"><xsl:apply-templates select="." mode="controller"><xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/><xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/></xsl:apply-templates></xsl:when><xsl:when test="$gs_main_interface='securityowner'"><xsl:apply-templates select="." mode="securityowner"><xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/><xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/></xsl:apply-templates></xsl:when><xsl:when test="$gs_main_interface='controls'"><xsl:apply-templates select="." mode="controls"><xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/><xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/></xsl:apply-templates></xsl:when><xsl:when test="$gs_main_interface='link'"><xsl:apply-templates select="." mode="link"><xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/><xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/></xsl:apply-templates></xsl:when><xsl:when test="$gs_main_interface='list'"><xsl:apply-templates select="." mode="list"><xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/><xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/><xsl:with-param name="gs_sub_interface" select="$gs_sub_interface"/><xsl:with-param name="gs_event_functions" select="$gs_event_functions"/></xsl:apply-templates></xsl:when><xsl:when test="$gs_main_interface='listname'"><xsl:apply-templates select="." mode="listname"><xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/><xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/></xsl:apply-templates></xsl:when><xsl:when test="$gs_main_interface='securitypermissions'"><xsl:apply-templates select="." mode="securitypermissions"><xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/><xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/></xsl:apply-templates></xsl:when><xsl:when test="$gs_main_interface='environment'"><xsl:apply-templates select="." mode="environment"><xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/><xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/></xsl:apply-templates></xsl:when><xsl:when test="not($gs_default_interface) or $gs_default_interface = 'default'"><xsl:apply-templates select="."><xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/><xsl:with-param name="gs_interface_mode" select="$gs_default_interface"/><xsl:with-param name="gs_current_external_value_node" select="$gs_current_external_value_node"/><xsl:with-param name="gs_title" select="$gs_title"/><xsl:with-param name="gs_submit" select="$gs_submit"/><xsl:with-param name="gs_description" select="$gs_description"/><xsl:with-param name="gs_event_functions" select="$gs_event_functions"/><xsl:with-param name="gs_current_value_node" select="$gs_current_value_node"/></xsl:apply-templates></xsl:when><xsl:otherwise><div class="gs-warning" xsl:is-literal="1">[<xsl:value-of select="name()"/>] @meta:interface [<xsl:value-of select="$gs_default_interface"/>=<xsl:value-of select="$gs_main_interface"/>] not caught during @mode=gs_view_render</div></xsl:otherwise></xsl:choose></xsl:template><!--/[/~Class/interface_translators.dxsl]--><!--[/~Class/class_introspection.dxsl] DXSL--><!--class attribute defaults--><xsl:template match="*" mode="gs_html_container"/><xsl:template match="*" mode="gs_debug"/><!--classes--><xsl:template match="interface:Overlay" mode="gs_classes_string" meta:base-class-count="0">Overlay</xsl:template><xsl:template match="*" mode="gs_classes_string" meta:base-class-count="2">DatabaseElement</xsl:template><xsl:template match="repository:*" mode="gs_classes_string" meta:base-class-count="5">Repository</xsl:template><xsl:template match="class:*" mode="gs_classes_string" meta:base-class-count="5">Class</xsl:template><xsl:template match="xmltransaction:*" mode="gs_classes_string" meta:base-class-count="5">Transaction</xsl:template><xsl:template match="html:*" mode="gs_classes_string" meta:base-class-count="5">HTML</xsl:template><xsl:template match="gs:*" mode="gs_classes_string" meta:base-class-count="5">GS</xsl:template><xsl:template match="css:*" mode="gs_classes_string" meta:base-class-count="5">CSS</xsl:template><xsl:template match="response:*" mode="gs_classes_string" meta:base-class-count="5">ResponseNamespace</xsl:template><xsl:template match="object:*" mode="gs_classes_string" meta:base-class-count="5">ObjectClass</xsl:template><xsl:template match="xsd:*" mode="gs_classes_string" meta:base-class-count="5">XSD</xsl:template><xsl:template match="interface:*" mode="gs_classes_string" meta:base-class-count="5">Interface</xsl:template><xsl:template match="xml:*" mode="gs_classes_string" meta:base-class-count="5">XML</xsl:template><xsl:template match="database:*" mode="gs_classes_string" meta:base-class-count="5">DatabaseNamespace</xsl:template><xsl:template match="xmltransaction:*" mode="gs_classes_string" meta:base-class-count="5">XMLTransaction</xsl:template><xsl:template match="xsl:*" mode="gs_classes_string" meta:base-class-count="5">XSL</xsl:template><xsl:template match="meta:*" mode="gs_classes_string" meta:base-class-count="5">Meta</xsl:template><xsl:template match="rx:*" mode="gs_classes_string" meta:base-class-count="5">RegularX</xsl:template><xsl:template match="javascript:*" mode="gs_classes_string" meta:base-class-count="5">JS</xsl:template><xsl:template match="interface:Forum" mode="gs_classes_string" meta:base-class-count="6">Forum</xsl:template><xsl:template match="interface:ForumPost" mode="gs_classes_string" meta:base-class-count="6">ForumPost</xsl:template><xsl:template match="xsl:include" mode="gs_classes_string" meta:base-class-count="6">XSLInclude</xsl:template><xsl:template match="interface:HTMLWebpage" mode="gs_classes_string" meta:base-class-count="6">HTMLWebpage</xsl:template><xsl:template match="javascript:dependency" mode="gs_classes_string" meta:base-class-count="6">JavaScriptDependency</xsl:template><xsl:template match="css:stylesheet" mode="gs_classes_string" meta:base-class-count="6">CSSStylesheet</xsl:template><xsl:template match="interface:Debugger" mode="gs_classes_string" meta:base-class-count="6">Debugger</xsl:template><xsl:template match="interface:MultiDocument" mode="gs_classes_string" meta:base-class-count="6">MultiDocument</xsl:template><xsl:template match="interface:Explorer" mode="gs_classes_string" meta:base-class-count="6">Explorer</xsl:template><xsl:template match="interface:IDE" mode="gs_classes_string" meta:base-class-count="6">IDE</xsl:template><xsl:template match="interface:Frame" mode="gs_classes_string" meta:base-class-count="6">Frame</xsl:template><xsl:template match="interface:MultiView" mode="gs_classes_string" meta:base-class-count="6">MultiView</xsl:template><xsl:template match="interface:VerticalFrameset|interface:HorizontalFrameset" mode="gs_classes_string" meta:base-class-count="6">Frameset</xsl:template><xsl:template match="interface:SharedInterfaces" mode="gs_classes_string" meta:base-class-count="6">SharedInterfaces</xsl:template><xsl:template match="interface:AJAXHTMLLoader" mode="gs_classes_string" meta:base-class-count="6">AJAXHTMLLoader</xsl:template><xsl:template match="interface:Collection" mode="gs_classes_string" meta:base-class-count="6">Collection</xsl:template><xsl:template match="interface:HTMLContainer" mode="gs_classes_string" meta:base-class-count="6">HTMLContainer</xsl:template><xsl:template match="xsd:schema" mode="gs_classes_string" meta:base-class-count="6">XSchema</xsl:template><xsl:template match="xsd:element|xsd:attribute|xsd:any" mode="gs_classes_string" meta:base-class-count="6">XSDField</xsl:template><xsl:template match="xsl:template" mode="gs_classes_string" meta:base-class-count="6">XSLTemplate</xsl:template><xsl:template match="javascript:code" mode="gs_classes_string" meta:base-class-count="6">JavaScript</xsl:template><xsl:template match="interface:QualifiedName" mode="gs_classes_string" meta:base-class-count="6">QualifiedName</xsl:template><xsl:template match="html:img" mode="gs_classes_string" meta:base-class-count="6">HTMLImage</xsl:template><xsl:template match="interface:Menu" mode="gs_classes_string" meta:base-class-count="6">Menu</xsl:template><xsl:template match="xsl:stylesheet" mode="gs_classes_string" meta:base-class-count="6">XSLStylesheet</xsl:template><xsl:template match="css:dependency" mode="gs_classes_string" meta:base-class-count="6">CSSDependency</xsl:template><xsl:template match="xsl:variable|xsl:param" mode="gs_classes_string" meta:base-class-count="6">XSLVariable</xsl:template><xsl:template match="object:Message" mode="gs_classes_string" meta:base-class-count="6">Message</xsl:template><xsl:template match="object:Response" mode="gs_classes_string" meta:base-class-count="6">Response</xsl:template><xsl:template match="object:LinkedServer" mode="gs_classes_string" meta:base-class-count="6">LinkedServer</xsl:template><xsl:template match="object:Request" mode="gs_classes_string" meta:base-class-count="6">Request</xsl:template><xsl:template match="repository:resources" mode="gs_classes_string" meta:base-class-count="6">Resources</xsl:template><xsl:template match="object:Website" mode="gs_classes_string" meta:base-class-count="6">Website</xsl:template><xsl:template match="object:User" mode="gs_classes_string" meta:base-class-count="6">User</xsl:template><xsl:template match="object:Database" mode="gs_classes_string" meta:base-class-count="6">Database</xsl:template><xsl:template match="object:Session" mode="gs_classes_string" meta:base-class-count="6">Session</xsl:template><xsl:template match="object:Service" mode="gs_classes_string" meta:base-class-count="6">Service</xsl:template><xsl:template match="object:Group" mode="gs_classes_string" meta:base-class-count="6">Group</xsl:template><xsl:template match="object:Server" mode="gs_classes_string" meta:base-class-count="6">Server</xsl:template><xsl:template match="object:MessageInterpretation" mode="gs_classes_string" meta:base-class-count="6">MessageInterpretation</xsl:template><xsl:template match="object:Person" mode="gs_classes_string" meta:base-class-count="7">Person</xsl:template><xsl:template match="interface:AdvancedForum" mode="gs_classes_string" meta:base-class-count="7">AdvancedForum</xsl:template><xsl:template match="xsd:attribute[@meta:editor-class = 'class:CodeMirrorEditor']" mode="gs_classes_string" meta:base-class-count="7">CodeMirrorEditor</xsl:template><xsl:template match="xsl:attribute[@meta:editor-class = 'class:Selector']" mode="gs_classes_string" meta:base-class-count="7">Selector</xsl:template><xsl:template match="interface:VerticalMenu" mode="gs_classes_string" meta:base-class-count="7">VerticalMenu</xsl:template><xsl:template match="interface:HorizontalMenu" mode="gs_classes_string" meta:base-class-count="7">HorizontalMenu</xsl:template><xsl:template match="object:HTTP" mode="gs_classes_string" meta:base-class-count="7">HTTP</xsl:template><xsl:template match="object:Error" mode="gs_classes_string" meta:base-class-count="7">Error</xsl:template><xsl:template match="object:Manager" mode="gs_classes_string" meta:base-class-count="8">Manager</xsl:template><xsl:template match="interface:ContextMenu" mode="gs_classes_string" meta:base-class-count="9">ContextMenu</xsl:template><xsl:template match="interface:SubMenu" mode="gs_classes_string" meta:base-class-count="9">SubMenu</xsl:template><!--base classes--><xsl:template match="*" mode="gs_base_classes_string" meta:base-class-count="2">BaseObject,HTMLObject</xsl:template><xsl:template match="repository:*" mode="gs_base_classes_string" meta:base-class-count="5">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject</xsl:template><xsl:template match="repository:*" mode="gs_html_container">ul</xsl:template><xsl:template match="class:*" mode="gs_base_classes_string" meta:base-class-count="5">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject</xsl:template><xsl:template match="xmltransaction:*" mode="gs_base_classes_string" meta:base-class-count="5">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject</xsl:template><xsl:template match="html:*" mode="gs_base_classes_string" meta:base-class-count="5">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject</xsl:template><xsl:template match="gs:*" mode="gs_base_classes_string" meta:base-class-count="5">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject</xsl:template><xsl:template match="css:*" mode="gs_base_classes_string" meta:base-class-count="5">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject</xsl:template><xsl:template match="response:*" mode="gs_base_classes_string" meta:base-class-count="5">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject</xsl:template><xsl:template match="object:*" mode="gs_base_classes_string" meta:base-class-count="5">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject</xsl:template><xsl:template match="xsd:*" mode="gs_base_classes_string" meta:base-class-count="5">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject</xsl:template><xsl:template match="interface:*" mode="gs_base_classes_string" meta:base-class-count="5">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject</xsl:template><xsl:template match="xml:*" mode="gs_base_classes_string" meta:base-class-count="5">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject</xsl:template><xsl:template match="database:*" mode="gs_base_classes_string" meta:base-class-count="5">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject</xsl:template><xsl:template match="xmltransaction:*" mode="gs_base_classes_string" meta:base-class-count="5">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject</xsl:template><xsl:template match="xsl:*" mode="gs_base_classes_string" meta:base-class-count="5">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject</xsl:template><xsl:template match="meta:*" mode="gs_base_classes_string" meta:base-class-count="5">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject</xsl:template><xsl:template match="rx:*" mode="gs_base_classes_string" meta:base-class-count="5">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject</xsl:template><xsl:template match="javascript:*" mode="gs_base_classes_string" meta:base-class-count="5">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject</xsl:template><xsl:template match="interface:Forum" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,Interface</xsl:template><xsl:template match="interface:ForumPost" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,Interface</xsl:template><xsl:template match="xsl:include" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,XSL</xsl:template><xsl:template match="interface:HTMLWebpage" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,ObjectClass</xsl:template><xsl:template match="interface:HTMLWebpage" mode="gs_html_container">html</xsl:template><xsl:template match="javascript:dependency" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,JS</xsl:template><xsl:template match="css:stylesheet" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,CSS</xsl:template><xsl:template match="interface:Debugger" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,Interface</xsl:template><xsl:template match="interface:MultiDocument" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,Interface</xsl:template><xsl:template match="interface:Explorer" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,Interface</xsl:template><xsl:template match="interface:IDE" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,Interface</xsl:template><xsl:template match="interface:Frame" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,Interface</xsl:template><xsl:template match="interface:MultiView" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,Interface</xsl:template><xsl:template match="interface:VerticalFrameset|interface:HorizontalFrameset" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,Interface</xsl:template><xsl:template match="interface:SharedInterfaces" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,Interface</xsl:template><xsl:template match="interface:AJAXHTMLLoader" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,Interface</xsl:template><xsl:template match="interface:Collection" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,Interface</xsl:template><xsl:template match="interface:Collection" mode="gs_html_container">ul</xsl:template><xsl:template match="interface:HTMLContainer" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,HTML</xsl:template><xsl:template match="xsd:schema" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,XSD</xsl:template><xsl:template match="xsd:element|xsd:attribute|xsd:any" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,XSD</xsl:template><xsl:template match="xsl:template" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,XSL</xsl:template><xsl:template match="javascript:code" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,JS</xsl:template><xsl:template match="interface:QualifiedName" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,Interface</xsl:template><xsl:template match="html:img" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,HTML</xsl:template><xsl:template match="interface:Menu" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,Interface</xsl:template><xsl:template match="interface:Menu" mode="gs_html_container">ul</xsl:template><xsl:template match="xsl:stylesheet" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,XSL</xsl:template><xsl:template match="css:dependency" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,CSS</xsl:template><xsl:template match="xsl:variable|xsl:param" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,XSL</xsl:template><xsl:template match="object:Message" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,ObjectClass</xsl:template><xsl:template match="object:Response" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,ObjectClass</xsl:template><xsl:template match="object:LinkedServer" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,ObjectClass</xsl:template><xsl:template match="object:Request" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,ObjectClass</xsl:template><xsl:template match="repository:resources" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,Repository,DatabaseObject,DatabaseElement,HTMLObject</xsl:template><xsl:template match="repository:resources" mode="gs_html_container">ul</xsl:template><xsl:template match="object:Website" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,ObjectClass</xsl:template><xsl:template match="object:User" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,ObjectClass</xsl:template><xsl:template match="object:Database" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,ObjectClass</xsl:template><xsl:template match="object:Session" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,ObjectClass</xsl:template><xsl:template match="object:Service" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,ObjectClass</xsl:template><xsl:template match="object:Group" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,ObjectClass</xsl:template><xsl:template match="object:Server" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,ObjectClass</xsl:template><xsl:template match="object:MessageInterpretation" mode="gs_base_classes_string" meta:base-class-count="6">VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,ObjectClass</xsl:template><xsl:template match="object:Person" mode="gs_base_classes_string" meta:base-class-count="7">VariableType,BaseObject,User,DatabaseObject,DatabaseElement,HTMLObject,ObjectClass</xsl:template><xsl:template match="interface:AdvancedForum" mode="gs_base_classes_string" meta:base-class-count="7">Forum,VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,Interface</xsl:template><xsl:template match="xsd:attribute[@meta:editor-class = 'class:CodeMirrorEditor']" mode="gs_base_classes_string" meta:base-class-count="7">XSDField,VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,XSD</xsl:template><xsl:template match="xsl:attribute[@meta:editor-class = 'class:Selector']" mode="gs_base_classes_string" meta:base-class-count="7">XSDField,VariableType,BaseObject,DatabaseObject,DatabaseElement,HTMLObject,XSD</xsl:template><xsl:template match="interface:VerticalMenu" mode="gs_base_classes_string" meta:base-class-count="7">VariableType,BaseObject,Menu,DatabaseObject,DatabaseElement,HTMLObject,Interface</xsl:template><xsl:template match="interface:VerticalMenu" mode="gs_html_container">ul</xsl:template><xsl:template match="interface:HorizontalMenu" mode="gs_base_classes_string" meta:base-class-count="7">VariableType,BaseObject,Menu,DatabaseObject,DatabaseElement,HTMLObject,Interface</xsl:template><xsl:template match="interface:HorizontalMenu" mode="gs_html_container">ul</xsl:template><xsl:template match="object:HTTP" mode="gs_base_classes_string" meta:base-class-count="7">VariableType,BaseObject,Service,DatabaseObject,DatabaseElement,HTMLObject,ObjectClass</xsl:template><xsl:template match="object:Error" mode="gs_base_classes_string" meta:base-class-count="7">VariableType,BaseObject,Message,DatabaseObject,DatabaseElement,HTMLObject,ObjectClass</xsl:template><xsl:template match="object:Manager" mode="gs_base_classes_string" meta:base-class-count="8">Person,VariableType,BaseObject,User,DatabaseObject,DatabaseElement,HTMLObject,ObjectClass</xsl:template><xsl:template match="interface:ContextMenu" mode="gs_base_classes_string" meta:base-class-count="9">VariableType,BaseObject,VerticalMenu,Overlay,Menu,DatabaseObject,DatabaseElement,HTMLObject,Interface</xsl:template><xsl:template match="interface:ContextMenu" mode="gs_html_container">ul</xsl:template><xsl:template match="interface:SubMenu" mode="gs_base_classes_string" meta:base-class-count="9">VariableType,BaseObject,VerticalMenu,Overlay,Menu,DatabaseObject,DatabaseElement,HTMLObject,Interface</xsl:template><xsl:template match="interface:SubMenu" mode="gs_html_container">ul</xsl:template><!--/[/~Class/class_introspection.dxsl]--><!--[/~Class/website_classes_html_view_templates.dxsl] DXSL--><!--
      ######## gs_complementary_templates #########
      $gs_website_classes               [99]
      $gs_client_side_class_stylesheets [64]
      xsl:template/@match attributes have been morphed to include all derived class:*/@elements
      xsl:sorted by database:base-class-count() ascending (last template takes precendence)
      type:             [complement]
    --><xsl:param name="gs_response_client_side" select="/object:Response" database:path-check="ignore"/><xsl:param name="gs_response_server_side" select="self::object:Response"/><xsl:param name="gs_response" select="$gs_response_client_side|$gs_response_server_side"/><xsl:param name="gs_environment" select="$gs_response/meta:environment"/><xsl:param name="gs_user" select="$gs_environment/object:User"/><xsl:param name="gs_request" select="$gs_environment/object:Request"/><xsl:param name="gs_session" select="$gs_environment/object:Session"/><xsl:param name="gs_website_root" select="$gs_environment/object:Website"/><xsl:param name="gs_context_database" select="$gs_environment/object:Database"/><xsl:param name="gs_resource_server_object" select="$gs_environment/object:LinkedServer"/><xsl:param name="db" select="$gs_context_database"/><xsl:param name="website" select="$gs_website_root"/><xsl:param name="gs_newline" select="'&#10;'"/><xsl:param name="gs_lowercase" select="'abcdefghijklmnopqrstuvwxyz'"/><xsl:param name="gs_uppercase" select="'ABCDEFGHIJKLMNOPQRSTUVWXYZ'"/><xsl:param name="gs_request_id" select="string($gs_request/@id)"/><xsl:param name="gs_query_string" select="$gs_request/gs:query-string"/><xsl:param name="gs_form" select="$gs_request/gs:form"/><xsl:param name="gs_configuration_flags" select="string($gs_query_string/@configuration-flags)"/><xsl:param name="gs_is_logged_in" select="boolean($gs_user)"/><xsl:param name="gs_isadministrator" select="boolean($gs_user/gs:groups/object:Group[xml:id='grp_1'])"/><xsl:param name="gs_resource_server" select="$gs_resource_server_object/@uri"/><xsl:param name="gs_request_url" select="$gs_request/gs:url"/><xsl:param name="gs_request_target" select="$gs_response/gs:data/*"/><xsl:variable name="NAMESPACE_XXX" select="'http://general_server.org/xmlnamespaces/dummyxsl/2006'"/><xsl:variable name="NAMESPACE_GS" select="'http://general_server.org/xmlnamespaces/general_server/2006'"/><xsl:variable name="NAMESPACE_EXSL" select="'http://exslt.org/common'"/><xsl:variable name="NAMESPACE_CONVERSATION" select="'http://general_server.org/xmlnamespaces/conversation/2006'"/><xsl:variable name="NAMESPACE_XSD" select="'http://www.w3.org/2001/XMLSchema'"/><xsl:variable name="NAMESPACE_HTML" select="'http://www.w3.org/1999/xhtml'"/><xsl:variable name="NAMESPACE_XSL" select="'http://www.w3.org/1999/XSL/Transform'"/><xsl:variable name="NAMESPACE_DYN" select="'http://exslt.org/dynamic'"/><xsl:variable name="NAMESPACE_DATABASE" select="'http://general_server.org/xmlnamespaces/database/2006'"/><xsl:variable name="NAMESPACE_SERVER" select="'http://general_server.org/xmlnamespaces/server/2006'"/><xsl:variable name="NAMESPACE_DEBUG" select="'http://general_server.org/xmlnamespaces/debug/2006'"/><xsl:variable name="NAMESPACE_SERVICE" select="'http://general_server.org/xmlnamespaces/service/2006'"/><xsl:variable name="NAMESPACE_REQUEST" select="'http://general_server.org/xmlnamespaces/request/2006'"/><xsl:variable name="NAMESPACE_RESPONSE" select="'http://general_server.org/xmlnamespaces/response/2006'"/><xsl:variable name="NAMESPACE_RX" select="'http://general_server.org/xmlnamespaces/rx/2006'"/><xsl:variable name="NAMESPACE_REPOSITORY" select="'http://general_server.org/xmlnamespaces/repository/2006'"/><xsl:variable name="NAMESPACE_OBJECT" select="'http://general_server.org/xmlnamespaces/object/2006'"/><xsl:variable name="NAMESPACE_INTERFACE" select="'http://general_server.org/xmlnamespaces/interface/2006'"/><xsl:variable name="NAMESPACE_JAVASCRIPT" select="'http://general_server.org/xmlnamespaces/javascript/2006'"/><xsl:variable name="NAMESPACE_CLASS" select="'http://general_server.org/xmlnamespaces/class/2006'"/><xsl:variable name="NAMESPACE_CSS" select="'http://general_server.org/xmlnamespaces/css/2006'"/><xsl:variable name="NAMESPACE_XMLSECURITY" select="'http://general_server.org/xmlnamespaces/xmlsecurity/2006'"/><xsl:variable name="NAMESPACE_XMLTRANSACTION" select="'http://general_server.org/xmlnamespaces/xmltransaction/2006'"/><xsl:variable name="NAMESPACE_REGEXP" select="'http://exslt.org/regular-expressions'"/><xsl:variable name="NAMESPACE_META" select="'http://general_server.org/xmlnamespaces/meta/2006'"/><xsl:variable name="NAMESPACE_SESSION" select="'http://general_server.org/xmlnamespaces/session/2006'"/><xsl:variable name="NAMESPACE_FLOW" select="'http://exslt.org/flow'"/><xsl:variable name="NAMESPACE_STR" select="'http://exslt.org/strings'"/><xsl:template match="object:Manager" mode="full_title" meta:base-class-count="8">
    <xsl:text>Dickhead.</xsl:text>
    (<xsl:value-of select="@test"/>)
  </xsl:template><xsl:template match="object:Test" meta:base-class-count="8"/><xsl:template match="object:Person|object:Manager" mode="full_title" meta:base-class-count="7">
    <xsl:text>Ms.</xsl:text>
    <xsl:value-of select="@test"/>
    <xsl:if xmlns="http://general_server.org/xmlnamespaces/general_server/2006" test="@test = 'wibble'"><div xmlns="http://www.w3.org/1999/xhtml">Wibble mode!</div></xsl:if>
  </xsl:template><xsl:template match="interface:Forum|interface:AdvancedForum" mode="default_content" meta:base-class-count="6">
    
    <div class="details">
      <xsl:apply-templates select="gs:*"/>

      <ul class="children">
        
        <xsl:apply-templates select="*" mode="summary"/>
      </ul>
    </div>
  </xsl:template><xsl:template match="interface:Forum/*|interface:AdvancedForum/*" mode="summary" meta:base-class-count="6"/><xsl:template match="interface:Forum/gs:title|interface:AdvancedForum/gs:title" meta:base-class-count="6">
    <h1 class="{local-name()}"><xsl:value-of select="."/></h1>
  </xsl:template><xsl:template match="interface:Forum/gs:description|interface:AdvancedForum/gs:description" meta:base-class-count="6">
    <div class="{local-name()}"><xsl:value-of select="."/></div>
  </xsl:template><xsl:template match="interface:ForumPost" mode="summary" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/>
    

    <li class="{$gs_html_identifier_class}">
      <div class="details">
        <div><xsl:value-of select="gs:title"/></div>
        <div><xsl:value-of select="gs:body"/></div>
      </div>
    </li>
  </xsl:template><xsl:template match="interface:HTMLWebpage" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/>
    
    
    
    <xsl:variable name="gs_environments" select="ancestor::*/meta:environment"/>

    
    <xsl:if test="not($gs_resource_server)"><xsl:comment>no $gs_resource_server: webpage @components libraries will be loaded from this same server</xsl:comment></xsl:if>
    <xsl:if test="html:head"><xsl:comment>html:head tag not relevant in interface:HTMLWebpage. you should use meta:head instead</xsl:comment></xsl:if>
    <xsl:if test="not(@general-server-framework)"><xsl:comment>the general server framework has not been included. nothing will work!</xsl:comment></xsl:if>
    <xsl:if test="not(@code-class)"><xsl:comment>class JavaScript has not been included. the objects will have no functionality</xsl:comment></xsl:if>
    <xsl:if test="not(@class-styles)"><xsl:comment>class CSS styles has not been included. bare non-formatted output</xsl:comment></xsl:if>
    <xsl:if test="not($gs_environments)"><xsl:comment>no meta:environment discovered (Server, LinkedServer, User, etc.)</xsl:comment></xsl:if>

    <html>
      <head>
        <xsl:copy-of select="meta:head/@*"/>

        
        <meta name="Copyright" content="Annesley Solutions ltd. All rights reserved"/>
        <meta name="Resource-Server" content="{$gs_resource_server}"/>
        <meta charset="UTF-8"/>

        
        <xsl:if test="not(meta:head/html:meta/@name='robots')"><meta name="robots" content=""/></xsl:if>
        <xsl:if test="not(meta:head/html:meta/@name='revisit-after')"><meta name="revisit-after" content="2"/></xsl:if>
        <xsl:if test="not(meta:head/html:meta/@name='reply-to')"><meta name="reply-to" content="annesley_newholm [at] yahoo [dot] it"/></xsl:if>
        <xsl:if test="not(meta:head/html:meta/@name='Rating')"><meta name="Rating" content="General"/></xsl:if>
        <xsl:if test="not(meta:head/html:meta/@name='Pragma')"><meta name="Pragma" content="no-cache"/></xsl:if>
        <xsl:if test="not(meta:head/html:meta/@name='Language')"><meta name="Language" content="en"/></xsl:if>
        <xsl:if test="not(meta:head/html:meta/@name='distribution')"><meta name="distribution" content="Global"/></xsl:if>
        <xsl:if test="not(meta:head/html:meta/@name='Classification')"><meta name="Classification" content="Research"/></xsl:if>
        <xsl:if test="not(meta:head/html:meta/@name='Author')"><meta name="Author" content="Annesley Newholm"/></xsl:if>

        <xsl:comment>webpage @components (<xsl:value-of select="count(@*)"/>)</xsl:comment>
        <xsl:apply-templates select="@*" mode="gs_view_render_head"/>

        
        <xsl:if test="$gs_environments">
          <xsl:comment>environment objects (<xsl:value-of select="count($gs_environments/*)"/>)</xsl:comment>
          <xsl:apply-templates select="$gs_environments/*" mode="gs_view_render"/>
        </xsl:if>

        
        <xsl:apply-templates select="meta:head/*" mode="gs_view_render"/>
      </head>

      <body id="body">
        <xsl:attribute name="class">
          <xsl:text>body </xsl:text>
          <xsl:if test="@body-class"><xsl:value-of select="@body-class"/><xsl:text> </xsl:text></xsl:if>
          <xsl:value-of select="$gs_html_identifier_class"/>
        </xsl:attribute>

        <xsl:apply-templates mode="gs_view_render"/>
        <div id="gs-copyright">Â© Annesley Solutions Ltd. 2013 All rights reserved.</div>
      </body>
    </html>
  </xsl:template><xsl:template match="interface:HTMLWebpage/html:head" meta:base-class-count="6"/><xsl:template match="interface:HTMLWebpage/html:head/html:meta[@name='Copyright']" meta:base-class-count="6">
    <xsl:comment>html:meta tag [<xsl:value-of select="@name"/>] not allowed</xsl:comment>
  </xsl:template><xsl:template match="interface:HTMLWebpage/html:head/html:meta[@charset]" meta:base-class-count="6">
    <xsl:comment>html:meta tag [@charset] not allowed</xsl:comment>
  </xsl:template><xsl:template match="interface:HTMLWebpage/@*" mode="gs_view_render_head" meta:base-class-count="6"/><xsl:template match="interface:HTMLWebpage/@response:classes" mode="gs_view_render_head" meta:base-class-count="6">
    <xsl:comment><xsl:value-of select="."/></xsl:comment>
  </xsl:template><xsl:template match="interface:HTMLWebpage/@title" mode="gs_view_render_head" meta:base-class-count="6">
    <title><xsl:value-of select="."/></title>
  </xsl:template><xsl:template match="interface:HTMLWebpage/@favicon" mode="gs_view_render_head" meta:base-class-count="6">
    <link rel="shortcut icon" href="{.}"/>
    <link rel="icon" href="{.}" type="image/x-icon"/>    
  </xsl:template><xsl:template match="interface:HTMLWebpage/@jQuery" mode="gs_view_render_head" meta:base-class-count="6">
    <xsl:apply-templates select="." mode="gs_view_render_libraries">
      <xsl:with-param name="gs_standard_libraries" select="'jquery'"/>
      <xsl:with-param name="gs_min" select="../@jQuery-delivery"/>
      <xsl:with-param name="gs_version" select="."/>
    </xsl:apply-templates>
  </xsl:template><xsl:template match="interface:HTMLWebpage/@jQueryUI" mode="gs_view_render_head" meta:base-class-count="6">
    <xsl:apply-templates select="." mode="gs_view_render_libraries">
      <xsl:with-param name="gs_standard_libraries" select="'jquery-ui'"/>
      <xsl:with-param name="gs_min" select="../@jQuery-delivery"/>
      <xsl:with-param name="gs_version" select="."/>
    </xsl:apply-templates>
  </xsl:template><xsl:template match="interface:HTMLWebpage/@standard-libraries" mode="gs_view_render_head" meta:base-class-count="6">
    <xsl:apply-templates select="." mode="gs_view_render_libraries"/>
  </xsl:template><xsl:template match="interface:HTMLWebpage/@general-server-framework" mode="gs_view_render_head" meta:base-class-count="6">
    <script type="text/javascript" src="/~JavaScript/repository:framework/javascript:code.js">
      <xsl:apply-templates select="." mode="gs_enforce_non_self_closing"/>
    </script>
    
  </xsl:template><xsl:template match="interface:HTMLWebpage/@class-styles" mode="gs_view_render_head" meta:base-class-count="6">
    
    
    <xsl:variable name="gs_classes_and_bases" select="string(../@response:classes-and-bases)"/>

    <link rel="stylesheet" type="text/css">
      <xsl:attribute name="href">
        <xsl:text>/</xsl:text>
        <xsl:if test="$gs_classes_and_bases">(<xsl:value-of select="$gs_classes_and_bases"/>)</xsl:if>
        <xsl:if test="not($gs_classes_and_bases)">$gs_website_classes</xsl:if>
        <xsl:text>/css:stylesheet.css</xsl:text>
      </xsl:attribute>
    </link>
  </xsl:template><xsl:template match="interface:HTMLWebpage/@code-class" mode="gs_view_render_head" meta:base-class-count="6">
    
    
    <xsl:variable name="gs_classes_and_bases" select="string(../@response:classes-and-bases)"/>
    
    <script type="text/javascript">
      <xsl:attribute name="src">
        <xsl:text>/</xsl:text>
        <xsl:if test="$gs_classes_and_bases">(<xsl:value-of select="$gs_classes_and_bases"/>)</xsl:if>
        <xsl:if test="not($gs_classes_and_bases)">$gs_website_classes</xsl:if>
        <xsl:text>.js</xsl:text>
      </xsl:attribute>

      <xsl:apply-templates select="." mode="gs_enforce_non_self_closing"/>
    </script>
  </xsl:template><xsl:template match="interface:HTMLWebpage/@code-class-dependencies" mode="gs_view_render_head" meta:base-class-count="6">
    <xsl:apply-templates select="." mode="gs_external_class_javascript_dependencies"/>
  </xsl:template><xsl:template match="interface:HTMLWebpage/@class-style-dependencies" mode="gs_view_render_head" meta:base-class-count="6">
    <xsl:apply-templates select="." mode="gs_external_class_css_dependencies"/>
  </xsl:template><xsl:template match="*|node()|@*|text()" mode="gs_view_render_libraries" meta:base-class-count="6"><xsl:param name="gs_standard_libraries" select="string(.)"/><xsl:param name="gs_version"/><xsl:param name="gs_minified"/>
    
    
    
    
    <xsl:variable name="gs_standard_library_fragment">
      <xsl:if test="contains($gs_standard_libraries, ',')"><xsl:value-of select="substring-before($gs_standard_libraries, ',')"/></xsl:if>
      <xsl:if test="not(contains($gs_standard_libraries, ','))"><xsl:value-of select="$gs_standard_libraries"/></xsl:if>
    </xsl:variable>
    <xsl:variable name="gs_standard_library" select="string($gs_standard_library_fragment)"/>

    
    <script type="text/javascript">
      <xsl:attribute name="src">
        <xsl:value-of select="$gs_resource_server"/>
        <xsl:text>/resources/shared/</xsl:text>
        <xsl:if test="starts-with($gs_standard_library, 'jquery')">jquery/</xsl:if>
        <xsl:if test="starts-with($gs_standard_library, 'jquery.')">plugins/</xsl:if>
        <xsl:value-of select="$gs_standard_library"/>
        <xsl:if test="$gs_version">-<xsl:value-of select="$gs_version"/></xsl:if>
        <xsl:if test="$gs_minified">.min</xsl:if>
        <xsl:text>.js</xsl:text>
      </xsl:attribute>  
      
      
      <xsl:if test="not($gs_resource_server)"><xsl:attribute name="gs:warning">including [<xsl:value-of select="$gs_standard_library"/>] from same server because $gs_resource_server not present</xsl:attribute></xsl:if>

      <xsl:apply-templates select="." mode="gs_enforce_non_self_closing"/>
    </script>
    
    <xsl:if test="contains($gs_standard_libraries, ',')">
      <xsl:apply-templates select="." mode="gs_view_render_libraries">
        <xsl:with-param name="gs_standard_libraries" select="substring-after($gs_standard_libraries, ',')"/>
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template><xsl:template match="html:html" mode="editor" meta:base-class-count="6">
    <xsl:apply-templates select="."/>
  </xsl:template><xsl:template match="html:html" mode="default_content" meta:base-class-count="6">
    
    <xsl:apply-templates select="*" mode="gs_view_render"/>
  </xsl:template><xsl:template match="javascript:dependency" mode="html" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/>
    
    

    <xsl:variable name="gs_object_name">
      <xsl:choose>
        <xsl:when test="not(@object-name) or @object-name = '' or @object-name = 'inherit'">
          <xsl:value-of select="local-name(ancestor::class:*[1])"/>
        </xsl:when>
        <xsl:otherwise><xsl:value-of select="@object-name"/></xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <script type="text/javascript" class="{$gs_html_identifier_class} details" xml:add-xml-id="no">
      <xsl:attribute name="src">
        <xsl:if test="starts-with(@uri, '/resources/')">
          <xsl:value-of select="$gs_resource_server"/>
        </xsl:if>
        <xsl:value-of select="@uri"/>
      </xsl:attribute>
      <xsl:if test="$gs_object_name"><xsl:attribute name="gs:object-name"><xsl:value-of select="$gs_object_name"/></xsl:attribute></xsl:if>

      <xsl:apply-templates select="." mode="gs_enforce_non_self_closing"/>
    </script>
  </xsl:template><xsl:template match="css:stylesheet" mode="standalone_contents" meta:base-class-count="6">
    
    <xsl:apply-templates select="." mode="html_details"/>
  </xsl:template><xsl:template match="css:stylesheet" mode="editor" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/>
    
    

    <xsl:variable name="gs_object_name">
      <xsl:choose>
        <xsl:when test="not(@name) or @name = '' or @name = 'inherit'">
          <xsl:text>CSS__(class-name)</xsl:text>
        </xsl:when>
        <xsl:otherwise>
          <xsl:text>CSS__</xsl:text><xsl:value-of select="@name"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <div class="{$gs_html_identifier_class} details" xml:add-xml-id="no">
      <xsl:apply-templates select="*" mode="editor">
        <xsl:with-param name="gs_object_name" select="normalize-space($gs_object_name)"/>
      </xsl:apply-templates>
    </div>
  </xsl:template><xsl:template match="css:stylesheet" mode="html" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/>
    
    

    
    <xsl:apply-templates select="css:dependency" mode="html"/>

    <style type="text/css" class="{$gs_html_identifier_class} details" xml:add-xml-id="no">
      <xsl:apply-templates select="." mode="html_details"/>
    </style>
  </xsl:template><xsl:template match="css:stylesheet" mode="html_details" meta:base-class-count="6">
    
    <xsl:variable name="gs_class" select="ancestor::class:*[1]"/>
    <xsl:variable name="gs_object_name">
      <xsl:choose>
        <xsl:when test="@name and @name = ''"/>
        <xsl:when test="$gs_class and (not(@name) or @name = 'inherit')">
          <xsl:text>CSS__</xsl:text><xsl:value-of select="local-name($gs_class)"/>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="@name"/>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:apply-templates select="*" mode="gs_css_stylesheet_clauses"/>
    
    <xsl:apply-templates>
      <xsl:with-param name="gs_object_name" select="normalize-space($gs_object_name)"/>
    </xsl:apply-templates>
  </xsl:template><xsl:template match="*" mode="gs_css_stylesheet_clauses" meta:base-class-count="6">
    <xsl:apply-templates select="*" mode="gs_css_stylesheet_clauses"/>
  </xsl:template><xsl:template match="css:stylesheet" mode="gs_listname" meta:base-class-count="6">
    <xsl:text>view</xsl:text>
  </xsl:template><xsl:template match="css:dependency" meta:base-class-count="6">
    <xsl:text>/* css:dependency [</xsl:text>
    <xsl:value-of select="@uri"/>
    <xsl:text>] needs to be rendered outside this stylesheet */</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template><xsl:template match="css:*/text()" meta:base-class-count="6">
    
    <xsl:value-of select="normalize-space(.)" disable-output-escaping="yes"/>
  </xsl:template><xsl:template match="css:raw" meta:base-class-count="6">
    <xsl:apply-templates/>
  </xsl:template><xsl:template match="css:section" meta:base-class-count="6"><xsl:param name="gs_object_name"/>
    
    <xsl:if test="@name">
      <xsl:value-of select="$gs_newline"/>
      <xsl:text>/* ################ </xsl:text>
      <xsl:value-of select="@name"/>
      <xsl:text> ################# */</xsl:text>
      <xsl:value-of select="$gs_newline"/>
    </xsl:if>

    <xsl:apply-templates>
      <xsl:with-param name="gs_object_name" select="$gs_object_name"/>
    </xsl:apply-templates>
  </xsl:template><xsl:template match="css:clause/css:path|css:path" meta:base-class-count="6">
    
  </xsl:template><xsl:template match="css:path" mode="gs_css_path_output" meta:base-class-count="6">
    
    <xsl:apply-templates/>
  </xsl:template><xsl:template match="css:clause/css:path" mode="gs_css_path_output" meta:base-class-count="6"><xsl:param name="gs_object_name"/>
    
    

    <xsl:if test="$gs_object_name">
      <xsl:text>.</xsl:text><xsl:value-of select="$gs_object_name"/>
    </xsl:if>

    
    <xsl:apply-templates select="../ancestor::css:*/css:path" mode="gs_css_path_output">
      <xsl:sort select="database:document-order()" data-type="number"/>
    </xsl:apply-templates>

    <xsl:apply-templates/>

    <xsl:if test="following-sibling::css:path">, </xsl:if>
  </xsl:template><xsl:template match="css:path/css:*" mode="gs_css_relation" meta:base-class-count="6"><xsl:param name="gs_default"/>
    
    
    
    <xsl:variable name="gs_setting_explicit" select="(@relation|../@relation)[1]"/>
    <xsl:variable name="gs_setting_fragment">
      <xsl:value-of select="$gs_setting_explicit"/>
      <xsl:if test="not($gs_setting_explicit)"><xsl:value-of select="$gs_default"/></xsl:if>
    </xsl:variable>
    <xsl:variable name="gs_setting" select="string($gs_setting_fragment)"/>
    
    <xsl:choose>
      <xsl:when test="$gs_setting = 'child-only'">
        <xsl:text disable-output-escaping="yes"> &gt; </xsl:text>
      </xsl:when>
      <xsl:when test="$gs_setting = 'self'"/>
      <xsl:otherwise>
        <xsl:text> </xsl:text>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template><xsl:template match="css:path/css:class" meta:base-class-count="6">
    <xsl:apply-templates select="." mode="gs_css_relation"/>
    <xsl:text>.</xsl:text><xsl:apply-templates/>
    <xsl:apply-templates select="@*|../@*"/>
  </xsl:template><xsl:template match="css:path/css:interface" meta:base-class-count="6">
    <xsl:apply-templates select="." mode="gs_css_relation">
      <xsl:with-param name="gs_default" select="'self'"/>
    </xsl:apply-templates>
    <xsl:text>.gs-interface-mode-</xsl:text><xsl:apply-templates/>
    <xsl:apply-templates select="@*|../@*"/>
  </xsl:template><xsl:template match="css:path/css:local-name" meta:base-class-count="6">
    <xsl:apply-templates select="." mode="gs_css_relation">
      <xsl:with-param name="gs_default" select="'self'"/>
    </xsl:apply-templates>
    <xsl:text>.gs-local-name-</xsl:text><xsl:apply-templates/>
    <xsl:apply-templates select="@*|../@*"/>
  </xsl:template><xsl:template match="css:path/css:object" meta:base-class-count="6">
    <xsl:apply-templates select="." mode="gs_css_relation"/>
    <xsl:text>.Class__</xsl:text><xsl:apply-templates/>
    <xsl:apply-templates select="@*|../@*"/>
  </xsl:template><xsl:template match="css:path/css:id" meta:base-class-count="6">
    <xsl:apply-templates select="." mode="gs_css_relation"/>
    <xsl:text>#</xsl:text><xsl:apply-templates/>
    <xsl:apply-templates select="@*|../@*"/>
  </xsl:template><xsl:template match="css:path/css:element" meta:base-class-count="6">
    <xsl:apply-templates select="." mode="gs_css_relation"/>
    <xsl:apply-templates/>
    <xsl:apply-templates select="@*|../@*"/>
  </xsl:template><xsl:template match="css:*/@*" meta:base-class-count="6"/><xsl:template match="css:*/@with-class" meta:base-class-count="6">
    <xsl:text>.</xsl:text><xsl:value-of select="."/>
  </xsl:template><xsl:template match="css:*/@with-pseudo" meta:base-class-count="6">
    <xsl:text>:</xsl:text><xsl:value-of select="."/>
  </xsl:template><xsl:template match="css:*/@with-name" meta:base-class-count="6">
    <xsl:variable name="gs_value" select="translate(., $gs_uppercase, $gs_lowercase)"/>
    
    <xsl:text>.gs-name-</xsl:text>
    <xsl:choose>
      <xsl:when test="contains($gs_value, ':')">
        <xsl:value-of select="substring-before($gs_value, ':')"/>
        <xsl:text>-</xsl:text>
        <xsl:value-of select="substring-after($gs_value, ':')"/>
      </xsl:when>
      <xsl:otherwise>
        <xsl:value-of select="$gs_value"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template><xsl:template match="css:clause" meta:base-class-count="6"><xsl:param name="gs_object_name"/>
    

    
    <xsl:if test="not(css:path)">
      <xsl:if test="$gs_object_name">
        <xsl:text>.</xsl:text><xsl:value-of select="$gs_object_name"/>
      </xsl:if>
      <xsl:apply-templates select="../ancestor::css:*/css:path" mode="gs_css_path_output">
        <xsl:sort select="database:document-order()" data-type="number"/>
      </xsl:apply-templates>
    </xsl:if>
    <xsl:apply-templates select="css:path" mode="gs_css_path_output">
      <xsl:with-param name="gs_object_name" select="$gs_object_name"/>
      <xsl:sort select="database:document-order()" data-type="number"/>
    </xsl:apply-templates>
    <xsl:text> {</xsl:text>

    
    <xsl:if test="@name">
      <xsl:text>/* </xsl:text>
      <xsl:value-of select="@name"/>
      <xsl:text> */</xsl:text>
    </xsl:if>

    <xsl:value-of select="$gs_newline"/>

    
    <xsl:apply-templates/>
    <xsl:text>}</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template><xsl:template match="css:clause/css:*" meta:base-class-count="6">
    
    <xsl:if test="css:*"><xsl:apply-templates select="css:*" mode="gs_css_item"/></xsl:if>
    <xsl:if test="not(css:*)"><xsl:apply-templates select="." mode="gs_css_item"/></xsl:if>
  </xsl:template><xsl:template match="css:clause" mode="gs_css_item_collection" meta:base-class-count="6"/><xsl:template match="css:*" mode="gs_css_item_collection" meta:base-class-count="6">
    <xsl:value-of select="local-name()"/>
    <xsl:text>-</xsl:text>
  </xsl:template><xsl:template match="css:*" mode="gs_css_item" meta:base-class-count="6">
    
    <xsl:text>  </xsl:text>
    <xsl:apply-templates select=".." mode="gs_css_item_collection"/>
    <xsl:value-of select="local-name()"/>
    <xsl:text>:</xsl:text>

    <xsl:choose>
      <xsl:when test="not(@type)"/>
      <xsl:when test="@type = 'hash'">#</xsl:when>
      <xsl:when test="@type = 'url'">
        <xsl:text>url(</xsl:text>
        <xsl:value-of select="$gs_resource_server"/>
      </xsl:when>
    </xsl:choose>

    <xsl:apply-templates/>

    <xsl:choose>
      <xsl:when test="not(@metric)"/>
      <xsl:when test="@metric = 'percentage'">%</xsl:when>
      <xsl:when test="@metric = 'percent'">%</xsl:when>
      <xsl:when test="@metric = 'pixel'">px</xsl:when>
      <xsl:when test="@metric = 'proportional'">em</xsl:when>
      <xsl:when test="@metric = 'point'">pt</xsl:when>
      <xsl:when test="@metric = 'millimeter'">mm</xsl:when>
      <xsl:when test="@metric = 'centimeter'">cm</xsl:when>
      <xsl:when test="@metric = 'inches'">in</xsl:when>
      <xsl:when test="@metric = 'inch'">in</xsl:when>
      <xsl:when test="@metric = 'pica'">pc</xsl:when>
      <xsl:when test="@metric = 'x-height'">ex</xsl:when>
    </xsl:choose>

    <xsl:choose>
      <xsl:when test="not(@type)"/>
      <xsl:when test="@type = 'url'">)</xsl:when>
    </xsl:choose>

    <xsl:if test="@important = 'yes' or ../@important = 'yes'">!important</xsl:if>
    <xsl:text>;</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template><xsl:template match="css:font-family" mode="gs_css_stylesheet_clauses" meta:base-class-count="6">
    
    <xsl:apply-templates select="str:split(., ',')" mode="gs_css_font_faces">
      <xsl:with-param name="gs_font_type" select="@type"/>
    </xsl:apply-templates>
  </xsl:template><xsl:template match="token[normalize-space(.)='Verdana']" mode="gs_css_font_faces" meta:base-class-count="6"/><xsl:template match="token[normalize-space(.)='Arial']" mode="gs_css_font_faces" meta:base-class-count="6"/><xsl:template match="token[normalize-space(.)='Helvetica']" mode="gs_css_font_faces" meta:base-class-count="6"/><xsl:template match="token[normalize-space(.)='Times New Roman']" mode="gs_css_font_faces" meta:base-class-count="6"/><xsl:template match="token[normalize-space(.)='Times']" mode="gs_css_font_faces" meta:base-class-count="6"/><xsl:template match="token[normalize-space(.)='Courier New']" mode="gs_css_font_faces" meta:base-class-count="6"/><xsl:template match="token[normalize-space(.)='Courier']" mode="gs_css_font_faces" meta:base-class-count="6"/><xsl:template match="token[normalize-space(.)='Georgia']" mode="gs_css_font_faces" meta:base-class-count="6"/><xsl:template match="token[normalize-space(.)='Palatino']" mode="gs_css_font_faces" meta:base-class-count="6"/><xsl:template match="token[normalize-space(.)='Garamond']" mode="gs_css_font_faces" meta:base-class-count="6"/><xsl:template match="token[normalize-space(.)='Bookman']" mode="gs_css_font_faces" meta:base-class-count="6"/><xsl:template match="token[normalize-space(.)='Comic Sans MS']" mode="gs_css_font_faces" meta:base-class-count="6"/><xsl:template match="token[normalize-space(.)='Trebuchet MS']" mode="gs_css_font_faces" meta:base-class-count="6"/><xsl:template match="token[normalize-space(.)='Arial Black']" mode="gs_css_font_faces" meta:base-class-count="6"/><xsl:template match="token[normalize-space(.)='Impact']" mode="gs_css_font_faces" meta:base-class-count="6"/><xsl:template match="token[normalize-space(.)='serif']" mode="gs_css_font_faces" meta:base-class-count="6"/><xsl:template match="token[normalize-space(.)='sans-serif']" mode="gs_css_font_faces" meta:base-class-count="6"/><xsl:template match="token" mode="gs_css_font_faces" meta:base-class-count="6"><xsl:param name="gs_font_type"/>
    
    <xsl:variable name="gs_font_name" select="normalize-space(.)"/>
    
    <xsl:text>@font-face {</xsl:text>
      <xsl:text>  font-family: </xsl:text><xsl:value-of select="$gs_font_name"/><xsl:text>;</xsl:text>
      <xsl:value-of select="$gs_newline"/>
      <xsl:text>  src: url(</xsl:text>
      <xsl:value-of select="$gs_resource_server"/>
      <xsl:text>/resources/shared/fonts/</xsl:text>
      <xsl:value-of select="$gs_font_name"/><xsl:text>/</xsl:text>
      <xsl:text>normal.</xsl:text>
      <xsl:value-of select="@type"/>
      <xsl:if test="not(@type)">ttf</xsl:if>
      <xsl:text>);</xsl:text>
    <xsl:text>}</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template><xsl:template match="css:icon" mode="gs_css_item" meta:base-class-count="6">
    <xsl:variable name="gs_css_filename" select="string(.)"/>

    <xsl:text>  </xsl:text>
    <xsl:if test="not($gs_css_filename)">
      <xsl:text>background-image:none;</xsl:text>
    </xsl:if>
    <xsl:if test="$gs_css_filename">
      <xsl:text>background-image:url(</xsl:text>
      <xsl:value-of select="$gs_resource_server"/>
      <xsl:text>/resources/shared/images/</xsl:text>
      <xsl:choose>
        <xsl:when test="@group and @group = ''"/>
        <xsl:when test="@group"><xsl:value-of select="@group"/>/</xsl:when>
        <xsl:otherwise test="not(@group)">icons/</xsl:otherwise>
      </xsl:choose>
      <xsl:value-of select="."/>
      <xsl:text>.</xsl:text>
      <xsl:if test="@type"><xsl:value-of select="@type"/></xsl:if>
      <xsl:if test="not(@type)">png</xsl:if>
      <xsl:text>);</xsl:text>
    </xsl:if>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template><xsl:template match="css:section" mode="editor" meta:base-class-count="6"><xsl:param name="gs_object_name"/>
    

    <div class="gs_css_section">
      <div class="gs_css_section_header">
        <xsl:if test="@name">
          <xsl:value-of select="$gs_newline"/>
          <xsl:text>/* ################ </xsl:text>
          <xsl:value-of select="@name"/>
          <xsl:text> ################# */</xsl:text>
          <xsl:value-of select="$gs_newline"/>
        </xsl:if>
      </div>

      <xsl:apply-templates select="*" mode="editor">
        <xsl:with-param name="gs_object_name" select="$gs_object_name"/>
      </xsl:apply-templates>
    </div>
  </xsl:template><xsl:template match="css:section/css:path" mode="editor" meta:base-class-count="6"/><xsl:template match="css:clause" mode="editor" meta:base-class-count="6"><xsl:param name="gs_object_name"/>
    

    <div class="gs_css_clause">
      <div class="gs_css_path_editor">
        <xsl:if test="not(css:path)"><div class="gs-warning">path missing! at least one path element is required, even if blank</div></xsl:if>
        <xsl:apply-templates select="css:path" mode="gs_css_path_output">
          <xsl:with-param name="gs_object_name" select="$gs_object_name"/>
          <xsl:sort select="database:document-order()" data-type="number"/>
        </xsl:apply-templates>
        <span class="gs_css_curlies before"> {</span>
      </div>
      <table class="gs_css_properties">
        <xsl:apply-templates select="*" mode="editor"/>
      </table>
      <div class="gs_css_curlies after">}</div>
    </div>
  </xsl:template><xsl:template match="css:clause/css:path" mode="editor" meta:base-class-count="6"/><xsl:template match="css:clause/*" mode="editor" meta:base-class-count="6">
    <xsl:variable name="name" select="@xml:id"/>
    <tr>
      <td class="gs_css_properties_label">
        <label for="{$name}"><xsl:value-of select="local-name()"/></label>
        <xsl:text>:</xsl:text>
      </td>
      <td>
        
        <form class="f_submit_ajax" meta:dom-method="set-value" method="post"><div>
          <input name="gs_xsd_xpath" type="hidden" value="/config/classes/general_server/client_functionality/CSSStylesheet/model"/>
          <input name="select" type="hidden" value="{@meta:xpath-to-node}"/>
          <input name="new_value" value="{.}"/>
          <input value="set" type="submit"/>
        </div></form>
      </td>
     </tr>
  </xsl:template><xsl:template match="interface:Debugger" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/>
    

    <div class="{$gs_html_identifier_class} gs-hidden" style="display:none">
      <div class="gs-debugger-title">Debugger</div>
      <xsl:apply-templates mode="gs_view_render"/>
      <ul class="gs-template-ids"/>
    </div>
  </xsl:template><xsl:template match="*" mode="gs_javascript_before_method" meta:base-class-count="6"><xsl:param name="gs_object_name" select="'BaseObject'"/>
    
    

    <xsl:if test="$gs_request/gs:query-string/gs:debug-javascript-functions = 'on'">
      <xsl:if test="not(@name = 'toString' or @name = 'valueOf' or @name = 'method' or @debug = 'off' or ../@debug = 'off')">
        <xsl:text>    //object.method logging injected by JavaScript class:</xsl:text>
        <xsl:value-of select="$gs_newline"/>
        <xsl:text>    if (window.Debug &amp;&amp; window.Debug.method) Debug.method(</xsl:text>
        <xsl:text>this,</xsl:text>
        <xsl:value-of select="$gs_object_name"/>
        <xsl:text>,"</xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:text>","</xsl:text><xsl:value-of select="@parameters"/><xsl:text>"</xsl:text>
        <xsl:text>,arguments);</xsl:text>
        <xsl:value-of select="$gs_newline"/>
      </xsl:if>
    </xsl:if>
  </xsl:template><xsl:template match="javascript:object" mode="gs_javascript_before_method" meta:base-class-count="6"><xsl:param name="gs_object_name" select="'BaseObject'"/>
    
    

    <xsl:if test="$gs_request/gs:query/gs:debug-javascript-functions = 'on'">
      <xsl:if test="not(@debug = 'off' or ../@debug = 'off')">
        <xsl:text>    //object.method logging injected by JavaScript class:</xsl:text>
        <xsl:value-of select="$gs_newline"/>
        <xsl:text>    if (window.Debug &amp;&amp; window.Debug.method) Debug.method(</xsl:text>
        <xsl:text>this,</xsl:text>
        <xsl:value-of select="$gs_object_name"/>
        <xsl:text>,"init","",arguments);</xsl:text>
        <xsl:value-of select="$gs_newline"/>
      </xsl:if>
    </xsl:if>
  </xsl:template><xsl:template match="interface:MultiDocument" mode="default_content" meta:base-class-count="6">
    
    <xsl:apply-templates select="interface:*" mode="gs_view_render"/>

    
    <ul class="gs-jquery-nodrag gs-sort-axis-x">
      <xsl:apply-templates select="gs:tab" mode="gs_full_tab"/>
    </ul>

    
    <xsl:apply-templates select="gs:tab" mode="gs_full_tab_container"/>
  </xsl:template><xsl:template match="gs:tab-template" mode="gs_full_tab" meta:base-class-count="6">
  </xsl:template><xsl:template match="gs:tab" mode="gs_full_tab" meta:base-class-count="6">
    <xsl:variable name="gs_tab_id"><xsl:if test="@id"><xsl:value-of select="@id"/></xsl:if><xsl:if test="not(@id)"><xsl:value-of select="translate(@title, ' ', '-')"/></xsl:if></xsl:variable>
    <li id="tab-{$gs_tab_id}-tab"><a href="#tab-{$gs_tab_id}"><xsl:value-of select="@title"/></a></li>
  </xsl:template><xsl:template match="gs:tab" mode="gs_full_tab_container" meta:base-class-count="6">
    <xsl:variable name="gs_tab_id"><xsl:if test="@id"><xsl:value-of select="@id"/></xsl:if><xsl:if test="not(@id)"><xsl:value-of select="translate(@title, ' ', '-')"/></xsl:if></xsl:variable>

    <div id="tab-{$gs_tab_id}" class="gs-tab-container">
      <xsl:apply-templates select="." mode="gs_full_tab_content"/>
    </div>
  </xsl:template><xsl:template match="gs:tab" mode="gs_full_tab_content" meta:base-class-count="6">
    
    <xsl:apply-templates mode="gs_view_render"/>
  </xsl:template><xsl:template match="interface:Explorer" mode="gs_event_functions" meta:base-class-count="6">
    <xsl:text>f_ready_setFrameHeights f_resize_setFrameHeights</xsl:text>
  </xsl:template><xsl:template match="interface:IDE/@title" mode="full_title" meta:base-class-count="6"/><xsl:template match="interface:IDE" mode="gs_doxygen_support" meta:base-class-count="6">
    
    
    <link href="{$gs_resource_server}/documentation/html/doxygen.css" rel="stylesheet" type="text/css"/>
    <link href="{$gs_resource_server}/documentation/html/search/search.css" rel="stylesheet" type="text/css"/>
    
    <style>
      .gs-html-fake-body-div .gs-warning {display:none;}
      .gs-html-fake-body-div #top {display:none;}
    </style>
  </xsl:template><xsl:template match="interface:IDE//xsd:schema[@name='search']" mode="gs_form_render_submit" meta:base-class-count="6">
    
    <div id="gs-submit-options" class="gs-search-option">
      <input id="gs-submit-text" class="gs-form-submit gs-first gs-selected" type="submit" name="gs--submit" value="text"/>
      <input id="gs-submit-xpath" class="gs-form-submit" type="submit" name="gs--submit" value="xpath"/>
      <input id="gs-submit-class" class="gs-form-submit" type="submit" name="gs--submit" value="class"/>
      <input id="gs-submit-xmlid" class="gs-form-submit gs-last" type="submit" name="gs--submit" value="xml:id"/>
    </div>
  </xsl:template><xsl:template match="interface:MultiView" mode="default_content" meta:base-class-count="6">
    
    <xsl:apply-templates select="gs:view" mode="gs_full_view_container"/>

    
    <xsl:apply-templates select="interface:*" mode="gs_view_render"/>
    <xsl:if test="not(interface:*)">
      <ul class="Object Class__SubMenu CSS__SubMenu CSS__Overlay CSS__VerticalMenu CSS__Menu gs-draggable">
        <xsl:apply-templates select="gs:view" mode="gs_full_view"/>
      </ul>
    </xsl:if>
  </xsl:template><xsl:template match="interface:MultiView/interface:SubMenu" mode="default_content" meta:base-class-count="6">
    <xsl:apply-templates select="../gs:view" mode="gs_full_view"/>
    <xsl:apply-templates mode="gs_view_render"/>
  </xsl:template><xsl:template match="interface:MultiView" mode="editor" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/>
    
    <xsl:apply-templates select=".">
      <xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/>
    </xsl:apply-templates>
  </xsl:template><xsl:template match="gs:view" mode="gs_full_view" meta:base-class-count="6">
    <xsl:variable name="gs_view_id"><xsl:if test="@id"><xsl:value-of select="@id"/></xsl:if><xsl:if test="not(@id)"><xsl:value-of select="translate(@title, ' ', '_')"/></xsl:if></xsl:variable>
    <li>
      <xsl:attribute name="class">
        <xsl:text> f_click_changeView</xsl:text>
        <xsl:text> view-</xsl:text><xsl:value-of select="$gs_view_id"/>
        <xsl:if test="@disabled"> disabled_<xsl:value-of select="@disabled"/></xsl:if>
        <xsl:if test="position() = 1">
          <xsl:text> first</xsl:text>
          <xsl:text> selected</xsl:text> 
        </xsl:if>
      </xsl:attribute>

      <xsl:value-of select="@title"/>

      
      <ul><xsl:apply-templates select="gs:sub-control" mode="gs_full_view_sub_control"/></ul>
    </li>
  </xsl:template><xsl:template match="gs:sub-control" mode="gs_full_view_sub_control" meta:base-class-count="6">
    <xsl:variable name="gs_control_id"><xsl:if test="@id"><xsl:value-of select="@id"/></xsl:if><xsl:if test="not(@id)"><xsl:value-of select="translate(@title, ' ', '-')"/></xsl:if></xsl:variable>
    <li>
      <xsl:attribute name="class">
        <xsl:text>f_click_</xsl:text><xsl:value-of select="$gs_control_id"/>
        <xsl:if test="position() = 1"> first</xsl:if>
      </xsl:attribute>

      <xsl:value-of select="@title"/>
    </li>
  </xsl:template><xsl:template match="gs:view" mode="gs_full_view_container" meta:base-class-count="6">
    <xsl:variable name="gs_view_id"><xsl:if test="@id"><xsl:value-of select="@id"/></xsl:if><xsl:if test="not(@id)"><xsl:value-of select="translate(@title, ' ', '-')"/></xsl:if></xsl:variable>

    <div>
      <xsl:attribute name="class">
        <xsl:text>gs-view-container</xsl:text>
        <xsl:text> view-</xsl:text><xsl:value-of select="$gs_view_id"/><xsl:text>-container</xsl:text>
        <xsl:if test="position() = 1">
          <xsl:text> first</xsl:text>
          <xsl:text> selected</xsl:text> 
        </xsl:if>
      </xsl:attribute>
      <xsl:attribute name="style">
        <xsl:if test="not(position() = 1)">display:none;</xsl:if>
      </xsl:attribute>

      <xsl:apply-templates mode="gs_full_view_content"/>
    </div>
  </xsl:template><xsl:template match="gs:view/gs:sub-control" mode="gs_full_view_content" meta:base-class-count="6">
    
  </xsl:template><xsl:template match="*" mode="gs_full_view_content" meta:base-class-count="6">
    
    <xsl:apply-templates select="." mode="gs_view_render"/>
  </xsl:template><xsl:template match="interface:AJAXHTMLLoader" mode="editor" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/><xsl:param name="gs_interface_mode"/>
    
    

    <div>
      <xsl:attribute name="class">
        <xsl:value-of select="$gs_html_identifier_class"/>
        <xsl:if test="@func-class-start-notify"> CSS__<xsl:value-of select="@func-class-start-notify"/></xsl:if>
      </xsl:attribute>
      
      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups">
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      </xsl:apply-templates>
    </div>
  </xsl:template><xsl:template match="xsd:attribute[@meta:editor-class = 'class:CodeMirrorEditor']" mode="xpathajax" meta:base-class-count="7"><xsl:param name="gs_html_identifier_class"/><xsl:param name="gs_current_external_value_node" select="FORCE_SELECT_EMPTY_NODESET"/>
    
    
     

    <xsl:variable name="gs_xsd_schema" select="ancestor::xsd:schema[1]"/>
    <xsl:variable name="gs_destination_xpath" select="$gs_xsd_schema/xsd:annotation/xsd:documentation[@meta:type='xpath-to-destination']"/>

    <div class="{$gs_html_identifier_class} f_presubmit_updateFormElement f_appear_refresh f_resize_refresh">
      
      <xsl:if test="not($gs_xsd_schema)"><div class="gs_warning">cannot find owner xsd:schema element</div></xsl:if>
      <xsl:if test="not($gs_destination_xpath)"><div class="gs_warning">cannot find xsd:documentation[@meta:type='xpath-to-destination'] in xsd:schema</div></xsl:if>

      <xsl:if test="$gs_destination_xpath">
        <textarea name="xml">loading...</textarea>
        <xsl:apply-templates select="." mode="gs_meta_data">
          <xsl:with-param name="gs_namespace_uri" select="'http://general_server.org/xmlnamespaces/general_server/2006'"/>
        </xsl:apply-templates>
        <span class="gs-xtransport gs-ajax-xpath"><xsl:value-of select="$gs_destination_xpath"/></span>
      </xsl:if>
      
      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups"/>
    </div>
  </xsl:template><xsl:template match="xsd:attribute[@meta:editor-class = 'class:CodeMirrorEditor']" mode="gs_context_menu_custom" meta:base-class-count="7">
    <li class="f_click_setBreakpoint">set breakpoint</li>
    <li class="f_click_clearBreakpoint">clear breakpoint</li>
  </xsl:template><xsl:template match="xsd:schema" mode="gs_listname" meta:base-class-count="6">
    <xsl:text>model</xsl:text>
    <xsl:if test="@name">Â (<xsl:value-of select="@name"/>)</xsl:if>
  </xsl:template><xsl:template match="xsd:schema[xsd:annotation/xsd:app-info[@meta:type='multi-form']]" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/><xsl:param name="gs_title" select="xsd:annotation/xsd:*[@meta:type='title']"/><xsl:param name="gs_submit" select="xsd:annotation/xsd:*[@meta:type='submit-caption']"/><xsl:param name="gs_description" select="xsd:annotation/xsd:*[@meta:type='description']"/><xsl:param name="gs_event_functions"/><xsl:param name="gs_current_value_node" select="xsd:annotation/xsd:*[@meta:type='edit-data']/*"/>
    
                   
                  
             
         
      

    
    <xsl:variable name="gs_xsd_multi_form" select="xsd:annotation/xsd:app-info[@meta:type='multi-form']"/>
    
    <xsl:variable name="gs_event_functions_default">
      <xsl:value-of select="$gs_event_functions"/>
      <xsl:if test="not($gs_event_functions)"><xsl:value-of select="xsd:annotation/xsd:*[@meta:type='event-functions']"/></xsl:if>
    </xsl:variable>

    
    <xsl:if test="not($gs_current_value_node)">
      <div class="gs-warning">no input nodes for form to render</div>
    </xsl:if>

    <xsl:if test="$gs_current_value_node">
      <ul class="gs-interface-mode-multi-form CSS__XSchema gs-xsd-multi-form-{count($gs_current_value_node)}">
        <xsl:variable name="gs_xsd_schema" select="."/>
        <xsl:choose>
          <xsl:when test="$gs_xsd_multi_form = '@*'">
            <xsl:for-each select="$gs_current_value_node/@*">
              <li>
                <xsl:apply-templates select="$gs_xsd_schema" mode="gs_form_render">
                  <xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/>
                  <xsl:with-param name="gs_title" select="$gs_title"/>               
                  <xsl:with-param name="gs_submit" select="$gs_submit"/>              
                  <xsl:with-param name="gs_description" select="$gs_description"/>         
                  <xsl:with-param name="gs_event_functions" select="$gs_event_functions_default"/>     
                  <xsl:with-param name="gs_current_value_node" select="."/>  
                </xsl:apply-templates>
              </li>
            </xsl:for-each>
          </xsl:when>
          <xsl:when test="$gs_xsd_multi_form = '*'">
            <xsl:for-each select="$gs_current_value_node">
              <li>
                <xsl:apply-templates select="$gs_xsd_schema" mode="gs_form_render">
                  <xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/>
                  <xsl:with-param name="gs_title" select="$gs_title"/>               
                  <xsl:with-param name="gs_submit" select="$gs_submit"/>              
                  <xsl:with-param name="gs_description" select="$gs_description"/>         
                  <xsl:with-param name="gs_event_functions" select="$gs_event_functions_default"/>     
                  <xsl:with-param name="gs_current_value_node" select="."/>  
                </xsl:apply-templates>
              </li>
            </xsl:for-each>
          </xsl:when>
          <xsl:when test="not(string($gs_xsd_multi_form))">
            <div class="gs-warning">multi-form mode blank. only * and @* are understood</div>
          </xsl:when>
          <xsl:otherwise>
            <div class="gs-warning">multi-form mode [<xsl:value-of select="$gs_xsd_multi_form"/>] not recognised. only * and @* are understood</div>
          </xsl:otherwise>
        </xsl:choose>
      </ul>
    </xsl:if>
  </xsl:template><xsl:template match="xsd:schema" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/><xsl:param name="gs_title" select="xsd:annotation/xsd:*[@meta:type='title']"/><xsl:param name="gs_submit" select="xsd:annotation/xsd:*[@meta:type='submit-caption']"/><xsl:param name="gs_description" select="xsd:annotation/xsd:*[@meta:type='description']"/><xsl:param name="gs_event_functions"/><xsl:param name="gs_current_value_node" select="xsd:annotation/xsd:*[@meta:type='edit-data']/*"/>
    
                   
                  
             
         
      

    
    <xsl:variable name="gs_event_functions_default">
      <xsl:value-of select="$gs_event_functions"/>
      <xsl:if test="not($gs_event_functions)"><xsl:value-of select="xsd:annotation/xsd:*[@meta:type='event-functions']"/></xsl:if>
    </xsl:variable>

    <xsl:apply-templates select="." mode="gs_form_render">
      <xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/>
      <xsl:with-param name="gs_title" select="$gs_title"/>               
      <xsl:with-param name="gs_submit" select="$gs_submit"/>              
      <xsl:with-param name="gs_description" select="$gs_description"/>         
      <xsl:with-param name="gs_event_functions" select="$gs_event_functions_default"/>     
      <xsl:with-param name="gs_current_value_node" select="$gs_current_value_node"/>  
    </xsl:apply-templates>
  </xsl:template><xsl:template match="xsd:schema" mode="gs_form_render" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/><xsl:param name="gs_title" select="xsd:annotation/xsd:*[@meta:type='title']"/><xsl:param name="gs_submit" select="xsd:annotation/xsd:*[@meta:type='submit-caption']"/><xsl:param name="gs_description" select="xsd:annotation/xsd:*[@meta:type='description']"/><xsl:param name="gs_event_functions" select="xsd:annotation/xsd:*[@meta:type='event-functions']"/><xsl:param name="gs_current_value_node" select="xsd:annotation/xsd:*[@meta:type='edit-data']/*"/>
    
                   
                  
             
         
      

    <xsl:variable name="gs_xsd_form_display" select="xsd:annotation[@meta:type='form-display']"/>
    <xsl:variable name="gs_xsd_data_processing" select="xsd:annotation[@meta:type='data-processing']"/>

    
    <xsl:if test="not($gs_xsd_data_processing)">
      <div class="gs-warning">
        xsd:annotation[@meta:type='data-processing'] missing from form 
        [<xsl:value-of select="local-name(ancestor::class:*[1])"/>::<xsl:value-of select="@name"/> <xsl:value-of select="@xml:id"/>]
      </div>
    </xsl:if>
    

    
    <xsl:variable name="gs_submit_defaulted">
      <xsl:value-of select="$gs_submit"/>
      <xsl:if test="not(string($gs_submit))">submit</xsl:if>
    </xsl:variable>

    <div>
      <xsl:attribute name="class">
        <xsl:value-of select="$gs_html_identifier_class"/>
        <xsl:if test="@name"><xsl:text> gs-xschema-name-</xsl:text><xsl:value-of select="@name"/></xsl:if>
        <xsl:text> </xsl:text><xsl:value-of select="$gs_event_functions"/>
        <xsl:if test="$gs_current_value_node"> gs-edit-mode</xsl:if>
        <xsl:choose>
          <xsl:when test="$gs_xsd_form_display/@meta:clent-side-only = 'yes'"><xsl:text> gs-clientside-only f_submit_</xsl:text><xsl:value-of select="@name"/></xsl:when>
          <xsl:when test="$gs_xsd_form_display/@meta:ajax = 'off'"> gs-ajax-off</xsl:when>
          <xsl:otherwise> f_submit_ajax</xsl:otherwise>
        </xsl:choose>
        <xsl:text> gs-orientation-</xsl:text>
        <xsl:choose>
          <xsl:when test="$gs_xsd_form_display/xsd:app-info[@meta:type='orientation']"><xsl:value-of select="$gs_xsd_form_display/xsd:app-info[@meta:type='orientation']"/></xsl:when>
          <xsl:otherwise>vertical</xsl:otherwise>
        </xsl:choose>
        <xsl:text> gs-style-</xsl:text>
        <xsl:choose>
          <xsl:when test="$gs_xsd_form_display/xsd:app-info[@meta:type='style']"><xsl:value-of select="$gs_xsd_form_display/xsd:app-info[@meta:type='style']"/></xsl:when>
          <xsl:otherwise>verbose</xsl:otherwise>
        </xsl:choose>
        <xsl:if test="contains($gs_xsd_form_display/xsd:app-info[@meta:type='saving-interaction'], 'ctrl-s')"> gs-form-saving-interaction-ctrl-s</xsl:if>
      </xsl:attribute>

      <xsl:if test="string($gs_title)">
        <h2 class="gs-form-title"><xsl:value-of select="$gs_title"/></h2>
      </xsl:if>

      
      <form class="details" method="POST"><div>
        <span class="gs-form-elements">
          <xsl:apply-templates select="xsd:complexType/xsd:sequence/xsd:*" mode="gs_view_render">
            <xsl:with-param name="gs_current_external_value_node" select="$gs_current_value_node"/>
          </xsl:apply-templates>
        </span>

        
        <span class="gs-meta-data gs-group-context gs-display-specific gs-persist gs-load-as-properties gs-namespace-prefix-meta">
          <input type="hidden" name="meta:xpath-to-xsd-data-processing" value="{$gs_xsd_data_processing/@meta:xpath-to-node}"/>
          <xsl:apply-templates select="xsd:annotation/xsd:documentation" mode="gs_form_render_documentation"/>
        </span>

        <xsl:if test="$gs_submit_defaulted">
          <xsl:apply-templates select="." mode="gs_form_render_submit">
            <xsl:with-param name="gs_submit_defaulted" select="$gs_submit_defaulted"/>
          </xsl:apply-templates>
        </xsl:if>
      </div></form>

      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups"/>
    </div>
  </xsl:template><xsl:template match="xsd:schema//xsd:sequence/xsd:*" mode="gs_form_render_check_required_documentation" meta:base-class-count="6">
    
  </xsl:template><xsl:template match="xsd:schema/xsd:annotation/xsd:documentation" mode="gs_form_render_documentation" meta:base-class-count="6">
    
    <xsl:if test="not(@meta:direct-render='no')">
      <xsl:if test="*">
        <div class="gs-warning">attempt to render documentation with child nodes, maybe you meant @meta:direct-render='no'?</div>
      </xsl:if>
      <xsl:if test="not(*)">
        <xsl:variable name="gs_type">
          <xsl:value-of select="substring-after(@meta:type, ':')"/>
          <xsl:if test="not(contains(@meta:type, ':'))"><xsl:value-of select="@meta:type"/></xsl:if>
        </xsl:variable>
        <input type="hidden" name="meta:{$gs_type}" value="{.}"/>
      </xsl:if>
    </xsl:if>
  </xsl:template><xsl:template match="xsd:schema" mode="gs_context_menu_custom" meta:base-class-count="6">
    <li class="f_submit_ajax">submit form</li>
  </xsl:template><xsl:template match="xsd:schema" mode="gs_form_render_submit" meta:base-class-count="6"><xsl:param name="gs_submit_defaulted"/>
    

    <input class="gs-form-submit" type="submit" name="gs--submit" value="{$gs_submit_defaulted}">
      
    </input>
  </xsl:template><xsl:template match="xsd:schema" mode="editor" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/>
    
    

    <xsl:if test="@name">
      <div class="gs-warning">
        inheritance on named schemas is not supported yet.
        this is just because the @name cannot be passed through to the class_xschema.xsl Tech processor
      </div>
    </xsl:if>

    <div class="{$gs_html_identifier_class} gs-interface-mode-editor">
      
      <xsl:if test="comment() and not(contains(comment()[1], '###'))">
        <div class="gs_description"><xsl:apply-templates select="comment()[1]" mode="gs_formatted_output"/></div>
      </xsl:if>

      <p class="gs_description">
        Organise the fields in your class here.
        Opaque ones are inherited...
      </p>

      <xsl:apply-templates select="xsd:annotation" mode="editor"/>

      <xsl:apply-templates select="xsd:complexType/xsd:sequence" mode="editor">
        <xsl:with-param name="gs_field_model" select="xsd:annotation[@meta:type='data-queries']/xsd:app-info[@meta:type='field-model']/xsd:schema"/>
        <xsl:with-param name="gs_field_types" select="xsd:annotation[@meta:type='data-queries']/xsd:app-info[@meta:type='field-types']/xsd:schema"/>
      </xsl:apply-templates>
    </div>
  </xsl:template><xsl:template match="xsd:sequence" mode="editor" meta:base-class-count="6"><xsl:param name="gs_field_model" select="ancestor::xsd:schema[1]/xsd:annotation[@meta:type='data-queries']/xsd:app-info[@meta:type='field-model']/xsd:schema"/><xsl:param name="gs_field_types" select="ancestor::xsd:schema[1]/xsd:annotation[@meta:type='data-queries']/xsd:app-info[@meta:type='field-types']/xsd:schema"/>
    
    
    

    <ul class="gs-xsd-editor-list gs-xsd-section gs-jquery-nodrag">
      
      <xsl:apply-templates select="*" mode="gs_view_render">
        <xsl:with-param name="gs_interface_mode" select="'editor'"/>
        <xsl:with-param name="gs_field_model" select="$gs_field_model"/>
        <xsl:with-param name="gs_field_types" select="$gs_field_types"/>
      </xsl:apply-templates>

      
      
    </ul>
  </xsl:template><xsl:template match="xsd:annotation|xsd:app-info|xsd:documentation" mode="editor" meta:base-class-count="6"/><xsl:template match="xsd:annotation[@meta:type = 'form-display']" mode="editor" meta:base-class-count="6">
    <xsl:apply-templates select="*" mode="editor"/>
  </xsl:template><xsl:template match="xsd:documentation[@meta:type = 'order']" mode="editor" meta:base-class-count="6"/><xsl:template match="xsd:annotation[@meta:type = 'data-processing']" mode="editor" meta:base-class-count="6">
    <xsl:apply-templates select="*" mode="editor"/>
  </xsl:template><xsl:template match="xsd:app-info[@meta:type = 'processing']" mode="editor" meta:base-class-count="6">
    <div class="gs-xsd-section">
      <select name="dom-method">
        <option>(dom-method)</option>
        <option>class-command</option>
      </select>

      <input name="interface-mode" value="{@meta:interface-mode}"/>
      <input name="select" value="{@meta:select}"/>
      <input name="description" value="{@meta:description}"/>
    </div>
  </xsl:template><xsl:template match="xsd:app-info[@meta:type = 'context']" mode="editor" meta:base-class-count="6">
    <xsl:apply-templates select="xsd:sequence" mode="editor"/>
  </xsl:template><xsl:template match="xsd:schema" mode="new" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/>
    
    <li class="{$gs_html_identifier_class} gs-semantic-only-markup">
      <ul class="CSS__VerticalMenu gs-interface-mode-default gs-semantic-only-markup">
        <xsl:if test="not(*)">
          <li class="gs-empty-message">nothing to be added</li>
        </xsl:if>
        <xsl:apply-templates select="*" mode="gs_view_render">
          <xsl:with-param name="gs_interface_mode" select="'new'"/>
        </xsl:apply-templates>
        <li class="gs-add-message"><a href="#">edit</a> the structure</li>
      </ul>
      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups"/>
    </li>
  </xsl:template><xsl:template match="xsd:schema" mode="set" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/>
    
    <li class="{$gs_html_identifier_class} gs-semantic-only-markup">
      <ul class="CSS__VerticalMenu gs-interface-mode-default gs-semantic-only-markup">
        <xsl:if test="not(*)">
          <li class="gs-empty-message">nothing to be set</li>
        </xsl:if>
        <xsl:apply-templates select="*" mode="gs_view_render">
          <xsl:with-param name="gs_interface_mode" select="'set'"/>
        </xsl:apply-templates>
        <li class="gs-add-message"><a href="#">edit</a> the structure</li>
      </ul>
      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups"/>
    </li>
  </xsl:template><xsl:template match="*" mode="selectoroption" meta:base-class-count="7"><xsl:param name="gs_html_identifier_class"/>
    

    <li class="{$gs_html_identifier_class}">
      <xsl:value-of select="local-name()"/>
    </li>
  </xsl:template><xsl:template match="xsd:element|xsd:attribute|xsd:any|xsl:attribute[@meta:editor-class = 'class:Selector']|xsd:attribute[@meta:editor-class = 'class:CodeMirrorEditor']" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/><xsl:param name="gs_interface_mode"/><xsl:param name="gs_current_external_value_node" select="FORCE_SELECT_EMPTY_NODESET"/>
    
    
    
    

    <xsl:variable name="gs_name" select="@name"/>

    <xsl:choose>
      <xsl:when test="not($gs_current_external_value_node)">
        <xsl:apply-templates select="." mode="gs_form_render_items_with_relevant_value">
          <xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/>
          <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
        </xsl:apply-templates>
      </xsl:when>

      <xsl:when test="@meta:current-value-type = 'singular'">
        <xsl:apply-templates select="." mode="gs_form_render_items_with_relevant_value">
          <xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/>
          <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
          <xsl:with-param name="gs_current_external_value_node" select="$gs_current_external_value_node"/>
        </xsl:apply-templates>
      </xsl:when>

      <xsl:when test="self::xsd:attribute">
        <xsl:apply-templates select="." mode="gs_form_render_items_with_relevant_value">
          <xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/>
          <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
          <xsl:with-param name="gs_current_external_value_node" select="$gs_current_external_value_node/@*[name() = $gs_name]"/>
        </xsl:apply-templates>
      </xsl:when>

      <xsl:when test="self::xsd:element">
        <xsl:apply-templates select="." mode="gs_form_render_items_with_relevant_value">
          <xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/>
          <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
          <xsl:with-param name="gs_current_external_value_node" select="$gs_current_external_value_node/*[name() = $gs_name]"/>
        </xsl:apply-templates>
      </xsl:when>
    </xsl:choose>
  </xsl:template><xsl:template match="xsd:element|xsd:attribute|xsd:any|xsl:attribute[@meta:editor-class = 'class:Selector']|xsd:attribute[@meta:editor-class = 'class:CodeMirrorEditor']" mode="gs_form_render_items_with_relevant_value" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/><xsl:param name="gs_interface_mode"/><xsl:param name="gs_current_external_value_node"/>
    
    
    
    

    <xsl:if test="string(@meta:editor-class)"><div class="gs-warning">@meta:editor-class [<xsl:value-of select="@meta:editor-class"/>] not found / processed</div></xsl:if>

    
    <xsl:variable name="gs_edit_mode"><xsl:if test="$gs_current_external_value_node">gs-edit-mode</xsl:if></xsl:variable>
    <xsl:variable name="gs_raw_name" select="@name"/>
    <xsl:variable name="gs_css_classes" select="@meta:css-classes"/>
    <xsl:variable name="gs_HTML_id" select="concat('gs-xsd-form-id-', translate(@xml:id, '_: ', '---'))"/>
    <xsl:variable name="gs_name">
      <xsl:choose>
        <xsl:when test="@meta:caption = '{name()}' and $gs_current_external_value_node">-<xsl:value-of select="name($gs_current_external_value_node)"/></xsl:when>
        <xsl:when test="string(@meta:caption)"><xsl:value-of select="@meta:caption"/></xsl:when>
        <xsl:when test="string(@name)"><xsl:value-of select="@name"/></xsl:when>
        <xsl:otherwise>no @meta:caption or @name</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="gs_value">
      <xsl:choose>
        <xsl:when test="$gs_current_external_value_node and self::xsd:attribute"><xsl:value-of select="$gs_current_external_value_node"/></xsl:when>
        <xsl:when test="$gs_current_external_value_node and self::xsd:element"><xsl:value-of select="$gs_current_external_value_node/text()"/></xsl:when>
        <xsl:when test="not($gs_current_external_value_node) and @default"><xsl:value-of select="@default"/></xsl:when>
        <xsl:otherwise/>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="gs_auto_create" select="@meta:auto-create"/>
    <xsl:variable name="gs_required" select="@minOccurs and not(@minOccurs = 0)"/>
    <xsl:variable name="gs_type_name">
      <xsl:if test="@type"><xsl:value-of select="@type"/></xsl:if>
      <xsl:if test="not(@type)">xsd:string</xsl:if>
    </xsl:variable>
    <xsl:variable name="gs_type_class" select="translate($gs_type_name, ':', '-')"/>

    <div class="{$gs_html_identifier_class} {$gs_edit_mode} gs-xsd-form-item {$gs_css_classes} gs-xsd-type-{$gs_type_class} gs-{$gs_raw_name} gs-required-{$gs_required} gs-auto-create-{$gs_auto_create} gs-min-occurs-{@minOccurs} gs-max-occurs-{@maxOccurs}">
      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups">
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      </xsl:apply-templates>

      <label class="gs-xsd-name" for="{$gs_HTML_id}">
        <xsl:value-of select="$gs_name"/>
        <span class="gs-colon">:</span>
      </label>

      <div class="gs-xsd-value gs-extended-attributes">
        <xsl:choose>
          <xsl:when test="$gs_auto_create = 'yes'"/>
          <xsl:when test="$gs_auto_create = 'ask'">
            <input id="{$gs_HTML_id}" name="{$gs_raw_name}" checked="1" type="checkbox"/>
          </xsl:when>
          <xsl:when test="@minOccurs = 0 and @maxOccurs = 'unbounded'"/>
          <xsl:otherwise>
            <xsl:apply-templates select="." mode="gs_form_render_item_input">
              <xsl:with-param name="gs_HTML_id" select="$gs_HTML_id"/>
              <xsl:with-param name="gs_raw_name" select="$gs_raw_name"/>
              <xsl:with-param name="gs_value" select="$gs_value"/> 
            </xsl:apply-templates>
          </xsl:otherwise>
        </xsl:choose>
        <xsl:apply-templates select="xsd:annotation[@meta:field-message-type]" mode="gs_form_render_item_input_field_message"/>
        <xsl:apply-templates select="xsd:simpleType/xsd:restriction/xsd:pattern" mode="gs_form_render_item_input_validation"/>
      </div>
      <xsl:apply-templates select="xsd:annotation[not(@meta:field-message-type)]" mode="gs_form_render_item_input_help"/>
      <div class="gs-xsd-type"><xsl:value-of select="$gs_type_name"/></div>

      
      
    </div>
  </xsl:template><xsl:template match="xsd:element|xsd:attribute|xsd:any|xsl:attribute[@meta:editor-class = 'class:Selector']|xsd:attribute[@meta:editor-class = 'class:CodeMirrorEditor']" mode="gs_form_render_item_input" meta:base-class-count="6"><xsl:param name="gs_HTML_id"/><xsl:param name="gs_raw_name"/><xsl:param name="gs_value"/>
    
    
    

    <xsl:if test="@type='xsd:boolean'">
      
      <input name="{$gs_raw_name}" type="hidden" value=""/>
    </xsl:if>

    <input id="{$gs_HTML_id}" name="{$gs_raw_name}">
      <xsl:attribute name="type">
        <xsl:choose>
          <xsl:when test="@type='xsd:boolean'">checkbox</xsl:when>
          <xsl:otherwise>text</xsl:otherwise>
        </xsl:choose>
      </xsl:attribute>
      <xsl:attribute name="value">
        <xsl:choose>
          <xsl:when test="@type='xsd:boolean'">on</xsl:when>
          <xsl:otherwise><xsl:value-of select="$gs_value"/></xsl:otherwise>
        </xsl:choose>
      </xsl:attribute>
      <xsl:if test="@meta:disabled"><xsl:attribute name="disabled">1</xsl:attribute></xsl:if>
    </input>
  </xsl:template><xsl:template match="xsd:restriction/xsd:pattern" mode="gs_form_render_item_input_validation" meta:base-class-count="6">
    <span class="gs-xtransport gs-required"><xsl:value-of select="@value"/></span>
  </xsl:template><xsl:template match="xsd:annotation" mode="gs_form_render_item_input_field_message" meta:base-class-count="6">
    
    <span class="gs-xtransport gs-xsd-field-message"><xsl:value-of select="xsd:documentation"/></span>
    <span class="gs-xtransport gs-xsd-field-message-type"><xsl:value-of select="@meta:field-message-type"/></span>
  </xsl:template><xsl:template match="xsd:annotation" mode="gs_form_render_item_input_help" meta:base-class-count="6">
    <div class="gs-xsd-help"><xsl:value-of select="xsd:documentation"/></div>
  </xsl:template><xsl:template match="xsd:element|xsd:attribute|xsd:any|xsl:attribute[@meta:editor-class = 'class:Selector']|xsd:attribute[@meta:editor-class = 'class:CodeMirrorEditor']" mode="editor" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/><xsl:param name="gs_field_model" select="ancestor::xsd:schema[1]/xsd:annotation[@meta:type='data-queries']/xsd:app-info[@meta:type='field-model']/xsd:schema"/><xsl:param name="gs_field_types" select="ancestor::xsd:schema[1]/xsd:annotation[@meta:type='data-queries']/xsd:app-info[@meta:type='field-types']/xsd:schema"/>
    
    
    

    <li class="{$gs_html_identifier_class} gs-inherited-{@meta:inherited} gs-class-{@meta:class} gs-type-{translate(@type, ':', '-')} gs-auto-create-{@meta:auto_create} gs-min-occurs-{@minOccurs} gs-max-occurs-{@maxOccurs}">
      <xsl:if test="not($gs_field_model)"><div class="gs-warning">field model (XSD) not found</div></xsl:if>

      <xsl:apply-templates select="$gs_field_model" mode="gs_view_render">
        <xsl:with-param name="gs_current_external_value_node" select="."/>
        <xsl:with-param name="gs_field_types" select="$gs_field_types"/>
      </xsl:apply-templates>

      
    </li>
  </xsl:template><xsl:template match="xsd:element|xsd:attribute|xsd:any|xsl:attribute[@meta:editor-class = 'class:Selector']|xsd:attribute[@meta:editor-class = 'class:CodeMirrorEditor']" mode="gs_xsd_occurs" meta:base-class-count="6"><xsl:param name="gs_setting"/><xsl:param name="gs_int" select="0"/>
    
    

    <xsl:apply-templates select="." mode="gs_xsd_occurs_output">
      <xsl:with-param name="gs_setting" select="$gs_setting"/>
      <xsl:with-param name="gs_int" select="$gs_int"/>
    </xsl:apply-templates>

    <xsl:if test="$gs_int &lt; 10">
      <xsl:apply-templates select="." mode="gs_xsd_occurs">
        <xsl:with-param name="gs_setting" select="$gs_setting"/>
        <xsl:with-param name="gs_int" select="$gs_int+1"/>
      </xsl:apply-templates>
    </xsl:if>
    <xsl:if test="not($gs_int &lt; 10)">
      <xsl:apply-templates select="." mode="gs_xsd_occurs_output">
        <xsl:with-param name="gs_setting" select="$gs_setting"/>
        <xsl:with-param name="gs_int" select="'unbounded'"/>
        <xsl:with-param name="gs_caption" select="'âˆž'"/>
      </xsl:apply-templates>
      <xsl:apply-templates select="." mode="gs_xsd_occurs_output">
        <xsl:with-param name="gs_setting" select="$gs_setting"/>
        <xsl:with-param name="gs_int" select="'other'"/>
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template><xsl:template match="xsd:element|xsd:attribute|xsd:any|xsl:attribute[@meta:editor-class = 'class:Selector']|xsd:attribute[@meta:editor-class = 'class:CodeMirrorEditor']" mode="gs_xsd_occurs_output" meta:base-class-count="6"><xsl:param name="gs_setting"/><xsl:param name="gs_int"/><xsl:param name="gs_caption"/>
    
    
    

    <option>
      <xsl:if test="$gs_int = $gs_setting"><xsl:attribute name="selected">true</xsl:attribute></xsl:if>
      <xsl:if test="$gs_caption"><xsl:value-of select="$gs_caption"/></xsl:if>
      <xsl:if test="not($gs_caption)"><xsl:value-of select="$gs_int"/></xsl:if>
    </option>
  </xsl:template><xsl:template match="xsd:element|xsd:attribute|xsd:any|xsd:sequence|xsl:attribute[@meta:editor-class = 'class:Selector']|xsd:sequence|xsd:attribute[@meta:editor-class = 'class:CodeMirrorEditor']|xsd:sequence" mode="gs_xsd_type_options" meta:base-class-count="6">
    
    <xsl:variable name="gs_xsd_schema" select="ancestor::xsd:schema[1]"/>
    <xsl:variable name="gs_xsd_annotation_type_info" select="$gs_xsd_schema/xsd:annotation[@meta:type='type-info']"/>

    <xsl:if test="not($gs_xsd_schema)"><div class="gs-warning">ancestor schema not found for type-info lists</div></xsl:if>
    <xsl:if test="not($gs_xsd_annotation_type_info)"><div class="gs-warning">xsd:annotation @meta:type=type-info not found for type lists</div></xsl:if>

    
    <xsl:apply-templates select="$gs_xsd_annotation_type_info/xsd:app-info/*" mode="gs_xsd_type_options_output">
      <xsl:with-param name="gs_element" select="."/>
    </xsl:apply-templates>
  </xsl:template><xsl:template match="*" mode="gs_xsd_type_options_output" meta:base-class-count="6"><xsl:param name="gs_element"/>
    

    <option>
      <xsl:if test="$gs_element and $gs_element/@type = name()"><xsl:attribute name="selected">1</xsl:attribute></xsl:if>
      <xsl:attribute name="value"><xsl:value-of select="local-name()"/></xsl:attribute>

      <xsl:if test="@name"><xsl:value-of select="@name"/></xsl:if>
      <xsl:if test="not(@name)"><xsl:value-of select="local-name()"/></xsl:if>
    </option>
  </xsl:template><xsl:template match="xsd:element|xsd:attribute|xsd:any|xsl:attribute[@meta:editor-class = 'class:Selector']|xsd:attribute[@meta:editor-class = 'class:CodeMirrorEditor']" mode="new" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/><xsl:param name="gs_interface_mode"/>
    
    

    <xsl:variable name="gs_type_class" select="class:*[1]"/>
    <xsl:variable name="gs_type" select="local-name($gs_type_class)"/>
    <xsl:variable name="gs_name">
      <xsl:if test="contains(@name, ':')"><xsl:value-of select="substring-after(@name, ':')"/></xsl:if>
      <xsl:if test="not(contains(@name, ':'))"><xsl:value-of select="@name"/></xsl:if>
      <xsl:if test="not(@name)"><xsl:value-of select="$gs_type"/></xsl:if>
    </xsl:variable>

    <li class="{$gs_html_identifier_class} f_click_new_{$gs_type} CSS__{$gs_type} gs-interface-mode-list">
      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups">
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      </xsl:apply-templates>
      
      <xsl:apply-templates select="$gs_type_class" mode="gs_view_render"/>
      <xsl:text>: </xsl:text>
      <span class="gs-name"><xsl:value-of select="$gs_name"/></span>
    </li>
  </xsl:template><xsl:template match="xsd:element|xsd:attribute|xsd:any|xsl:attribute[@meta:editor-class = 'class:Selector']|xsd:attribute[@meta:editor-class = 'class:CodeMirrorEditor']" mode="set" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/><xsl:param name="gs_interface_mode"/>
    
    

    <xsl:variable name="gs_type" select="local-name(*)"/>
    <xsl:variable name="gs_name_namespace" select="substring-before(@name, ':')"/>
    <xsl:variable name="gs_name">
      <xsl:if test="contains(@name, ':')"><xsl:value-of select="substring-after(@name, ':')"/></xsl:if>
      <xsl:if test="not(contains(@name, ':'))"><xsl:value-of select="@name"/></xsl:if>
    </xsl:variable>

    <li class="{$gs_html_identifier_class} f_click_new_{$gs_type} CSS__{$gs_type} gs-interface-mode-list">
      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups"/>
      <span class="gs-namespace">
        <xsl:value-of select="$gs_name_namespace"/>
        <xsl:if test="$gs_name_namespace">:</xsl:if>
      </span>
      <span class="gs-name"><xsl:value-of select="$gs_name"/></span>
      <xsl:text> </xsl:text>
      <span class="gs-type">(<xsl:value-of select="$gs_type"/>)</span>
    </li>
  </xsl:template><xsl:template match="xsl:template" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/>
    

    <div class="{$gs_html_identifier_class}">
      Templatey
    </div>
  </xsl:template><xsl:template match="xsl:template" mode="gs_listname" meta:base-class-count="6">
    <xsl:if test="@name">{<xsl:value-of select="@name"/>}</xsl:if>
    <xsl:if test="@match">[<xsl:value-of select="@match"/>]</xsl:if>
    <xsl:if test="@mode"> (<xsl:value-of select="@mode"/>)</xsl:if>
  </xsl:template><xsl:template match="xsl:template" mode="view" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/><xsl:param name="gs_interface_mode"/>
    
    

    <xsl:variable name="gs_mode">
      <xsl:value-of select="@mode"/>
      <xsl:if test="not(@mode)">default</xsl:if>
    </xsl:variable>

    <li class="{$gs_html_identifier_class} f_click_loadView_{$gs_mode} gs-xsl-javascript-view">
      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups">
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      </xsl:apply-templates>
      <xsl:value-of select="translate($gs_mode, '_', ' ')"/>
    </li>
  </xsl:template><xsl:template match="xsl:template" mode="gs_javascript_xsl_templates" meta:base-class-count="6"><xsl:param name="gs_object_name"/>
    
    

    <xsl:variable name="gs_function_name">
      <xsl:apply-templates select="." mode="gs_dash_to_camel">
        <xsl:with-param name="gs_string" select="@mode"/>
      </xsl:apply-templates>
    </xsl:variable>

    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words">
      <xsl:with-param name="gs_name" select="@mode"/>
    </xsl:apply-templates>

    <xsl:variable name="gs_prototype_path">
      <xsl:value-of select="$gs_object_name"/><xsl:text>.prototype.</xsl:text><xsl:value-of select="$gs_function_name"/>
    </xsl:variable>

    
    <xsl:text>var f=</xsl:text>
    <xsl:value-of select="$gs_prototype_path"/>
    <xsl:text>=function</xsl:text>
    <xsl:text> </xsl:text><xsl:value-of select="$gs_function_name"/>
    <xsl:text>() {</xsl:text>
      <xsl:value-of select="$gs_newline"/>
      <xsl:if test="$gs_stage_dev">
        <xsl:text>      //arguments: jElements</xsl:text>
        <xsl:apply-templates select="xsl:param" mode="gs_javascript_xsl_templates"/>
        <xsl:text>, fCallbackSuccess, fCallbackFailure</xsl:text>
        <xsl:value-of select="$gs_newline"/>
        <xsl:text>      //xJST() = executeJavaScriptXSLTemplate()</xsl:text>
        <xsl:value-of select="$gs_newline"/>
      </xsl:if>
      <xsl:text>      this.xJST(</xsl:text>
      <xsl:value-of select="$gs_prototype_path"/>
      <xsl:text>, arguments);</xsl:text>
      <xsl:value-of select="$gs_newline"/>
    <xsl:text>}</xsl:text>
    <xsl:value-of select="$gs_newline"/>

    
    <xsl:apply-templates select="@*" mode="gs_function_attributes"/>

    
    <xsl:if test="$gs_stage_dev">
      <xsl:text>//ordered xsl template param names for collating the incoming function arguments</xsl:text>
      <xsl:value-of select="$gs_newline"/>
    </xsl:if>
    <xsl:if test="xsl:param">
      <xsl:text>f.tPs=[</xsl:text>
      <xsl:apply-templates select="xsl:param" mode="gs_javascript_xsl_templates_names"/>
      <xsl:text>];</xsl:text>
      <xsl:value-of select="$gs_newline"/>
    </xsl:if>
    
    
    <xsl:text>f.functionType="method";</xsl:text>
    <xsl:value-of select="$gs_newline"/>

    <xsl:apply-templates select="." mode="gs_javascript_after_function"/>
  </xsl:template><xsl:template match="xsl:param" mode="gs_javascript_xsl_templates_names" meta:base-class-count="6">
    <xsl:text>"</xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text>"</xsl:text>
    <xsl:if test="not(position() = last())">,</xsl:if>
  </xsl:template><xsl:template match="xsl:param" mode="gs_javascript_xsl_templates" meta:base-class-count="6">
    <xsl:text>, </xsl:text>
    <xsl:value-of select="@name"/>
  </xsl:template><xsl:template match="xsl:template" mode="controller" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/><xsl:param name="gs_interface_mode"/>
    
    
    

    <li>
      <xsl:attribute name="class">
        <xsl:value-of select="$gs_html_identifier_class"/>
        <xsl:text> f_click_menuitem_</xsl:text><xsl:value-of select="@mode"/>
        <xsl:text> f_submit_menuitem_</xsl:text><xsl:value-of select="@mode"/>
        
        <xsl:text> gs-form-include</xsl:text>
        <xsl:text> gs-xsl-javascript-function</xsl:text>
      </xsl:attribute>
      
      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups">
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      </xsl:apply-templates>
      
      <span class="Field gs-name"><xsl:value-of select="translate(@mode, '_', ' ')"/></span>
      
      <xsl:if test="xsl:param">
        <form class="Object Class__XSchema gs-xsl-template-parameters f_click_nothing"><div>
          <xsl:apply-templates select="xsl:param" mode="gs_view_render">
            <xsl:with-param name="gs_interface_mode" select="'controller'"/>
          </xsl:apply-templates>
        </div></form>
      </xsl:if>
    </li>
  </xsl:template><xsl:template match="javascript:ifdef" meta:base-class-count="6"><xsl:param name="gs_object_name"/>
    
    
    <xsl:if test="function-available('dyn:evaluate') and dyn:evaluate(@test)">
      <xsl:apply-templates>
        <xsl:with-param name="gs_object_name" select="$gs_object_name"/>
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template><xsl:template match="javascript:stage-dev" meta:base-class-count="6"><xsl:param name="gs_object_name"/>
    
    <xsl:if test="$gs_stage_dev">
      <xsl:apply-templates>
        <xsl:with-param name="gs_object_name" select="$gs_object_name"/>
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template><xsl:template match="javascript:code" mode="standalone_contents" meta:base-class-count="6"><xsl:param name="gs_class" select="parent::class:*"/>
    
    
    <xsl:apply-templates select="." mode="html_details">
      <xsl:with-param name="gs_class" select="$gs_class"/>
    </xsl:apply-templates>
  </xsl:template><xsl:template match="javascript:code" mode="editor" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/>
    
    

    <div class="{$gs_html_identifier_class} details" xml:add-xml-id="no">
      
      <ul><xsl:apply-templates select="javascript:dependency" mode="editor">
        <xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/>
      </xsl:apply-templates></ul>

      <xsl:apply-templates mode="editor"/>
    </div>
  </xsl:template><xsl:template match="javascript:code" mode="html" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/>
    
    

    
    <xsl:apply-templates select="javascript:dependency" mode="html">
      <xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/>
    </xsl:apply-templates>

    <script type="text/javascript" class="{$gs_html_identifier_class} details" xml:add-xml-id="no">
      <xsl:text>//&lt;!--</xsl:text>
      <xsl:apply-templates select="." mode="html_details"/>
      <xsl:text>//--&gt;</xsl:text>
    </script>
  </xsl:template><xsl:template match="javascript:code" mode="html_details" meta:base-class-count="6"><xsl:param name="gs_class" select="parent::class:*"/>
    
    
    <xsl:apply-templates select="javascript:*">
      <xsl:with-param name="gs_class" select="$gs_class"/>
    </xsl:apply-templates>
  </xsl:template><xsl:template match="javascript:code" mode="gs_listname" meta:base-class-count="6">
    <xsl:text>controller</xsl:text>
  </xsl:template><xsl:template match="html:script" mode="gs_listname" meta:base-class-count="6">
    
    <xsl:choose>
      <xsl:when test="@meta:list-name"><xsl:value-of select="@meta:list-name"/></xsl:when>
      <xsl:when test="@src">
        <xsl:if test="contains(@src, '/')"><xsl:value-of select="substring-after-last(@src, '/')"/></xsl:if>
        <xsl:if test="not(contains(@src, '/'))"><xsl:value-of select="@src"/></xsl:if>
      </xsl:when>
      <xsl:otherwise>script (inline)</xsl:otherwise>
    </xsl:choose>
  </xsl:template><xsl:template match="javascript:*/text()" meta:base-class-count="6">
    <xsl:value-of select="." disable-output-escaping="yes"/>
  </xsl:template><xsl:template match="javascript:raw" meta:base-class-count="6">
    
    <xsl:apply-templates/>
  </xsl:template><xsl:template match="javascript:dependency" meta:base-class-count="6">
    <xsl:text>//javascript:dependency on [</xsl:text>
    <xsl:value-of select="@uri"/>
    <xsl:text>] rendered outside this script</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template><xsl:template match="javascript:with-param" meta:base-class-count="6">
    alert(document.XMLDocument);
    var <xsl:value-of select="@name"/> = "<xsl:value-of select="translate(@select, '&quot;', '_')"/>";
  </xsl:template><xsl:template match="javascript:assert" meta:base-class-count="6">
    if (!<xsl:value-of select="@test"/>) alert("<xsl:value-of select="translate(@message, '&quot;', '_')"/>");
  </xsl:template><xsl:template match="javascript:define|javascript:global-variable" meta:base-class-count="6">
    
    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>

    <xsl:text>window["</xsl:text><xsl:value-of select="@name"/><xsl:text>"]</xsl:text>
    <xsl:if test="string(.)">
      <xsl:text> = </xsl:text>
      <xsl:value-of select="."/>
    </xsl:if>
    <xsl:if test="not(string(.))"> = null</xsl:if>
    <xsl:text>;</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template><xsl:template match="javascript:override" meta:base-class-count="6">
    
    <xsl:value-of select="@name"/>
    <xsl:text> = function(</xsl:text>
    <xsl:value-of select="xjs:parameters(@parameters, @name)"/>
    <xsl:text>) {</xsl:text>
      <xsl:apply-templates select="." mode="gs_javascript_before_method"/>
      <xsl:apply-templates select="@requires"/>
      <xsl:if test="not(@parameter-checks = 'off' or ancestor::javascript:code[1][@parameter-checks = 'off'])"><xsl:value-of select="xjs:parameter-checks(@parameters, @name, 6, $gs_stage_dev)"/></xsl:if>
      <xsl:apply-templates/>
    <xsl:text>}</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template><xsl:template match="javascript:global-function" meta:base-class-count="6">
    
    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>
    <xsl:variable name="gs_prototype_path">window.<xsl:value-of select="@name"/></xsl:variable>

    
    <xsl:text>function </xsl:text>
    <xsl:value-of select="@name"/><xsl:text>(</xsl:text>
    <xsl:value-of select="xjs:parameters(@parameters, @name)"/>
    <xsl:text>) {</xsl:text>
      <xsl:apply-templates select="." mode="gs_javascript_before_method"/>
      <xsl:apply-templates select="@requires"/>
      <xsl:if test="not(@parameter-checks = 'off' or ancestor::javascript:code[1][@parameter-checks = 'off'])"><xsl:value-of select="xjs:parameter-checks(@parameters, @name, 6, $gs_stage_dev)"/></xsl:if>
      <xsl:apply-templates/>
    <xsl:text>}</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template><xsl:template match="javascript:global-init" meta:base-class-count="6"><xsl:param name="gs_object_name" select="'Global'"/>
    
    
    
    <xsl:variable name="gs_funcname">
      <xsl:if test="@name"><xsl:value-of select="@name"/></xsl:if>
      <xsl:if test="not(@name)"><xsl:value-of select="concat('global-init_',translate(@xml:id, ' -[](){}&amp;#', '_'))"/></xsl:if>
    </xsl:variable>
    <xsl:variable name="gs_prototype_path"><xsl:value-of select="$gs_object_name"/>.<xsl:value-of select="$gs_funcname"/></xsl:variable>

    
    <xsl:text>var f=</xsl:text>
    <xsl:value-of select="$gs_prototype_path"/>
    <xsl:text>=function </xsl:text><xsl:value-of select="$gs_funcname"/><xsl:text>(oResults) {</xsl:text>
    <xsl:value-of select="$gs_newline"/>
      <xsl:apply-templates select="." mode="gs_javascript_before_method"/>
      <xsl:apply-templates select="@requires"/>
      <xsl:apply-templates/>
    <xsl:text>}</xsl:text>
    <xsl:value-of select="$gs_newline"/>

    
    <xsl:text>f.functionType="globalInit";</xsl:text>
    <xsl:value-of select="$gs_newline"/>
    
    <xsl:apply-templates select="@*" mode="gs_function_attributes"/>
    
    
    <xsl:if test="$gs_object_name = 'Global'">
      <xsl:text>OO.registerMethod(window.Global,f);</xsl:text>
      <xsl:value-of select="$gs_newline"/>
    </xsl:if>
    
    
    <xsl:text>OO.executeGlobalInit(f</xsl:text>
    <xsl:if test="@delay">,<xsl:value-of select="@delay"/></xsl:if>
    <xsl:text>);</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template><xsl:template match="javascript:jquery-expression-extension" meta:base-class-count="6">
    
    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>
    <xsl:variable name="gs_prototype_path"><xsl:text>jQuery.expr[':'].</xsl:text><xsl:value-of select="@name"/></xsl:variable>

    
    <xsl:value-of select="$gs_prototype_path"/><xsl:text> = function(</xsl:text>
    <xsl:value-of select="xjs:parameters(@parameters, @name)"/>
    <xsl:text>) {</xsl:text>
      <xsl:if test="@override = 'true'"> //(@override=true)</xsl:if>
      <xsl:value-of select="$gs_newline"/>
      <xsl:if test="not(@parameter-checks = 'off' or ancestor::javascript:code[1][@parameter-checks = 'off'])"><xsl:value-of select="xjs:parameter-checks(@parameters, @name, 6, $gs_stage_dev)"/></xsl:if>
      <xsl:apply-templates/>
    <xsl:text>}</xsl:text>
    <xsl:value-of select="$gs_newline"/>

    <xsl:value-of select="$gs_prototype_path"/><xsl:text>.extension = true;</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template><xsl:template match="javascript:jquery-extension" meta:base-class-count="6">
    
    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>
    <xsl:variable name="gs_prototype_path"><xsl:text>jQuery.fn.</xsl:text><xsl:value-of select="@name"/></xsl:variable>

    
    <xsl:text>var fOriginal=</xsl:text><xsl:value-of select="$gs_prototype_path"/><xsl:text>;</xsl:text>
    <xsl:value-of select="$gs_newline"/>
    
    
    <xsl:text>var f=</xsl:text>
    <xsl:value-of select="$gs_prototype_path"/><xsl:text> = function(</xsl:text>
    <xsl:value-of select="xjs:parameters(@parameters, @name)"/>
    <xsl:text>) {</xsl:text>
      <xsl:if test="@override = 'true'"> //(@override=true)</xsl:if>
      <xsl:value-of select="$gs_newline"/>
      <xsl:if test="not(@parameter-checks = 'off' or ancestor::javascript:code[1][@parameter-checks = 'off'])">
        <xsl:value-of select="xjs:parameter-checks(@parameters, @name, 6, $gs_stage_dev)"/>
      </xsl:if>
      <xsl:apply-templates/>
    <xsl:text>}</xsl:text>
    <xsl:value-of select="$gs_newline"/>

    <xsl:text>if (fOriginal) f.fOriginal = fOriginal;</xsl:text>
    <xsl:value-of select="$gs_newline"/>

    <xsl:text>f.extension = true;</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template><xsl:template match="javascript:jquery-static-extension" meta:base-class-count="6">
    
    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>
    <xsl:variable name="gs_prototype_path"><xsl:text>jQuery.</xsl:text><xsl:value-of select="@name"/></xsl:variable>

    
    <xsl:value-of select="$gs_prototype_path"/><xsl:text> = function(</xsl:text>
    <xsl:value-of select="xjs:parameters(@parameters, @name)"/>
    <xsl:text>) {</xsl:text>
      <xsl:value-of select="$gs_newline"/>
      <xsl:if test="not(@parameter-checks = 'off' or ancestor::javascript:code[1][@parameter-checks = 'off'])">
        <xsl:value-of select="xjs:parameter-checks(@parameters, @name, 6, $gs_stage_dev)"/>
      </xsl:if>
      <xsl:apply-templates/>
    <xsl:text>}</xsl:text>
    <xsl:value-of select="$gs_newline"/>

    <xsl:value-of select="$gs_prototype_path"/><xsl:text>.extension = true;</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template><xsl:template match="javascript:jquery-event-extension" meta:base-class-count="6">
    
    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>

    <xsl:text>jQuery.Event.prototype.</xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text> = function(</xsl:text>
    <xsl:value-of select="xjs:parameters(@parameters, @name)"/>
    <xsl:text>) {</xsl:text>
      <xsl:value-of select="$gs_newline"/>
      <xsl:if test="not(@parameter-checks = 'off' or ancestor::javascript:code[1][@parameter-checks = 'off'])"><xsl:value-of select="xjs:parameter-checks(@parameters, @name, 6, $gs_stage_dev)"/></xsl:if>
      <xsl:apply-templates/>
    <xsl:text>}</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template><xsl:template match="javascript:javascript-static-extension-property" meta:base-class-count="6">
    
    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>

    <xsl:value-of select="@object-name"/>
    <xsl:text>.</xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text> = </xsl:text>
    <xsl:value-of select="."/>
    <xsl:text>;</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template><xsl:template match="javascript:javascript-extension|javascript:javascript-static-extension" meta:base-class-count="6">
    
    <xsl:if test="@object-name = 'Object' and not(@name = 'toString')">
      <xsl:text>/* Object extension of [</xsl:text>
      <xsl:value-of select="@name"/>
      <xsl:text>] ignored */</xsl:text>
      <xsl:value-of select="$gs_newline"/>
      <xsl:text>alert('cannot extend the javascript Object because it breaks all for-each() loops including jQuery')</xsl:text>
    </xsl:if>

    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>

    <xsl:if test="not(@object-name = 'Object' and not(@name = 'toString'))">
      <xsl:variable name="gs_function_name">
        <xsl:value-of select="@object-name"/>
        <xsl:if test="not(self::javascript:javascript-static-extension)"><xsl:text>.prototype</xsl:text></xsl:if>
        <xsl:text>.</xsl:text>
        <xsl:value-of select="@name"/>
      </xsl:variable>

      <xsl:if test="not(@override)">
        <xsl:text>if (typeof </xsl:text>
        <xsl:value-of select="$gs_function_name"/>
        <xsl:text> !== 'function') {</xsl:text>
        <xsl:value-of select="$gs_newline"/>
      </xsl:if>
        <xsl:value-of select="$gs_function_name"/>
        <xsl:text> = function(</xsl:text>
        <xsl:value-of select="xjs:parameters(@parameters, $gs_function_name)"/>
        <xsl:text>) {</xsl:text>
          <xsl:value-of select="$gs_newline"/>
          <xsl:if test="not(@parameter-checks = 'off' or ancestor::javascript:code[1][@parameter-checks = 'off'])"><xsl:value-of select="xjs:parameter-checks(@parameters, $gs_function_name, 6, $gs_stage_dev)"/></xsl:if>
          <xsl:apply-templates/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$gs_newline"/>

        <xsl:value-of select="$gs_function_name"/>
        <xsl:text>.exec = function(){return true;}</xsl:text>
        <xsl:value-of select="$gs_newline"/>
      <xsl:if test="not(@override)">}</xsl:if>
      <xsl:value-of select="$gs_newline"/>
    </xsl:if>

    <xsl:if test="@override = 'true'">
      <xsl:value-of select="$gs_prototype_path"/><xsl:text>.override = true;</xsl:text>
      <xsl:value-of select="$gs_newline"/>
    </xsl:if>

    <xsl:value-of select="$gs_newline"/>
  </xsl:template><xsl:template match="javascript:object|javascript:namespace" meta:base-class-count="6"><xsl:param name="gs_class" select="parent::javascript:code/parent::class:*"/>
    

    
    

    <xsl:variable name="gs_inheritance_order" select="database:base-class-count($gs_class)"/>

    
    <xsl:variable name="gs_object_name_fragment">
      <xsl:choose>
        <xsl:when test="not(@name) or @name = '' or @name = 'inherit'"><xsl:value-of select="local-name($gs_class)"/></xsl:when>
        <xsl:otherwise><xsl:value-of select="@name"/></xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="gs_object_name" select="string($gs_object_name_fragment)"/>

    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words">
      <xsl:with-param name="gs_name" select="$gs_object_name"/>
    </xsl:apply-templates>

    <xsl:if test="not($gs_object_name)">
      <xsl:text>if (window.console) {window.console.error("object without an @name");}</xsl:text>
    </xsl:if>
    <xsl:if test="$gs_object_name">
      <xsl:text>function </xsl:text>
      <xsl:value-of select="$gs_object_name"/>
      <xsl:text>(</xsl:text>
      <xsl:value-of select="xjs:parameters(javascript:init/@parameters, $gs_object_name)"/>
      <xsl:text>) {</xsl:text>
        <xsl:value-of select="$gs_newline"/>

        
        <xsl:text>  if (!OO.isCreateForInheritance(arguments)) {</xsl:text>
        <xsl:value-of select="$gs_newline"/>

        <xsl:if test="not(javascript:init/@parameter-checks = 'off' or ancestor::javascript:code[1][@parameter-checks = 'off'])">
          <xsl:value-of select="xjs:parameter-checks(javascript:init/@parameters, $gs_object_name, 6, $gs_stage_dev)"/>
          <xsl:value-of select="$gs_newline"/>
        </xsl:if>

        
        <xsl:apply-templates select="." mode="gs_javascript_before_method"/>

        
        <xsl:if test="$gs_stage_dev">
          <xsl:text>      </xsl:text>
          <xsl:text>Function.cpfs(window.</xsl:text>
          <xsl:value-of select="$gs_object_name"/>
          <xsl:text>);</xsl:text>
          <xsl:value-of select="$gs_newline"/>
        </xsl:if>
      
        <xsl:apply-templates select="@requires"/>
        
        
        <xsl:text>    if (!this.classDerivedConstructor) {</xsl:text><xsl:value-of select="$gs_newline"/>
          <xsl:text>      this.classDerivedConstructor=</xsl:text><xsl:value-of select="$gs_object_name"/><xsl:text>;</xsl:text><xsl:value-of select="$gs_newline"/>
          <xsl:value-of select="$gs_newline"/>

          
          <xsl:if test="$gs_class/@is-singleton">
            <xsl:text>    OO.setSingleton(this);</xsl:text>
            <xsl:value-of select="$gs_newline"/>
          </xsl:if>
        <xsl:text>    }</xsl:text>
        <xsl:value-of select="$gs_newline"/>

        
        

        
        <xsl:if test="not(javascript:init/@chain = 'false') and $gs_class/class:*">
          <xsl:value-of select="$gs_newline"/>
          <xsl:text>      </xsl:text>
          <xsl:text>var oChainResult = OO.chainMethod(</xsl:text>
          <xsl:value-of select="$gs_object_name"/>
          <xsl:text>, this, undefined, arguments);</xsl:text>
          <xsl:value-of select="$gs_newline"/>
        </xsl:if>

        
        <xsl:apply-templates select="javascript:object-property" mode="gs_in_object_init"/>

        
        <xsl:apply-templates select="javascript:init" mode="gs_in_object_init"/>

        
        <xsl:text>  }</xsl:text>
        <xsl:value-of select="$gs_newline"/>
      <xsl:text>}</xsl:text>
      <xsl:value-of select="$gs_newline"/>
      
      <xsl:value-of select="$gs_object_name"/>
      <xsl:text>.notDefinedClass=true;</xsl:text>
      <xsl:value-of select="$gs_newline"/>
      
      
      <xsl:if test="string(@inherits-javascript-primitive)">
        <xsl:if test="@inherits-javascript-primitive = 'Object'">//</xsl:if>
        <xsl:text>OO.inheritFrom(</xsl:text>
        <xsl:value-of select="$gs_object_name"/>
        <xsl:text>, window.</xsl:text>
        <xsl:value-of select="@inherits-javascript-primitive"/>
        <xsl:text>);</xsl:text>
        <xsl:value-of select="$gs_newline"/>
      </xsl:if>

      <xsl:if test="$gs_class">
        
        

        
        

        
        <xsl:value-of select="$gs_object_name"/><xsl:text>.inheritanceOrder = </xsl:text>
        <xsl:value-of select="$gs_inheritance_order"/>
        <xsl:text>;</xsl:text>
        <xsl:value-of select="$gs_newline"/>

        
        <xsl:apply-templates select="$gs_class/@*[namespace-uri()='']" mode="gs_javascript_class_attributes">
          <xsl:with-param name="gs_object_name" select="$gs_object_name"/>
        </xsl:apply-templates>

        
        <xsl:apply-templates select="$gs_class/class:*/javascript:code/javascript:object[not(@name) or @name = 'inherit']" mode="gs_inheritance_render">
          <xsl:with-param name="gs_class" select="$gs_class"/>
          <xsl:with-param name="gs_object_name" select="$gs_object_name"/>
          <xsl:sort select="../../@is-library-wrapper" order="descending"/>
          <xsl:sort select="database:base-class-count(../..)" data-type="number" order="descending"/>
        </xsl:apply-templates>

        
        <xsl:apply-templates select="$gs_class/xsl:stylesheet[str:boolean(@controller)]/xsl:template" mode="gs_javascript_xsl_templates">
          <xsl:with-param name="gs_object_name" select="$gs_object_name"/>
        </xsl:apply-templates>
      </xsl:if>
      
      <xsl:apply-templates select="$gs_class/repository:friends/class:*" mode="gs_javascript_friends">
        <xsl:with-param name="gs_object_name" select="$gs_object_name"/>
      </xsl:apply-templates>

      
      <xsl:apply-templates select="javascript:*">
        <xsl:with-param name="gs_object_name" select="$gs_object_name"/>
      </xsl:apply-templates>

      
      <xsl:text>OO.registerClass(</xsl:text>
      <xsl:value-of select="$gs_object_name"/>
      <xsl:text>);</xsl:text>
      <xsl:value-of select="$gs_newline"/>

      <xsl:value-of select="$gs_newline"/>
      <xsl:value-of select="$gs_newline"/>
    </xsl:if>
  </xsl:template><xsl:template match="javascript:object/javascript:object" meta:base-class-count="6">
    <xsl:comment>sub-objects [<xsl:value-of select="../@xml:id"/>/<xsl:value-of select="@xml:id"/>] not supported yet</xsl:comment>
  </xsl:template><xsl:template match="*" mode="gs_context_menu_views" meta:base-class-count="6"/><xsl:template match="*" mode="gs_context_menu_controllers" meta:base-class-count="6"/><xsl:template match="@base-class-count|@name|@is-library-wrapper" mode="gs_javascript_class_attributes" meta:base-class-count="6"/><xsl:template match="@*" mode="gs_javascript_class_attributes" meta:base-class-count="6"><xsl:param name="gs_object_name"/>
    
    <xsl:text>OO.addCP(</xsl:text>
    <xsl:value-of select="$gs_object_name"/>
    <xsl:text>,"</xsl:text><xsl:value-of select="name()"/><xsl:text>"</xsl:text>
    <xsl:text>,</xsl:text>
    <xsl:text>"</xsl:text><xsl:value-of select="." disable-output-escaping="yes"/><xsl:text>"</xsl:text>
    <xsl:text>);</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template><xsl:template match="javascript:object" mode="gs_inheritance_render_base_list" meta:base-class-count="6">
    <xsl:value-of select="local-name(../parent::class:*)"/>
    <xsl:if test="not(position() = last())">,</xsl:if>
  </xsl:template><xsl:template match="javascript:object" mode="gs_inheritance_render_init_chain" meta:base-class-count="6">
    <xsl:value-of select="local-name(../parent::class:*)"/>
    <xsl:text>.apply(this, arguments);</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template><xsl:template match="class:*" mode="gs_javascript_friends" meta:base-class-count="6"><xsl:param name="gs_object_name"/>
    
    
    <xsl:if test="$gs_stage_dev">
      
      <xsl:text>OO.addFriend(</xsl:text>
      <xsl:value-of select="$gs_object_name"/>
      <xsl:text>,window.</xsl:text>
      <xsl:value-of select="local-name()"/>
      <xsl:text>);</xsl:text>
      <xsl:value-of select="$gs_newline"/>
    </xsl:if>
  </xsl:template><xsl:template match="javascript:object" mode="gs_inheritance_render" meta:base-class-count="6"><xsl:param name="gs_class"/><xsl:param name="gs_object_name"/>
    
    
    

    <xsl:variable name="gs_base_class" select="ancestor::class:*[1]"/>

    <xsl:text>OO.registerDerivedClass(</xsl:text>
    <xsl:value-of select="$gs_object_name"/>
    <xsl:text>, window.</xsl:text><xsl:value-of select="local-name($gs_base_class)"/>
    <xsl:text>);</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template><xsl:template match="javascript:object/javascript:init" mode="gs_in_object_init" meta:base-class-count="6">
    
    <xsl:apply-templates/>
  </xsl:template><xsl:template match="javascript:object/javascript:init" meta:base-class-count="6"/><xsl:template match="javascript:enum" meta:base-class-count="6"><xsl:param name="gs_object_name"/>
    

    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>

    <xsl:value-of select="$gs_object_name"/><xsl:text>.</xsl:text><xsl:value-of select="@name"/><xsl:text> = {</xsl:text>
    <xsl:apply-templates/>
    <xsl:text>};</xsl:text>
  </xsl:template><xsl:template match="javascript:conversion" mode="gs_function_name" meta:base-class-count="6">
    <xsl:value-of select="concat('to', @to-type)"/>
  </xsl:template><xsl:template match="javascript:enum/javascript:value" meta:base-class-count="6">
    <xsl:text>"</xsl:text><xsl:value-of select="@name"/><xsl:text>"</xsl:text>:<xsl:value-of select="."/><xsl:text>,</xsl:text>
  </xsl:template><xsl:template match="javascript:object/javascript:object-property" meta:base-class-count="6"/><xsl:template match="javascript:object/javascript:object-property" mode="gs_in_object_init" meta:base-class-count="6"><xsl:param name="gs_object_name"/>
    
    

    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>

    <xsl:text>this.</xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text> = </xsl:text>
    <xsl:if test="not(node())">undefined</xsl:if>
    <xsl:apply-templates/>
    <xsl:text>;</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template><xsl:template match="javascript:object/javascript:fixed-prototype-property" meta:base-class-count="6"><xsl:param name="gs_object_name"/>
    
    

    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>

    <xsl:variable name="gs_prototype_path">
      <xsl:value-of select="$gs_object_name"/><xsl:text>.prototype.</xsl:text><xsl:value-of select="@name"/>
    </xsl:variable>

    <xsl:value-of select="$gs_prototype_path"/>
    <xsl:text> = </xsl:text>
    <xsl:if test="not(node())">undefined</xsl:if>
    <xsl:apply-templates/>
    <xsl:text>;</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template><xsl:template match="*|@*|text()" mode="gs_check_javascript_reserved_words" meta:base-class-count="6">
    
  </xsl:template><xsl:template match="*" mode="gs_javascript_function_init" meta:base-class-count="6"/><xsl:template match="*" mode="gs_javascript_after_function" meta:base-class-count="6"/><xsl:template match="@name|@parameters|@xml:id|@is-protected|@parameter-checks" mode="gs_function_attributes" meta:base-class-count="6"/><xsl:template match="javascript:static-method/@*|javascript:conversion/@*" mode="gs_function_attributes" meta:base-class-count="6"/><xsl:template match="@*" mode="gs_function_attributes" meta:base-class-count="6"><xsl:param name="gs_object_name"/>
    
    
    <xsl:text>OO.addMP(f,"</xsl:text>
    <xsl:value-of select="name()"/><xsl:text>"</xsl:text>
    <xsl:text>,</xsl:text>
    <xsl:text>"</xsl:text><xsl:value-of select="." disable-output-escaping="yes"/><xsl:text>"</xsl:text>
    <xsl:text>);</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template><xsl:template match="javascript:event-handler" mode="gs_javascript_function_init" meta:base-class-count="6">
    <xsl:variable name="gs_action" select="string(@action)"/>
    <xsl:variable name="gs_content" select="string(.)"/>
    <xsl:if test="not($gs_content) and $gs_action and ../javascript:method[@name = $gs_action]">
      <xsl:text>      </xsl:text>
      <xsl:text>var oDirectRunResult = this.</xsl:text>
      <xsl:value-of select="$gs_action"/>
      <xsl:text>.apply(this, arguments);</xsl:text>
      <xsl:value-of select="$gs_newline"/>
    </xsl:if>
  </xsl:template><xsl:template match="javascript:static-event-handler" mode="gs_javascript_function_init" meta:base-class-count="6">
    <xsl:variable name="gs_action" select="string(@action)"/>
    <xsl:variable name="gs_content" select="string(.)"/>
    <xsl:if test="not($gs_content) and $gs_action and ../javascript:static-method[@name = $gs_action]">
      <xsl:text>      </xsl:text>
      <xsl:text>var oDirectRunResult = this.</xsl:text>
      <xsl:value-of select="$gs_action"/>
      <xsl:text>.apply(this, arguments);</xsl:text>
      <xsl:value-of select="$gs_newline"/>
    </xsl:if>
  </xsl:template><xsl:template match="javascript:*" mode="gs_function_name" meta:base-class-count="6">
    <xsl:value-of select="@name"/>
  </xsl:template><xsl:template match="javascript:event-handler|javascript:static-event-handler" mode="gs_function_name" meta:base-class-count="6">
    
    <xsl:text>f_</xsl:text>
    <xsl:value-of select="@event"/>
    <xsl:if test="string(@action)">_<xsl:value-of select="@action"/></xsl:if>
  </xsl:template><xsl:template match="javascript:method|javascript:conversion|javascript:event-handler|javascript:capability" meta:base-class-count="6"><xsl:param name="gs_object_name"/>
    
    

    <xsl:variable name="gs_function_name">
      <xsl:apply-templates select="." mode="gs_function_name"/>
    </xsl:variable>

    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>

    <xsl:variable name="gs_prototype_path">
      <xsl:value-of select="$gs_object_name"/><xsl:text>.prototype.</xsl:text><xsl:value-of select="$gs_function_name"/>
    </xsl:variable>

    
    <xsl:text>var f=</xsl:text>
    <xsl:value-of select="$gs_prototype_path"/>
    <xsl:text>=function</xsl:text>
    <xsl:text> </xsl:text><xsl:value-of select="$gs_function_name"/>
    <xsl:text>(</xsl:text>
    <xsl:value-of select="xjs:parameters(@parameters, $gs_function_name)"/>
    <xsl:text>) {</xsl:text>
      <xsl:value-of select="$gs_newline"/>
      
      
      
      <xsl:apply-templates select="." mode="gs_javascript_before_method"/>
      
      
      <xsl:if test="$gs_stage_dev">
        <xsl:text>      </xsl:text>
        <xsl:text>Function.cpfs(</xsl:text>
        <xsl:value-of select="$gs_prototype_path"/>
        <xsl:text>,</xsl:text>
        <xsl:if test="@is-protected = 'true'">true</xsl:if>
        <xsl:if test="not(@is-protected = 'true')">false</xsl:if>
        <xsl:text>);</xsl:text>
        <xsl:value-of select="$gs_newline"/>
      </xsl:if>
      
      <xsl:apply-templates select="@requires"/>

      <xsl:apply-templates select="." mode="gs_javascript_function_init"/>

      <xsl:if test="@chain = 'true'">
        <xsl:text>      </xsl:text>
        <xsl:text>var fF=</xsl:text><xsl:value-of select="$gs_object_name"/><xsl:text>.prototype.</xsl:text><xsl:value-of select="$gs_function_name"/><xsl:text>;</xsl:text>
        <xsl:value-of select="$gs_newline"/>
        <xsl:text>      </xsl:text>
        <xsl:text>var oChainResult = OO.chainMethod(fF.classDerivedConstructor, this, fF.functionName(), arguments);</xsl:text>
        <xsl:value-of select="$gs_newline"/>
      </xsl:if>
      <xsl:if test="not(@parameter-checks = 'off' or ancestor::javascript:code[1][@parameter-checks = 'off'])">
        <xsl:value-of select="xjs:parameter-checks(@parameters, $gs_function_name, 6, $gs_stage_dev)"/>
      </xsl:if>

      
      <xsl:apply-templates/>
    <xsl:text>}</xsl:text>
    <xsl:value-of select="$gs_newline"/>

    
    <xsl:text>f.functionType="</xsl:text>
    <xsl:apply-templates select="." mode="gs_dash_to_camel">
      <xsl:with-param name="gs_string" select="local-name()"/>
    </xsl:apply-templates>
    <xsl:text>";</xsl:text>
    <xsl:value-of select="$gs_newline"/>
    
    
    <xsl:if test="@is-protected = 'true'">
      <xsl:text>f.isProtected=true;</xsl:text>
      <xsl:value-of select="$gs_newline"/>
    </xsl:if>

    <xsl:apply-templates select="@*" mode="gs_function_attributes"/>

    <xsl:apply-templates select="." mode="gs_javascript_after_function"/>
  </xsl:template><xsl:template match="javascript:*/@requires" meta:base-class-count="6"><xsl:param name="gs_requires" select="string(.)"/>
    
    
    <xsl:if test="$gs_stage_dev">
      <xsl:variable name="gs_requires_part_fragment">
        <xsl:value-of select="substring-before($gs_requires, ',')"/>
        <xsl:if test="not(contains($gs_requires, ','))"><xsl:value-of select="$gs_requires"/></xsl:if>
      </xsl:variable>
      <xsl:variable name="gs_requires_part" select="normalize-space($gs_requires_part_fragment)"/>
      
      <xsl:if test="$gs_requires_part">
        <xsl:text>      </xsl:text>
        <xsl:text>if ((window.</xsl:text>
        <xsl:value-of select="$gs_requires_part"/>
        <xsl:text> === undefined)) Debug.error(this, "[window.</xsl:text>
        <xsl:value-of select="$gs_requires_part"/>
        <xsl:text>] required for [</xsl:text>
        <xsl:value-of select="../@name"/>
        <xsl:text>]");</xsl:text>
        <xsl:value-of select="$gs_newline"/>
      </xsl:if>
      
      <xsl:if test="contains($gs_requires, ',')">
        <xsl:apply-templates select=".">
          <xsl:with-param name="gs_requires" select="substring-after($gs_requires, ',')"/>
        </xsl:apply-templates>
      </xsl:if>
    </xsl:if>
  </xsl:template><xsl:template match="javascript:object/javascript:static-method|javascript:namespace/javascript:static-method|javascript:object/javascript:static-event-handler|javascript:object/javascript:static-capability" meta:base-class-count="6"><xsl:param name="gs_object_name"/>
    
    
    
    <xsl:variable name="gs_function_name">
      <xsl:apply-templates select="." mode="gs_function_name"/>
    </xsl:variable>

    <xsl:variable name="gs_prototype_path">
      <xsl:value-of select="$gs_object_name"/><xsl:text>.</xsl:text><xsl:value-of select="$gs_function_name"/>
    </xsl:variable>

    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>

    <xsl:text>var f=</xsl:text>
    <xsl:value-of select="$gs_prototype_path"/>
    <xsl:text> = function</xsl:text>
    <xsl:text> </xsl:text><xsl:value-of select="$gs_function_name"/>
    <xsl:text>(</xsl:text>
    <xsl:value-of select="xjs:parameters(@parameters, $gs_function_name)"/>
    <xsl:text>) {</xsl:text>
      <xsl:value-of select="$gs_newline"/>
      
      <xsl:apply-templates select="." mode="gs_javascript_before_method"/>

      
      <xsl:if test="$gs_stage_dev">
        <xsl:text>      </xsl:text>
        <xsl:text>Function.cpfs(</xsl:text>
        <xsl:value-of select="$gs_prototype_path"/>
        <xsl:text>,</xsl:text>
        <xsl:if test="@is-protected = 'true'">true</xsl:if>
        <xsl:if test="not(@is-protected = 'true')">false</xsl:if>
        <xsl:text>);</xsl:text>
        <xsl:value-of select="$gs_newline"/>
      </xsl:if>
      
      <xsl:apply-templates select="@requires"/>

      <xsl:apply-templates select="." mode="gs_javascript_function_init"/>
      
      <xsl:if test="not(@parameter-checks = 'off' or ancestor::javascript:code[1][@parameter-checks = 'off'])">
        <xsl:value-of select="xjs:parameter-checks(@parameters, $gs_function_name, 6, $gs_stage_dev)"/>
      </xsl:if>
      
      <xsl:apply-templates/>
    <xsl:text>}</xsl:text>
    <xsl:value-of select="$gs_newline"/>

    
    <xsl:if test="@is-protected = 'true'">
      <xsl:text>f.isProtected=true;</xsl:text>
      <xsl:value-of select="$gs_newline"/>
    </xsl:if>

    
    

    
    <xsl:text>f.functionType="</xsl:text>
    <xsl:apply-templates select="." mode="gs_dash_to_camel">
      <xsl:with-param name="gs_string" select="local-name()"/>
    </xsl:apply-templates>
    <xsl:text>";</xsl:text>
    <xsl:value-of select="$gs_newline"/>

    <xsl:apply-templates select="@*" mode="gs_function_attributes"/>

    <xsl:apply-templates select="." mode="gs_javascript_after_function"/>
  </xsl:template><xsl:template match="javascript:object/javascript:static-property|javascript:namespace/javascript:static-property" meta:base-class-count="6"><xsl:param name="gs_object_name"/>
    
    
    
    <xsl:variable name="gs_property_path">
      <xsl:value-of select="$gs_object_name"/><xsl:text>.</xsl:text><xsl:value-of select="@name"/>
    </xsl:variable>

    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>

    <xsl:variable name="gs_property_value" select="normalize-space(.)"/>
    <xsl:variable name="gs_is_literal" select="not($gs_property_value) or (starts-with($gs_property_value, '$') or starts-with($gs_property_value, '/') or starts-with($gs_property_value, '{') or starts-with($gs_property_value, '[') or starts-with($gs_property_value, $gs_quote) or starts-with($gs_property_value, $gs_apos) or $gs_property_value = 'true' or $gs_property_value = 'false' or (translate($gs_property_value, concat($gs_uppercase, '_'), '') = '') or (translate($gs_property_value, '0123456789.', '') = '') or $gs_property_value = 'new Object()' or $gs_property_value = 'jQuery()' or $gs_property_value = 'new Array()' or $gs_property_value = 'new Cache()')"/>
    <xsl:variable name="gs_delay" select="$gs_property_value and not(@delay = 'false') and not($gs_is_literal)"/>

    <xsl:if test="$gs_delay">
      <xsl:text>//delayed because it seems to be a non-literal</xsl:text>
      <xsl:value-of select="$gs_newline"/>
      <xsl:text>setTimeout(function(){</xsl:text>
      <xsl:value-of select="$gs_newline"/>
    </xsl:if>
      <xsl:value-of select="$gs_property_path"/>
      <xsl:text> = </xsl:text>
      <xsl:apply-templates/> 
      <xsl:if test="not($gs_property_value)">undefined</xsl:if>
      <xsl:text>;</xsl:text>
      <xsl:value-of select="$gs_newline"/>
    <xsl:if test="$gs_delay">
      <xsl:text>}, 0);</xsl:text>
      <xsl:value-of select="$gs_newline"/>
    </xsl:if>
  </xsl:template><xsl:template match="xsd:schema" mode="gs_javascript_xsd_model" meta:base-class-count="6"><xsl:param name="gs_object_name"/>
    
  
    
    <xsl:value-of select="$gs_object_name"/><xsl:text>.model = jQuery('</xsl:text>
    
    
    <xsl:text>');</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template><xsl:template match="javascript:object" mode="editor" meta:base-class-count="6">
    <pre>
      <xsl:apply-templates select="."/>
    </pre>
  </xsl:template><xsl:template match="javascript:dependency" mode="editor" meta:base-class-count="6">
    <li><xsl:value-of select="@uri"/></li>
  </xsl:template><xsl:template match="javascript:object" mode="eventhandler" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/>
    

    
    <li class="{$gs_html_identifier_class} gs-semantic-only-markup">
      <ul class="CSS__VerticalMenu gs-interface-mode-default gs-semantic-only-markup">
        <xsl:if test="not(*)">
          <li class="gs-empty-message">no event handlers</li>
        </xsl:if>
        <xsl:apply-templates mode="gs_view_render">
          <xsl:with-param name="gs_interface_mode" select="'eventhandler'"/>
        </xsl:apply-templates>
        <li class="gs-add-message"><a href="#">add</a> an event-handler</li>
      </ul>
    </li>
  </xsl:template><xsl:template match="javascript:event-handler" mode="eventhandler" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/><xsl:param name="gs_interface_mode"/>
    
    

    <li class="{$gs_html_identifier_class} gs-xsl-javascript-function">
      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups">
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      </xsl:apply-templates>
      <xsl:text>f_</xsl:text>
      <xsl:value-of select="@event"/>
      <xsl:if test="@action">_<xsl:value-of select="@action"/></xsl:if>
      <xsl:if test="@selector"> (<xsl:value-of select="@selector" disable-output-escaping="yes"/>)</xsl:if>
    </li>
  </xsl:template><xsl:template match="interface:QualifiedName" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/>
    
    

    <xsl:variable name="gs_delimiter">
      <xsl:if test="@delimiter"><xsl:value-of select="@delimiter"/></xsl:if>
      <xsl:if test="not(@delimiter)"> :: </xsl:if>
    </xsl:variable>

    <div class="{$gs_html_identifier_class}">
      <xsl:apply-templates mode="gs_delimit">
        <xsl:with-param name="gs_delimiter" select="$gs_delimiter"/>
      </xsl:apply-templates>
      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups"/>
    </div>
  </xsl:template><xsl:template match="*" mode="gs_delimit" meta:base-class-count="6"><xsl:param name="gs_delimiter" select="', '"/>
    
    
    <xsl:if test="@name">
      <xsl:value-of select="@name"/>
      <xsl:if test="not(last())"><xsl:value-of select="$gs_delimiter"/></xsl:if>
    </xsl:if>
  </xsl:template><xsl:template match="interface:Overlay|interface:SubMenu|interface:ContextMenu" mode="gs_extra_classes" meta:base-class-count="0">
    
    <xsl:text>gs-item-</xsl:text><xsl:apply-templates select="." mode="gs_extra_classes_overlay_transition"/>
    <xsl:text> gs-exclusivity-</xsl:text><xsl:apply-templates select="." mode="gs_extra_classes_overlay_exclusivity"/>
    <xsl:text> gs-position-</xsl:text><xsl:apply-templates select="." mode="gs_extra_classes_overlay_position"/>
  </xsl:template><xsl:template match="*" mode="gs_extra_classes_overlay_transition" meta:base-class-count="0"/><xsl:template match="*" mode="gs_extra_classes_overlay_exclusivity" meta:base-class-count="0"/><xsl:template match="*" mode="gs_extra_classes_overlay_position" meta:base-class-count="0"/><xsl:template match="interface:ContextMenu" mode="gs_extra_classes_overlay_transition" meta:base-class-count="9">instant</xsl:template><xsl:template match="interface:ContextMenu" mode="gs_extra_classes_overlay_exclusivity" meta:base-class-count="9">global</xsl:template><xsl:template match="interface:ContextMenu" mode="gs_extra_classes_overlay_position" meta:base-class-count="9">mouse</xsl:template><xsl:template match="interface:Menu//gs:menu-section|interface:HorizontalMenu//gs:menu-section|interface:VerticalMenu//gs:menu-section|interface:SubMenu//gs:menu-section|interface:ContextMenu//gs:menu-section" meta:base-class-count="6">
    <xsl:if test="*">
      <xsl:if test="preceding-sibling::*">
        <li class="gs-menu-section"/>
      </xsl:if>
      <xsl:apply-templates select="*"/>
    </xsl:if>
  </xsl:template><xsl:template match="interface:Menu//gs:menu-section|interface:HorizontalMenu//gs:menu-section|interface:VerticalMenu//gs:menu-section|interface:SubMenu//gs:menu-section|interface:ContextMenu//gs:menu-section" meta:base-class-count="6">
    <xsl:if test="*">
      <xsl:if test="preceding-sibling::*">
        <li class="gs-menu-section"/>
      </xsl:if>
      <xsl:apply-templates select="*"/>
    </xsl:if>
  </xsl:template><xsl:template match="interface:SubMenu" mode="gs_extra_classes_overlay_exclusivity" meta:base-class-count="9">sibling</xsl:template><xsl:template match="xsl:stylesheet" mode="editor" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/>
    

    <div class="{$gs_html_identifier_class} gs-clickable">
      
      <xsl:if test="comment() and not(contains(comment()[1], '###'))">
        <div class="gs_description"><xsl:apply-templates select="comment()[1]" mode="gs_formatted_output"/></div>
      </xsl:if>

      
      <xsl:if test="not(namespace::*[name() = ''] = 'http://www.w3.org/1999/xhtml')">
        <div class="gs-warning">
          default xmlns namespace is not set to http://www.w3.org/1999/xhtml
          (<xsl:value-of select="namespace::*[name() = '']"/>)
        </div>
      </xsl:if>

      <xsl:if test="xsl:template[@name]">
        <div class="gs-warning">
          named templates are dicouraged because they cannot be overridden or re-xsl:included
          use &lt;xsl:template match="*" mode="&lt;name&gt;"&gt; instead
        </div>
      </xsl:if>

      <xsl:if test=".//xsl:value-of">
        <div class="gs-warning">
          xsl:value-of is discouraged because they cannot be overridden
          use &lt;xsl:apply-templates select="$item"/&gt; instead
          which will use the standard templates to output the text result
        </div>
      </xsl:if>

      <xsl:if test=".//xsl:copy-of">
        <div class="gs-warning">
          xsl:copy-of is discouraged because they cannot be overridden
          use &lt;xsl:apply-templates select="$item" mode="gs_view_render" /&gt; instead
          which will use the standard templates to output the tree
        </div>
      </xsl:if>

      <xsl:if test=".//xsl:element[not(@namespace)]">
        <div class="gs-warning">
          dynamic @name xsl:element should have a @namespace attribute
          otherwise qualified QNames, e.g. "gs:example", may cause namespace prefix not found errors
          i.e. &lt;xsl:element name="{local-name($gs_element)}" namespace="{namespace-uri($gs_element)}"&gt;
        </div>
      </xsl:if>


      
      <xsl:if test="@response:server-side-only='true'">
        <xsl:variable name="gs_extension_element_prefixes" select="concat(' ', @extension-element-prefixes, ' ')"/>
        <xsl:variable name="gs_missing_extension_element_prefixes">
          <xsl:if test="not(contains($gs_extension_element_prefixes, ' debug '))">debug </xsl:if>
          <xsl:if test="not(contains($gs_extension_element_prefixes, ' request '))">request </xsl:if>
          <xsl:if test="not(contains($gs_extension_element_prefixes, ' response '))">response </xsl:if>
          <xsl:if test="not(contains($gs_extension_element_prefixes, ' interface '))">interface </xsl:if>
          <xsl:if test="not(contains($gs_extension_element_prefixes, ' object '))">object </xsl:if>
          <xsl:if test="not(contains($gs_extension_element_prefixes, ' regexp '))">regexp </xsl:if>
          <xsl:if test="not(contains($gs_extension_element_prefixes, ' str '))">str </xsl:if>
          <xsl:if test="not(contains($gs_extension_element_prefixes, ' flow '))">flow </xsl:if>
          <xsl:if test="not(contains($gs_extension_element_prefixes, ' database '))">database </xsl:if>
          <xsl:if test="not(contains($gs_extension_element_prefixes, ' repository '))">repository </xsl:if>
        </xsl:variable>
        <xsl:if test="not($gs_missing_extension_element_prefixes = '')">
          <div class="gs-warning">
            @extension-element-prefixes does not contain namespaces:
            <xsl:value-of select="$gs_missing_extension_element_prefixes"/>
          </div>
        </xsl:if>

        <xsl:if test="not(namespace::gs)">
          <div class="gs-warning">general_server xmlns namespace (gs) is not set</div>
        </xsl:if>
      </xsl:if>

      
      <xsl:if test="not(@response:server-side-only='true') and meta:class/class:*">
      </xsl:if>

      
      <xsl:if test="not(@response:server-side-only='true' or meta:class/class:*)">
        <xsl:if test="not(xsl:template[@match = 'gs:root'])">
          <div class="gs-warning">
            no meta:root handler template &lt;xsl:template match="gs:root" /&gt;
          </div>
        </xsl:if>

        <xsl:if test="xsl:template[@match = '/']">
          <div class="gs-warning">
            template &lt;xsl:template match="/" /&gt; is not advised
            use the &lt;xsl:template match="gs:root" /&gt; handler instead
          </div>
        </xsl:if>
      </xsl:if>

      
      <h2>settings</h2>
      <ul id="gs_settings">
        <xsl:apply-templates select="@meta:*" mode="xsl_stylesheet_editor"/>
        <xsl:apply-templates select="xsl:output/@*" mode="xsl_stylesheet_editor"/>
      </ul>

      <h2>data <span class="note">
        <a href="/id('{@xml:id}')?data-only=1" target="_blank">view (TODO: wrong host on this link = wrong default database)</a>
		| <a href="data_selector" target="_blank">add new data set...</a>
        </span>
      </h2>
      <ul id="gs_queries">
        <xsl:apply-templates select="database:query|interface:dynamic" mode="xsl_stylesheet_editor"/>
        <xsl:if test="not(database:query)"><li>no data sets yet</li></xsl:if>
      </ul>

      <h2>processing <span class="note">
		<a href="/shared/data_selector" target="_blank">xsl:include a shared stylesheet...</a>
		| <a href="/shared/data_selector" target="_blank">set new global xsl:variable...</a>
		| <a href="/shared/data_selector" target="_blank">add new xsl:template...</a>
        </span>
      </h2>
      <ul id="xsl_includes">
        <xsl:apply-templates select="xsl:include" mode="xsl_stylesheet_editor"/>
      </ul>
      <ul id="xsl_variables">
        <xsl:apply-templates select="xsl:variable" mode="xsl_stylesheet_editor"/>
      </ul>
      <ul id="xsl_templates">
        <xsl:apply-templates select="xsl:template" mode="xsl_stylesheet_editor"/>
      </ul>
    </div>
  </xsl:template><xsl:template match="database:query" mode="xsl_stylesheet_editor" meta:base-class-count="6">
    <li>
      <xsl:attribute name="class">
        <xsl:text>xsl_stylesheet_editor_gs_query</xsl:text>
        <xsl:text> DatabaseElement gs-xml-id-</xsl:text><xsl:value-of select="@xml:id"/>
        <xsl:if test="@evaluate = 'yes'"> gs-evaluate</xsl:if>
      </xsl:attribute>

      <input name="name" value="{@name}"/>
      <xsl:text>=</xsl:text>
      <input id="gs_evaluate_{@xml:id}" type="checkbox" name="evaluate" value="{@evaluate}"/>
      <label for="gs_evaluate_{@xml:id}">evaluate</label>
      <a class="gs_data_editor_link" href="data_selector" target="_blank">
        <xsl:value-of select="@data | ."/>
      </a>
      <input name="mode" value="{@interface-mode}"/>
      <input name="transform" value="{@transform}"/>
    </li>
  </xsl:template><xsl:template match="database:query" mode="gs_context_menu_custom" meta:base-class-count="6">
    <li class="f_click_select_children">browse data</li>
  </xsl:template><xsl:template match="xsl:output/@*" mode="xsl_stylesheet_editor" meta:base-class-count="6">
    <form><input id="gs_option_{name()}" name="{name()}" type="checkbox"/><label for="gs_option_{name()}"><xsl:value-of select="name()"/></label></form>
  </xsl:template><xsl:template match="xsl:include" mode="xsl_stylesheet_editor" meta:base-class-count="6">
    <li>
      <xsl:attribute name="class">
        <xsl:text>xsl_stylesheet_editor_xsl_include</xsl:text>
      </xsl:attribute>

      <xsl:if test="@xpath"><xsl:value-of select="@xpath"/></xsl:if>
      <xsl:if test="not(@xpath)"><xsl:value-of select="@href"/></xsl:if>
    </li>
  </xsl:template><xsl:template match="xsl:variable" mode="xsl_stylesheet_editor" meta:base-class-count="6">
    <li>
      <xsl:attribute name="class">
        <xsl:text>xsl_stylesheet_editor_xsl_variable</xsl:text>
      </xsl:attribute>

      <xsl:value-of select="."/>
    </li>
  </xsl:template><xsl:template match="xsl:stylesheet/@*" mode="xsl_stylesheet_editor" meta:base-class-count="6"/><xsl:template match="xsl:stylesheet/@meta:*" mode="xsl_stylesheet_editor" meta:base-class-count="6">
    <form><input id="gs_option_{name()}" name="{name()}" type="checkbox"/><label for="gs_option_{name()}"><xsl:value-of select="name()"/></label></form>
  </xsl:template><xsl:template match="xsl:template" mode="xsl_stylesheet_editor" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/>
    

    <li class="{$gs_html_identifier_class}">
      <xsl:attribute name="class">
        <xsl:value-of select="$gs_html_identifier_class"/>
        <xsl:text> xsl_stylesheet_editor_xsl_template</xsl:text>
        <xsl:if test="@response:server-side-only = 'true'"> gs-server-side-only</xsl:if>
        <xsl:if test="@match = 'gs:form'"> gs-form-processor</xsl:if>
        <xsl:if test="@match = 'gs:root'"> gs-main-template</xsl:if>
      </xsl:attribute>

      <xsl:apply-templates mode="gs_client_side_xsl_output_warnings"/>

      <a class="bare" href="~IDE/repository:interfaces/multiview?xpath-to-node=id('{@xml:id}')&amp;editor_tab=xmleditor">
        <div class="xsl_stylesheet_editor_xsl_template_title">
          <span class="name">
            <xsl:choose>
              <xsl:when test="@match = 'gs:root'">main &lt;html&gt; page template (gs:root)</xsl:when>
              <xsl:when test="@match = 'gs:form'">server side form processing (gs:form)</xsl:when>
              <xsl:otherwise><xsl:value-of select="@match|@name"/></xsl:otherwise>
            </xsl:choose>
          </span>

          <span class="mode">Â <xsl:value-of select="@mode"/></span>

          <xsl:if test="comment()">
            
            <div class="gs_description"><xsl:apply-templates select="comment()[1]" mode="gs_formatted_output"/></div>
          </xsl:if>
        </div>
      </a>
    </li>
  </xsl:template><xsl:template match="node()" mode="gs_formatted_output" meta:base-class-count="6">
    
    <ul>
      <xsl:apply-templates select="." mode="gs_formatted_output_recursive">
        <xsl:with-param name="text" select="string(.)"/>
      </xsl:apply-templates>
    </ul>
  </xsl:template><xsl:template match="node()" mode="gs_formatted_output_recursive" meta:base-class-count="6"><xsl:param name="text"/>
    

    <xsl:variable name="gs_splitter" select="'  '"/>

    <xsl:variable name="text_part">
      <xsl:if test="contains($text, $gs_splitter)"><xsl:value-of select="substring-before($text, $gs_splitter)"/></xsl:if>
      <xsl:if test="not(contains($text, $gs_splitter))"><xsl:value-of select="$text"/></xsl:if>
    </xsl:variable>
    <xsl:variable name="normal_text_part" select="normalize-space($text_part)"/>
    <xsl:variable name="next_part" select="substring-after($text, $gs_splitter)"/>

    <xsl:if test="$normal_text_part">
      <li>
        <xsl:choose>
          <xsl:when test="starts-with($normal_text_part, 'TODO:')">
            <span class="todo">TODO:</span>
            <xsl:value-of select="substring-after($normal_text_part, 'TODO:')"/>
          </xsl:when>
          <xsl:otherwise><xsl:value-of select="$normal_text_part"/></xsl:otherwise>
        </xsl:choose>
      </li>
    </xsl:if>

    <xsl:if test="normalize-space($next_part)">
      <xsl:apply-templates select="." mode="gs_formatted_output_recursive">
        <xsl:with-param name="text" select="$next_part"/>
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template><xsl:template match="xsl:stylesheet" mode="gs_listname" meta:base-class-count="6">
    <xsl:if test="@name"><xsl:value-of select="@name"/></xsl:if>
    <xsl:if test="not(@name)">stylesheet</xsl:if>
  </xsl:template><xsl:template match="xsl:stylesheet" mode="view" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/>
    

    
    <li class="{$gs_html_identifier_class} gs-semantic-only-markup">
      <ul class="CSS__VerticalMenu gs-interface-mode-default gs-semantic-only-markup gs-inherited-{@gs:inherited}">
        <xsl:if test="not(*) and not(@gs:inherited)">
          <li class="gs-empty-message">no views</li>
        </xsl:if>
        <xsl:apply-templates select="*" mode="gs_view_render">
          <xsl:with-param name="gs_interface_mode" select="'view'"/>
        </xsl:apply-templates>
        <xsl:if test="not(@gs:inherited)">
          <li class="gs-add-message"><a href="#">add</a> a view</li>
        </xsl:if>
      </ul>
    </li>
  </xsl:template><xsl:template match="xsl:stylesheet" mode="controller" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/>
    

    
    <li class="{$gs_html_identifier_class} gs-semantic-only-markup">
      <ul class="CSS__VerticalMenu gs-interface-mode-default gs-semantic-only-markup gs-inherited-{@gs:inherited}">
        <xsl:if test="not(*) and not(@gs:inherited)">
          <li class="gs-empty-message">no controllers</li>
        </xsl:if>
        <xsl:apply-templates select="*" mode="gs_view_render">
          <xsl:with-param name="gs_interface_mode" select="'controller'"/>
        </xsl:apply-templates>
        <xsl:if test="not(@gs:inherited)">
          <li class="gs-add-message"><a href="#">add</a> a controller</li>
        </xsl:if>
      </ul>
    </li>
  </xsl:template><xsl:template match="css:dependency" mode="html" meta:base-class-count="6">
    
    <xsl:variable name="gs_object_name">
      <xsl:choose>
        <xsl:when test="not(@object-name) or @object-name = '' or @object-name = 'inherit'">
          <xsl:text>CSS__</xsl:text><xsl:value-of select="local-name(ancestor::class:*[1])"/>
        </xsl:when>
        <xsl:otherwise><xsl:value-of select="@object-name"/></xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <link rel="stylesheet" xml:add-xml-id="no">
      <xsl:attribute name="href">
        <xsl:if test="starts-with(@uri, '/resources/')">
          <xsl:value-of select="$gs_resource_server"/>
        </xsl:if>
        <xsl:value-of select="@uri"/>
      </xsl:attribute>
      <xsl:if test="@meta:reason"><xsl:copy-of select="gs:reason"/></xsl:if>
      <xsl:if test="$gs_object_name"><xsl:attribute name="gs:object-name" select="{$gs_object_name}"/></xsl:if>
    </link>
  </xsl:template><xsl:template match="xsl:variable|xsl:param" mode="controller" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/>
    
    
    <div class="{$gs_html_identifier_class} gs-xsd-value gs-extended-attributes">
      <input name="{@name}" value="{@select}"/>
      <span class="gs-xtransport gs-xsd-field-message"><xsl:value-of select="@name"/></span>
      <span class="gs-xtransport gs-xsd-field-message-type">H</span>
      <span class="gs-xtransport gs-required">[a-zA-Z][-a-zA-Z0-9_.]{3}</span>
    </div>
  </xsl:template><xsl:template match="repository:*" mode="editor" meta:base-class-count="5"><xsl:param name="gs_html_identifier_class"/>
    

    <ul class="{$gs_html_identifier_class}">
      <xsl:apply-templates select="*" mode="gs_view_render">
        <xsl:with-param name="gs_interface_mode" select="'list'"/>
        <xsl:with-param name="gs_event_functions" select="'f_click_selectchild'"/>
      </xsl:apply-templates>
    </ul>
  </xsl:template><xsl:template match="object:Response" meta:base-class-count="6">
    
    <xsl:if test="not($gs_response)"><xsl:comment>no $gs_response</xsl:comment></xsl:if>
    <xsl:if test="not($gs_environment)"><xsl:comment>no $gs_environment</xsl:comment></xsl:if>
    <xsl:if test="not($gs_website_root)"><xsl:comment>no $gs_website_root</xsl:comment></xsl:if>
    <xsl:if test="not($gs_context_database)"><xsl:comment>no $gs_context_database</xsl:comment></xsl:if>
    <xsl:if test="not($gs_resource_server_object)"><xsl:comment>no $gs_resource_server_object</xsl:comment></xsl:if>
    <xsl:if test="not($gs_resource_server)"><xsl:comment>no $gs_resource_server</xsl:comment></xsl:if>
    <xsl:if test="not($gs_request_target)"><xsl:comment>no $gs_request_target</xsl:comment></xsl:if>

    <xsl:if test="@server-side-xslt = 'true'">
      
      <xsl:text>HTTP/1.0 200 OK</xsl:text><xsl:value-of select="$gs_newline"/>
      <xsl:text>Server: general_server/0.9 Alpha</xsl:text><xsl:value-of select="$gs_newline"/>
      <xsl:text>X-Powered-by: general_server/0.9 Alpha</xsl:text><xsl:value-of select="$gs_newline"/>
      <xsl:text>Date: Wed, 29 Apr 2015 16:25:21 GMT</xsl:text><xsl:value-of select="$gs_newline"/>
      <xsl:text>Content-Type: text/html</xsl:text><xsl:value-of select="$gs_newline"/>
      <xsl:text>Content-Length: [response:token-content-length]</xsl:text><xsl:value-of select="$gs_newline"/>
      <xsl:text>Header-complete:yes[response:headers-end]</xsl:text>
    </xsl:if>
    
    <xsl:apply-templates select="*" mode="gs_view_render"/>
  </xsl:template><xsl:template match="object:Response/gs:data" meta:base-class-count="6">
    <xsl:apply-templates select="*" mode="gs_view_render">
      <xsl:with-param name="gs_interface_mode" select="parent::object:Response/@gs:force-data-interface"/>
    </xsl:apply-templates>
  </xsl:template><xsl:template match="object:Request" mode="default_content" meta:base-class-count="6">
    
    <xsl:text>Styley</xsl:text>
  </xsl:template><xsl:template match="object:Request" mode="gs_listname" meta:base-class-count="6">
    <xsl:if test="@name"><xsl:value-of select="@name"/></xsl:if>
    <xsl:if test="not(@name)">request</xsl:if>
  </xsl:template><xsl:template match="object:Request/@url" mode="gs_list_attributes" meta:base-class-count="6">
    <li class="attribute_{local-name()}"><xsl:value-of select="name()"/> (<xsl:value-of select="."/>)</li>
  </xsl:template><xsl:template match="object:Request/@method[.='POST']" mode="gs_list_attributes" meta:base-class-count="6">
    <li class="attribute_{local-name()} attribute_value_post"><xsl:value-of select="name()"/> (<xsl:value-of select="."/>)</li>
  </xsl:template><xsl:template match="xsl:template/@match|xsl:output/@method|gs:HTTP/@type|object:Session/@id" meta:base-class-count="6">
    
    <li class="attribute_{local-name()}"><xsl:value-of select="name()"/> (<xsl:value-of select="."/>)</li>
  </xsl:template><xsl:template match="object:User|object:Person|object:Manager" mode="securityowner" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/>
    

    <div class="{$gs_html_identifier_class} details">
      <img src="{$gs_resource_server}/resources/shared/images/icons/{@tree-icon}.png"/>
      <div class="gs-name"><xsl:value-of select="@name"/></div>
      <div class="gs-security-relationship">owner</div>
    </div>
  </xsl:template><xsl:template match="object:User|object:Person|object:Manager" mode="default_content" meta:base-class-count="6">
    
    <img src="{$gs_resource_server}/resources/shared/images/avatar_blank_gray.jpg"/>

    <div class="full-name">
      <xsl:apply-templates select="." mode="full_title"/>
      <xsl:value-of select="gs:name"/>
    </div>
  </xsl:template><xsl:template match="object:User|object:Person|object:Manager" mode="full_title" meta:base-class-count="6">
    <xsl:value-of select="@name"/>
  </xsl:template><xsl:template match="object:User|object:Person|object:Manager" mode="controls" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/>
    

    <div class="{$gs_html_identifier_class}">
      <div class="gs-float-left gs-clickable gs_dropdown gs-toggle-click-exclusive-global">
        <div class="gs-menu-dropdown-image"><xsl:apply-templates select="@name"/></div>
        <ul>
          <li class="f_click_profile">profile</li>
          <li class="f_click_logout">logout</li>
        </ul>
      </div>

      
      <div class="gs-float-right">
        <div class="gs-menu-dropdown-image">notifications (0)</div>
        <xsl:apply-templates select="repository:notifications" mode="controls"/>
      </div>

      
      <xsl:apply-templates select="repository:errors" mode="controls"/>

      
      <xsl:apply-templates select="gs:debug" mode="controls"/>
    </div>
  </xsl:template><xsl:template match="object:User/repository:errors|object:Person/repository:errors|object:Manager/repository:errors" mode="controls" meta:base-class-count="6">
    <div class="gs-float-right f_mouseover_show gs-toggle-hover-exclusive-global">
      <div class="gs-menu-dropdown-image">errors (<xsl:value-of select="count(*)"/>)</div>
      <ul id="gs_errors">
        <xsl:apply-templates select="*" mode="controls"/>
      </ul>
    </div>
  </xsl:template><xsl:template match="object:User/repository:errors/gs:error|object:Person/repository:errors/gs:error|object:Manager/repository:errors/gs:error" mode="controls" meta:base-class-count="6">
    <li>error</li>
  </xsl:template><xsl:template match="gs:debug" mode="controls" meta:base-class-count="6">
    <div class="gs-float-right gs-clickable gs_dropdown gs-toggle-click-exclusive-global">
      <div class="gs-menu-dropdown-image">debug (<xsl:value-of select="count(gs:breakpoints/*)"/>)</div>
      <xsl:apply-templates select="*" mode="controls"/>
    </div>
  </xsl:template><xsl:template match="gs:debug/gs:breakpoints" mode="controls" meta:base-class-count="6">
    <ul id="gs_breakpoints">
      <xsl:apply-templates select="*" mode="controls"/>
    </ul>
  </xsl:template><xsl:template match="gs:breakpoints/*" mode="controls" meta:base-class-count="6">
    <li><xsl:value-of select="local-name()"/> (<xsl:value-of select="@xml:id"/>)</li>
  </xsl:template><xsl:template match="gs:breakpoints/*/*" meta:base-class-count="6"/><xsl:template match="gs:breakpoints/*/*" meta:base-class-count="6"/><xsl:template match="gs:breakpoints/*/*" mode="list" meta:base-class-count="6"><xsl:param name="gs_event_functions"/>
    
  </xsl:template><xsl:template match="object:Session" mode="controls" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/>
    

    <div class="{$gs_html_identifier_class} details">
      <xsl:apply-templates select="object:User" mode="gs_view_render">
        <xsl:with-param name="gs_interface_mode" select="'controls'"/>
      </xsl:apply-templates>
      <xsl:apply-templates select="xsd:schema" mode="gs_view_render"/>

      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups"/>
    </div>
  </xsl:template><xsl:template match="fake-template-source-warning" meta:base-class-count="7">
    <script>alert('if you are seeing this it is because the browser has rendered the XSL and is now processing the XSL as HTML. it will also request all the html:links in the XSL templates with variables and throw server errors.\n\ntry using view-source:url');</script>
  </xsl:template><xsl:template match="/" meta:base-class-count="7"><xsl:param name="gs_interface_mode"/>
    

    <xsl:apply-templates select="*" mode="gs_view_render">
      <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
    </xsl:apply-templates>
  </xsl:template><xsl:template match="*" meta:base-class-count="7"><xsl:param name="gs_interface_mode"/>
    

    <xsl:if test="@gs:interface-mode">
      <div class="gs-warning">
        modeless template called on [<xsl:value-of select="name()"/>]
        with @gs:interface-mode [<xsl:value-of select="@gs:interface-mode"/>]
        should have been caught by @gs:interface-mode xsl:templates
        are they not included?
      </div>
    </xsl:if>

    <xsl:if test="not(@gs:interface-mode)">
      <div class="gs-warning">
        element [<xsl:value-of select="name()"/>] called without mode.
        this means it is not inherited from DatabaseElement which has a modeless match template for everything:
          <b>xsl:template @match=*</b>
        template order may be compromised
      </div>
    </xsl:if>
  </xsl:template><xsl:template match="object:Service|object:HTTP" mode="default_content" meta:base-class-count="6">
    
    <xsl:text>Servicey</xsl:text>
  </xsl:template><xsl:template match="object:Service/@port|object:HTTP/@port" mode="gs_list_attributes" meta:base-class-count="6">
    <li class="attribute_{local-name()}"><xsl:value-of select="name()"/> (<xsl:value-of select="."/>)</li>
  </xsl:template><xsl:template match="object:Service|object:HTTP" mode="gs_context_menu_custom" meta:base-class-count="6">
    <li class="f_click_stop">stop</li>
  </xsl:template><xsl:template match="object:Group" mode="securityowner" meta:base-class-count="6"><xsl:param name="gs_html_identifier_class"/>
    

    <div class="{$gs_html_identifier_class}">
      <img src="{$gs_resource_server}/resources/shared/images/icons/{@tree-icon}.png"/>
      <div class="gs-name"><xsl:value-of select="@name"/></div>
      <div class="gs-security-relationship">group</div>
    </div>
  </xsl:template><xsl:template match="xmltransaction:*" mode="gs_context_menu_custom" meta:base-class-count="5">
    <li class="f_click_run">run</li>
  </xsl:template><xsl:template match="object:MessageInterpretation" mode="default_content" meta:base-class-count="6">
    
    <xsl:text>MessageInterpretation</xsl:text>
  </xsl:template><xsl:template match="object:MessageInterpretation" mode="gs_listname" meta:base-class-count="6">
    <xsl:value-of select="@name"/>
  </xsl:template><xsl:template match="*" mode="gs_inherited_css_classes" meta:base-class-count="4"><xsl:param name="gs_class_definitions" select="."/>
    
    

    <xsl:variable name="gs_class_definition">
      <xsl:if test="contains($gs_class_definitions, ',')"><xsl:value-of select="normalize-space(substring-before($gs_class_definitions, ','))"/></xsl:if>
      <xsl:if test="not(contains($gs_class_definitions, ','))"><xsl:value-of select="normalize-space($gs_class_definitions)"/></xsl:if>
    </xsl:variable>

    <xsl:text> CSS__</xsl:text><xsl:value-of select="$gs_class_definition"/><xsl:text> </xsl:text>
    <xsl:if test="contains($gs_class_definitions, ',')">
      <xsl:apply-templates select="." mode="gs_inherited_css_classes">
        <xsl:with-param name="gs_class_definitions" select="substring-after($gs_class_definitions, ',')"/>
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template><xsl:template match="*" mode="gs_class_definitions" meta:base-class-count="4"><xsl:param name="gs_class_definitions"/>
    
    

    <xsl:variable name="gs_class_definition">
      <xsl:if test="contains($gs_class_definitions, ',')"><xsl:value-of select="normalize-space(substring-before($gs_class_definitions, ','))"/></xsl:if>
      <xsl:if test="not(contains($gs_class_definitions, ','))"><xsl:value-of select="normalize-space($gs_class_definitions)"/></xsl:if>
    </xsl:variable>

    <xsl:text> Class__</xsl:text><xsl:value-of select="$gs_class_definition"/><xsl:text> </xsl:text>
    <xsl:if test="contains($gs_class_definitions, ',')">
      <xsl:apply-templates select="." mode="gs_class_definitions">
        <xsl:with-param name="gs_class_definitions" select="substring-after($gs_class_definitions, ',')"/>
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template><xsl:template match="*" mode="link" meta:base-class-count="2"><xsl:param name="gs_html_identifier_class"/><xsl:param name="gs_interface_mode"/>
    
    

    <li class="{$gs_html_identifier_class} gs-toggle-hover-exclusive-global">
      <a class="details" href="/{@meta:xpath-to-node-client}" target="_top">
        <xsl:choose>
          <xsl:when test="string(@meta:menu-item)"><xsl:value-of select="@meta:menu-item"/></xsl:when>
          <xsl:when test="string(@name)"><xsl:value-of select="@name"/></xsl:when>
          <xsl:otherwise><xsl:value-of select="local-name()"/></xsl:otherwise>
        </xsl:choose>
      </a>

      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups">
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      </xsl:apply-templates>

      
      <xsl:if test="*">
        <ul>
          <xsl:apply-templates select="*" mode="gs_view_render">
            <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
          </xsl:apply-templates>
        </ul>
      </xsl:if>
    </li>
  </xsl:template><xsl:template match="*" mode="gs_extra_classes" meta:base-class-count="2"/><xsl:template match="*" mode="default_content" meta:base-class-count="2"><xsl:param name="gs_interface_mode"/>
    
    

    <xsl:apply-templates select="@title|@title-1|@title-2|@title-3|@title-4|@title-5" mode="full_title"/>
    <xsl:apply-templates mode="gs_view_render"/>
  </xsl:template><xsl:template match="@title" mode="full_title" meta:base-class-count="2">
    <div class="gs-title"><xsl:value-of select="."/></div>
  </xsl:template><xsl:template match="@title-1" mode="full_title" meta:base-class-count="2">
    <h1><xsl:value-of select="."/></h1>
  </xsl:template><xsl:template match="@title-2" mode="full_title" meta:base-class-count="2">
    <h2><xsl:value-of select="."/></h2>
  </xsl:template><xsl:template match="@title-3" mode="full_title" meta:base-class-count="2">
    <h3><xsl:value-of select="."/></h3>
  </xsl:template><xsl:template match="@title-4" mode="full_title" meta:base-class-count="2">
    <h4><xsl:value-of select="."/></h4>
  </xsl:template><xsl:template match="@title-5" mode="full_title" meta:base-class-count="2">
    <h5><xsl:value-of select="."/></h5>
  </xsl:template><xsl:template match="*" mode="gs_html_identifier_class" meta:base-class-count="2"><xsl:param name="gs_interface_mode" select="@gs:interface-mode"/><xsl:param name="gs_event_functions" select="@meta:event-functions"/><xsl:param name="gs_display_class" select="@gs:display-class"/><xsl:param name="gs_no_xml_id" select="false()"/>
    
    
    
    
     

    
    <xsl:variable name="gs_classes_fragment"><xsl:apply-templates select="." mode="gs_classes_string"/></xsl:variable>
    <xsl:variable name="gs_classes" select="string($gs_classes_fragment)"/>
    <xsl:text>Object </xsl:text>
    <xsl:if test="$gs_classes">
      <xsl:variable name="gs_base_classes_fragment"><xsl:apply-templates select="." mode="gs_base_classes_string"/></xsl:variable>
      <xsl:variable name="gs_base_classes" select="string($gs_base_classes_fragment)"/>
      <xsl:apply-templates select="." mode="gs_class_definitions">
        <xsl:with-param name="gs_class_definitions" select="$gs_classes"/>
      </xsl:apply-templates>
      <xsl:apply-templates select="." mode="gs_inherited_css_classes">
        <xsl:with-param name="gs_class_definitions" select="$gs_classes"/>
      </xsl:apply-templates>
      <xsl:apply-templates select="." mode="gs_inherited_css_classes">
        <xsl:with-param name="gs_class_definitions" select="$gs_base_classes"/>
      </xsl:apply-templates>
    </xsl:if>
    <xsl:if test="not($gs_classes)">
      <xsl:text>Class__DatabaseElement</xsl:text>
    </xsl:if>

    
    <xsl:text> gs-interface-mode-</xsl:text>
    <xsl:value-of select="translate($gs_interface_mode, '.', '-')"/>
    <xsl:if test="not($gs_interface_mode)">default</xsl:if>
    <xsl:if test="contains($gs_interface_mode, '-')"> gs-interface-mode-<xsl:value-of select="substring-before($gs_interface_mode, '-')"/></xsl:if>

    <xsl:variable name="gs_namespace_prefix">
      <xsl:if test="@meta:fully-qualified-prefix"><xsl:value-of select="@meta:fully-qualified-prefix"/></xsl:if>
      <xsl:if test="not(@meta:fully-qualified-prefix)"><xsl:value-of select="substring-before(name(), ':')"/></xsl:if>
    </xsl:variable>
    <xsl:variable name="gs_has_children" select="not(@meta:child-count = 0)"/>

    
    <xsl:if test="$gs_display_class"><xsl:text> </xsl:text><xsl:value-of select="$gs_display_class"/></xsl:if>
    <xsl:if test="$gs_event_functions"><xsl:text> </xsl:text><xsl:value-of select="$gs_event_functions"/></xsl:if>
    <xsl:if test="not($gs_interface_mode)">
      <xsl:text> </xsl:text><xsl:apply-templates select="." mode="gs_event_functions"/>
    </xsl:if>

    
    <xsl:text> gs-name-</xsl:text>
    <xsl:value-of select="translate(translate(local-name(), $gs_uppercase, $gs_lowercase), '_', '-')"/>
    <xsl:if test="string(@name) and not(@name = local-name())">
      <xsl:text>-</xsl:text>
      <xsl:value-of select="translate(translate(@name, $gs_uppercase, $gs_lowercase), ' _:', '---')"/>
      <xsl:text> </xsl:text>
      <xsl:value-of select="translate(@name, ' _:', '---')"/>
    </xsl:if>
    <xsl:if test="@meta:is-hard-linked = 'true'"> gs-is-hard-linked</xsl:if>
    <xsl:if test="@meta:is-hard-link = 'true'"> gs-is-hard-link</xsl:if>
    <xsl:if test="@meta:is-registered = 'true'"> gs-is-registered</xsl:if>
    <xsl:if test="$gs_has_children"> gs-children-exist</xsl:if>
    <xsl:if test="not($gs_has_children)"> gs-children-none</xsl:if>
    <xsl:text> gs-local-name-</xsl:text><xsl:value-of select="local-name()"/>
    <xsl:if test="$gs_namespace_prefix"><xsl:text> namespace_</xsl:text><xsl:value-of select="$gs_namespace_prefix"/></xsl:if>
    <xsl:if test="not($gs_namespace_prefix)"><xsl:text> gs-no-namespace</xsl:text></xsl:if>
    <xsl:if test="@xml:id and not($gs_no_xml_id)">
      <xsl:text> gs-has-xml-id</xsl:text>
      <xsl:text> gs-xml-id-</xsl:text><xsl:value-of select="@xml:id"/>
    </xsl:if>
    
    
    <xsl:if test="@html:draggable"> gs-draggable</xsl:if>

    <xsl:text> </xsl:text>
    <xsl:apply-templates select="." mode="gs_extra_classes"/>
  </xsl:template><xsl:template match="*" mode="gs_event_functions" meta:base-class-count="2"/><xsl:template match="*" mode="gs_meta_data_standard_groups" meta:base-class-count="2"><xsl:param name="gs_interface_mode" select="'default'"/>
    

    
    <xsl:apply-templates select="." mode="gs_meta_data">
      <xsl:with-param name="gs_namespace_uri" select="'http://general_server.org/xmlnamespaces/general_server/2006'"/>
      <xsl:with-param name="gs_group_name" select="'gs-data'"/>
      <xsl:with-param name="gs_display_specific" select="true()"/>
      <xsl:with-param name="gs_js_load_as" select="'properties'"/>
    </xsl:apply-templates>

    
    <xsl:if test="$gs_interface_mode = 'default'">
      <xsl:apply-templates select="." mode="gs_meta_data">
        <xsl:with-param name="gs_namespace_uri" select="'http://www.w3.org/1999/xhtml'"/>
        <xsl:with-param name="gs_group_name" select="'html-jquery'"/>
        <xsl:with-param name="gs_display_specific" select="true()"/>
        <xsl:with-param name="gs_js_load_as" select="'function-calls'"/>
      </xsl:apply-templates>
    </xsl:if>

    
    <xsl:apply-templates select="." mode="gs_meta_data">
      <xsl:with-param name="gs_namespace_uri" select="'http://general_server.org/xmlnamespaces/meta/2006'"/>
      <xsl:with-param name="gs_group_name" select="'meta-data'"/>
      <xsl:with-param name="gs_display_specific" select="false()"/>
      <xsl:with-param name="gs_js_load_as" select="'properties'"/>
    </xsl:apply-templates>

    
    <xsl:apply-templates select="." mode="gs_meta_data">
      <xsl:with-param name="gs_namespace_uri" select="'http://www.w3.org/XML/1998/namespace'"/>
      <xsl:with-param name="gs_group_name" select="'meta-data'"/>
      <xsl:with-param name="gs_display_specific" select="false()"/>
      <xsl:with-param name="gs_js_load_as" select="'properties'"/>
      <xsl:with-param name="gs_include_namespace" select="true()"/>
    </xsl:apply-templates>

    
    <xsl:apply-templates select="." mode="gs_meta_data">
      <xsl:with-param name="gs_namespace_uri" select="'http://www.w3.org/1999/XSL/Transform'"/>
      <xsl:with-param name="gs_group_name" select="'meta-data'"/>
      <xsl:with-param name="gs_display_specific" select="false()"/>
      <xsl:with-param name="gs_js_load_as" select="'properties'"/>
      <xsl:with-param name="gs_include_namespace" select="true()"/>
    </xsl:apply-templates>

    
    <xsl:apply-templates select="." mode="gs_meta_data">
      
      <xsl:with-param name="gs_group_name" select="'data'"/>
      <xsl:with-param name="gs_display_specific" select="false()"/>
      <xsl:with-param name="gs_js_load_as" select="'properties'"/>
    </xsl:apply-templates>
  </xsl:template><xsl:template match="*" mode="gs_meta_data" meta:base-class-count="2"><xsl:param name="gs_namespace_uri"/><xsl:param name="gs_meta_data" select="@*[namespace-uri() = $gs_namespace_uri]"/><xsl:param name="gs_group_name"/><xsl:param name="gs_js_load_as" select="'Options'"/><xsl:param name="gs_display_specific" select="false()"/><xsl:param name="gs_include_namespace" select="false()"/>
      
    
      
    
      
    
    
    

    
    <xsl:if test="$gs_meta_data">
      <xsl:variable name="gs_prefix" select="substring-before(name($gs_meta_data), ':')"/>

      
      <div style="display:none;">
        <xsl:attribute name="class">
          <xsl:text>gs-meta-data</xsl:text>
          <xsl:if test="$gs_prefix"> gs-namespace-prefix-<xsl:value-of select="$gs_prefix"/></xsl:if>
          <xsl:if test="not($gs_prefix)"> gs-namespace-no-prefix</xsl:if>
          <xsl:if test="$gs_group_name"> gs-group-<xsl:value-of select="$gs_group_name"/></xsl:if>
          <xsl:if test="$gs_js_load_as"> gs-load-as-<xsl:value-of select="$gs_js_load_as"/></xsl:if>
          <xsl:if test="$gs_display_specific"> gs-display-specific</xsl:if>
          <xsl:if test="$gs_include_namespace"> gs-include-namespace</xsl:if>
        </xsl:attribute>

        <xsl:apply-templates select="$gs_meta_data" mode="gs_xtransport_values"/>
      </div>
    </xsl:if>
  </xsl:template><xsl:template match="*|@*" mode="gs_xtransport_values" meta:base-class-count="2">
    
    <xsl:if test="string(.)"><div class="{local-name()}"><xsl:value-of select="."/></div></xsl:if>
  </xsl:template><xsl:template match="text()" mode="gs_xtransport_values" meta:base-class-count="2">
    
    <xsl:if test="string(.)"><div class="gs-text"><xsl:value-of select="."/></div></xsl:if>
  </xsl:template><xsl:template match="*" mode="list" meta:base-class-count="2"><xsl:param name="gs_html_identifier_class"/><xsl:param name="gs_interface_mode"/><xsl:param name="gs_sub_interface"/><xsl:param name="gs_event_functions"/>
    
    
    
     
    
    

    
    <xsl:variable name="gs_child_count" select="number(@meta:child-count)"/>
    <xsl:variable name="gs_children_retrieved" select="count(interface:Collection/*)"/>
    <xsl:variable name="gs_children_retrieved_class">
      <xsl:choose>
        <xsl:when test="not($gs_child_count)">gs-children-retrieved-none-exist</xsl:when>
        <xsl:when test="not($gs_children_retrieved)">gs-children-retrieved-none</xsl:when>
        <xsl:when test="$gs_child_count &gt; $gs_children_retrieved">gs-children-retrieved-patial</xsl:when>
        <xsl:otherwise>gs-children-retrieved-all</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    
    <li class="{$gs_html_identifier_class} {$gs_event_functions} {$gs_children_retrieved_class} f_click_select_children">
      <xsl:apply-templates select="." mode="gs_list_childControl">
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
        <xsl:with-param name="gs_children_retrieved_class" select="$gs_children_retrieved_class"/>
      </xsl:apply-templates>
      <xsl:apply-templates select="." mode="gs_list_details">
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
        <xsl:with-param name="gs_children_retrieved_class" select="$gs_children_retrieved_class"/>
      </xsl:apply-templates>
      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups">
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
        <xsl:with-param name="gs_children_retrieved_class" select="$gs_children_retrieved_class"/>
      </xsl:apply-templates>

      
      <xsl:apply-templates select="*" mode="gs_view_render">
        
      </xsl:apply-templates>
    </li>
  </xsl:template><xsl:template match="*" mode="gs_list_childControl" meta:base-class-count="2"><xsl:param name="gs_interface_mode"/><xsl:param name="gs_children_retrieved_class"/>
    
    
    
    <div>
      <xsl:attribute name="class">
        <xsl:text>gs-list-child-control gs-toggle gs-clickable f_click_toggleCSSChildClassDOFromEventDisplayObject_children</xsl:text>
        
        <xsl:if test="$gs_children_retrieved_class = 'gs-children-retrieved-all'"> expanded</xsl:if>
      </xsl:attribute>
      <xsl:text> </xsl:text>
    </div>
  </xsl:template><xsl:template match="*" mode="gs_list_details" meta:base-class-count="2">
    
    <xsl:variable name="gs_namespace_prefix">
      <xsl:if test="@meta:fully-qualified-prefix"><xsl:value-of select="@meta:fully-qualified-prefix"/></xsl:if>
      <xsl:if test="not(@meta:fully-qualified-prefix)"><xsl:value-of select="substring-before(name(), ':')"/></xsl:if>
    </xsl:variable>

    <div class="details gs-clickable">
      <xsl:attribute name="style">
        <xsl:apply-templates select="@*" mode="gs_list_style"/>
      </xsl:attribute>

      <xsl:apply-templates select="." mode="gs_list_pre_additions"/>

      <xsl:if test="$gs_namespace_prefix">
        <span class="namespace_prefix"><xsl:value-of select="$gs_namespace_prefix"/><xsl:text>:</xsl:text></span>
      </xsl:if>
      <span class="name"><xsl:apply-templates select="." mode="gs_listname"/></span>
      <span class="child-count">
        <xsl:text>(</xsl:text>
        <xsl:value-of select="@meta:child-count"/>
        <xsl:text>)</xsl:text>
      </span>

      <span class="indicators">
        <xsl:if test="@meta:is-hard-link = 'true'"><img class="indicator-is-hard-link" src="{$gs_resource_server}/resources/shared/images/spacer.png"/></xsl:if>
        <xsl:if test="@meta:is-registered = 'true'"><img class="indicator-is-registered" src="{$gs_resource_server}/resources/shared/images/spacer.png"/></xsl:if>
        <xsl:if test="@gs:transient-area"><img class="indicator-is-transient-area" src="{$gs_resource_server}/resources/shared/images/spacer.png"/></xsl:if>
        <xsl:if test="@xml:id-policy-area"><img class="indicator-id-policy-area" src="{$gs_resource_server}/resources/shared/images/spacer.png"/></xsl:if>
      </span>

      
      <div class="gs-xtransport xpath"><xsl:value-of select="name()"/></div>
      <ul class="attributes"><xsl:apply-templates select="@*" mode="gs_list_attributes"/></ul>

      <xsl:apply-templates select="." mode="gs_list_post_additions"/>
    </div>
  </xsl:template><xsl:template match="*" mode="gs_list_pre_additions" meta:base-class-count="2"/><xsl:template match="*" mode="gs_list_post_additions" meta:base-class-count="2"/><xsl:template match="@*" mode="gs_list_style" meta:base-class-count="2"/><xsl:template match="@tree-icon" mode="gs_list_style" meta:base-class-count="2">
    <xsl:text>background-image:url(</xsl:text>
    <xsl:value-of select="$gs_resource_server"/>
    <xsl:text>/resources/shared/images/icons/</xsl:text>
    <xsl:value-of select="."/>
    <xsl:text>.png);</xsl:text>
  </xsl:template><xsl:template match="*" mode="listname" meta:base-class-count="2"><xsl:param name="gs_html_identifier_class"/><xsl:param name="gs_interface_mode"/>
    
    
    
    <span class="{$gs_html_identifier_class}">
      <xsl:apply-templates select="." mode="gs_listname"/>
      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups">
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      </xsl:apply-templates>
    </span>
  </xsl:template><xsl:template match="*" mode="gs_listname" meta:base-class-count="2">
    <xsl:choose>
      <xsl:when test="@meta:list-name"><xsl:apply-templates select="@meta:list-name" mode="gs_field_attribute"/></xsl:when>
      <xsl:when test="@name"><xsl:apply-templates select="@name" mode="gs_field_attribute"/></xsl:when>
      <xsl:otherwise>
        <xsl:apply-templates select="." mode="gs_field_localname"/>
        <xsl:text> </xsl:text>
        <xsl:apply-templates select="@name" mode="gs_field_attribute"/>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template><xsl:template match="@*" mode="gs_list_attributes" meta:base-class-count="2"/><xsl:template match="*" mode="securitypermissions" meta:base-class-count="2"><xsl:param name="gs_html_identifier_class"/>
    
    

    
    <xsl:variable name="gs_xmlsecurity_permissons">
      <xsl:if test="@xmlsecurity:permissions and string-length(string(@xmlsecurity:permissions)) = 3"><xsl:value-of select="@xmlsecurity:permissions"/></xsl:if>
      <xsl:if test="not(@xmlsecurity:permissions and string-length(string(@xmlsecurity:permissions)) = 3)">777</xsl:if>
    </xsl:variable>

    <div class="{$gs_html_identifier_class}">
      <div class="gs-securitypermissions-number">
        [<xsl:value-of select="$gs_xmlsecurity_permissons"/>]
        <xsl:if test="@xmlsecurity:permissions"> /
          [<xsl:value-of select="@xmlsecurity:permissions"/>]
        </xsl:if>
        <xsl:if test="not(@xmlsecurity:permissions)">(defaulted)</xsl:if>
        <xsl:if test="@xmlsecurity:permissions and not(string-length(string(@xmlsecurity:permissions)) = 3)">(invalid)</xsl:if>
      </div>

      <table>
        <tr><th>context</th><th>read</th><th>write</th><th>execute</th></tr>

        <tr><td class="gs-context">owner</td>
          <xsl:apply-templates select="." mode="gs_security_unix_permission"><xsl:with-param name="digit" select="substring($gs_xmlsecurity_permissons, 1, 1)"/></xsl:apply-templates>
        </tr>
        <tr><td class="gs-context">group</td>
          <xsl:apply-templates select="." mode="gs_security_unix_permission"><xsl:with-param name="digit" select="substring($gs_xmlsecurity_permissons, 2, 1)"/></xsl:apply-templates>
        </tr>
        <tr><td class="gs-context">other</td>
          <xsl:apply-templates select="." mode="gs_security_unix_permission"><xsl:with-param name="digit" select="substring($gs_xmlsecurity_permissons, 3, 1)"/></xsl:apply-templates>
        </tr>
      </table>
    </div>
  </xsl:template><xsl:template match="*" mode="gs_security_unix_permission" meta:base-class-count="2"><xsl:param name="digit"/>
    

    <td><input class="gs-read" name="read" type="checkbox">
      <xsl:if test="floor($digit div 4) mod 2 = 1"><xsl:attribute name="checked">1</xsl:attribute></xsl:if>
    </input></td>
    <td><input class="gs-write" name="write" type="checkbox">
      <xsl:if test="floor($digit div 2) mod 2 = 1"><xsl:attribute name="checked">1</xsl:attribute></xsl:if>
    </input></td>
    <td><input class="gs-execute" name="execute" type="checkbox">
      <xsl:if test="floor($digit div 1) mod 2 = 1"><xsl:attribute name="checked">1</xsl:attribute></xsl:if>
    </input></td>
  </xsl:template><xsl:template match="*" mode="environment" meta:base-class-count="2"><xsl:param name="gs_html_identifier_class"/><xsl:param name="gs_interface_mode"/>
    
    
    

    
    <script class="{$gs_html_identifier_class}">
      <xsl:text>/* GSProperties: </xsl:text>
      <properties>
        <xsl:apply-templates select="." mode="gs_meta_data_standard_groups">
          <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
        </xsl:apply-templates>
      </properties>
      <xsl:text> */</xsl:text>
    </script>
  </xsl:template><xsl:template match="@*" mode="gs_field_name_attribute" meta:base-class-count="2">
    <span class="Field gs-field-name-attribute">
      <xsl:apply-templates select="."/>
    </span>
  </xsl:template><xsl:template match="*" mode="gs_field_localname" meta:base-class-count="2">
    <span class="Field gs-field-localname">
      <xsl:value-of select="translate(local-name(), '_', ' ')"/>
    </span>
  </xsl:template><xsl:template match="@*" mode="gs_field_attribute" meta:base-class-count="2">
    <span class="Field gs-field-attribute gs-name-{local-name()}">
      <xsl:apply-templates select="."/>
    </span>
  </xsl:template><xsl:template match="@*" mode="gs_field_element" meta:base-class-count="2">
    <span class="Field gs-field-element gs-name-{local-name()}">
      <xsl:apply-templates select="."/>
    </span>
  </xsl:template><xsl:template match="*|@*" mode="gs_enforce_non_self_closing" meta:base-class-count="5">
    
    <xsl:text>window.gs_enforce_non_self_closing</xsl:text>
  </xsl:template><xsl:template match="html:*" meta:base-class-count="5">
    
    <xsl:copy>
      
      <xsl:copy-of select="@*[not(self::class)]"/>
      <xsl:attribute name="class">
        <xsl:value-of select="@class"/>
        <xsl:if test="position() = 1"> gs-first</xsl:if>
        <xsl:if test="position() = last()"> gs-last</xsl:if>
      </xsl:attribute>

      <xsl:apply-templates mode="gs_view_render"/>
    </xsl:copy>
  </xsl:template><xsl:template match="*" mode="gs_client_side_xsl_warnings" meta:base-class-count="5">
    <xsl:apply-templates select="*" mode="gs_client_side_xsl_warnings"/>
  </xsl:template><xsl:template match="html:*" mode="gs_client_side_xsl_warnings" meta:base-class-count="5">
    <xsl:apply-templates select="@*[contains(.,'{')]" mode="gs_client_side_xsl_warnings"/>
  </xsl:template><xsl:template match="@*" mode="gs_client_side_xsl_warnings" meta:base-class-count="5">
    <xsl:if test="contains(., 'database:') or contains(., 'str:')">
      <div class="gs-warning">
	attribute dynamic value @<xsl:value-of select="name()"/>=<xsl:value-of select="."/> is not advised
	it will cause FireFox to critically fail and Chrome will silently not compile it
      </div>
    </xsl:if>
  </xsl:template><xsl:template match="xsl:value-of" mode="gs_client_side_xsl_warnings" meta:base-class-count="5">
    <xsl:if test="contains(@select, 'database:') or contains(@select, 'str:')">
      <div class="gs-warning">
	value-of &lt;xsl:value-of select="<xsl:value-of select="@select"/>" /&gt; is not advised
	it will cause FireFox to critically fail and Chrome will silently not compile it
      </div>
    </xsl:if>
  </xsl:template><xsl:template match="meta:*" meta:base-class-count="5">
    
  </xsl:template><xsl:template match="*" meta:base-class-count="2"><xsl:param name="gs_html_identifier_class"/><xsl:param name="gs_interface_mode"/>
    
    
    

    <xsl:variable name="gs_container_tag_fragment">
      <xsl:apply-templates select="." mode="gs_html_container"/>
    </xsl:variable>
    <xsl:variable name="gs_container_tag_defaulted_fragment">
      <xsl:value-of select="$gs_container_tag_fragment"/>
      <xsl:if test="not(string($gs_container_tag_fragment))">div</xsl:if>
    </xsl:variable>
    <xsl:variable name="gs_container_tag" select="string($gs_container_tag_defaulted_fragment)"/>

    <xsl:element name="{$gs_container_tag}">
      <xsl:attribute name="class"><xsl:value-of select="$gs_html_identifier_class"/></xsl:attribute>

      <xsl:apply-templates select="." mode="default_content">
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      </xsl:apply-templates>

      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups">
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      </xsl:apply-templates>
    </xsl:element>
  </xsl:template><xsl:template match="xsd:attribute[@meta:editor-class = 'class:Selector']" meta:base-class-count="7"><xsl:param name="gs_html_identifier_class"/><xsl:param name="gs_current_external_value_node" select="FORCE_SELECT_EMPTY_NODESET"/>
    
    
    

    <xsl:variable name="gs_current_value" select="$gs_current_external_value_node/@*[name() = @name]"/>

    <div class="{$gs_html_identifier_class}">
      <input name="{@name}" value="{$gs_current_value}"/>
      
      <database:query class="Object Class__AJAXHTMLLoader" is-client-ondemand="true" data-context-id="{@meta:selector-options-id}" data-context="{@meta:selector-options}" data="."/>
    </div>
  </xsl:template><xsl:template match="xsd:attribute[@meta:editor-class = 'class:CodeMirrorEditor']" meta:base-class-count="7"><xsl:param name="gs_html_identifier_class"/><xsl:param name="gs_current_external_value_node" select="FORCE_SELECT_EMPTY_NODESET"/>
    
    
    

    <div class="{$gs_html_identifier_class} f_presubmit_updateFormElement f_appear_refresh f_resize_refresh">
      <textarea name="xml">
        <xsl:copy-of select="$gs_current_external_value_node"/>
      </textarea>

      <xsl:apply-templates select="." mode="gs_meta_data">
        <xsl:with-param name="gs_namespace_uri" select="'http://general_server.org/xmlnamespaces/general_server/2006'"/>
      </xsl:apply-templates>
    </div>
  </xsl:template><!--/[/~Class/website_classes_html_view_templates.dxsl]--><!--[/~Class/website_classes_html_dependencies.dxsl] DXSL--><!--
      ######## gs_external_class_css_dependencies and gs_external_class_javascript_dependencies #########
      $gs_website_classes               [99]
      type:             [complement]
    --><xsl:template match="@*|node()" mode="gs_external_class_javascript_dependencies"><xsl:comment>gs_external_class_javascript_dependencies</xsl:comment><script type="text/javascript" class="gs_external_class_javascript_dependencies details" xml:add-xml-id="no" xsl:is-literal="1" src="http://general-resources-server.localhost/resources/shared/jquery/plugins/splitter/js/jquery.splitter-0.14.0.js" gs:object-name="Explorer">window.gs_enforce_non_self_closing</script><script type="text/javascript" class="gs_external_class_javascript_dependencies details" xml:add-xml-id="no" xsl:is-literal="1" src="http://general-resources-server.localhost/resources/shared/codemirror/codemirror-3.20/lib/codemirror.js" gs:object-name="CodeMirrorEditor">window.gs_enforce_non_self_closing</script><script type="text/javascript" class="gs_external_class_javascript_dependencies details" xml:add-xml-id="no" xsl:is-literal="1" src="http://general-resources-server.localhost/resources/shared/codemirror/codemirror-3.20/addon/fold/foldcode.js" gs:object-name="CodeMirrorEditor">window.gs_enforce_non_self_closing</script><script type="text/javascript" class="gs_external_class_javascript_dependencies details" xml:add-xml-id="no" xsl:is-literal="1" src="http://general-resources-server.localhost/resources/shared/codemirror/codemirror-3.20/mode/javascript/javascript.js" gs:object-name="CodeMirrorEditor">window.gs_enforce_non_self_closing</script><script type="text/javascript" class="gs_external_class_javascript_dependencies details" xml:add-xml-id="no" xsl:is-literal="1" src="http://general-resources-server.localhost/resources/shared/codemirror/codemirror-3.20/mode/xml/xml.js" gs:object-name="CodeMirrorEditor">window.gs_enforce_non_self_closing</script><script type="text/javascript" class="gs_external_class_javascript_dependencies details" xml:add-xml-id="no" xsl:is-literal="1" src="http://general-resources-server.localhost/resources/shared/codemirror/gs-xml.js" gs:object-name="CodeMirrorEditor">window.gs_enforce_non_self_closing</script><script type="text/javascript" class="gs_external_class_javascript_dependencies details" xml:add-xml-id="no" xsl:is-literal="1" src="http://general-resources-server.localhost/resources/shared/codemirror/codemirror-3.1/addon/format/formatting.js" gs:object-name="CodeMirrorEditor">window.gs_enforce_non_self_closing</script></xsl:template><xsl:template match="@*|node()" mode="gs_external_class_css_dependencies"><xsl:comment>gs_external_class_css_dependencies</xsl:comment><link rel="stylesheet" xml:add-xml-id="no" xsl:is-literal="1" href="http://general-resources-server.localhost/resources/shared/jquery/plugins/splitter/css/jquery.splitter.css" gs:object-name=""/><link rel="stylesheet" xml:add-xml-id="no" xsl:is-literal="1" href="http://general-resources-server.localhost/resources/shared/codemirror/codemirror-3.20/lib/codemirror.css" gs:object-name=""/></xsl:template><!--/[/~Class/website_classes_html_dependencies.dxsl]--></xsl:stylesheet>
