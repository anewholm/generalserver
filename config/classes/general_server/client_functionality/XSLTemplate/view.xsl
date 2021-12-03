<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:interface="http://general_server.org/xmlnamespaces/interface/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" name="view" version="1.0">
  <xsl:template match="xsl:template" meta:interface-template="yes">
    <xsl:param name="gs_html_identifier_class"/>

    <div class="{$gs_html_identifier_class}">
      Templatey
    </div>
  </xsl:template>

  <xsl:template match="xsl:template" mode="gs_listname">
    <xsl:if test="@name">{<xsl:value-of select="@name"/>}</xsl:if>
    <xsl:if test="@match">[<xsl:value-of select="@match"/>]</xsl:if>
    <xsl:if test="@mode"> (<xsl:value-of select="@mode"/>)</xsl:if>
  </xsl:template>

  <xsl:template match="xsl:template" mode="view">
    <xsl:param name="gs_html_identifier_class"/>
    <xsl:param name="gs_interface_mode"/>

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
  </xsl:template>

  <xsl:template match="xsl:template" mode="gs_javascript_xsl_templates">
    <!-- call the associated "stored procedure" xsl:template
      $(User).changename("Bill") <= called from JOO
      xJST() = executeJavaScriptXSLTemplate()
      
      function changename(jDisplayObjects, new_name, fCallbackSuccess, fCallbackFailure) {
        this.xJST(User.prototype.changename, arguments);
      }
      f.templateParams=["new_name"];
      f.mode="changename";
    -->
    <xsl:param name="gs_object_name"/>

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

    <!-- function definition -->
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

    <!-- @mode is important here and 
      used by the updateWithControllerTemplate() call 
    -->
    <xsl:apply-templates select="@*" mode="gs_function_attributes"/>

    <!-- ordered template param name Intorspection 
      will be used to map incoming anonymous parameters to xsl:params IN ORDER
      used by the updateWithControllerTemplate() call
    -->
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
    
    <!-- f.functionType="method" required for Intorspection -->
    <xsl:text>f.functionType="method";</xsl:text>
    <xsl:value-of select="$gs_newline"/>

    <xsl:apply-templates select="." mode="gs_javascript_after_function"/>
  </xsl:template>

  <xsl:template match="xsl:param" mode="gs_javascript_xsl_templates_names">
    <xsl:text>"</xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text>"</xsl:text>
    <xsl:if test="not(position() = last())">,</xsl:if>
  </xsl:template>
  
  <xsl:template match="xsl:param" mode="gs_javascript_xsl_templates">
    <xsl:text>, </xsl:text>
    <xsl:value-of select="@name"/>
  </xsl:template>
  
  <xsl:template match="xsl:template" mode="controller">
    <!-- gs-form-include ensures that the xsl:param information is included in the submit -->
    <xsl:param name="gs_html_identifier_class"/>
    <xsl:param name="gs_interface_mode"/>

    <li>
      <xsl:attribute name="class">
        <xsl:value-of select="$gs_html_identifier_class"/>
        <xsl:text> f_click_menuitem_</xsl:text><xsl:value-of select="@mode"/>
        <xsl:text> f_submit_menuitem_</xsl:text><xsl:value-of select="@mode"/>
        <!-- gs-form-include ensures that the xsl:param information is included in the submit -->
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
  </xsl:template>
</xsl:stylesheet>