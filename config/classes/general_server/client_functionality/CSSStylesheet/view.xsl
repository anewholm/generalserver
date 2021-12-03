<xsl:stylesheet response:server-side-only="yes" xmlns:str="http://exslt.org/strings" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" name="view" version="1.0" extension-element-prefixes="debug">
  <xsl:template match="css:stylesheet" mode="standalone_contents">
    <!--
      SERVER-SIDE-ONLY: uses database:document-order() because it is optional on the server side
      
      the standalone textual contents of the stylesheet without any markup
      used for replies to direct HTTP requests for stylesheets
      GET /admin.css HTTP/1.1
      output example:
        /* CSSStylesheet DatabaseObject:
         *   @xml:id:
         *   @xpath-to-node:
         */
        body {
          border:1px solid black;
        }
    -->
    <xsl:apply-templates select="." mode="html_details"/>
  </xsl:template>

  <xsl:template match="css:stylesheet" mode="editor" meta:interface-template="yes">
    <!--
      presents a HTML eitable stylesheet
      each clause is a standard DatabaseElement and can be saved via the normal set-attribute / change-name API calls
      AJAX linked to update live contents as changes / additions / deletions to clauses happen
      output example (clause by clause):
        <form class="DatabaseElement gs-xml-id-idx_2344 gs-interface-mode-editor f_onchange_ajax_save css_clause">
          <input class="css_path" value="body" />
          <div class="css_property_pair">
            <input class="css_property_name" value="background-image" />
            <select class="css_property_value">
              <option>url(/blah.gif)</option>
            </select>
          </div>
        </form>
        ...
    -->
    <xsl:param name="gs_html_identifier_class"/>

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
  </xsl:template>

  <xsl:template match="css:stylesheet" mode="html" meta:interface-template="yes">
    <!--
      the HTML equivalent of the stylesheet
      used when pre-transforming an xsl:stylesheet that contains GS css:* [or javascript:*]
      output example:
        <style rel="text/css" class="Object Class__CSSStylesheet gs-xml-id-idx_2344 gs-interface-mode-html">
          body {
            border:1px solid black;
          }
        </style>
      no @xml:id on these as they are re-used and will cause repeat @xml:id issues on the client
    -->
    <xsl:param name="gs_html_identifier_class"/>

    <!-- these must be rendered first outside the script tags -->
    <xsl:apply-templates select="css:dependency" mode="html"/>

    <style type="text/css" class="{$gs_html_identifier_class} details" xml:add-xml-id="no">
      <xsl:apply-templates select="." mode="html_details"/>
    </style>
  </xsl:template>

  <xsl:template match="css:stylesheet" mode="html_details">
    <!--
      the textual contents of the stylesheet
      inherit the class name for the objects if no name directly given
      because used to pump out class stylesheets also
    -->
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
  </xsl:template>

  <xsl:template match="*" mode="gs_css_stylesheet_clauses">
    <xsl:apply-templates select="*" mode="gs_css_stylesheet_clauses"/>
  </xsl:template>
  
  <xsl:template match="css:stylesheet" mode="gs_listname">
    <xsl:text>view</xsl:text>
  </xsl:template>

  <!-- ############################################### css children ############################################### -->
  <xsl:template match="css:dependency">
    <xsl:text>/* css:dependency [</xsl:text>
    <xsl:value-of select="@uri"/>
    <xsl:text>] needs to be rendered outside this stylesheet */</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template>

  <xsl:template match="css:*/text()">
    <!-- avoid ampersand escaping 
      The MessageInterpretation controller should use:
      <xsl:output @method=text
      to avoid text value escaping
    -->
    <xsl:value-of select="normalize-space(.)" disable-output-escaping="yes"/>
  </xsl:template>

  <xsl:template match="css:raw">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="css:section">
    <xsl:param name="gs_object_name"/>
    <xsl:if test="@name">
      <xsl:text>/* ################ </xsl:text>
      <xsl:value-of select="@name"/>
      <xsl:text> ################# */</xsl:text>
      <xsl:value-of select="$gs_newline"/>
    </xsl:if>

    <xsl:apply-templates>
      <xsl:with-param name="gs_object_name" select="$gs_object_name"/>
    </xsl:apply-templates>
  </xsl:template>

  <!-- ############################################### path handling ############################################### -->
  <xsl:template match="css:clause/css:path|css:path">
    <!-- only output paths in specific path mode initiated by css:clause -->
  </xsl:template>

  <xsl:template match="css:path" mode="gs_css_path_output">
    <!-- ancestor grouping paths only -->
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="css:clause/css:path" mode="gs_css_path_output">
    <!-- primary clause multiple path output -->
    <xsl:param name="gs_object_name"/>

    <xsl:if test="$gs_object_name">
      <xsl:text>.</xsl:text><xsl:value-of select="$gs_object_name"/>
    </xsl:if>

    <!--
      don't do immediate parent because this is a child of that parent
      only singular path inheritance is supported
      NOTE: the css:clause needs at least one empty path to place the ancestory in
    -->
    <xsl:apply-templates select="../ancestor::css:*/css:path" mode="gs_css_path_output">
      <xsl:sort select="database:document-order()" data-type="number"/>
    </xsl:apply-templates>

    <xsl:apply-templates/>

    <xsl:if test="following-sibling::css:path">, </xsl:if>
  </xsl:template>

  <xsl:template match="css:path/css:*" mode="gs_css_relation">
    <!-- @relation can be on the parent also, e.g.
      <css:path relation="child-only">
      will act on all path elements
    -->
    <xsl:param name="gs_default"/>
    
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
  </xsl:template>

  <!-- ############################################### path types ############################################### 
    css:path can contain the same @modifiers that each element can
    e.g. <css:path @with-pseudo=first
      places :first on ALL css:path/*
  -->
  <xsl:template match="css:path/css:class">
    <xsl:apply-templates select="." mode="gs_css_relation"/>
    <xsl:text>.</xsl:text><xsl:apply-templates/>
    <xsl:apply-templates select="@*|../@*"/>
  </xsl:template>

  <xsl:template match="css:path/css:interface">
    <xsl:apply-templates select="." mode="gs_css_relation">
      <xsl:with-param name="gs_default" select="'self'"/>
    </xsl:apply-templates>
    <xsl:text>.gs-interface-mode-</xsl:text><xsl:apply-templates/>
    <xsl:apply-templates select="@*|../@*"/>
  </xsl:template>

  <xsl:template match="css:path/css:local-name">
    <xsl:apply-templates select="." mode="gs_css_relation">
      <xsl:with-param name="gs_default" select="'self'"/>
    </xsl:apply-templates>
    <xsl:text>.gs-local-name-</xsl:text><xsl:apply-templates/>
    <xsl:apply-templates select="@*|../@*"/>
  </xsl:template>

  <xsl:template match="css:path/css:object">
    <xsl:apply-templates select="." mode="gs_css_relation"/>
    <xsl:text>.Class__</xsl:text><xsl:apply-templates/>
    <xsl:apply-templates select="@*|../@*"/>
  </xsl:template>

  <xsl:template match="css:path/css:id">
    <xsl:apply-templates select="." mode="gs_css_relation"/>
    <xsl:text>#</xsl:text><xsl:apply-templates/>
    <xsl:apply-templates select="@*|../@*"/>
  </xsl:template>

  <xsl:template match="css:path/css:element">
    <xsl:apply-templates select="." mode="gs_css_relation"/>
    <xsl:apply-templates/>
    <xsl:apply-templates select="@*|../@*"/>
  </xsl:template>

  <!-- ############################################### path modifiers ############################################### -->
  <xsl:template match="css:*/@*"/>

  <xsl:template match="css:*/@with-class">
    <xsl:text>.</xsl:text><xsl:value-of select="."/>
  </xsl:template>

  <xsl:template match="css:*/@with-pseudo">
    <xsl:text>:</xsl:text><xsl:value-of select="."/>
  </xsl:template>

  <xsl:template match="css:*/@with-name">
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
  </xsl:template>

  <!-- ############################################### clause handling ############################################### -->
  <xsl:template match="css:clause">
    <xsl:param name="gs_object_name"/>

    <!-- multiple name and embedded path output -->
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

    <!-- documentation -->
    <xsl:if test="@name">
      <xsl:text>/* </xsl:text>
      <xsl:value-of select="@name"/>
      <xsl:text> */</xsl:text>
    </xsl:if>

    <xsl:value-of select="$gs_newline"/>

    <!-- path without mode is ignored and general css statements are at the same level -->
    <xsl:apply-templates/>
    <xsl:text>}</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template>

  <xsl:template match="css:clause/css:*">
    <!-- 2 types of clause, sections and direct -->
    <xsl:if test="css:*"><xsl:apply-templates select="css:*" mode="gs_css_item"/></xsl:if>
    <xsl:if test="not(css:*)"><xsl:apply-templates select="." mode="gs_css_item"/></xsl:if>
  </xsl:template>

  <xsl:template match="css:clause" mode="gs_css_item_collection"/>

  <xsl:template match="css:*" mode="gs_css_item_collection">
    <xsl:value-of select="local-name()"/>
    <xsl:text>-</xsl:text>
  </xsl:template>

  <xsl:template match="css:*" mode="gs_css_item">
    <!-- generic change XML to CSS
      <background-image type="url">thing.png</background-image>
      background-image:url(thing.png);
    -->
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
  </xsl:template>

  <!-- ############################################### stylesheet clauses ############################################### -->
  <xsl:template match="css:font-family" mode="gs_css_stylesheet_clauses">
    <!-- detect unsupported fonts and include them from the resources server -->
    <xsl:apply-templates select="str:split(., ',')" mode="gs_css_font_faces">
      <xsl:with-param name="gs_font_type" select="@type"/>
    </xsl:apply-templates>
  </xsl:template>
  
  <xsl:template match="token[normalize-space(.)='Verdana']" mode="gs_css_font_faces"/>
  <xsl:template match="token[normalize-space(.)='Arial']" mode="gs_css_font_faces"/>
  <xsl:template match="token[normalize-space(.)='Helvetica']" mode="gs_css_font_faces"/>
  <xsl:template match="token[normalize-space(.)='Times New Roman']" mode="gs_css_font_faces"/>
  <xsl:template match="token[normalize-space(.)='Times']" mode="gs_css_font_faces"/>
  <xsl:template match="token[normalize-space(.)='Courier New']" mode="gs_css_font_faces"/>
  <xsl:template match="token[normalize-space(.)='Courier']" mode="gs_css_font_faces"/>
  <xsl:template match="token[normalize-space(.)='Georgia']" mode="gs_css_font_faces"/>
  <xsl:template match="token[normalize-space(.)='Palatino']" mode="gs_css_font_faces"/>
  <xsl:template match="token[normalize-space(.)='Garamond']" mode="gs_css_font_faces"/>
  <xsl:template match="token[normalize-space(.)='Bookman']" mode="gs_css_font_faces"/>
  <xsl:template match="token[normalize-space(.)='Comic Sans MS']" mode="gs_css_font_faces"/>
  <xsl:template match="token[normalize-space(.)='Trebuchet MS']" mode="gs_css_font_faces"/>
  <xsl:template match="token[normalize-space(.)='Arial Black']" mode="gs_css_font_faces"/>
  <xsl:template match="token[normalize-space(.)='Impact']" mode="gs_css_font_faces"/>
  <xsl:template match="token[normalize-space(.)='serif']" mode="gs_css_font_faces"/>
  <xsl:template match="token[normalize-space(.)='sans-serif']" mode="gs_css_font_faces"/>
  
  <xsl:template match="token" mode="gs_css_font_faces">
    <xsl:param name="gs_font_type"/>
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
  </xsl:template>
  
  <!-- ############################################### extension items ############################################### -->
  <xsl:template match="css:icon" mode="gs_css_item">
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
  </xsl:template>

  <!-- ############################################### editor mode css children ############################################### -->
  <xsl:template match="css:section" mode="editor">
    <xsl:param name="gs_object_name"/>

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
  </xsl:template>

  <xsl:template match="css:section/css:path" mode="editor"/>

  <xsl:template match="css:clause" mode="editor">
    <xsl:param name="gs_object_name"/>

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
  </xsl:template>

  <xsl:template match="css:clause/css:path" mode="editor"/>

  <xsl:template match="css:clause/*" mode="editor">
    <xsl:variable name="name" select="@xml:id"/>
    <tr>
      <td class="gs_css_properties_label">
        <label for="{$name}"><xsl:value-of select="local-name()"/></label>
        <xsl:text>:</xsl:text>
      </td>
      <td>
        <!-- TODO: move to proper xsd rendering for this form -->
        <form class="f_submit_ajax" meta:dom-method="set-value" method="post"><div>
          <input name="gs_xsd_xpath" type="hidden" value="/config/classes/general_server/client_functionality/CSSStylesheet/model"/>
          <input name="select" type="hidden" value="{@meta:xpath-to-node}"/>
          <input name="new_value" value="{.}"/>
          <input value="set" type="submit"/>
        </div></form>
      </td>
     </tr>
  </xsl:template>
</xsl:stylesheet>
