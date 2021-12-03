<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" name="view" version="1.0">
  <!-- display templates for the XSLStylesheet Class
    mostly used in the administration suite for editing purposes
  -->

  <!-- ####################################### editor ####################################### -->
  <xsl:template match="xsl:stylesheet" mode="editor" meta:interface-template="yes">
    <xsl:param name="gs_html_identifier_class"/>

    <div class="{$gs_html_identifier_class} gs-clickable">
      <!-- header -->
      <xsl:if test="comment() and not(contains(comment()[1], '###'))">
        <div class="gs_description"><xsl:apply-templates select="comment()[1]" mode="gs_formatted_output"/></div>
      </xsl:if>

      <!-- #################### server-side and client-side warnings -->
      <xsl:if test="not(namespace::*[name() = ''] = 'http://www.w3.org/1999/xhtml')">
        <div class="gs-warning">
          default xmlns namespace is not set to http://www.w3.org/1999/xhtml
          (<xsl:value-of select="namespace::*[name() = '']"/>)
        </div>
      </xsl:if>

      <xsl:if test="xsl:template[@name]">
        <div class="gs-warning">
          named templates are dicouraged because they cannot be overridden or re- xsl:included
          use <![CDATA[<xsl:template match="*" mode="<name>"> instead]]>
        </div>
      </xsl:if>

      <xsl:if test=".//xsl:value-of">
        <div class="gs-warning">
          xsl:value-of is discouraged because they cannot be overridden
          use <![CDATA[<xsl:apply-templates select="$item"/>]]> instead
          which will use the standard templates to output the text result
        </div>
      </xsl:if>

      <xsl:if test=".//xsl:copy-of">
        <div class="gs-warning">
          xsl:copy-of is discouraged because they cannot be overridden
          use <![CDATA[<xsl:apply-templates select="$item" mode="gs_view_render" />]]> instead
          which will use the standard templates to output the tree
        </div>
      </xsl:if>

      <xsl:if test=".//xsl:element[not(@namespace)]">
        <div class="gs-warning">
          dynamic @name xsl:element should have a @namespace attribute
          otherwise qualified QNames, e.g. "gs:example", may cause namespace prefix not found errors
          i.e. <![CDATA[<xsl:element name="{local-name($gs_element)}" namespace="{namespace-uri($gs_element)}">]]>
        </div>
      </xsl:if>


      <!-- #################### server-side only warnings -->
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

      <!-- #################### class view client-side only warnings -->
      <xsl:if test="not(@response:server-side-only='true') and meta:class/class:*">
      </xsl:if>

      <!-- #################### website template client-side only warnings -->
      <xsl:if test="not(@response:server-side-only='true' or meta:class/class:*)">
        <xsl:if test="not(xsl:template[@match = 'gs:root'])">
          <div class="gs-warning">
            no meta:root handler template <![CDATA[<xsl:template match="gs:root" />]]>
          </div>
        </xsl:if>

        <xsl:if test="xsl:template[@match = '/']">
          <div class="gs-warning">
            template <![CDATA[<xsl:template match="/" />]]> is not advised
            use the <![CDATA[<xsl:template match="gs:root" />]]> handler instead
          </div>
        </xsl:if>
      </xsl:if>

      <!-- components -->
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
  </xsl:template>

  <xsl:template match="database:query" mode="xsl_stylesheet_editor">
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
  </xsl:template>

  <xsl:template match="database:query" mode="gs_context_menu_custom">
    <li class="f_click_select_children">browse data</li>
  </xsl:template>

  <xsl:template match="xsl:output/@*" mode="xsl_stylesheet_editor">
    <form><input id="gs_option_{name()}" name="{name()}" type="checkbox"/><label for="gs_option_{name()}"><xsl:value-of select="name()"/></label></form>
  </xsl:template>

  <xsl:template match="xsl:include" mode="xsl_stylesheet_editor">
    <li>
      <xsl:attribute name="class">
        <xsl:text>xsl_stylesheet_editor_xsl_include</xsl:text>
      </xsl:attribute>

      <xsl:if test="@xpath"><xsl:value-of select="@xpath"/></xsl:if>
      <xsl:if test="not(@xpath)"><xsl:value-of select="@href"/></xsl:if>
    </li>
  </xsl:template>

  <xsl:template match="xsl:variable" mode="xsl_stylesheet_editor">
    <li>
      <xsl:attribute name="class">
        <xsl:text>xsl_stylesheet_editor_xsl_variable</xsl:text>
      </xsl:attribute>

      <xsl:value-of select="."/>
    </li>
  </xsl:template>

  <xsl:template match="xsl:stylesheet/@*" mode="xsl_stylesheet_editor"/>

  <xsl:template match="xsl:stylesheet/@meta:*" mode="xsl_stylesheet_editor">
    <form><input id="gs_option_{name()}" name="{name()}" type="checkbox"/><label for="gs_option_{name()}"><xsl:value-of select="name()"/></label></form>
  </xsl:template>

  <!-- editor mode XSLTemplate children -->
  <xsl:template match="xsl:template" mode="xsl_stylesheet_editor">
    <xsl:param name="gs_html_identifier_class"/>

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

          <span class="mode"> <xsl:value-of select="@mode"/></span>

          <xsl:if test="comment()">
            <!-- TODO: convert linespaces to HTML -->
            <div class="gs_description"><xsl:apply-templates select="comment()[1]" mode="gs_formatted_output"/></div>
          </xsl:if>
        </div>
      </a>
    </li>
  </xsl:template>

  <!-- ####################################### formatted output ####################################### -->
  <xsl:template match="node()" mode="gs_formatted_output">
    <!--
       convert unix new line character in to an HTML <li> line
       TODO: move this template in to a generic client and server side - utilities area
    -->
    <ul>
      <xsl:apply-templates select="." mode="gs_formatted_output_recursive">
        <xsl:with-param name="text" select="string(.)"/>
      </xsl:apply-templates>
    </ul>
  </xsl:template>

  <xsl:template match="node()" mode="gs_formatted_output_recursive">
    <xsl:param name="text"/>

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
  </xsl:template>

  <!-- ####################################### list ####################################### -->
  <xsl:template match="xsl:stylesheet" mode="gs_listname">
    <xsl:if test="@name"><xsl:value-of select="@name"/></xsl:if>
    <xsl:if test="not(@name)">stylesheet</xsl:if>
  </xsl:template>

  <!-- ####################################### view / controller ####################################### -->
  <xsl:template match="xsl:stylesheet" mode="view" meta:interface-template="yes">
    <xsl:param name="gs_html_identifier_class"/>

    <!-- we always want a record of those objects involved -->
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
  </xsl:template>

  <xsl:template match="xsl:stylesheet" mode="controller" meta:interface-template="yes">
    <xsl:param name="gs_html_identifier_class"/>

    <!-- we always want a record of those objects involved -->
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
  </xsl:template>
</xsl:stylesheet>
