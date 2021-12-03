<xsl:stylesheet xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" response:server-side-only="true" name="inheritance_render" version="1.0" extension-element-prefixes="flow database str debug request regexp repository">
  <!-- ##############################################################################################
    INHERITANCE_STAND_ALONE true()
    INHERITANCE_COMPLIMENT  false()
    INHERITANCE_COMPILED    true()
    INHERITANCE_EXTENSION   false()

    database:class-xschema([string className|node class:*, xschema-name, ($INHERITANCE_STAND_ALONE | $INHERITANCE_COMPLIMENT), ($INHERITANCE_EXTENSION | $INHERITANCE_COMPILED)])
    database:class-xschema(~Manager, '', $INHERITANCE_COMPLIMENT, $INHERITANCE_COMPILED)   => html form XSD editor
    database:class-xschema(~Person,  '', $INHERITANCE_STAND_ALONE, $INHERITANCE_EXTENSION) => export a full XSD check schema

    compile the pseudo xschema for a class taking into account it's xsd inheritance chain
    <class:Manager>
      <class:User>...</class:User>  [usually hardlinked] base class
      <xsd:schema>...</xsd:schema>
    </class:Manager>

    xschema-name:
      Classes have ONE xschema. other, named xschemas, are for other activities, e.g. login, property controls
      this XSL does not currently work on named schemas

    only $INHERITANCE_COMPLIMENT $INHERITANCE_COMPILED IS_USED at the moment for compiled XSD editing
    DRI is NOT_CURRENTLY_USED

    ($INHERITANCE_STAND_ALONE | $INHERITANCE_COMPLIMENT)
      controls whether the dependent complexTypes are included in the same single schema or referenced,
        e.g. <xsd:element type="User" => the User complexType
      $INHERITANCE_STAND_ALONE is NOT_CURRENTLY_USED but WILL_BE_USED for the Referential Integrity and Exports of XSDs
      $INHERITANCE_COMPLIMENT therefore means that the other XSD definitions for the xsd:element types need to be requested separately
      $INHERITANCE_COMPLIMENT IS_USED for XSD editing because the dependent complexTypes are stated, their details not shown

    ($INHERITANCE_EXTENSION | $INHERITANCE_COMPILED)
      $INHERITANCE_COMPILED indicates that all inherited field definitions need to be compiled in to one linear XSD sequence
        gs_xsd_field_inheritance is used in to one linear xsd:complexType/(xsd:element|xsd:attribute)
        in the order stated in the root class xsd:annotation @meta:type=form-display / xsd:documentation @meta:type=order
        USED by the xschema editor to present all the relevant inherited fields
        meta:inherited and meta:base-class are stamped on the xsd:elements to indicate easily their source
        WILL_BE_USED also when generating / validating / creating new objects
      $INHERITANCE_EXTENSION indicates that inheritance should use the natural xsd:extension inheritance system
        it is also true that the XSD editor could navigate the xsd:extensions to display inheritence information
        NOT_CURRENTLY_USED but WILL_BE_USED for Referential Integrity

    the highest level singular directives take precedence
      xsd:annotation @meta:type=form-display
      xsd:annotation @meta:type=data-processing


    ############## FLOW:
    gs_inheritance_render_xsd:      $INHERITANCE_STAND_ALONE (xsd:schema, gs_inheritance_render_xschema_directives, gs_inheritance_render_xschema_dependent_types => gs_xsd_complex_type)
    gs_inheritance_render_xschema_main: gs_inheritance_render_xschema_comment, gs_xsd_complex_type
    gs_xsd_complex_type: xsd:complexType/xsd:sequence => gs_xsd_field_inheritance
    gs_xsd_field_inheritance: $INHERITANCE_COMPILED gs_xsd_field_inheritance or $INHERITANCE_EXTENSION xsd:extension, gs_xsd_field_inheritance_output
    gs_inheritance_render_xschema_dependent_types:
  -->
  <xsl:include xpath="~AJAXHTMLLoader/interface_render"/>
  <xsl:include xpath="~Interface/interface_render"/>
  <xsl:include xpath="/object:Server[1]/repository:system_transforms[1]/interface_render"/>

  <!-- TODO: global inheritance params are only here because GS cannot send params directly to templates currently -->
  <xsl:param name="gs_inheritance_render_xschema_compiled" select="false()"/>
  <xsl:param name="gs_xschema_name"/>
  
  <xsl:template match="class:*" mode="gs_inheritance_render_xschema_standalone">
    <!-- also renders dependent types -->
    <xsl:param name="gs_inheritance_render_xschema_compiled" select="$gs_inheritance_render_xschema_compiled"/>
    <xsl:param name="gs_xschema_name" select="$gs_xschema_name"/>
    <xsl:param name="gs_data_select"/>
    <xsl:param name="gs_interface_mode"/>
    <xsl:param name="gs_display_class"/>
    <xsl:param name="gs_documentation_fields"/> <!-- e.g. @destination -->

    <debug:NOT_CURRENTLY_USED because="WILL_BE_USED for the Referential Integrity and Exports of XSDs"/>

    <xsl:variable name="gs_xschema" select="xsd:schema[@name = $gs_xschema_name or (not($gs_xschema_name) and not(@name))]"/>

    <xsd:schema>
      <xsl:apply-templates select="$gs_xschema" mode="gs_interface_render_adorn_node">
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
        <xsl:with-param name="gs_display_class" select="$gs_display_class"/>
      </xsl:apply-templates>

      <!-- TODO: add in the xsd:documentation stuff -->

      <!-- comments and xsd:complexType -->
      <xsl:apply-templates select="." mode="gs_inheritance_render_xschema_main">
        <xsl:with-param name="gs_inheritance_render_xschema_standalone" select="true()"/>
        <xsl:with-param name="gs_inheritance_render_xschema_compiled" select="$gs_inheritance_render_xschema_compiled"/>
        <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
        <xsl:with-param name="gs_xschema" select="$gs_xschema"/>
      </xsl:apply-templates>

      <!-- top level xsd:app-info -->
      <xsl:apply-templates select="." mode="gs_inheritance_render_xschema_directives">
        <xsl:with-param name="gs_inheritance_render_xschema_standalone" select="true()"/>
        <xsl:with-param name="gs_inheritance_render_xschema_compiled" select="$gs_inheritance_render_xschema_compiled"/>
        <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
        <xsl:with-param name="gs_xschema" select="$gs_xschema"/>
      </xsl:apply-templates>

      <!-- xsd:element @type="class:Person" -->
      <xsl:apply-templates select="." mode="gs_xsd_dependent_type">
        <xsl:with-param name="gs_inheritance_render_xschema_standalone" select="true()"/>
        <xsl:with-param name="gs_inheritance_render_xschema_compiled" select="$gs_inheritance_render_xschema_compiled"/>
        <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
        <xsl:with-param name="gs_xschema" select="$gs_xschema"/>
        <xsl:with-param name="gs_not_first_class" select="false()"/>
      </xsl:apply-templates>
    </xsd:schema>
  </xsl:template>

  <xsl:template match="class:*" mode="gs_inheritance_render_xschema_complement">
    <!-- xsd:schema with only one xsd:complexType also:
      xsd:annotation for type info
      xsd:annotation for accompanying edit data
      if user requires a <xsd:schema> wrapper then must create it themselves
      does not render dependent types
      directives not relevant: probably for Integrity check, not for specific organisation
    -->
    <xsl:param name="gs_inheritance_render_xschema_standalone" select="false()"/>
    <xsl:param name="gs_inheritance_render_xschema_compiled" select="$gs_inheritance_render_xschema_compiled"/>
    <xsl:param name="gs_xschema_name" select="$gs_xschema_name"/>
    <xsl:param name="gs_data_select"/>
    <xsl:param name="gs_interface_mode"/>
    <xsl:param name="gs_display_class"/>
    <xsl:param name="gs_documentation_fields"/> <!-- e.g. @destination -->

    <!-- xsd:schema maybe NULL for this Class. however, it may exist in class-bases -->
    <xsl:variable name="gs_xschema" select="xsd:schema[@name = $gs_xschema_name or (not($gs_xschema_name) and not(@name))]"/>

    <xsd:schema>
      <xsl:apply-templates select="$gs_xschema" mode="gs_interface_render_adorn_node">
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
        <xsl:with-param name="gs_display_class" select="$gs_display_class"/>
      </xsl:apply-templates>
      
      <!-- FLEXIBLE [$gs_interface_mode dependent] data-queries (normal gs_interface_render style):
        classes can require additional data for their data elements during output
        allows each class to apply DYNAMIC data-queries for it's data element response
        called during gs_view_render_data data output for each element
        Class xsl:include interface_render.xsl is inappropriate because it must be xsl:include
          and has more difficulty using database:query
        @interface-render-output=placeholder will suppress this
      -->
      <database:without-node-mask hardlink-policy="suppress-exact-repeat">
        <xsl:variable name="gs_data_queries_include" select="~XSchema/repository:data-queries/*[not(@meta:data-query-type)]"/>

        <xsl:if test="$gs_data_queries_include">
          <xsd:annotation meta:type="data-queries">
            <!-- recommended use @data-container-name xsd:app-info or xsd:documentation here -->
            <xsl:apply-templates select="$gs_data_queries_include" mode="gs_interface_render">
              <xsl:with-param name="gs_outer_interface_mode" select="$gs_interface_mode"/>
              <xsl:with-param name="gs_current" select="."/>
              <xsl:with-param name="gs_class_definitions" select="~XSchema"/>
            </xsl:apply-templates>
          </xsd:annotation>
        </xsl:if>
      </database:without-node-mask>

      <!-- xsd:documentation dynamic configurable
        ===== Always included dynamic data:
        class:*/xsd:schema location: class/xschema-name
        generation style:            standalone/compiled
        edit data (node-set)         edit-data
        class-name

        === Common optional
        destination
        name
      -->
      <xsd:annotation meta:type="context">
        <xsl:comment>
          Dynamic context information for processing of the entered data.
          for example: the destination for the new data, or the Class to create
          Should be passed back to the server with the data,
          normally in the HTML world it would be rendered in input @type=hidden.
          ALL xsd:annotation/xsd:documentation should be rendered and passed back
        </xsl:comment>
        <xsd:documentation meta:type="is-standalone" meta:direct-render="no"><xsl:value-of select="$gs_inheritance_render_xschema_standalone"/></xsd:documentation>
        <xsd:documentation meta:type="is-compiled" meta:direct-render="no"><xsl:value-of select="$gs_inheritance_render_xschema_compiled"/></xsd:documentation>

        <xsl:if test="$gs_documentation_fields">
          <xsl:apply-templates select="$gs_documentation_fields" mode="gs_inheritance_render_xschema_documentation_fields"/>
        </xsl:if>

        <!-- TODO: database:class-xschema-node-mask() will also call this template to render the xschema @data -->
        <xsl:if test="$gs_data_select">
          <xsd:documentation meta:type="edit-data" meta:direct-render="no">
            <xsl:comment>
              Edit data. Values should be mapped to the xsd:complexType layout below for editing.
              xsd:documentation @meta:type=xpath-to-destination required for where to overwrite the values.
            </xsl:comment>
            <database:with-node-mask select="$gs_data_select" node-mask=".|*">
              <xsl:copy-of select="$gs_data_select"/>
            </database:with-node-mask>
          </xsd:documentation>
        </xsl:if>
      </xsd:annotation>

      <!-- take the first inherited xsl:annotation[types] in the self-or-base classes:
        @meta:type=data-processing:
          processing @meta:dom-method, ...
          inputs @meta:settings-mode=*
          required-documentation xsd:complexType

        @meta:type=form-display:
          title, submit-caption, orientation, style, saving-interaction
          order (NOT_COMPLETE)
      -->
      <xsl:apply-templates select="." mode="gs_inheritance_render_xschema_directives">
        <xsl:with-param name="gs_inheritance_render_xschema_standalone" select="true()"/>
        <xsl:with-param name="gs_inheritance_render_xschema_compiled" select="$gs_inheritance_render_xschema_compiled"/>
        <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
        <xsl:with-param name="gs_xschema" select="$gs_xschema"/>
      </xsl:apply-templates>

      <!-- xsd:complexType and xsl:comment -->
      <xsl:apply-templates select="." mode="gs_inheritance_render_xschema_main">
        <xsl:with-param name="gs_inheritance_render_xschema_standalone" select="false()"/>
        <xsl:with-param name="gs_inheritance_render_xschema_compiled" select="$gs_inheritance_render_xschema_compiled"/>
        <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
        <xsl:with-param name="gs_xschema" select="$gs_xschema"/>
      </xsl:apply-templates>
    </xsd:schema>
  </xsl:template>

  <xsl:template match="*" mode="gs_type_info_names">
    <xsl:copy/>
  </xsl:template>

  <xsl:template match="*|@*" mode="gs_inheritance_render_xschema_documentation_fields">
    <!-- dynamic fields only. use xsd:app-info for fixed processing information
      these will be rendered in the actual input form for return to the server
    -->
    <xsd:documentation meta:type="{local-name()}">
      <xsl:apply-templates select="." mode="gs_serialised_value"/>
    </xsd:documentation>
  </xsl:template>

  <xsl:template match="class:*" mode="gs_inheritance_render_xschema_main">
    <xsl:param name="gs_inheritance_render_xschema_standalone"/>
    <xsl:param name="gs_inheritance_render_xschema_compiled"/>
    <xsl:param name="gs_xschema_name"/>
    <xsl:param name="gs_xschema"/>

    <xsl:apply-templates select="." mode="gs_inheritance_render_xschema_comment">
      <xsl:with-param name="gs_inheritance_render_xschema_standalone" select="$gs_inheritance_render_xschema_standalone"/>
      <xsl:with-param name="gs_inheritance_render_xschema_compiled" select="$gs_inheritance_render_xschema_compiled"/>
      <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
      <xsl:with-param name="gs_xschema" select="$gs_xschema"/>
    </xsl:apply-templates>

    <xsl:apply-templates select="." mode="gs_xsd_complex_type">
      <xsl:with-param name="gs_inheritance_render_xschema_standalone" select="$gs_inheritance_render_xschema_standalone"/>
      <xsl:with-param name="gs_inheritance_render_xschema_compiled" select="$gs_inheritance_render_xschema_compiled"/>
      <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
      <xsl:with-param name="gs_xschema" select="$gs_xschema"/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="class:*" mode="gs_inheritance_render_xschema_comment">
    <xsl:param name="gs_inheritance_render_xschema_standalone"/>
    <xsl:param name="gs_inheritance_render_xschema_compiled"/>
    <xsl:param name="gs_xschema_name"/>
    <xsl:param name="gs_xschema"/>

    <xsl:comment>
      ######################################### XSchema for Class__<xsl:value-of select="local-name()"/>
      [<xsl:if test="$gs_inheritance_render_xschema_standalone">standalone</xsl:if>][<xsl:if test="$gs_inheritance_render_xschema_compiled">compiled</xsl:if>]
      xschema-name:    [<xsl:value-of select="$gs_xschema_name"/>]
      base classes:    [<xsl:value-of select="str:list(database:base-classes-all(local-name()))"/>]
      element          [<xsl:value-of select="@elements"/>]
    </xsl:comment>
  </xsl:template>

  <xsl:template match="class:*" mode="gs_inheritance_render_xschema_directives">
    <xsl:param name="gs_inheritance_render_xschema_standalone"/>
    <xsl:param name="gs_inheritance_render_xschema_compiled"/>
    <xsl:param name="gs_xschema_name"/>
    <xsl:param name="gs_xschema"/>

    <!-- xsl:copy-of the FIRST inherited xsl:annotation[form-display] in the self-or-base classes -->
    <xsl:apply-templates select="." mode="gs_xsd_directive_inheritance">
      <xsl:with-param name="gs_inheritance_render_xschema_standalone" select="$gs_inheritance_render_xschema_standalone"/>
      <xsl:with-param name="gs_inheritance_render_xschema_compiled" select="$gs_inheritance_render_xschema_compiled"/>
      <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
      <xsl:with-param name="gs_xschema" select="$gs_xschema"/>
      <xsl:with-param name="gs_type" select="'form-display'"/>
    </xsl:apply-templates>

    <!-- xsl:copy-of the FIRST inherited xsl:annotation[data-processing] in the self-or-base classes -->
    <xsl:apply-templates select="." mode="gs_xsd_directive_inheritance">
      <xsl:with-param name="gs_inheritance_render_xschema_standalone" select="$gs_inheritance_render_xschema_standalone"/>
      <xsl:with-param name="gs_inheritance_render_xschema_compiled" select="$gs_inheritance_render_xschema_compiled"/>
      <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
      <xsl:with-param name="gs_xschema" select="$gs_xschema"/>
      <xsl:with-param name="gs_type" select="'data-processing'"/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="class:*" mode="gs_xsd_dependent_type">
    <xsl:param name="gs_inheritance_render_xschema_standalone"/>
    <xsl:param name="gs_inheritance_render_xschema_compiled"/>
    <xsl:param name="gs_xschema_name"/>
    <xsl:param name="gs_not_first_class" select="true()"/>

    <!-- recursion down inheritance tree, e.g. class:User -->
    <xsl:if test="not($gs_inheritance_render_xschema_compiled = true())">
      <xsl:apply-templates select="class:*" mode="gs_xsd_dependent_type">
        <xsl:with-param name="gs_inheritance_render_xschema_standalone" select="$gs_inheritance_render_xschema_standalone"/>
        <xsl:with-param name="gs_inheritance_render_xschema_compiled" select="$gs_inheritance_render_xschema_compiled"/>
        <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
      </xsl:apply-templates>
    </xsl:if>

    <!-- recurse field dependencies, e.g. xsd:element @type="class:Person" -->
    <xsl:apply-templates select="(xsd:element|xsd:attribute)[@type and not(starts-with(@type, 'xsd:'))]" mode="gs_xsd_dependent_field_type">
      <xsl:with-param name="gs_inheritance_render_xschema_standalone" select="$gs_inheritance_render_xschema_standalone"/>
      <xsl:with-param name="gs_inheritance_render_xschema_compiled" select="$gs_inheritance_render_xschema_compiled"/>
      <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
    </xsl:apply-templates>

    <!-- this xsd:complexType output, this will also be compiled if requested! -->
    <xsl:if test="$gs_not_first_class">
      <xsl:apply-templates select="." mode="gs_xsd_complex_type">
        <xsl:with-param name="gs_inheritance_render_xschema_standalone" select="$gs_inheritance_render_xschema_standalone"/>
        <xsl:with-param name="gs_inheritance_render_xschema_compiled" select="$gs_inheritance_render_xschema_compiled"/>
        <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>

  <xsl:template match="xsd:*[@type]" mode="gs_xsd_dependent_field_type">
    <xsl:param name="gs_inheritance_render_xschema_standalone"/>
    <xsl:param name="gs_inheritance_render_xschema_compiled"/>
    <xsl:param name="gs_xschema_name"/>

    <xsl:variable name="gs_type_class" select="database:class(@type)" xsl:error-policy="continue"/>
    <xsl:if test="not($gs_type_class)"><debug:server-message output="xsd:element @type [{@type}] not found"/></xsl:if>
    <xsl:if test="$gs_type_class">
      <xsl:apply-templates select="$gs_type_class" mode="gs_xsd_dependent_type">
        <xsl:with-param name="gs_inheritance_render_xschema_standalone" select="$gs_inheritance_render_xschema_standalone"/>
        <xsl:with-param name="gs_inheritance_render_xschema_compiled" select="$gs_inheritance_render_xschema_compiled"/>
        <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>

  <xsl:template match="class:*" mode="gs_xsd_directive_inheritance">
    <xsl:param name="gs_inheritance_render_xschema_standalone"/>
    <xsl:param name="gs_inheritance_render_xschema_compiled"/>
    <xsl:param name="gs_xschema_name"/>
    <xsl:param name="gs_type"/>

    <xsl:variable name="gs_xschema" select="xsd:schema[@name = $gs_xschema_name or (not($gs_xschema_name) and not(@name))]"/>
    <xsl:variable name="gs_annotation" select="$gs_xschema/xsd:annotation[@meta:type=$gs_type]"/>

    <!-- no xsl:copy-of here because we need to render the {} values
      e.g. submit-caption="add a {local-name(ancestor::class:*[1])}"
      otherwise descend to the next base class to check for this xsl:annotation[type]
    -->
    <xsl:apply-templates select="$gs_annotation" mode="gs_xsd_directive_inheritance_render"/>
    <xsl:if test="not($gs_annotation)">
      <xsl:apply-templates select="class:*" mode="gs_xsd_directive_inheritance">
        <xsl:with-param name="gs_inheritance_render_xschema_standalone" select="$gs_inheritance_render_xschema_standalone"/>
        <xsl:with-param name="gs_inheritance_render_xschema_compiled" select="$gs_inheritance_render_xschema_compiled"/>
        <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
        <xsl:with-param name="gs_type" select="$gs_type"/>
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>

  <xsl:template match="xsd:annotation" mode="gs_xsd_directive_inheritance_render">
    <xsl:copy>
      <!-- we need to render the {} values
        @xml:id is REQUIRED in these situations to link back to the server side processing directives
        @meta:type is also REQUIRED
        other @attributes like @client-side-only are useful
      -->
      <xsl:apply-templates select="." mode="gs_interface_render_adorn_node"/>
      <xsl:apply-templates select="@*" mode="gs_interface_render"/>
      <xsl:apply-templates select="*" mode="gs_xsd_directive_inheritance_render"/>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="xsd:app-info|xsd:documentation" mode="gs_xsd_directive_inheritance_render">
    <xsl:copy>
      <xsl:apply-templates select="@*" mode="gs_interface_render"/>
      <xsl:apply-templates select="text()" mode="gs_interface_render"/>
      <xsl:copy-of select="*"/>
    </xsl:copy>
  </xsl:template>

  <xsl:template match="class:*" mode="gs_xsd_complex_type">
    <xsl:param name="gs_inheritance_render_xschema_standalone"/>
    <xsl:param name="gs_inheritance_render_xschema_compiled"/>
    <xsl:param name="gs_xschema_name"/>

    <xsd:complexType name="{name()}" xml:add-xml-id="no">
      <xsl:attribute name="xml:id">gs-xsd-complex-type-<xsl:value-of select="local-name()"/></xsl:attribute>
      <xsd:sequence xml:add-xml-id="no">
        <xsl:apply-templates select="." mode="gs_xsd_field_inheritance">
          <xsl:with-param name="gs_inheritance_render_xschema_standalone" select="$gs_inheritance_render_xschema_standalone"/>
          <xsl:with-param name="gs_inheritance_render_xschema_compiled" select="$gs_inheritance_render_xschema_compiled"/>
          <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
          <xsl:with-param name="gs_main_class" select="."/>
        </xsl:apply-templates>
      </xsd:sequence>
    </xsd:complexType>
  </xsl:template>

  <xsl:template match="class:*" mode="gs_xsd_field_inheritance">
    <!-- already within the xsd:complexType/xsd:sequence -->
    <xsl:param name="gs_inheritance_render_xschema_standalone"/>
    <xsl:param name="gs_inheritance_render_xschema_compiled"/>
    <xsl:param name="gs_xschema_name"/>
    <xsl:param name="gs_main_class" select="."/>

    <xsl:variable name="gs_xschema" select="xsd:schema[@name = $gs_xschema_name or (not($gs_xschema_name) and not(@name))]"/>
    <xsl:variable name="gs_sequence" select="$gs_xschema/xsd:complexType/xsd:sequence"/>

    <xsl:if test="class:*">
      <xsl:if test="$gs_inheritance_render_xschema_compiled = false()">
        <xsd:extension base="{local-name(class:*)}" xml:add-xml-id="no"/>
      </xsl:if>

      <xsl:if test="$gs_inheritance_render_xschema_compiled = true()">
        <!-- RECURSIVE field inheritance -->
        <xsl:apply-templates select="class:*" mode="gs_xsd_field_inheritance">
          <!-- use a param because of hardlinking possibility -->
          <xsl:with-param name="gs_inheritance_render_xschema_standalone" select="$gs_inheritance_render_xschema_standalone"/>
          <xsl:with-param name="gs_inheritance_render_xschema_compiled" select="$gs_inheritance_render_xschema_compiled"/>
          <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
          <xsl:with-param name="gs_main_class" select="$gs_main_class"/>
        </xsl:apply-templates>
      </xsl:if>
    </xsl:if>

    <xsl:apply-templates select="$gs_sequence/xsd:*" mode="gs_xsd_field_inheritance_output">
      <!-- use a param because of hardlinking possibility -->
      <xsl:with-param name="gs_inheritance_render_xschema_standalone" select="$gs_inheritance_render_xschema_standalone"/>
      <xsl:with-param name="gs_inheritance_render_xschema_compiled" select="$gs_inheritance_render_xschema_compiled"/>
      <xsl:with-param name="gs_xschema_name" select="$gs_xschema_name"/>
      <xsl:with-param name="gs_main_class" select="$gs_main_class"/>
      <xsl:with-param name="gs_current_class" select="."/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="xsd:element|xsd:attribute|xsd:any" mode="gs_xsd_field_inheritance_output">
    <xsl:param name="gs_inheritance_render_xschema_standalone"/>
    <xsl:param name="gs_inheritance_render_xschema_compiled"/>
    <xsl:param name="gs_xschema_name"/>
    <xsl:param name="gs_main_class"/>
    <xsl:param name="gs_current_class"/>

    <xsl:copy>
      <xsl:apply-templates select="." mode="gs_interface_render_adorn_node"/>
      <xsl:attribute name="meta:class"><xsl:value-of select="local-name($gs_current_class)"/></xsl:attribute>
      <xsl:if test="not(database:has-same-node($gs_main_class, $gs_current_class))"><xsl:attribute name="gs:inherited">true</xsl:attribute></xsl:if>

      <xsl:copy-of select="*"/>
    </xsl:copy>
  </xsl:template>

  <!-- ##############################################################################################
    database:class-xschema-node-mask([node, interface, class])
    database:class-xschema(object:Manager,'full')

    THIS is a transform: it outputs a COPY of the nodes
    so it cannot be used as a node-mask applied to an existing node
    applies the compiled stand-alone inherited class-xschema() for the class of this node
    the xschema can contain @gs:interface-mode indicators that restrict elements based on the interface
      <xsd:element gs:interface-modes="full,summary" />
      <xsd:element gs:interface-modes-except="summary" />
    we take advantage of hooks in class_xschema
  -->
  <xsl:param name="gs_debug_xsd_node_mask" select="false()"/>

  <xsl:template match="class:*" mode="gs_xsd_node_mask">
    <xsl:param name="gs_input_node" select="$gs_input_node"/>
    <xsl:param name="gs_interface_mode" select="$gs_interface_mode"/>

    <debug:server-message if="$gs_debug_xsd_node_mask" output="{local-name()} node-mask:"/>

    <database:select-input-node select="$gs_input_node"/>
    <xsl:element name="{local-name($gs_input_node)}" namespace="{namespace-uri($gs_input_node)}">
      <xsl:copy-of select="@xml:id"/>
      <xsl:if test="$gs_interface_mode"><xsl:attribute name="meta:interface"><xsl:value-of select="$gs_interface_mode"/></xsl:attribute></xsl:if>

      <xsl:apply-templates select="." mode="gs_xsd_node_mask_field_inheritance">
        <xsl:with-param name="gs_input_node" select="$gs_input_node"/>
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      </xsl:apply-templates>
    </xsl:element>
  </xsl:template>

  <xsl:template match="class:*" mode="gs_xsd_node_mask_field_inheritance">
    <xsl:param name="gs_input_node"/>
    <xsl:param name="gs_interface_mode"/>

    <xsl:variable name="gs_xschema" select="xsd:schema[not(@name) or @name = '']"/>

    <!-- recurse down first -->
    <xsl:apply-templates select="class:*" mode="gs_xsd_node_mask_field_inheritance">
      <xsl:with-param name="gs_input_node" select="$gs_input_node"/>
      <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
    </xsl:apply-templates>

    <!-- fields: if there are any at this level -->
    <debug:server-message if="$gs_debug_xsd_node_mask" output="-- processing {local-name()} fields"/>
    <xsl:apply-templates select="$gs_xschema/xsd:complexType/xsd:sequence/xsd:*" mode="gs_xsd_node_mask_fields">
      <xsl:with-param name="gs_input_node" select="$gs_input_node"/>
      <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="xsd:any" mode="gs_xsd_node_mask_fields">
    <xsl:param name="gs_input_node"/>
    <xsl:param name="gs_interface_mode"/>

    <xsl:variable name="gs_xsd_any" select="."/>

    <xsl:if test="(not(@gs:interface-modes) or ($gs_interface_mode and contains(@gs:interface-modes, $gs_interface_mode))) and (not(@gs:interface-modes-except) or not($gs_interface_mode) or not(contains(@gs:interface-modes-except, $gs_interface_mode)))">
      <xsl:for-each select="$gs_input_node/*">
        <xsl:apply-templates select="$gs_xsd_any" mode="gs_xsd_node_mask_fields_output">
          <xsl:with-param name="gs_element" select="."/>
          <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
        </xsl:apply-templates>
      </xsl:for-each>
    </xsl:if>
  </xsl:template>

  <xsl:template match="xsd:element" mode="gs_xsd_node_mask_fields">
    <xsl:param name="gs_input_node"/>
    <xsl:param name="gs_interface_mode"/>

    <xsl:variable name="gs_name" select="@name"/>
    <xsl:variable name="gs_elements" select="$gs_input_node/*[name() = $gs_name]"/>
    <xsl:variable name="gs_xsd_element" select="."/>

    <xsl:if test="$gs_elements">
      <xsl:if test="(not(@gs:interface-modes) or ($gs_interface_mode and contains(@gs:interface-modes, $gs_interface_mode))) and (not(@gs:interface-modes-except) or not($gs_interface_mode) or not(contains(@gs:interface-modes-except, $gs_interface_mode)))">
        <xsl:for-each select="$gs_elements">
          <xsl:apply-templates select="$gs_xsd_element" mode="gs_xsd_node_mask_fields_output">
            <xsl:with-param name="gs_element" select="."/>
            <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
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
    <xsl:param name="gs_interface_mode"/>

    <xsl:variable name="gs_name" select="@name"/>
    <xsl:variable name="gs_attribute" select="$gs_input_node/@*[name() = $gs_name]"/>

    <xsl:if test="not($gs_name = 'xml:id' or $gs_name = 'gs:interface-mode')">
      <xsl:if test="(not(@gs:interface-modes) or ($gs_interface_mode and contains(@gs:interface-modes, $gs_interface_mode))) and (not(@gs:interface-modes-except) or not($gs_interface_mode) or not(contains(@gs:interface-modes-except, $gs_interface_mode)))">
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
    <xsl:param name="gs_interface_mode"/>

    <debug:server-message if="$gs_debug_xsd_node_mask" output="[{name($gs_element)}]"/>
    <database:select-input-node select="$gs_element"/>
    <!-- if xsl:element tries to create an un-registered namespace prefix it will fail -->
    <xsl:element name="{local-name($gs_element)}" namespace="{namespace-uri($gs_element)}">
      <!-- sub-attributes first -->
      <xsl:apply-templates select="xsd:attribute" mode="gs_xsd_node_mask_fields">
        <xsl:with-param name="gs_input_node" select="$gs_element"/>
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      </xsl:apply-templates>

      <xsl:copy-of select="@xml:id"/>
      <xsl:if test="$gs_interface_mode"><xsl:attribute name="meta:interface"><xsl:value-of select="$gs_interface_mode"/></xsl:attribute></xsl:if>

      <!-- text and comments, after the attributes -->
      <xsl:copy-of select="$gs_element/text()"/>
      <xsl:copy-of select="$gs_element/comment()"/>

      <!-- sub-elements -->
      <xsl:apply-templates select="xsd:element" mode="gs_xsd_node_mask_fields">
        <xsl:with-param name="gs_input_node" select="$gs_element"/>
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      </xsl:apply-templates>
    </xsl:element>
  </xsl:template>
</xsl:stylesheet>
