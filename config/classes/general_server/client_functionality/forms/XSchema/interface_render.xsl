<xsl:stylesheet xmlns:dyn="http://exslt.org/dynamic" xmlns:flow="http://exslt.org/flow" xmlns:conversation="http://general_server.org/xmlnamespaces/conversation/2006" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:exsl="http://exslt.org/common" xmlns:class="http://general_server.org/xmlnamespaces/class/2006" xmlns:user="http://general_server.org/xmlnamespaces/user/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:database="http://general_server.org/xmlnamespaces/database/2006" response:server-side-only="true" name="interface_render" version="1.0" extension-element-prefixes="dyn str regexp database server conversation request debug exsl">
  <xsl:template match="xsd:add|xsd:render" mode="gs_interface_render">
    <!-- xsd:add @xpath-to-destination [@class @xschema-name @interface-mode]
      [xml:hardlink => xsd:element|xsd:attribute|xsd:any] statement allowing the addition to the destination class!
      this ensures referential and application integrity when change types
    -->
    <xsl:param name="gs_current"/>      <!-- used by data-queries for $gs_current node -->
    <xsl:param name="gs_xschema_name" select="str:dynamic(@xschema-name)"/>

    <!-- <xsd:add> destination class XSD acceptance statements
      <xsd:any />  xsd:add/@class is needed in the case of xsd:any
      <xsd:element type="class:Group" /> acceptance statement dictates the type to add, xsd:add/@class is ignored
    -->
    <xsl:param name="gs_xsd_destination_class_acceptance_statement" select="xsd:element|xsd:attribute|xsd:any"/>
    <xsl:param name="gs_xsd_destination_class_acceptance_statement_class" select="database:class($gs_xsd_destination_class_acceptance_statement/@type)"/>
    <xsl:param name="gs_any_class" select="dyn:evaluate(str:dynamic(@class))"/>

    <!-- standard database:query params -->
    <xsl:param name="gs_condition" select="str:dynamic(@condition)"/>
    <xsl:param name="gs_outer_interface_mode"/>
    <xsl:param name="gs_outer_interface_mode_condition" select="str:dynamic(@interface-mode-condition)"/>
    <xsl:param name="gs_interface_mode" select="str:dynamic(@interface-mode)"/>
    <xsl:param name="gs_display_class" select="str:dynamic(@display-class)"/>

    <!-- relevant data rendering params -->
    <xsl:param name="gs_interface_render_output"/>

    <xsl:if test="$gs_interface_render_output = 'placeholder'"><xsl:copy-of select="."/></xsl:if>
    <xsl:if test="not($gs_interface_render_output = 'placeholder')">
      <!-- conditional returns for the data. useful in Classes that have varied data requirements. -->
      <xsl:if test="not($gs_condition) or dyn:evaluate($gs_condition)">
        <xsl:if test="not($gs_outer_interface_mode_condition) or $gs_outer_interface_mode = $gs_outer_interface_mode_condition">
          <xsl:variable name="gs_class" select="flow:first-set-not-empty($gs_xsd_destination_class_acceptance_statement_class, $gs_any_class)"/>

          <!-- checks -->
          <xsl:if test="self::xsd:add">
            <xsl:if test="not(@xpath-to-destination)"><debug:server-message output="xsd:add @xpath-to-destination required" type="warning"/></xsl:if>
            <xsl:if test="not($gs_xsd_destination_class_acceptance_statement)"><debug:server-message output="xsd:add has no [hardlinked] (xsd:element|xsd:attribute|xsd:any) child acceptance statement indicating that it can be added to its intended parent class" type="warning"/></xsl:if>
            <xsl:if test="xsd:any and not(@class)"><debug:server-message output="xsd:add with an xsd:any acceptance statement requires xsd:add/@class to indicate which class to add" type="warning"/></xsl:if>
            <xsl:if test="xsd:element|xsd:attribute and @class"><debug:server-message output="xsd:add with a specific xsd:element|xsd:attribute acceptance statement will ignore xsd:add/@class because the acceptance statement indicates what to add" type="warning"/></xsl:if>
          </xsl:if>
          <xsl:if test="not($gs_class)">
            <debug:server-message output="[{name()}] does not understand which class to add [{@class}] [{name($gs_xsd_destination_class_acceptance_statement)}]" type="warning"/>
            <debug:server-message output-node="$gs_request" max-depth="5"/>
          </xsl:if>
          
          <xsl:apply-templates select="$gs_class" mode="gs_inheritance_render_xschema_complement">
            <xsl:with-param name="gs_inheritance_render_xschema_compiled" select="true()"/>
            <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
            <xsl:with-param name="gs_display_class" select="$gs_display_class"/>
            <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
            <xsl:with-param name="gs_documentation_fields" select="@*"/>
          </xsl:apply-templates>
        </xsl:if>
      </xsl:if>
    </xsl:if>
  </xsl:template>

  <xsl:template match="xsd:edit" mode="gs_interface_render">
    <!-- xsd:edit @data [@evaluate @database] [@class @xschema-name @interface-mode]
      [and/or child document]

      @data would be rendered with database:class-xschema-node-mask()
    -->
    <xsl:param name="gs_override_class" select="database:class(str:dynamic(@class))"/>
    <xsl:param name="gs_xschema_name" select="str:dynamic(@xschema-name)"/>
    <xsl:param name="gs_interface_mode"/>

    <!-- standard database:query params -->
    <xsl:param name="gs_data_context" select="str:dynamic(@data-context)"/>
    <xsl:param name="gs_data_context_id" select="str:dynamic(@data-context-id)"/>
    <xsl:param name="gs_child_doc" select="*"/>                         <!-- OR REQUIRED: the data!  -->
    <xsl:param name="gs_condition" select="str:dynamic(@condition)"/>
    <xsl:param name="gs_outer_interface_mode_condition" select="str:dynamic(@interface-mode-condition)"/>
    <xsl:param name="gs_form_interface" select="str:dynamic(@interface-mode)"/>
    <xsl:param name="gs_form_display_class" select="str:dynamic(@display-class)"/>

    <!-- relevant data rendering params -->
    <xsl:param name="gs_interface_render_output"/>

    <xsl:if test="$gs_interface_render_output = 'placeholder'"><xsl:copy-of select="."/></xsl:if>
    <xsl:if test="not($gs_interface_render_output = 'placeholder')">
      <!-- conditional returns for the data. useful in Classes that have varied data requirements. -->
      <xsl:if test="not($gs_condition) or dyn:evaluate($gs_condition)">
        <xsl:if test="not($gs_outer_interface_mode_condition) or $gs_interface_mode = $gs_outer_interface_mode_condition">
          <!-- COPIED code database:query -->
          <xsl:variable name="gs_xpath_fragment">
            <xsl:apply-templates select="." mode="gs_interface_render_database_query_xpath"/>
          </xsl:variable>
          <xsl:variable name="gs_xpath" select="string($gs_xpath_fragment)"/>

          <xsl:variable name="gs_query_database" select="/"/>
          <xsl:variable name="gs_query_context" select="flow:first-set-not-empty(id($gs_data_context_id), dyn:map($gs_query_database, $gs_data_context), $gs_query_database)"/>
          <xsl:variable name="gs_data_select_data" select="dyn:map($gs_query_context, $gs_xpath)"/>
          <!-- NOTE: that both $gs_data_select_data $gs_child_doc can return multiple nodes -->
          <xsl:variable name="gs_data_select" select="$gs_data_select_data | $gs_child_doc[not($gs_data_select_data)]"/>
          
          <xsl:variable name="gs_first_element_first_class" select="database:classes($gs_data_select[1])[1]"/>
          <xsl:variable name="gs_classes" select="flow:first-set-not-empty($gs_override_class, $gs_first_element_first_class)"/>
          <xsl:variable name="gs_class" select="$gs_classes[1]"/>

          <!-- extra dynamic fields => xsd:annotation/xsd:documentation => input @hidden -->
          <xsl:variable name="gs_documentation_fields_fragment">
            <gs:dff xpath-to-destination="{conversation:xpath-to-node($gs_data_select)}" name="{name($gs_data_select)}"/>
          </xsl:variable>
          <xsl:variable name="gs_documentation_fields" select="exsl:node-set($gs_documentation_fields_fragment)/gs:dff/@*"/>

          <!-- checks -->
          <xsl:if test="not(@data) and not(*)"><debug:server-message output="xsd:edit @data or child document required"/></xsl:if>
          <xsl:if test="not($gs_data_select)"><debug:server-message output="xsd:edit @data gave empty result set"/></xsl:if>
          <xsl:if test="count($gs_data_select) &gt; 1"><debug:server-message output="xsd:edit @data gives [{count($gs_data_select)}] nodes, using [{local-name($gs_data_select[1])}] only"/></xsl:if>
          <xsl:if test="not($gs_classes)"><debug:server-message output="xsd:edit @data has no class"/></xsl:if>
          <xsl:if test="count($gs_classes) &gt; 1"><debug:server-message output="xsd:edit @data has [{count($gs_classes)}] classes, using [{local-name($gs_classes[1])}]"/></xsl:if>

          <xsl:apply-templates select="$gs_class" mode="gs_inheritance_render_xschema_complement">
            <xsl:with-param name="gs_inheritance_render_xschema_compiled" select="true()"/>
            <xsl:with-param name="gs_interface_mode" select="$gs_form_interface"/>
            <xsl:with-param name="gs_display_class" select="$gs_form_display_class"/>
            <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
            <xsl:with-param name="gs_data_select" select="$gs_data_select"/>
            <xsl:with-param name="gs_documentation_fields" select="$gs_documentation_fields"/>
          </xsl:apply-templates>
        </xsl:if>
      </xsl:if>
    </xsl:if>
  </xsl:template>

  <xsl:template match="xsd:edit-multiple" mode="gs_interface_render">
    <!-- relevant data rendering params -->
    <xsl:param name="gs_interface_render_output"/>

    <xsl:if test="$gs_interface_render_output = 'placeholder'"><xsl:copy-of select="."/></xsl:if>
    <xsl:if test="not($gs_interface_render_output = 'placeholder')">
      <!-- xsd:edit-multiple @data [@class @xschema-name] -->
      <xsl:if test="not(@data)"/>
    </xsl:if>
  </xsl:template>

  <xsl:template match="xsd:move-to-trash" mode="gs_interface_render">
    <!-- xsd:move-to-trash @data -->
    <xsl:if test="not(@data)"/>
    <debug:NOT_COMPLETE/>
  </xsl:template>

  <xsl:template match="xsd:delete" mode="gs_interface_render">
    <!-- xsd:delete @data -->
    <xsl:if test="not(@data)"/>
    <debug:NOT_COMPLETE/>
  </xsl:template>
</xsl:stylesheet>
