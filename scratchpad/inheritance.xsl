<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:response-xml="http://general_server.org/xmlnamespaces/response/2006" response-xml:server-side-only="true" name="inheritance_render" version="1.0" extension-element-prefixes="flow database str debug request response regexp repository" object:cpp-component="yes">
  <!-- ##############################################################################################
    database:class-xschema([string className|node class:*, xschema-name, (#STAND_ALONE | #COMPLIMENT), (#EXTENSION | #COMPILED)])
    database:class-xschema(~Manager, '', #COMPLIMENT, #COMPILED)   => html form XSD editor
    database:class-xschema(~Person,  '', #STAND_ALONE, #EXTENSION) => export a full XSD check schema

    compile the pseudo xschema for a class taking into account it's xsd inheritance chain
    <class:Manager>
      <class:User>...</class:User>  [usually hardlinked] base class
      <xsd:schema>...</xsd:schema>
    </class:Manager>

    xschema-name:
      Classes have ONE xschema. other, named xschemas, are for other activities, e.g. login, property controls
      this XSL does not currently work on named schemas

    only #COMPLIMENT #COMPILED IS_USED at the moment for compiled XSD editing
    DRI is NOT_CURRENTLY_USED

    (#STAND_ALONE | #COMPLIMENT)
      controls whether the dependent complexTypes are included in the same single schema or referenced,
        e.g. <xsd:element type="User" => the User complexType
      #STAND_ALONE is NOT_CURRENTLY_USED but WILL_BE_USED for the Referential Integrity and Exports of XSDs
      #COMPLIMENT therefore means that the other XSD definitions for the xsd:element types need to be requested separately
      #COMPLIMENT IS_USED for XSD editing because the dependent complexTypes are stated, their details not shown

    (#EXTENSION | #COMPILED)
      #COMPILED indicates that all inherited field definitions need to be compiled in to one linear XSD sequence
        gs_xsd_field_inheritance is used in to one linear xsd:complexType/(xsd:element|xsd:attribute)
        in the order stated in the root class xsd:annotation @gs:type=form-display / xsd:documentation @gs:type=order
        USED by the xschema editor to present all the relevant inherited fields
        gs:inherited and gs:base-class are stamped on the xsd:elements to indicate easily their source
        WILL_BE_USED also when generating / validating / creating new objects
      #EXTENSION indicates that inheritance should use the natural xsd:extension inheritance system
        it is also true that the XSD editor could navigate the xsd:extensions to display inheritence information
        NOT_CURRENTLY_USED but WILL_BE_USED for Referential Integrity

    the highest level singular directives take precedence
      xsd:annotation @gs:type=form-display
      xsd:annotation @gs:type=data-processing


    ############## FLOW:
    gs_inheritance_render:      #STAND_ALONE (xsd:schema, gs_inheritance_render_directives, gs_inheritance_render_dependent_types => gs_xsd_complex_type)
    gs_inheritance_render_main: gs_inheritance_render_comment, gs_xsd_complex_type
    gs_xsd_complex_type: xsd:complexType/xsd:sequence => gs_xsd_field_inheritance
    gs_xsd_field_inheritance: #COMPILED gs_xsd_field_inheritance or #EXTENSION xsd:extension, gs_xsd_field_inheritance_output

    gs_inheritance_render_dependent_types:
  -->
  <xsl:include xpath="~Database/name::data_render" />

  <xsl:param name="gs_inheritance_render_compiled" select="#EXTENSION"/>
  <xsl:param name="gs_xschema_name"/>

  <xsl:template match="xsd:schema" mode="gs_inheritance_render_standalone">
    <xsl:param name="gs_inheritance_render_compiled" select="$gs_inheritance_render_compiled"/>
    <xsl:param name="gs_xschema_name" select="@name"/>
    <xsl:param name="gs_display_mode"/>

    <!-- can be sent through for efficiency -->
    <xsl:param name="gs_class" select="ancestor::class:*[1]" />
    <xsl:param name="gs_data_select" />

    <debug:NOT_CURRENTLY_USED because="WILL_BE_USED for the Referential Integrity and Exports of XSDs"/>

    <xsd:schema xml:add-xml-id="no" gs:classes="XSchema">
      <xsl:copy-of select="@*[not(.=../@gs:classes)]"/>
      <xsl:if test="$gs_display_mode and response:can-have-attribute('gs:display-mode')"><xsl:attribute name="gs:display-mode"><xsl:value-of select="$gs_display_mode"/></xsl:attribute></xsl:if>

      <!-- useful for including data information for edit operations
        TODO: database:class-xschema-node-mask() will also call this template
        and calculate the inherited xschema layout twice
      -->
      <xsl:if test="$gs_data_select">
        <xsd:annotation gs:type="form-data">
          <database:with-node-mask select="$gs_data_select" node-mask="database:class-xschema-node-mask($gs_data_select)">
            <xsl:copy-of select="$gs_data_select" />
          </database:with-node-mask>
        </xsd:annotation>
      </xsl:if>

      <xsl:apply-templates select="." mode="gs_inheritance_render_main">
        <xsl:with-param name="gs_inheritance_render_stand_alone" select="$gs_inheritance_render_stand_alone"/>
        <xsl:with-param name="gs_inheritance_render_compiled" select="$gs_inheritance_render_compiled"/>
        <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
      </xsl:apply-templates>

      <!-- top level xsd:app-info -->
      <xsl:apply-templates select="." mode="gs_inheritance_render_directives">
        <xsl:with-param name="gs_inheritance_render_stand_alone" select="$gs_inheritance_render_stand_alone"/>
        <xsl:with-param name="gs_inheritance_render_compiled" select="$gs_inheritance_render_compiled"/>
        <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
      </xsl:apply-templates>

      <!-- xsd:element @type="class:Person" -->
      <xsl:apply-templates select="$gs_class" mode="gs_xsd_dependent_type">
        <xsl:with-param name="gs_inheritance_render_stand_alone" select="$gs_inheritance_render_stand_alone"/>
        <xsl:with-param name="gs_inheritance_render_compiled" select="$gs_inheritance_render_compiled"/>
        <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
        <xsl:with-param name="gs_not_first_class" select="false()"/>
      </xsl:apply-templates>
    </xsd:schema>
  </xsl:template>

  <xsl:template match="xsd:schema" mode="gs_inheritance_render_compliment">
    <xsl:param name="gs_inheritance_render_compiled" select="$gs_inheritance_render_compiled"/>
    <xsl:param name="gs_xschema_name" select="@name"/>
    <xsl:param name="gs_display_mode"/>

    <!-- can be sent through for efficiency -->
    <xsl:param name="gs_class" select="ancestor::class:*[1]" />

    <!-- xsd:complexType only
      directives not relevant: probably for presenting / editing the entire Class layout
      with all inherited fields at least shown
    -->
    <xsl:apply-templates select="." mode="gs_inheritance_render_main">
      <xsl:with-param name="gs_inheritance_render_stand_alone" select="$gs_inheritance_render_stand_alone"/>
      <xsl:with-param name="gs_inheritance_render_compiled" select="$gs_inheritance_render_compiled"/>
      <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="xsd:schema" mode="gs_inheritance_render_main">
    <xsl:param name="gs_inheritance_render_stand_alone"/>
    <xsl:param name="gs_inheritance_render_compiled"/>
    <xsl:param name="gs_xschema_name"/>

    <xsl:apply-templates select="." mode="gs_inheritance_render_comment">
      <xsl:with-param name="gs_inheritance_render_stand_alone" select="$gs_inheritance_render_stand_alone"/>
      <xsl:with-param name="gs_inheritance_render_compiled" select="$gs_inheritance_render_compiled"/>
      <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
    </xsl:apply-templates>

    <xsl:apply-templates select="." mode="gs_xsd_complex_type">
      <xsl:with-param name="gs_inheritance_render_stand_alone" select="$gs_inheritance_render_stand_alone"/>
      <xsl:with-param name="gs_inheritance_render_compiled" select="$gs_inheritance_render_compiled"/>
      <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="xsd:schema" mode="gs_inheritance_render_comment">
    <xsl:param name="gs_inheritance_render_stand_alone"/>
    <xsl:param name="gs_inheritance_render_compiled"/>
    <xsl:param name="gs_xschema_name"/>

    <xsl:variable name="gs_class" select="ancestor::class:*[1]" />

    <xsl:comment>
      ######################################### XSchema for Class__<xsl:value-of select="local-name($gs_class)"/>
      [<xsl:if test="$gs_inheritance_render_stand_alone">stand-alone</xsl:if>][<xsl:if test="$gs_inheritance_render_compiled">compiled</xsl:if>]
      xschema-name:    [<xsl:value-of select="$gs_xschema_name"/>]
      element          [<xsl:value-of select="@elements"/>]
    </xsl:comment>
  </xsl:template>

  <xsl:template match="xsd:schema" mode="gs_inheritance_render_directives">
    <xsl:param name="gs_inheritance_render_stand_alone"/>
    <xsl:param name="gs_inheritance_render_compiled"/>
    <xsl:param name="gs_xschema_name"/>

    <xsl:apply-templates select="." mode="gs_xsd_directive_inheritance">
      <xsl:with-param name="gs_inheritance_render_stand_alone" select="$gs_inheritance_render_stand_alone"/>
      <xsl:with-param name="gs_inheritance_render_compiled" select="$gs_inheritance_render_compiled"/>
      <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
      <xsl:with-param name="gs_type" select="'form-display'"/>
    </xsl:apply-templates>
    <xsl:apply-templates select="." mode="gs_xsd_directive_inheritance">
      <xsl:with-param name="gs_inheritance_render_stand_alone" select="$gs_inheritance_render_stand_alone"/>
      <xsl:with-param name="gs_inheritance_render_compiled" select="$gs_inheritance_render_compiled"/>
      <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
      <xsl:with-param name="gs_type" select="'data-processing'"/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="class:*" mode="gs_xsd_dependent_type">
    <xsl:param name="gs_inheritance_render_stand_alone"/>
    <xsl:param name="gs_inheritance_render_compiled"/>
    <xsl:param name="gs_xschema_name"/>
    <xsl:param name="gs_not_first_class" select="true()"/>

    <!-- recursion down inheritance tree, e.g. class:User -->
    <xsl:if test="not($gs_inheritance_render_compiled = #COMPILED)">
      <xsl:apply-templates select="class:*" mode="gs_xsd_dependent_type">
        <xsl:with-param name="gs_inheritance_render_stand_alone" select="$gs_inheritance_render_stand_alone"/>
        <xsl:with-param name="gs_inheritance_render_compiled" select="$gs_inheritance_render_compiled"/>
        <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
      </xsl:apply-templates>
    </xsl:if>

    <!-- recurse field dependencies, e.g. xsd:element @type="class:Person" -->
    <xsl:apply-templates select="(xsd:element|xsd:attribute)[@type and not(starts-with(@type, 'xsd:'))]" mode="gs_xsd_dependent_field_type">
      <xsl:with-param name="gs_inheritance_render_stand_alone" select="$gs_inheritance_render_stand_alone"/>
      <xsl:with-param name="gs_inheritance_render_compiled" select="$gs_inheritance_render_compiled"/>
      <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
    </xsl:apply-templates>

    <!-- this xsd:complexType output, this will also be compiled if requested! -->
    <xsl:if test="$gs_not_first_class">
      <xsl:apply-templates select="." mode="gs_xsd_complex_type">
        <xsl:with-param name="gs_inheritance_render_stand_alone" select="$gs_inheritance_render_stand_alone"/>
        <xsl:with-param name="gs_inheritance_render_compiled" select="$gs_inheritance_render_compiled"/>
        <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>

  <xsl:template match="xsd:*" mode="gs_xsd_dependent_field_type">
    <xsl:param name="gs_inheritance_render_stand_alone"/>
    <xsl:param name="gs_inheritance_render_compiled"/>
    <xsl:param name="gs_xschema_name"/>

    <xsl:variable name="gs_type_class" select="database:class(@type)" gs:error-policy="continue"/>
    <xsl:if test="not($gs_type_class)"><debug:server-message output="xsd:element @type [{@type}] not found"/></xsl:if>
    <xsl:if test="$gs_type_class">
      <xsl:apply-templates select="$gs_type_class" mode="gs_xsd_dependent_type">
        <xsl:with-param name="gs_inheritance_render_stand_alone" select="$gs_inheritance_render_stand_alone"/>
        <xsl:with-param name="gs_inheritance_render_compiled" select="$gs_inheritance_render_compiled"/>
        <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>

  <xsl:template match="class:*" mode="gs_xsd_directive_inheritance">
    <xsl:param name="gs_inheritance_render_stand_alone"/>
    <xsl:param name="gs_inheritance_render_compiled"/>
    <xsl:param name="gs_xschema_name"/>
    <xsl:param name="gs_type"/>

    <xsl:variable name="gs_xschema" select="xsd:schema[@name = $gs_xschema_name or (not($gs_xschema_name) and not(@name))]"/>
    <xsl:variable name="gs_annotation" select="$gs_xschema/xsd:annotation[@gs:type=$gs_type]"/>

    <xsl:if test="$gs_annotation">
      <xsl:copy-of select="$gs_annotation"/>
    </xsl:if>
    <xsl:if test="not($gs_annotation)">
      <xsl:apply-templates select="class:*" mode="gs_xsd_directive_inheritance">
        <xsl:with-param name="gs_inheritance_render_stand_alone" select="$gs_inheritance_render_stand_alone"/>
        <xsl:with-param name="gs_inheritance_render_compiled" select="$gs_inheritance_render_compiled"/>
        <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
        <xsl:with-param name="gs_type" select="$gs_type"/>
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>

  <xsl:template match="class:*" mode="gs_xsd_complex_type">
    <xsl:param name="gs_inheritance_render_stand_alone"/>
    <xsl:param name="gs_inheritance_render_compiled"/>
    <xsl:param name="gs_xschema_name"/>

    <xsd:complexType name="{name()}" xml:add-xml-id="no">
      <xsl:attribute name="xml:id">gs-xsd-complex-type-<xsl:value-of select="local-name()"/></xsl:attribute>
      <xsd:sequence xml:add-xml-id="no">
        <xsl:apply-templates select="." mode="gs_xsd_field_inheritance">
          <xsl:with-param name="gs_inheritance_render_stand_alone" select="$gs_inheritance_render_stand_alone"/>
          <xsl:with-param name="gs_inheritance_render_compiled" select="$gs_inheritance_render_compiled"/>
          <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
          <xsl:with-param name="gs_main_class" select="."/>
        </xsl:apply-templates>
      </xsd:sequence>
    </xsd:complexType>
  </xsl:template>

  <xsl:template match="class:*" mode="gs_xsd_field_inheritance">
    <!-- already within the xsd:complexType/xsd:sequence -->
    <xsl:param name="gs_inheritance_render_stand_alone"/>
    <xsl:param name="gs_inheritance_render_compiled"/>
    <xsl:param name="gs_xschema_name"/>
    <xsl:param name="gs_main_class"/>

    <xsl:variable name="gs_xschema" select="xsd:schema[@name = $gs_xschema_name or (not($gs_xschema_name) and not(@name))]"/>
    <xsl:variable name="gs_sequence" select="$gs_xschema/xsd:complexType/xsd:sequence"/>

    <xsl:if test="class:*">
      <xsl:if test="$gs_inheritance_render_compiled = #EXTENSION">
        <xsd:extension base="{local-name(class:*)}" xml:add-xml-id="no"/>
      </xsl:if>

      <xsl:if test="$gs_inheritance_render_compiled = #COMPILED">
        <!-- RECURSIVE field inheritance -->
        <xsl:apply-templates select="class:*" mode="gs_xsd_field_inheritance">
          <!-- use a param because of hardlinking possibility -->
          <xsl:with-param name="gs_inheritance_render_stand_alone" select="$gs_inheritance_render_stand_alone"/>
          <xsl:with-param name="gs_inheritance_render_compiled" select="$gs_inheritance_render_compiled"/>
          <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
          <xsl:with-param name="gs_main_class" select="$gs_main_class"/>
        </xsl:apply-templates>
      </xsl:if>
    </xsl:if>

    <xsl:apply-templates select="$gs_sequence/xsd:*" mode="gs_xsd_field_inheritance_output">
      <!-- use a param because of hardlinking possibility -->
      <xsl:with-param name="gs_inheritance_render_stand_alone" select="$gs_inheritance_render_stand_alone"/>
      <xsl:with-param name="gs_inheritance_render_compiled" select="$gs_inheritance_render_compiled"/>
      <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
      <xsl:with-param name="gs_main_class" select="$gs_main_class"/>
      <xsl:with-param name="gs_current_class" select="."/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="xsd:*" mode="gs_xsd_field_inheritance_output">
    <xsl:param name="gs_inheritance_render_stand_alone"/>
    <xsl:param name="gs_inheritance_render_compiled"/>
    <xsl:param name="gs_xschema_name"/>
    <xsl:param name="gs_main_class"/>
    <xsl:param name="gs_current_class"/>

    <xsl:variable name="gs_class_definitions" select="database:classes()"/>
    <xsl:variable name="gs_base_class_definitions" select="database:base-classes-all()"/>

    <xsl:copy>
      <xsl:copy-of select="@*[not(self::gs:class = . or self::gs:inherited = .)]"/>
      <xsl:attribute name="gs:class"><xsl:value-of select="local-name($gs_current_class)"/></xsl:attribute>
      <xsl:if test="not(database:has-same-node($gs_main_class, $gs_current_class))"><xsl:attribute name="gs:inherited">true</xsl:attribute></xsl:if>
      <xsl:if test="$gs_class_definitions">
        <xsl:attribute name="gs:classes"><xsl:apply-templates select="$gs_class_definitions" mode="gs_data_render_classes"/></xsl:attribute>
        <xsl:attribute name="gs:base-classes"><xsl:apply-templates select="$gs_base_class_definitions" mode="gs_data_render_classes"/></xsl:attribute>
      </xsl:if>

      <xsl:copy-of select="*"/>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="class:*" mode="gs_data_render_classes">
    <xsl:value-of select="local-name()"/>
    <xsl:if test="not(position() = last())">,</xsl:if>
  </xsl:template>


  <!-- ##############################################################################################
    database:class-xschema-node-mask([node, display-mode, class])
    database:class-xschema(object:Manager,'full')

    THIS is a transform: it outputs a COPY of the nodes
    so it cannot be used as a node-mask applied to an existing node
    applies the compiled stand-alone inherited class-xschema() for the class of this node
    the xschema can contain @gs:display-mode indicators that restrict elements based on the display-mode
      <xsd:element gs:display-modes="full,summary" />
      <xsd:element gs:display-modes-except="summary" />
    we take advantage of hooks in name::class_xschema
  -->
  <!-- nodes aren't declared: xsl:param name="gs_input_node"/ -->
  <!-- let's deal with this later!: xsl:param name="gs_xschema_name" select="$gs_xschema_name" / -->

  <xsl:param name="gs_debug_xsd_node_mask" select="false()"/>

  <xsl:template match="class:*" mode="gs_xsd_node_mask">
    <xsl:param name="gs_input_node" select="$gs_input_node"/>
    <xsl:param name="gs_display_mode"/>

    <debug:server-message if="$gs_debug_xsd_node_mask" output="{local-name()} node-mask:"/>

    <database:select-input-node select="$gs_input_node"/>
    <xsl:element name="{local-name($gs_input_node)}" namespace="{namespace-uri($gs_input_node)}">
      <xsl:copy-of select="@xml:id"/>
      <xsl:if test="$gs_display_mode"><xsl:attribute name="gs:display_mode"><xsl:value-of select="$gs_display_mode"/></xsl:attribute></xsl:if>

      <xsl:apply-templates select="." mode="gs_xsd_node_mask_field_inheritance">
        <xsl:with-param name="gs_input_node" select="$gs_input_node"/>
        <xsl:with-param name="gs_display_mode" select="$gs_display_mode"/>
      </xsl:apply-templates>
    </xsl:element>
  </xsl:template>

  <xsl:template match="class:*" mode="gs_xsd_node_mask_field_inheritance">
    <xsl:param name="gs_input_node"/>
    <xsl:param name="gs_display_mode"/>

    <xsl:variable name="gs_xschema" select="xsd:schema[not(@name) or @name = '']"/>

    <!-- recurse down first -->
    <xsl:apply-templates select="class:*" mode="gs_xsd_node_mask_field_inheritance">
      <xsl:with-param name="gs_input_node" select="$gs_input_node"/>
      <xsl:with-param name="gs_display_mode" select="$gs_display_mode"/>
    </xsl:apply-templates>

    <!-- fields: if there are any at this level -->
    <debug:server-message if="$gs_debug_xsd_node_mask" output="-- processing {local-name()} fields"/>
    <xsl:apply-templates select="$gs_xschema/xsd:complexType/xsd:sequence/xsd:*" mode="gs_xsd_node_mask_fields">
      <xsl:with-param name="gs_input_node" select="$gs_input_node"/>
      <xsl:with-param name="gs_display_mode" select="$gs_display_mode"/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="xsd:any" mode="gs_xsd_node_mask_fields">
    <xsl:param name="gs_input_node"/>
    <xsl:param name="gs_display_mode"/>

    <xsl:variable name="gs_xsd_any" select="."/>

    <xsl:if test="(not(@gs:display-modes) or ($gs_display_mode and contains(@gs:display-modes, $gs_display_mode))) and (not(@gs:display-modes-except) or not($gs_display_mode) or not(contains(@gs:display-modes-except, $gs_display_mode)))">
      <xsl:for-each select="$gs_input_node/*">
        <xsl:apply-templates select="$gs_xsd_any" mode="gs_xsd_node_mask_fields_output">
          <xsl:with-param name="gs_element" select="."/>
          <xsl:with-param name="gs_display_mode" select="$gs_display_mode"/>
        </xsl:apply-templates>
      </xsl:for-each>
    </xsl:if>
  </xsl:template>

  <xsl:template match="xsd:element" mode="gs_xsd_node_mask_fields">
    <xsl:param name="gs_input_node"/>
    <xsl:param name="gs_display_mode"/>

    <xsl:variable name="gs_name" select="@name"/>
    <xsl:variable name="gs_elements" select="$gs_input_node/*[name() = $gs_name]"/>
    <xsl:variable name="gs_xsd_element" select="."/>

    <xsl:if test="$gs_elements">
      <xsl:if test="(not(@gs:display-modes) or ($gs_display_mode and contains(@gs:display-modes, $gs_display_mode))) and (not(@gs:display-modes-except) or not($gs_display_mode) or not(contains(@gs:display-modes-except, $gs_display_mode)))">
        <xsl:for-each select="$gs_elements">
          <xsl:apply-templates select="$gs_xsd_element" mode="gs_xsd_node_mask_fields_output">
            <xsl:with-param name="gs_element" select="."/>
            <xsl:with-param name="gs_display_mode" select="$gs_display_mode"/>
          </xsl:apply-templates>
        </xsl:for-each>
      </xsl:if>
    </xsl:if>
    <xsl:if test="not($gs_elements)">
      <debug:server-message if="$gs_debug_xsd_node_mask" output="[{$gs_name}] not found"/>
    </xsl:if>
  </xsl:template>

  <xsl:template match="xsd:attribute" mode="gs_xsd_node_mask_fields">
    <xsl:param name="gs_input_node"/>
    <xsl:param name="gs_display_mode"/>

    <xsl:variable name="gs_name" select="@name"/>
    <xsl:variable name="gs_attribute" select="$gs_input_node/@*[name() = $gs_name]"/>

    <xsl:if test="not($gs_name = 'xml:id' or $gs_name = 'gs:display-mode')">
      <xsl:if test="(not(@gs:display-modes) or ($gs_display_mode and contains(@gs:display-modes, $gs_display_mode))) and (not(@gs:display-modes-except) or not($gs_display_mode) or not(contains(@gs:display-modes-except, $gs_display_mode)))">
        <xsl:if test="$gs_attribute">
          <debug:server-message if="$gs_debug_xsd_node_mask" output="[@{$gs_name}] =&gt; [{$gs_attribute}]"/>
          <!-- if xsl:element tries to create an un-registered namespace prefix it will fail -->
          <xsl:attribute name="{local-name($gs_attribute)}" namespace="{namespace-uri($gs_attribute)}"><xsl:value-of select="$gs_attribute"/></xsl:attribute>
        </xsl:if>
        <xsl:if test="not($gs_attribute)">
          <debug:server-message if="$gs_debug_xsd_node_mask" output="[@{$gs_name}] not found"/>
        </xsl:if>
      </xsl:if>
    </xsl:if>
  </xsl:template>

  <xsl:template match="xsd:*" mode="gs_xsd_node_mask_fields_output">
    <xsl:param name="gs_element"/>
    <xsl:param name="gs_display_mode"/>

    <debug:server-message if="$gs_debug_xsd_node_mask" output="[{name($gs_element)}]"/>
    <database:select-input-node select="$gs_element"/>
    <!-- if xsl:element tries to create an un-registered namespace prefix it will fail -->
    <xsl:element name="{local-name($gs_element)}" namespace="{namespace-uri($gs_element)}">
      <!-- sub-attributes first -->
      <xsl:apply-templates select="xsd:attribute" mode="gs_xsd_node_mask_fields">
        <xsl:with-param name="gs_input_node" select="$gs_element"/>
        <xsl:with-param name="gs_display_mode" select="$gs_display_mode"/>
      </xsl:apply-templates>

      <xsl:copy-of select="@xml:id"/>
      <xsl:if test="$gs_display_mode"><xsl:attribute name="gs:display_mode"><xsl:value-of select="$gs_display_mode"/></xsl:attribute></xsl:if>

      <!-- text and comments, after the attributes -->
      <xsl:copy-of select="$gs_element/text()"/>
      <xsl:copy-of select="$gs_element/comment()"/>

      <!-- sub-elements -->
      <xsl:apply-templates select="xsd:element" mode="gs_xsd_node_mask_fields">
        <xsl:with-param name="gs_input_node" select="$gs_element"/>
        <xsl:with-param name="gs_display_mode" select="$gs_display_mode"/>
      </xsl:apply-templates>
    </xsl:element>
  </xsl:template>
</xsl:stylesheet>