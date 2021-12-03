<xsl:stylesheet xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xxx="http://general_server.org/xmlnamespaces/dummyxsl/2006" xmlns:meta="http://general_server.org/xmlnamespaces/meta/2006" xmlns:debug="http://general_server.org/xmlnamespaces/debug/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:class="http://general_server.org/xmlnamespaces/class/2006" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns="http://www.w3.org/1999/xhtml" name="view" version="1.0" extension-element-prefixes="debug">
  <!-- display templates for the XSchema Class
    mostly used in the administration suite for editing purposes
    and producing object add and edit HTML forms
  -->
  <xsl:template match="xsd:schema" mode="gs_listname">
    <xsl:text>model</xsl:text>
    <xsl:if test="@name"> (<xsl:value-of select="@name"/>)</xsl:if>
  </xsl:template>

  <xsl:template match="xsd:schema[xsd:annotation/xsd:app-info[@meta:type='multi-form']]" meta:interface-template="yes">
    <xsl:param name="gs_html_identifier_class"/>
    <xsl:param name="gs_title" select="xsd:annotation/xsd:*[@meta:type='title']"/>               <!-- optional: e.g. add a thing -->
    <xsl:param name="gs_submit" select="xsd:annotation/xsd:*[@meta:type='submit-caption']"/>              <!-- optional: e.g. add a thing -->
    <xsl:param name="gs_description" select="xsd:annotation/xsd:*[@meta:type='description']"/>         <!-- optional: e.g. add a thing -->
    <xsl:param name="gs_event_functions"/>     <!-- optional: HTML class additions -->
    <xsl:param name="gs_current_value_node" select="xsd:annotation/xsd:*[@meta:type='edit-data']/*"/>  <!-- optional: editing current object copy with values -->

    <!-- defaults: xsd:annotations, parameters -->
    <xsl:variable name="gs_xsd_multi_form" select="xsd:annotation/xsd:app-info[@meta:type='multi-form']"/>
    <!-- meta:interface-template must be consistent with xsl:param @name/@select 
      so we calculate this common xsl:param default NOT in its @select
    -->
    <xsl:variable name="gs_event_functions_default">
      <xsl:value-of select="$gs_event_functions"/>
      <xsl:if test="not($gs_event_functions)"><xsl:value-of select="xsd:annotation/xsd:*[@meta:type='event-functions']"/></xsl:if>
    </xsl:variable>

    <!-- checks -->
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
                  <xsl:with-param name="gs_title" select="$gs_title"/>               <!-- optional: e.g. add a thing -->
                  <xsl:with-param name="gs_submit" select="$gs_submit"/>              <!-- optional: e.g. add a thing -->
                  <xsl:with-param name="gs_description" select="$gs_description"/>         <!-- optional: e.g. add a thing -->
                  <xsl:with-param name="gs_event_functions" select="$gs_event_functions_default"/>     <!-- optional: HTML class additions -->
                  <xsl:with-param name="gs_current_value_node" select="."/>  <!-- optional: editing current object copy with values -->
                </xsl:apply-templates>
              </li>
            </xsl:for-each>
          </xsl:when>
          <xsl:when test="$gs_xsd_multi_form = '*'">
            <xsl:for-each select="$gs_current_value_node">
              <li>
                <xsl:apply-templates select="$gs_xsd_schema" mode="gs_form_render">
                  <xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/>
                  <xsl:with-param name="gs_title" select="$gs_title"/>               <!-- optional: e.g. add a thing -->
                  <xsl:with-param name="gs_submit" select="$gs_submit"/>              <!-- optional: e.g. add a thing -->
                  <xsl:with-param name="gs_description" select="$gs_description"/>         <!-- optional: e.g. add a thing -->
                  <xsl:with-param name="gs_event_functions" select="$gs_event_functions_default"/>     <!-- optional: HTML class additions -->
                  <xsl:with-param name="gs_current_value_node" select="."/>  <!-- optional: editing current object copy with values -->
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
  </xsl:template>

  <xsl:template match="xsd:schema" meta:interface-template="yes">
    <xsl:param name="gs_html_identifier_class"/>
    <xsl:param name="gs_title" select="xsd:annotation/xsd:*[@meta:type='title']"/>               <!-- optional: e.g. add a thing -->
    <xsl:param name="gs_submit" select="xsd:annotation/xsd:*[@meta:type='submit-caption']"/>              <!-- optional: e.g. add a thing -->
    <xsl:param name="gs_description" select="xsd:annotation/xsd:*[@meta:type='description']"/>         <!-- optional: e.g. add a thing -->
    <xsl:param name="gs_event_functions"/>     <!-- optional: HTML class additions -->
    <xsl:param name="gs_current_value_node" select="xsd:annotation/xsd:*[@meta:type='edit-data']/*"/>  <!-- optional: editing current object copy with values -->

    <!-- meta:interface-template must be consistent with xsl:param @name/@select 
      so we calculate this common xsl:param default NOT in its @select
    -->
    <xsl:variable name="gs_event_functions_default">
      <xsl:value-of select="$gs_event_functions"/>
      <xsl:if test="not($gs_event_functions)"><xsl:value-of select="xsd:annotation/xsd:*[@meta:type='event-functions']"/></xsl:if>
    </xsl:variable>

    <xsl:apply-templates select="." mode="gs_form_render">
      <xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/>
      <xsl:with-param name="gs_title" select="$gs_title"/>               <!-- optional: e.g. add a thing -->
      <xsl:with-param name="gs_submit" select="$gs_submit"/>              <!-- optional: e.g. add a thing -->
      <xsl:with-param name="gs_description" select="$gs_description"/>         <!-- optional: e.g. add a thing -->
      <xsl:with-param name="gs_event_functions" select="$gs_event_functions_default"/>     <!-- optional: HTML class additions -->
      <xsl:with-param name="gs_current_value_node" select="$gs_current_value_node"/>  <!-- optional: editing current object copy with values -->
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="xsd:schema" mode="gs_form_render">
    <xsl:param name="gs_html_identifier_class"/>
    <xsl:param name="gs_title" select="xsd:annotation/xsd:*[@meta:type='title']"/>               <!-- optional: e.g. add a thing -->
    <xsl:param name="gs_submit" select="xsd:annotation/xsd:*[@meta:type='submit-caption']"/>              <!-- optional: e.g. add a thing -->
    <xsl:param name="gs_description" select="xsd:annotation/xsd:*[@meta:type='description']"/>         <!-- optional: e.g. add a thing -->
    <xsl:param name="gs_event_functions" select="xsd:annotation/xsd:*[@meta:type='event-functions']"/>     <!-- optional: HTML class additions -->
    <xsl:param name="gs_current_value_node" select="xsd:annotation/xsd:*[@meta:type='edit-data']/*"/>  <!-- optional: editing current object copy with values -->

    <xsl:variable name="gs_xsd_form_display" select="xsd:annotation[@meta:type='form-display']"/>
    <xsl:variable name="gs_xsd_data_processing" select="xsd:annotation[@meta:type='data-processing']"/>

    <!-- checks -->
    <xsl:if test="not($gs_xsd_data_processing)">
      <div class="gs-warning">
        xsd:annotation[@meta:type='data-processing'] missing from form 
        [<xsl:value-of select="local-name(ancestor::class:*[1])"/>::<xsl:value-of select="@name"/> <xsl:value-of select="@xml:id"/>]
      </div>
    </xsl:if>
    <!-- TODO: xsl:apply-templates select="xsd:annotation/xsd:app-info[@meta:type='required-documentation']/xsd:sequence/xsd:*" mode="gs_form_render_check_required_documentation"></xsl:apply-templates -->

    <!-- we always want a valid submit button because otherwise the form cannot be submitted on some browsers -->
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

      <!-- submit a POST form to myself
        @action=POST is irrelevant, the Server MessageInterpretation will always:
        ignore the URL, process the form and then redirect
        @method=POST causes the POST Server MessageInterpretation to take control because of the POST HTTP header
      -->
      <form class="details" method="POST"><div>
        <span class="gs-form-elements">
          <xsl:apply-templates select="xsd:complexType/xsd:sequence/xsd:*" mode="gs_view_render">
            <xsl:with-param name="gs_current_external_value_node" select="$gs_current_value_node"/>
          </xsl:apply-templates>
        </span>

        <!-- xsd:documentation @meta:type should be WITHOUT prefix
          this meta-data is pointed to by the sAdditionalMetaLocations static_property
          place context AFTER the XSD declared fields so that the fields initially match the intention
          when serialising the form data for javascript calls
        -->
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
  </xsl:template>

  <xsl:template match="xsd:schema//xsd:sequence/xsd:*" mode="gs_form_render_check_required_documentation">
    <!-- TODO: check that there is adequate documentation for the form -->
  </xsl:template>

  <xsl:template match="xsd:schema/xsd:annotation/xsd:documentation" mode="gs_form_render_documentation">
    <!-- we require the meta: namespace to avoid conflict with real form input names -->
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
  </xsl:template>

  <xsl:template match="xsd:schema" mode="gs_context_menu_custom">
    <li class="f_submit_ajax">submit form</li>
  </xsl:template>

  <xsl:template match="xsd:schema" mode="gs_form_render_submit">
    <xsl:param name="gs_submit_defaulted"/>

    <input class="gs-form-submit" type="submit" name="gs--submit" value="{$gs_submit_defaulted}">
      <!-- we DO NOT do the following because
        we want to bubble up the ctrl-save event form the focused input
        in the case of multiple access key forms
      <xsl:if test="$gs_xsd_form_ctrl_s">
        <xsl:attribute name="accesskey">s</xsl:attribute>
      </xsl:if -->
    </input>
  </xsl:template>


  <!-- ####################################### editor ####################################### -->
  <xsl:template match="xsd:schema" mode="editor">
    <!-- the xsd:render data-query replacement produces the data here
      so we are acting on the compiled inherited xschema here
    -->
    <xsl:param name="gs_html_identifier_class"/>

    <xsl:if test="@name">
      <div class="gs-warning">
        inheritance on named schemas is not supported yet.
        this is just because the @name cannot be passed through to the class_xschema.xsl Tech processor
      </div>
    </xsl:if>

    <div class="{$gs_html_identifier_class} gs-interface-mode-editor">
      <!-- header -->
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
  </xsl:template>

  <xsl:template match="xsd:sequence" mode="editor">
    <!-- model and info required
      ~XSchema data-queries include this in editor mode
    -->
    <xsl:param name="gs_field_model" select="ancestor::xsd:schema[1]/xsd:annotation[@meta:type='data-queries']/xsd:app-info[@meta:type='field-model']/xsd:schema"/>
    <xsl:param name="gs_field_types" select="ancestor::xsd:schema[1]/xsd:annotation[@meta:type='data-queries']/xsd:app-info[@meta:type='field-types']/xsd:schema"/>

    <ul class="gs-xsd-editor-list gs-xsd-section gs-jquery-nodrag">
      <!-- fields -->
      <xsl:apply-templates select="*" mode="gs_view_render">
        <xsl:with-param name="gs_interface_mode" select="'editor'"/>
        <xsl:with-param name="gs_field_model" select="$gs_field_model"/>
        <xsl:with-param name="gs_field_types" select="$gs_field_types"/>
      </xsl:apply-templates>

      <!-- add new field -->
      <!-- li class="gs-xsd-editor-list-add-field">
        <form class="f_submit_ajax"><div>
          <div class="gs-xtransport"/>

          <div class="gs-xsd-name gs-extended-attributes">
            <input class="input_small" name="field_name"/>
            <span class="gs-xtransport gs-xsd-field-message">field name</span>
            <span class="gs-xtransport gs-xsd-field-message-type">H</span>
            <span class="gs-xtransport gs-required">[a-zA-Z][-a-zA-Z0-9_.]{3}</span>
          </div>

          <select class="gs-xsd-type-selector">
            <option value="">(select type)</option>
            <xsl:apply-templates select="." mode="gs_xsd_type_options"/>
          </select>

          <div class="gs-xsd-default gs-extended-attributes">
            <input name="default"/>
            <span class="gs-xtransport gs-xsd-field-message">default</span>
            <span class="gs-xtransport gs-xsd-field-message-type">H</span>
          </div>

          <select class="gs-xsd-min-occurs-selector"><xsl:apply-templates select="." mode="gs_xsd_occurs"/></select>
          <select class="gs-xsd-max-occurs-selector"><xsl:apply-templates select="." mode="gs_xsd_occurs"/></select>

          <input class="gs-submit" type="submit" name="add field" value="add field"/>
        </div></form>
      </li -->
    </ul>
  </xsl:template>

  <xsl:template match="xsd:annotation|xsd:app-info|xsd:documentation" mode="editor"/>

  <xsl:template match="xsd:annotation[@meta:type = 'form-display']" mode="editor">
    <xsl:apply-templates select="*" mode="editor"/>
  </xsl:template>

  <xsl:template match="xsd:documentation[@meta:type = 'order']" mode="editor"/>

  <xsl:template match="xsd:annotation[@meta:type = 'data-processing']" mode="editor">
    <xsl:apply-templates select="*" mode="editor"/>
  </xsl:template>

  <xsl:template match="xsd:app-info[@meta:type = 'processing']" mode="editor">
    <div class="gs-xsd-section">
      <select name="dom-method">
        <option>(dom-method)</option>
        <option>class-command</option>
      </select>

      <input name="interface-mode" value="{@meta:interface-mode}"/>
      <input name="select" value="{@meta:select}"/>
      <input name="description" value="{@meta:description}"/>
    </div>
  </xsl:template>

  <xsl:template match="xsd:app-info[@meta:type = 'context']" mode="editor">
    <xsl:apply-templates select="xsd:sequence" mode="editor"/>
  </xsl:template>

  <!-- ####################################### news ####################################### -->
  <xsl:template match="xsd:schema" mode="new" meta:interface-template="yes">
    <xsl:param name="gs_html_identifier_class"/>
    <xsl:param name="gs_class" select="ancestor::class:*[1]"/>
    
    <li class="{$gs_html_identifier_class} gs-semantic-only-markup">
      <ul class="CSS__VerticalMenu gs-interface-mode-default gs-semantic-only-markup">
        <xsl:if test="not(*)">
          <li class="gs-empty-message">nothing to be added</li>
        </xsl:if>
        <xsl:apply-templates select="*" mode="gs_view_render">
          <xsl:with-param name="gs_interface_mode" select="'new'"/>
        </xsl:apply-templates>
        <li class="gs-add-message">
          <a class="gs-action-edit" href="#">edit</a> 
          <span class="gs-class-name"><xsl:value-of select="local-name($gs_class)"/></span>
          structure
        </li>
      </ul>
      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups"/>
    </li>
  </xsl:template>

  <xsl:template match="xsd:schema" mode="set" meta:interface-template="yes">
    <xsl:param name="gs_html_identifier_class"/>
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
  </xsl:template>
</xsl:stylesheet>
