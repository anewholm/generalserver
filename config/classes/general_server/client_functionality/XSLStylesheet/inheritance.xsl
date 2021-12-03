<xsl:stylesheet xmlns:xxx="http://general_server.org/xmlnamespaces/dummyxsl/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" name="inheritance_render" response:server-side-only="true" version="1.0" extension-element-prefixes="flow database str debug request regexp repository">
  <!--
    database:class-xstylesheet(class [, stand-alone, client-side-only])
    compile the pseudo xstylesheet for a class taking into account it's xsl inheritance chain
    this is a server side system transform only!
    example: http://general_server.localhost:8776/api/database:class-xstylesheet('User')?create-meta-context-attributes=none&node-mask=.|*&copy-query-attributes=no
      <class:Manager>
        <class:User>...</class:User>  [usually hardlinked] base class
        <xsl:stylesheet>...</xsl:stylesheet>
      </class:Manager>

    class template: an xsl:template with a @match that begins with the owning class:* @elements
      e.g. <xsl:template match="object:User">
    general template: non class:*/@elements specific
      e.g. <xsl:template match="*">

    @mode=gs_inheritance_render_xsl_standalone:
      creates a completely different stylesheet!
      all templates along the inheritance chain are compiled in to one stylesheet
      including class templates, with morphed @match, and general templates

    @mode=gs_inheritance_render_xsl_complement (not-stand-alone):
      designed to complement all the other related class stylesheet templates without repetition
      class templates are compiled with @match statements that include all derived classe elements
      general tmeplates

    client-side-only:
      client templates (not marked @response:server-side-only)

    server-side (not-client-side-only):
      client and server templates (everything!)
  -->
  <xsl:include xpath="/config/classes/general_server/system_management/Class/interface_translators"/>
  <xsl:include xpath="~XSLTemplate/inheritance_render"/>

  <xsl:param name="gs_client_side_only" select="true()"/>

  <xsl:namespace-alias result-prefix="xsl" stylesheet-prefix="xxx"/>

  <!-- ####################################################################################### -->
  <!-- ####################################################################################### -->
  <!-- ####################################################################################### -->
  <xsl:template match="xsl:stylesheet" mode="gs_inheritance_render_xsl_complement">
    <!-- TODO: this should also work off class:* because the required template might not exist... -->
    <debug:NOT_CURRENTLY_USED because="@mode=gs_complementary_templates is used directly"/>

    <xxx:stylesheet stand-alone="false" extension-element-prefixes="{database:extension-element-prefixes()}">
      <xsl:copy-of select="@*[not(self::stand-alone = . or self::extension-element-prefixes = .)]"/>
      <xsl:copy-of select="xsl:output|xsl:variable|xsl:param"/>
      <xsl:copy-of select="xsl:import|xsl:include"/>
      <xsl:apply-templates select="." mode="gs_complementary_templates"/>
    </xxx:stylesheet>
  </xsl:template>

  <xsl:template match="xsl:stylesheet" mode="gs_complementary_templates">
    <!-- NOTE: this template is used directly by the Class__XSLStylesheet/HTTP.xsl server
      it morphs the xsl:template/@match attribute as always
      we include also 1 and 2 sub-directory xsl:templates
        e.g. repository:views/xsl:stylesheet @name="HTML"
    -->
    <!-- TODO: this should also work off class:* because the required template might not exist... -->
    <debug:NOT_CURRENTLY_USED because="@mode=gs_complementary_templates is used directly"/>

    <xsl:variable name="gs_class" select="ancestor::class:*[1]"/>
    <xsl:variable name="gs_server_and_client_side" select="not($gs_client_side_only)"/>
    <xsl:variable name="gs_all_stylesheets" select="$gs_class/xsl:stylesheet|$gs_class/repository:*[str:not(@response:server-side-only)]/xsl:stylesheet|$gs_class/repository:*[str:not(@response:server-side-only)]/repository:*[str:not(@response:server-side-only)]/xsl:stylesheet"/>
    <xsl:variable name="gs_context_stylesheets" select="$gs_all_stylesheets[$gs_server_and_client_side or ($gs_client_side_only and str:not(@response:server-side-only))]"/>
    <xsl:variable name="gs_context_templates" select="$gs_context_stylesheets/xsl:template[$gs_server_and_client_side or ($gs_client_side_only and str:not(@response:server-side-only))]"/>

    <!-- i.e. only the global HTTP/client_environment_variables.xsl -->
    <xsl:copy-of select="$gs_context_stylesheets/xsl:param|$gs_context_stylesheets/xsl:variable"/>

    <xsl:if test="$gs_context_templates">
      <xsl:comment>######### Class__<xsl:value-of select="local-name()"/></xsl:comment>

      <!-- here we morph the xsl:template @match attributes to include derived @elements
        xsl:tmeplate contents are COPIED directly. they should NOT be altered.
      -->
      <xsl:apply-templates select="$gs_context_templates" mode="gs_inheritance_render_xsl">
        <xsl:with-param name="gs_class" select="$gs_class"/>
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>


  <!-- ####################################################################################### -->
  <!-- ####################################################################################### -->
  <!-- ####################################################################################### -->
  <xsl:template match="xsl:stylesheet" mode="gs_inheritance_render_xsl_standalone">
    <xsl:variable name="gs_class" select="ancestor::class:*[1]"/>

    <!-- TODO: this should also work off class:* because the required template might not exist... -->
    <debug:NOT_CURRENTLY_USED because="@mode=gs_complementary_templates is used directly"/>

    <xxx:stylesheet stand-alone="true" extension-element-prefixes="{database:extension-element-prefixes()}">
      <xsl:copy-of select="@*[not(self::stand-alone = . or self::extension-element-prefixes = .)]"/>

      <xsl:comment>
        ######################################### Class__<xsl:value-of select="local-name($gs_class)"/>
        [stand-alone] class xStylesheet for [<xsl:value-of select="local-name($gs_class)"/>]
          xsl:template/@match attributes have been morphed to include all derived class:*/@elements
        type:             [stand-alone]
        client-side-only: [<xsl:value-of select="$gs_client_side_only"/>] relates to @response:server-side-only
        element:          [<xsl:value-of select="$gs_class/@elements"/>]
      </xsl:comment>

      <!-- TODO: apply-templates? -->
      <xsl:copy-of select="xsl:output|xsl:variable|xsl:param"/>
      <xsl:copy-of select="xsl:import|xsl:include"/>

      <!-- inherited templates -->
      <xsl:apply-templates select="$gs_class/class:*" mode="gs_xsl_template_inheritance"/>

      <!-- TODO: @gs:interface-mode translation -->
      <!-- xsl:comment>@interface-mode templates for @interface-mode =&gt; xsl @mode</xsl:comment>
      <xsl:variable name="gs_all_class_stylesheets" select="xsl:stylesheet|repository:*/descendant-or-self::xsl:stylesheet"/>
      <xsl:variable name="gs_client_side_class_stylesheets" select="$gs_all_class_stylesheets[str:not(@response:server-side-only)]"/>
      <xsl:variable name="gs_interface_class_templates" select="$gs_client_side_class_stylesheets/xsl:template[@mode and str:boolean(@meta:interface-template)]"/>
      <xsl:apply-templates select="$gs_client_side_class_stylesheets[1]" mode="gs_generate_XSL">
        <xsl:with-param name="gs_distinct_interface_templates" select="database:distinct($gs_interface_class_templates, '@mode')"/>
      </xsl:apply-templates -->

      <!-- copy *all* the main stylesheet templates (class and general)
        we include also sub-directory xsl:templates
        e.g. repository:views/xsl:stylesheet @name="HTML"
      -->
      <xsl:variable name="gs_server_and_client_side" select="not($gs_client_side_only)"/>
      <xsl:variable name="gs_all_stylesheets" select="$gs_class/xsl:stylesheet|$gs_class/repository:*[str:not(@response:server-side-only)]/xsl:stylesheet|$gs_class/repository:*[str:not(@response:server-side-only)]/repository:*[str:not(@response:server-side-only)]/xsl:stylesheet"/>
      <xsl:variable name="gs_context_stylesheets" select="$gs_all_stylesheets[$gs_server_and_client_side or ($gs_client_side_only and str:not(@response:server-side-only))]"/>
      <xsl:variable name="gs_context_templates" select="$gs_context_stylesheets/xsl:template[$gs_server_and_client_side or ($gs_client_side_only and str:not(@response:server-side-only))]"/>
      <xsl:if test="$gs_context_templates">
        <xsl:comment>###################################### <xsl:value-of select="local-name(.)"/> all templates [<xsl:value-of select="@elements"/>]</xsl:comment>
        <xsl:apply-templates select="$gs_context_templates" mode="gs_inheritance_render_xsl"/>
      </xsl:if>
    </xxx:stylesheet>
  </xsl:template>

  <xsl:template match="class:*" mode="gs_xsl_template_inheritance">
    <xsl:variable name="gs_this_class_elements" select="database:derived-template-match()"/>
    <xsl:variable name="gs_server_and_client_side" select="not($gs_client_side_only)"/>
    <xsl:variable name="gs_context_templates" select="xsl:stylesheet/xsl:template[$gs_server_and_client_side or ($gs_client_side_only and str:not(@response:server-side-only))]"/>

    <debug:NOT_CURRENTLY_USED because="xpath honours inheritance now"/>
    
    <!-- other elements -->
    <xsl:copy-of select="xsl:variable"/>

    <!-- TODO: @gs:interface-mode translation -->
    <!-- xsl:comment>@interface-mode templates for @interface-mode =&gt; xsl @mode</xsl:comment>
    <xsl:variable name="gs_all_class_stylesheets" select="xsl:stylesheet|repository:*/descendant-or-self::xsl:stylesheet"/>
    <xsl:variable name="gs_client_side_class_stylesheets" select="$gs_all_class_stylesheets[str:not(@response:server-side-only)]"/>
    <xsl:variable name="gs_interface_class_templates" select="$gs_client_side_class_stylesheets/xsl:template[@mode and str:boolean(@meta:interface-template)]"/>
    <xsl:apply-templates select="$gs_client_side_class_stylesheets[1]" mode="gs_generate_XSL">
      <xsl:with-param name="gs_distinct_interface_templates" select="database:distinct($gs_interface_class_templates, '@mode')"/>
    </xsl:apply-templates -->

    <!-- inherit first for lower specificity -->
    <xsl:apply-templates select="class:*" mode="gs_xsl_template_inheritance"/>

    <!-- copy the class elements templates. the match will be morphed to the main @elements for this one
      e.g. <xsl:template match="object:User[...]"> where <class:User namespace-prefix="object" match="User"
    -->
    <xsl:variable name="gs_class_templates" select="$gs_context_templates[starts-with(@match, $gs_this_class_elements)]"/>
    <xsl:if test="$gs_this_class_elements and $gs_class_templates">
      <xsl:comment>###################################### <xsl:value-of select="local-name(.)"/> class templates [<xsl:value-of select="$gs_this_class_elements"/> =&gt; <xsl:value-of select="$gs_main_class_elements"/>]</xsl:comment>
      <xsl:apply-templates select="$gs_class_templates" mode="gs_inheritance_render_xsl">
        <xsl:with-param name="gs_class" select="."/>
      </xsl:apply-templates>
    </xsl:if>

    <!-- copy the non-class inherited templates
      e.g. <xsl:template match="*">
    -->
    <!-- xsl:variable name="gs_general_templates" select="$gs_context_templates[not(@match = $gs_this_class_elements) and not(@match = '*')]"/>
    <xsl:if test="$gs_general_templates">
      <xsl:comment>###################################### <xsl:value-of select="local-name(.)"/> general templates</xsl:comment>
      <xsl:copy-of select="$gs_general_templates"/>
    </xsl:if -->
  </xsl:template>
</xsl:stylesheet>
