<xsl:stylesheet xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xxx="http://general_server.org/xmlnamespaces/dummyxsl/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:class="http://general_server.org/xmlnamespaces/class/2006" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns:meta="http://general_server.org/xmlnamespaces/meta/2006" xmlns="http://www.w3.org/1999/xhtml" name="view" version="1.0" extension-element-prefixes="debug">
  <xsl:template match="xsd:element|xsd:attribute|xsd:any" meta:interface-template="yes">
    <!-- traversal of the $gs_current_external_value_node in-line with the xsd:*
      xsd:* @meta:current-value-type="singular" indicates that the element works with a single $gs_current_external_value_node
      $gs_current_external_value_node MUST be a node-set, CANNOT be a tree-fragment
        <anyelement @attribute1=3 @attribute2=6 ...> (xsd:attribute's)
          <element1>example</element1> (xsd:element's)
          <element2>example</element2>
          ...
        </anyelement>
        matched on name()
    -->
    <xsl:param name="gs_html_identifier_class"/>
    <xsl:param name="gs_interface_mode"/>
    <xsl:param name="gs_current_external_value_node" select="FORCE_SELECT_EMPTY_NODESET"/>

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
  </xsl:template>

  <xsl:template match="xsd:element|xsd:attribute|xsd:any" mode="gs_form_render_items_with_relevant_value">
    <!-- $gs_current_external_value_node holds the actual node that is being edited, not its parent
      for an attribute it should be the actual @attribute node (name will NOT be checked)
      for an element if should be the actual <element>
      text() nodes will be used for the value
      @meta:auto-create:
        yes: show only that the item will be created
        ask: checkbox for if the user wants creation or not (requires minOccurs="0")
    -->
    <xsl:param name="gs_html_identifier_class"/>
    <xsl:param name="gs_interface_mode"/>
    <xsl:param name="gs_current_external_value_node"/>

    <xsl:if test="string(@meta:editor-class)"><div class="gs-warning">@meta:editor-class [<xsl:value-of select="@meta:editor-class"/>] not found / processed</div></xsl:if>

    <!-- important that the FQN of the class element is sent through here so it matches that when the form is submitted below -->
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
              <xsl:with-param name="gs_value" select="$gs_value"/> <!-- string -->
            </xsl:apply-templates>
          </xsl:otherwise>
        </xsl:choose>
        <xsl:apply-templates select="xsd:annotation[@meta:field-message-type]" mode="gs_form_render_item_input_field_message"/>
        <xsl:apply-templates select="xsd:simpleType/xsd:restriction/xsd:pattern" mode="gs_form_render_item_input_validation"/>
      </div>
      <xsl:apply-templates select="xsd:annotation[not(@meta:field-message-type)]" mode="gs_form_render_item_input_help"/>
      <div class="gs-xsd-type"><xsl:value-of select="$gs_type_name"/></div>

      <!-- XSchema apply sub xsd:attribute xsd:element honour ordering -->
      <!-- xsl:apply-templates select="xsd:attribute|xsd:element" mode="gs_form_render_items_with_relevant_value">
        <xsl:with-param name="gs_current_external_value_node" select="$gs_current_external_value_node"/>
      </xsl:apply-templates -->
    </div>
  </xsl:template>

  <xsl:template match="xsd:element|xsd:attribute|xsd:any" mode="gs_form_render_item_input">
    <xsl:param name="gs_HTML_id"/>
    <xsl:param name="gs_raw_name"/>
    <xsl:param name="gs_value"/>

    <xsl:if test="@type='xsd:boolean'">
      <!-- ensure that checkboxes always return a value -->
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
  </xsl:template>

  <xsl:template match="xsd:restriction/xsd:pattern" mode="gs_form_render_item_input_validation">
    <span class="gs-xtransport gs-required"><xsl:value-of select="@value"/></span>
  </xsl:template>

  <xsl:template match="xsd:annotation" mode="gs_form_render_item_input_field_message">
    <!-- maximum only one field-message-type because they conflict. S: suggestion, D: default, H: help -->
    <span class="gs-xtransport gs-xsd-field-message"><xsl:value-of select="xsd:documentation"/></span>
    <span class="gs-xtransport gs-xsd-field-message-type"><xsl:value-of select="@meta:field-message-type"/></span>
  </xsl:template>

  <xsl:template match="xsd:annotation" mode="gs_form_render_item_input_help">
    <div class="gs-xsd-help"><xsl:value-of select="xsd:documentation"/></div>
  </xsl:template>

  <!-- ####################################### editor ####################################### -->
  <xsl:template match="xsd:element|xsd:attribute|xsd:any" meta:interface-template="yes" mode="editor">
    <xsl:param name="gs_html_identifier_class"/>
    <xsl:param name="gs_field_model" select="ancestor::xsd:schema[1]/xsd:annotation[@meta:type='data-queries']/xsd:app-info[@meta:type='field-model']/xsd:schema"/>
    <xsl:param name="gs_field_types" select="ancestor::xsd:schema[1]/xsd:annotation[@meta:type='data-queries']/xsd:app-info[@meta:type='field-types']/xsd:schema"/>

    <li class="{$gs_html_identifier_class} gs-inherited-{@meta:inherited} gs-class-{@meta:class} gs-type-{translate(@type, ':', '-')} gs-auto-create-{@meta:auto_create} gs-min-occurs-{@minOccurs} gs-max-occurs-{@maxOccurs}">
      <xsl:if test="not($gs_field_model)"><div class="gs-warning">field model (XSD) not found</div></xsl:if>

      <xsl:apply-templates select="$gs_field_model" mode="gs_view_render">
        <xsl:with-param name="gs_current_external_value_node" select="."/>
        <xsl:with-param name="gs_field_types" select="$gs_field_types"/>
      </xsl:apply-templates>

      <!-- form class="f_submit_ajax"><div>
        <div class="gs-xtransport"><input type="hidden" name="xml:id" value="{@xml:id}"/></div>

        <div class="gs-xsd-name gs-extended-attributes">
          <input class="input_small" name="{@name}" value="{@name}"/>
          <span class="gs-xtransport gs-xsd-field-message">field name</span>
          <span class="gs-xtransport gs-xsd-field-message-type">H</span>
          <span class="gs-xtransport gs-required">[a-zA-Z][-a-zA-Z0-9_.]{3}</span>
        </div>

        <select class="gs-xsd-type-selector">
          <option value="{@type}">(<xsl:value-of select="@type"/>)</option>
          <xsl:apply-templates select="." mode="gs_xsd_type_options"/>
        </select>

        <div class="gs-xsd-default gs-extended-attributes">
          <input name="default"/>
          <span class="gs-xtransport gs-xsd-field-message">default</span>
          <span class="gs-xtransport gs-xsd-field-message-type">H</span>
        </div>

        <select class="gs-xsd-min-occurs-selector">
          <xsl:apply-templates select="." mode="gs_xsd_occurs">
            <xsl:with-param name="gs_setting" select="@minOccurs"/>
          </xsl:apply-templates>
        </select>
        <select class="gs-xsd-max-occurs-selector">
          <xsl:apply-templates select="." mode="gs_xsd_occurs">
            <xsl:with-param name="gs_setting" select="@maxOccurs"/>
          </xsl:apply-templates>
        </select>

        <input type="submit" disabled="1" name="update field" value="update field"/>

        <xsl:if test="@meta:inherited"><a href="#" class="gs-xsd-info">(<xsl:value-of select="@meta:class"/>)</a></xsl:if>
      </div></form -->
    </li>
  </xsl:template>

  <xsl:template match="xsd:element|xsd:attribute|xsd:any" mode="gs_xsd_occurs">
    <xsl:param name="gs_setting"/>
    <xsl:param name="gs_int" select="0"/>

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
        <xsl:with-param name="gs_caption" select="'∞'"/>
      </xsl:apply-templates>
      <xsl:apply-templates select="." mode="gs_xsd_occurs_output">
        <xsl:with-param name="gs_setting" select="$gs_setting"/>
        <xsl:with-param name="gs_int" select="'other'"/>
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>

  <xsl:template match="xsd:element|xsd:attribute|xsd:any" mode="gs_xsd_occurs_output">
    <xsl:param name="gs_setting"/>
    <xsl:param name="gs_int"/>
    <xsl:param name="gs_caption"/>

    <option>
      <xsl:if test="$gs_int = $gs_setting"><xsl:attribute name="selected">true</xsl:attribute></xsl:if>
      <xsl:if test="$gs_caption"><xsl:value-of select="$gs_caption"/></xsl:if>
      <xsl:if test="not($gs_caption)"><xsl:value-of select="$gs_int"/></xsl:if>
    </option>
  </xsl:template>

  <xsl:template match="xsd:element|xsd:attribute|xsd:any|xsd:sequence" mode="gs_xsd_type_options">
    <!-- xsd:sequence included for the non-existent add field -->
    <xsl:variable name="gs_xsd_schema" select="ancestor::xsd:schema[1]"/>
    <xsl:variable name="gs_xsd_annotation_type_info" select="$gs_xsd_schema/xsd:annotation[@meta:type='type-info']"/>

    <xsl:if test="not($gs_xsd_schema)"><div class="gs-warning">ancestor schema not found for type-info lists</div></xsl:if>
    <xsl:if test="not($gs_xsd_annotation_type_info)"><div class="gs-warning">xsd:annotation @meta:type=type-info not found for type lists</div></xsl:if>

    <!-- there are 2 xsd:app-info: xsl-types and classes -->
    <xsl:apply-templates select="$gs_xsd_annotation_type_info/xsd:app-info/*" mode="gs_xsd_type_options_output">
      <xsl:with-param name="gs_element" select="."/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="*" mode="gs_xsd_type_options_output">
    <xsl:param name="gs_element"/>

    <option>
      <xsl:if test="$gs_element and $gs_element/@type = name()"><xsl:attribute name="selected">1</xsl:attribute></xsl:if>
      <xsl:attribute name="value"><xsl:value-of select="local-name()"/></xsl:attribute>

      <xsl:if test="@name"><xsl:value-of select="@name"/></xsl:if>
      <xsl:if test="not(@name)"><xsl:value-of select="local-name()"/></xsl:if>
    </option>
  </xsl:template>

  <!-- ####################################### news ####################################### -->
  <xsl:template match="xsd:element|xsd:attribute|xsd:any" mode="new" meta:interface-template="yes">
    <xsl:param name="gs_html_identifier_class"/>
    <xsl:param name="gs_interface_mode"/>

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
      <!-- a class:* or xsd:* should be the only children
        @gs:interface-mode=list with the children access hidden
        => .Object.CSS__Class
      -->
      <xsl:apply-templates select="$gs_type_class" mode="gs_view_render"/>
      <xsl:text>: </xsl:text>
      <span class="gs-name"><xsl:value-of select="$gs_name"/></span>
    </li>
  </xsl:template>

  <xsl:template match="xsd:element|xsd:attribute|xsd:any" mode="set" meta:interface-template="yes">
    <xsl:param name="gs_html_identifier_class"/>
    <xsl:param name="gs_interface_mode"/>

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
  </xsl:template>
</xsl:stylesheet>
