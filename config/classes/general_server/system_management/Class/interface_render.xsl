<xsl:stylesheet xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:conversation="http://general_server.org/xmlnamespaces/conversation/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:exsl="http://exslt.org/common" xmlns:class="http://general_server.org/xmlnamespaces/class/2006" xmlns:user="http://general_server.org/xmlnamespaces/user/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" response:server-side-only="true" name="interface_render" version="1.0" extension-element-prefixes="dyn str regexp database server conversation request debug exsl">
  <xsl:include xpath="/object:Server[1]/repository:system_transforms[1]/interface_render"/>

  <xsl:template match="*" mode="contextmenu_news">
    <xsl:variable name="gs_class_definitions" select="database:classes()"/>
    <xsl:variable name="gs_base_classes" select="database:base-classes-all()"/>
    <xsl:variable name="gs_classes_all" select="$gs_class_definitions|$gs_base_classes"/>

    <interface:SubMenu>
      <xsl:attribute name="title">
        <xsl:text>to </xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:if test="not(@name)"><xsl:value-of select="local-name()"/></xsl:if>
      </xsl:attribute>
      
      <xsl:apply-templates select="$gs_class_definitions" mode="gs_context_menu_news_template_list">
        <xsl:with-param name="gs_object" select="."/>
      </xsl:apply-templates>
      
      <xsl:if test="@meta:classes-context-menu-news">
        <!-- we need to find the xsd:element @type=* field so that we can render it properly
          and, of course, to check that this anything request is actually valid XSD
        -->
        <xsl:variable name="gs_xsd_anything_schema" select="$gs_classes_all/xsd:schema[not(@name)]"/>
        <xsl:variable name="gs_xsd_anything_field" select="$gs_xsd_anything_schema/xsd:complexType/xsd:sequence/xsd:any[1]"/>

        <xsl:if test="$gs_xsd_anything_field">
          <xsd:schema gs:interface-mode="new" meta:xpath-to-node="{conversation:xpath-to-node($gs_xsd_anything_schema)}">
            <xsl:attribute name="xml:id"><xsl:value-of select="$gs_xsd_anything_schema/@xml:id"/></xsl:attribute>
            <xsl:apply-templates select="@meta:classes-context-menu-news" mode="gs_context_menu_news_node_list">
              <xsl:with-param name="gs_context_menu_news" select="@meta:classes-context-menu-news"/>
              <xsl:with-param name="gs_object" select="."/>
              <xsl:with-param name="gs_xsd_anything_field" select="$gs_xsd_anything_field"/>
            </xsl:apply-templates>
          </xsd:schema>
        </xsl:if>
        <xsl:if test="not($gs_xsd_anything_field)">
          <debug:server-message output="[{name(.)}/{@xml:id}] does not have the required xsd:any field for flexible additions" type="warning"/>
          <div class="gs-warning">Class does not have required xsd:any field</div>
        </xsl:if>
      </xsl:if>
    </interface:SubMenu>
  </xsl:template>

  <xsl:template match="*" mode="contextmenu_sets">
    <xsl:variable name="gs_class_definitions" select="database:classes()"/>

    <interface:SubMenu>
      <xsl:attribute name="title">
        <xsl:text>on </xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:if test="not(@name)"><xsl:value-of select="local-name()"/></xsl:if>
      </xsl:attribute>

      <xsl:apply-templates select="$gs_class_definitions" mode="gs_context_menu_sets_template_list">
        <xsl:with-param name="gs_object" select="."/>
      </xsl:apply-templates>
    </interface:SubMenu>
  </xsl:template>

  <xsl:template match="*" mode="contextmenu_views">
    <xsl:variable name="gs_class_definitions" select="database:classes()"/>
    <xsl:variable name="gs_base_classes" select="database:base-classes-all()"/>
    <xsl:variable name="gs_classes_all" select="$gs_class_definitions|$gs_base_classes"/>

    <xsl:if test="$gs_class_definitions/@elements">
      <xsl:variable name="gs_views_element_count" select="count($gs_classes_all/view/xsl:template[@match = ../parent::class:*/@elements and str:not(@response:server-side-only) and str:not(@meta:javascript-template)])"/>
      <xsl:if test="$gs_views_element_count">
        <interface:SubMenu>
          <xsl:attribute name="title">
            <xsl:text>for </xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:if test="not(@name)"><xsl:value-of select="local-name()"/></xsl:if>
          </xsl:attribute>
          
          <xsl:apply-templates select="$gs_class_definitions" mode="gs_context_menu_views_template_list">
            <xsl:with-param name="gs_object" select="."/>
          </xsl:apply-templates>
        </interface:SubMenu>
      </xsl:if>
    </xsl:if>
  </xsl:template>

  <xsl:template match="*" mode="contextmenu_controllers">
    <xsl:variable name="gs_class_definitions" select="database:classes()"/>
    <xsl:variable name="gs_base_classes" select="database:base-classes-all()"/>
    <xsl:variable name="gs_classes_all" select="$gs_class_definitions|$gs_base_classes"/>

    <xsl:if test="$gs_class_definitions/@elements">
      <xsl:variable name="gs_controllers_element_count" select="count($gs_classes_all/xsl:stylesheet[str:boolean(@response:server-side-only)]/xsl:template|$gs_classes_all/controller/xsl:template|$gs_classes_all/xsl:stylesheet/xsl:template[str:boolean(@response:server-side-only)]|$gs_classes_all/xsl:stylesheet/xsl:template[str:boolean(@meta:javascript-template)])"/>
      <xsl:if test="$gs_controllers_element_count">
        <interface:SubMenu>
          <xsl:attribute name="title">
            <xsl:text>of </xsl:text>
            <xsl:value-of select="@name"/>
            <xsl:if test="not(@name)"><xsl:value-of select="local-name()"/></xsl:if>
          </xsl:attribute>
          
          <xsl:apply-templates select="$gs_class_definitions" mode="gs_context_menu_controllers_template_list">
            <xsl:with-param name="gs_object" select="."/>
          </xsl:apply-templates>
        </interface:SubMenu>
      </xsl:if>
    </xsl:if>
  </xsl:template>

  <xsl:template match="*" mode="contextmenu_event_handlers">
    <xsl:variable name="gs_class_definitions" select="database:classes()"/>

    <interface:SubMenu>
      <xsl:attribute name="title">
        <xsl:text>on </xsl:text>
        <xsl:value-of select="@name"/>
        <xsl:if test="not(@name)"><xsl:value-of select="local-name()"/></xsl:if>
      </xsl:attribute>
      
      <xsl:apply-templates select="$gs_class_definitions" mode="gs_context_menu_event_handlers_template_list">
        <xsl:with-param name="gs_object" select="."/>
      </xsl:apply-templates>
    </interface:SubMenu>
  </xsl:template>

  <!-- ############################################ add / new ############################################ -->
  <xsl:template match="@meta:classes-context-menu-news" mode="gs_context_menu_news_node_list">
    <!-- also: <element> @meta:classes-context-menu-news => menu-items -->
    <xsl:param name="gs_object" select="FORCE_SELECT_EMPTY_NODESET"/>
    <xsl:param name="gs_context_menu_news"/>
    <xsl:param name="gs_xsd_anything_field"/>

    <xsl:if test="$gs_context_menu_news">
      <xsl:variable name="gs_type_full">
        <xsl:if test="contains($gs_context_menu_news, ' ')"><xsl:value-of select="substring-before($gs_context_menu_news, ' ')"/></xsl:if>
        <xsl:if test="not(contains($gs_context_menu_news, ' '))"><xsl:value-of select="$gs_context_menu_news"/></xsl:if>
      </xsl:variable>
      <!-- TODO: need to not use id() here -->
      <xsl:variable name="gs_type_class" select="id(concat('Class__', $gs_type_full))"/>

      <xsl:if test="$gs_type_class">
        <xsl:apply-templates select="$gs_xsd_anything_field" mode="gs_context_menu_news_template_list">
          <xsl:with-param name="gs_object" select="$gs_object"/>
          <xsl:with-param name="gs_type_class" select="$gs_type_class"/>
        </xsl:apply-templates>
      </xsl:if>
      <xsl:else>
        <div class="gs-warning">~[<xsl:value-of select="$gs_type_full"/>] class not found</div>
      </xsl:else>

      <xsl:apply-templates select="." mode="gs_context_menu_news_node_list">
        <xsl:with-param name="gs_object" select="$gs_object"/>
        <xsl:with-param name="gs_context_menu_news" select="substring-after($gs_context_menu_news, ' ')"/>
        <xsl:with-param name="gs_xsd_anything_field" select="$gs_xsd_anything_field"/>
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>

  <xsl:template match="class:*" mode="gs_context_menu_news_template_list">
    <xsl:param name="gs_object" select="FORCE_SELECT_EMPTY_NODESET"/>

    <!-- inherited classes -->
    <xsl:apply-templates select="class:*" mode="gs_context_menu_news_template_list">
      <xsl:with-param name="gs_object" select="$gs_object"/>
    </xsl:apply-templates>

    <!-- this class schema class:* -->
    <xsl:apply-templates select="xsd:schema[not(@name)][1]" mode="gs_context_menu_news_template_list">
      <xsl:with-param name="gs_object" select="$gs_object"/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="xsd:schema" mode="gs_context_menu_news_template_list">
    <!-- original object included so we can analyse how many of each xsd:element are already present -->
    <xsl:param name="gs_object" select="FORCE_SELECT_EMPTY_NODESET"/>

    <!-- TODO: which fields? -->
    <xsl:variable name="gs_fields_multiple" select="xsd:complexType/xsd:sequence/xsd:element[@maxOccurs &gt; 1 or @maxOccurs = 'unbounded']"/>
    <xsl:variable name="gs_fields" select="$gs_fields_multiple"/>

    <xsl:copy>
      <xsl:apply-templates select="." mode="gs_interface_render_adorn_node">
        <xsl:with-param name="gs_interface_mode" select="'new'"/>
      </xsl:apply-templates>
      <xsl:apply-templates select="@*" mode="gs_interface_render"/>

      <xsl:apply-templates select="$gs_fields" mode="gs_context_menu_news_template_list">
        <xsl:with-param name="gs_object" select="$gs_object"/>
      </xsl:apply-templates>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="xsd:element|xsd:attribute" mode="gs_context_menu_news_template_list">
    <!-- ONLY @type=class:<Class>
      $gs_type_class is calculated from the @type 
        @name=object:Forum @type=class:Forum
        @name=title        @title=xsd:string
      where class:Forum refers to another xsd:schema
      important to analyse the @type, not the @name
      because the Class/@elements might be thing:* and thus allow flexible @name
    -->
    <xsl:param name="gs_object" select="FORCE_SELECT_EMPTY_NODESET"/>

    <xsl:variable name="gs_type" select="substring-after(@type, ':')"/>
    <xsl:variable name="gs_type_full" select="concat('Class__', $gs_type)"/>
    <xsl:variable name="gs_type_class" select="id($gs_type_full)"/>
    
    <xsl:variable name="gs_element_name" select="@name"/>
    <xsl:variable name="gs_element" select="$gs_object/*[name() = $gs_element_name]"/>

    <xsl:copy>
      <xsl:apply-templates select="@*" mode="gs_interface_render"/>
      <xsl:apply-templates select="." mode="gs_interface_render_adorn_node"/>
      <xsl:apply-templates select="$gs_type_class" mode="gs_context_menu_news_template_list_class"/>
    </xsl:copy>
  </xsl:template>
  
  <xsl:template match="xsd:any" mode="gs_context_menu_news_template_list">
    <!-- $gs_type_class is required parameter for xsd:any 
      no @type attribute!!!
      it will be sent through from the @meta:classes-context-menu-news
      and added in to the mix
    -->
    <xsl:param name="gs_object" select="FORCE_SELECT_EMPTY_NODESET"/>
    <xsl:param name="gs_type_class" select="FORCE_SELECT_EMPTY_NODESET"/>

    <xsl:variable name="gs_element_name" select="@name"/>
    <xsl:variable name="gs_element" select="$gs_object/*[name() = $gs_element_name]"/>

    <xsl:copy>
      <xsl:attribute name="gs:type"><xsl:value-of select="local-name($gs_type_class)"/></xsl:attribute>
      <xsl:apply-templates select="@*" mode="gs_interface_render"/>
      <xsl:apply-templates select="." mode="gs_interface_render_adorn_node"/>
      <xsl:apply-templates select="$gs_type_class" mode="gs_context_menu_news_template_list_class"/>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="class:*" mode="gs_context_menu_news_template_list_class">
    <xsl:copy>
      <xsl:apply-templates select="." mode="gs_interface_render_adorn_node">
        <xsl:with-param name="gs_interface_mode" select="'listname'"/>
      </xsl:apply-templates>
      <xsl:apply-templates select="@*" mode="gs_interface_render"/>
    </xsl:copy>
  </xsl:template>
  
  <!-- ############################################ sets ############################################ -->
  <xsl:template match="class:*" mode="gs_context_menu_sets_template_list">
    <xsl:param name="gs_object" select="FORCE_SELECT_EMPTY_NODESET"/>

    <!-- inherited classes -->
    <xsl:apply-templates select="class:*" mode="gs_context_menu_sets_template_list">
      <xsl:with-param name="gs_object" select="$gs_object"/>
    </xsl:apply-templates>

    <!-- this class schema class:* -->
    <xsl:apply-templates select="xsd:schema[not(@name)][1]" mode="gs_context_menu_sets_template_list">
      <xsl:with-param name="gs_object" select="$gs_object"/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="xsd:schema" mode="gs_context_menu_sets_template_list">
    <!-- original object included so we can analyse how many of each xsd:element are already present -->
    <xsl:param name="gs_object" select="FORCE_SELECT_EMPTY_NODESET"/>

    <!-- TODO: which fields? -->
    <xsl:variable name="gs_fields_attributes" select="xsd:complexType/xsd:sequence/xsd:attribute"/>
    <xsl:variable name="gs_fields_single_elements" select="xsd:complexType/xsd:sequence/xsd:element[@maxOccurs = 1]"/>
    <xsl:variable name="gs_fields" select="$gs_fields_attributes|$gs_fields_single_elements"/>

    <xsl:copy>
      <xsl:apply-templates select="@*" mode="gs_interface_render"/>
      <xsl:apply-templates select="." mode="gs_interface_render_adorn_node">
        <xsl:with-param name="gs_interface_mode" select="'set'"/>
      </xsl:apply-templates>

      <xsl:apply-templates select="$gs_fields" mode="gs_context_menu_sets_template_list">
        <xsl:with-param name="gs_object" select="$gs_object"/>
      </xsl:apply-templates>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="xsd:element|xsd:attribute" mode="gs_context_menu_sets_template_list">
    <xsl:param name="gs_object" select="FORCE_SELECT_EMPTY_NODESET"/>

    <!-- TODO: current value analysis -->
    <xsl:variable name="gs_element_name" select="@name"/>
    <xsl:variable name="gs_element" select="$gs_object/*[name() = $gs_element_name]"/>

    <xsl:copy>
      <xsl:apply-templates select="." mode="gs_interface_render_adorn_node"/>
      <xsl:apply-templates select="@*" mode="gs_interface_render"/>
    </xsl:copy>
  </xsl:template>

  <!-- ######################################### views ######################################### -->
  <xsl:template match="class:*" mode="gs_context_menu_views_template_list">
    <xsl:param name="gs_object" select="FORCE_SELECT_EMPTY_NODESET"/>
    <xsl:param name="gs_class" select="."/> <!-- to know inherited or not -->

    <!-- inheritance -->
    <xsl:apply-templates select="class:*" mode="gs_context_menu_views_template_list">
      <xsl:with-param name="gs_object" select="$gs_object"/>
      <xsl:with-param name="gs_class" select="$gs_class"/>
    </xsl:apply-templates>

    <xsl:if test="@elements">
      <xsl:apply-templates select="xsl:stylesheet|repository:*/xsl:stylesheet|repository:*/repository:*/xsl:stylesheet" mode="gs_context_menu_views_template_list">
        <xsl:with-param name="gs_object" select="$gs_object"/>
        <xsl:with-param name="gs_inherited" select="database:has-same-node($gs_class, .)"/>
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>

  <xsl:template match="xsl:stylesheet" mode="gs_context_menu_views_template_list">
    <xsl:param name="gs_object" select="FORCE_SELECT_EMPTY_NODESET"/>
    <xsl:param name="gs_inherited" select="false()"/>

    <xsl:copy>
      <xsl:apply-templates select="@*" mode="gs_interface_render"/>
      <xsl:apply-templates select="." mode="gs_interface_render_adorn_node">
        <xsl:with-param name="gs_interface_mode" select="'view'"/>
      </xsl:apply-templates>
      <xsl:apply-templates select="xsl:template[str:boolean(@meta:interface-template)]" mode="gs_context_menu_views_template_list">
        <xsl:with-param name="gs_object" select="$gs_object"/>
        <xsl:with-param name="gs_inherited" select="$gs_inherited"/>
      </xsl:apply-templates>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="xsl:template" mode="gs_context_menu_views_template_list">
    <xsl:param name="gs_object" select="FORCE_SELECT_EMPTY_NODESET"/>
    <xsl:param name="gs_inherited" select="false()"/>

    <xsl:if test="not(starts-with(@mode, 'gs_')) and str:not(@gs:is-protected)">
      <xsl:copy>
        <xsl:if test="$gs_inherited"><xsl:attribute name="gs:inherited">true</xsl:attribute></xsl:if>
        <xsl:apply-templates select="." mode="gs_interface_render_adorn_node"/>
        <xsl:apply-templates select="@*" mode="gs_interface_render"/>
      </xsl:copy>
    </xsl:if>
  </xsl:template>

  <!-- ############################################ controllers ############################################ -->
  <xsl:template match="class:*" mode="gs_context_menu_controllers_template_list">
    <xsl:param name="gs_object" select="FORCE_SELECT_EMPTY_NODESET"/>
    <xsl:param name="gs_class" select="."/> <!-- to know inherited or not -->

    <!-- inheritance -->
    <xsl:apply-templates select="class:*" mode="gs_context_menu_controllers_template_list">
      <xsl:with-param name="gs_object" select="$gs_object"/>
      <xsl:with-param name="gs_class" select="$gs_class"/>
    </xsl:apply-templates>

    <xsl:apply-templates select="xsl:stylesheet[str:boolean(@controller)]" mode="gs_context_menu_controllers_template_list">
      <xsl:with-param name="gs_object" select="$gs_object"/>
      <xsl:with-param name="gs_inherited" select="database:has-same-node($gs_class, .)"/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="xsl:stylesheet" mode="gs_context_menu_controllers_template_list">
    <xsl:param name="gs_object" select="FORCE_SELECT_EMPTY_NODESET"/>
    <xsl:param name="gs_inherited" select="false()"/>

    <xsl:copy>
      <xsl:if test="$gs_inherited"><xsl:attribute name="gs:inherited">true</xsl:attribute></xsl:if>
      <xsl:apply-templates select="@*" mode="gs_interface_render"/>
      <xsl:apply-templates select="." mode="gs_interface_render_adorn_node">
        <xsl:with-param name="gs_interface_mode" select="'controller'"/>
      </xsl:apply-templates>
      <xsl:apply-templates select="xsl:template" mode="gs_context_menu_controllers_template_list">
        <xsl:with-param name="gs_object" select="$gs_object"/>
        <xsl:with-param name="gs_inherited" select="$gs_inherited"/>
        <xsl:sort select="@mode"/>
      </xsl:apply-templates>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="xsl:template" mode="gs_context_menu_controllers_template_list">
    <xsl:param name="gs_object" select="FORCE_SELECT_EMPTY_NODESET"/>
    <xsl:param name="gs_inherited" select="false()"/>

    <xsl:if test="not(starts-with(@mode, 'gs_')) and str:not(@gs:is-protected)">
      <xsl:copy>
        <xsl:apply-templates select="." mode="gs_interface_render_adorn_node"/>
        <xsl:apply-templates select="@*" mode="gs_interface_render"/>
        <xsl:if test="$gs_inherited"><xsl:attribute name="gs:inherited">true</xsl:attribute></xsl:if>
        <xsl:copy-of select="xsl:param"/>
      </xsl:copy>
    </xsl:if>
  </xsl:template>

  <!-- ############################################ event-handlers ############################################ -->
  <xsl:template match="class:*" mode="gs_context_menu_event_handlers_template_list">
    <xsl:param name="gs_object" select="FORCE_SELECT_EMPTY_NODESET"/>

    <!-- inheritance -->
    <xsl:apply-templates select="class:*" mode="gs_context_menu_event_handlers_template_list">
      <xsl:with-param name="gs_object" select="$gs_object"/>
    </xsl:apply-templates>

    <xsl:apply-templates select="javascript:code/javascript:object[not(@name)]" mode="gs_context_menu_event_handlers_template_list">
      <xsl:with-param name="gs_object" select="$gs_object"/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="javascript:object" mode="gs_context_menu_event_handlers_template_list">
    <xsl:param name="gs_object" select="FORCE_SELECT_EMPTY_NODESET"/>

    <xsl:copy>
      <xsl:apply-templates select="@*" mode="gs_interface_render"/>
      <xsl:apply-templates select="." mode="gs_interface_render_adorn_node">
        <xsl:with-param name="gs_interface_mode" select="'eventhandler'"/>
      </xsl:apply-templates>
      <xsl:apply-templates select="javascript:event-handler" mode="gs_context_menu_event_handlers_template_list">
        <xsl:with-param name="gs_object" select="$gs_object"/>
        <xsl:sort select="@event"/>
      </xsl:apply-templates>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="javascript:event-handler" mode="gs_context_menu_event_handlers_template_list">
    <xsl:param name="gs_object" select="FORCE_SELECT_EMPTY_NODESET"/>
    <xsl:copy>
      <!-- we stamp the meta:$gs_object info so that it is standalone -->
      <xsl:apply-templates select="." mode="gs_interface_render_adorn_node"/>
      <xsl:apply-templates select="@*" mode="gs_interface_render"/>
    </xsl:copy>
  </xsl:template>
</xsl:stylesheet>
