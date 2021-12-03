<xsl:stylesheet response:server-side-only="yes" xmlns:xjs="http://general_server.org/xmlnamespaces/xjavascript/2006" xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:xxx="http://general_server.org/xmlnamespaces/dummyxsl/2006" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" name="view" version="1.0">
  <!--
    SERVER-SIDE-ONLY: uses database:base-class-count() for inheritance order
      ancestor::class:* is not necessarily available at view time
    
    javascript transforms server side also
    because it is an extension_handler
    and also transforms parts of stylesheets as they are served

    so it requires this client-side html markup include
  -->
  <!-- ################################# preprocessor ################################################ -->
  <xsl:template match="javascript:ifdef">
    <!-- server side only -->
    <xsl:param name="gs_object_name"/>
    <xsl:if test="function-available('dyn:evaluate') and dyn:evaluate(@test)">
      <xsl:apply-templates>
        <xsl:with-param name="gs_object_name" select="$gs_object_name"/>
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>
  
  <xsl:template match="javascript:stage-dev">
    <xsl:param name="gs_object_name"/>
    <xsl:if test="$gs_stage_dev">
      <xsl:apply-templates>
        <xsl:with-param name="gs_object_name" select="$gs_object_name"/>
      </xsl:apply-templates>
    </xsl:if>
  </xsl:template>

  
  <!-- ###################################### code ############################################ -->
  <xsl:template match="javascript:code" mode="standalone_contents">
    <!--
      the standalone textual contents of the script file without any markup
      used for replies to direct HTTP requests for javascript
      GET /admin.js HTTP/1.1
      output example:
        /* HTMLScript DatabaseObject:
         *   @xml:id:
         *   @xpath-to-node:
         */
        function Car {
          ...
        }
    -->
    <xsl:param name="gs_class" select="parent::class:*"/>
    <xsl:apply-templates select="." mode="html_details">
      <xsl:with-param name="gs_class" select="$gs_class"/>
    </xsl:apply-templates>
  </xsl:template>
  
  <xsl:template match="javascript:code" mode="editor" meta:interface-template="yes">
    <!--
      presents a HTML editable script
      each method is a standard DatabaseElement and can be saved via the normal set-attribute / change-name API calls
      AJAX linked to update live contents as changes / additions / deletions to clauses happen
      output example (method by method):
        <form class="DatabaseElement gs-xml-id-idx_2344 gs-interface-mode-editor f_onchange_ajax_save javascript_method">
          <textarea />
        </form>
        ...
    -->
    <xsl:param name="gs_html_identifier_class"/>

    <div class="{$gs_html_identifier_class} details" xml:add-xml-id="no">
      <!-- these must be rendered first outside the script tags -->
      <ul><xsl:apply-templates select="javascript:dependency" mode="editor">
        <xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class"/>
      </xsl:apply-templates></ul>

      <xsl:apply-templates mode="editor"/>
    </div>
  </xsl:template>

  <xsl:template match="javascript:code" mode="html" meta:interface-template="yes">
    <!--
      the HTML equivalent of the script tags
      used when pre-transforming an xsl:stylesheet that contains GS javascript:*
      output example:
        <script type="text/javascript" class="Object Class__JavaScript gs-xml-id-idx_2344 gs-interface-mode-html">
          function Car {
          }
        </script>
      no @xml:id on these as they are re-used and will cause repeat @xml:id issues on the client

      the manual comments (//&lt;!- -) prevent the Linux AJAX XSLTProcessor from mis-interpreting the javascript entities
    -->
    <xsl:param name="gs_html_identifier_class"/>

    <!-- these must be rendered first outside the script tags -->
    <xsl:apply-templates select="javascript:dependency" mode="html">
      <xsl:with-param name="gs_html_identifier_class" select="$gs_html_identifier_class" />
    </xsl:apply-templates>

    <script type="text/javascript" class="{$gs_html_identifier_class} details" xml:add-xml-id="no">
      <xsl:text>//&lt;!--</xsl:text>
      <xsl:apply-templates select="." mode="html_details"/>
      <xsl:text>//--&gt;</xsl:text>
    </script>
  </xsl:template>

  <xsl:template match="javascript:code" mode="html_details">
    <!--
      the textual contents of the script
      inherit the class name for the objects if no name directly given
      because used to pump out class scripts also
    -->
    <xsl:param name="gs_class" select="parent::class:*"/>
    <xsl:apply-templates select="javascript:*">
      <xsl:with-param name="gs_class" select="$gs_class"/>
    </xsl:apply-templates>
  </xsl:template>

  <xsl:template match="javascript:code" mode="gs_listname">
    <xsl:text>controller</xsl:text>
  </xsl:template>

  <xsl:template match="html:script" mode="gs_listname">
    <!-- listname failover -->
    <xsl:choose>
      <xsl:when test="@meta:list-name"><xsl:value-of select="@meta:list-name"/></xsl:when>
      <xsl:when test="@src">
        <xsl:if test="contains(@src, '/')"><xsl:value-of select="substring-after-last(@src, '/')"/></xsl:if>
        <xsl:if test="not(contains(@src, '/'))"><xsl:value-of select="@src"/></xsl:if>
      </xsl:when>
      <xsl:otherwise>script (inline)</xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <!-- ############################################################################################################
    this is the basic jquery aware but non-gs javascript XML transform
    ####### it handles:
      objects (name inheritance aware)
        + methods, static and public
      global functions, calls and variables
      simple collection
      raw output

    ####### usage situations:
      inline javascript code in a html:head
        <javascript:*> ... </javascript:*>
        output in mode="gs_interface_render" during xsl:stylesheet output to client
        which will include the <html:script> tags around it
      GS objects linked in from html:script @src
        <javascript:object >
        additional injected GS code in to the objects
        and name intelligence
      gs_handler for the javascript namespace
        calls with xpath: /alert_js
        or recursion:     /alert.js
        <javascript:*>

  -->

  <!-- avoid ampersand escaping 
    The MessageInterpretation controller should use:
    <xsl:output @method=text
    to avoid text value escaping
  -->
  <xsl:template match="javascript:*/text()">
    <xsl:value-of select="." disable-output-escaping="yes"/>
  </xsl:template>

  <xsl:template match="javascript:raw">
    <!-- useful for
      bringing things like jQuery in to the XML mix
        hold all the jQuery code in a directory with directory.metaxml = <javascript:raw>
      raw direct javascript output
      sub-item in a collection
    -->
    <xsl:apply-templates/>
  </xsl:template>

  <!-- ##################################################### global functionality ##################################################### -->
  <xsl:template match="javascript:dependency">
    <xsl:text>//javascript:dependency on [</xsl:text>
    <xsl:value-of select="@uri"/>
    <xsl:text>] rendered outside this script</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template>

  <xsl:template match="javascript:with-param">
    alert(document.XMLDocument);
    var <xsl:value-of select="@name"/> = "<xsl:value-of select="translate(@select, '&quot;', '_')"/>";
  </xsl:template>

  <xsl:template match="javascript:assert">
    if (!<xsl:value-of select="@test"/>) alert("<xsl:value-of select="translate(@message, '&quot;', '_')"/>");
  </xsl:template>

  <xsl:template match="javascript:define|javascript:global-variable">
    <!-- window["x"] = y;
      <javascript:define name=example><xsl:value-of select="@thing"/></javascript:define>
      TODO: javascript:global-variable escape the variable name!
    -->
    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>

    <xsl:text>window["</xsl:text><xsl:value-of select="@name"/><xsl:text>"]</xsl:text>
    <xsl:if test="string(.)">
      <xsl:text> = </xsl:text>
      <xsl:value-of select="."/>
    </xsl:if>
    <xsl:if test="not(string(.))"> = null</xsl:if>
    <xsl:text>;</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template>

  <xsl:template match="javascript:override">
    <!-- name = function(parameters) { ... } -->
    <xsl:value-of select="@name"/>
    <xsl:text> = function(</xsl:text>
    <xsl:value-of select="xjs:parameters(@parameters, @name)"/>
    <xsl:text>) {</xsl:text>
      <xsl:apply-templates select="." mode="gs_javascript_before_method"/>
      <xsl:apply-templates select="@requires"/>
      <xsl:if test="not(@parameter-checks = 'off' or ancestor::javascript:code[1][@parameter-checks = 'off'])"><xsl:value-of select="xjs:parameter-checks(@parameters, @name, 6, $gs_stage_dev)"/></xsl:if>
      <xsl:apply-templates/>
    <xsl:text>}</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template>

  <xsl:template match="javascript:global-function">
    <!-- function name(parameters) { ... } -->
    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>
    <xsl:variable name="gs_prototype_path">window.<xsl:value-of select="@name"/></xsl:variable>

    <!-- method -->
    <xsl:text>function </xsl:text>
    <xsl:value-of select="@name"/><xsl:text>(</xsl:text>
    <xsl:value-of select="xjs:parameters(@parameters, @name)"/>
    <xsl:text>) {</xsl:text>
      <xsl:apply-templates select="." mode="gs_javascript_before_method"/>
      <xsl:apply-templates select="@requires"/>
      <xsl:if test="not(@parameter-checks = 'off' or ancestor::javascript:code[1][@parameter-checks = 'off'])"><xsl:value-of select="xjs:parameter-checks(@parameters, @name, 6, $gs_stage_dev)"/></xsl:if>
      <xsl:apply-templates/>
    <xsl:text>}</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template>

  <xsl:template match="javascript:global-init">
    <!-- ready
      jQuery is passed to the document.ready event, not an event :(
      so we assemble a fake "ready" Event with a document target

      function f(fJQuery) {
        var eEvent = Event.factory("ready", document);
        ...
      }
      jQuery(document).ready(f)
        @delay runs the code on document.ready;                   default:true
      includes class global-init
    -->
    <xsl:param name="gs_object_name" select="'Global'"/>
    
    <xsl:variable name="gs_funcname">
      <xsl:if test="@name"><xsl:value-of select="@name"/></xsl:if>
      <xsl:if test="not(@name)"><xsl:value-of select="concat('global-init_',translate(@xml:id, ' -[](){}&amp;#', '_'))"/></xsl:if>
    </xsl:variable>
    <xsl:variable name="gs_prototype_path"><xsl:value-of select="$gs_object_name"/>.<xsl:value-of select="$gs_funcname"/></xsl:variable>

    <!-- method -->
    <xsl:text>var f=</xsl:text>
    <xsl:value-of select="$gs_prototype_path"/>
    <xsl:text>=function </xsl:text><xsl:value-of select="$gs_funcname"/><xsl:text>(oResults) {</xsl:text>
    <xsl:value-of select="$gs_newline"/>
      <xsl:apply-templates select="." mode="gs_javascript_before_method"/>
      <xsl:apply-templates select="@requires"/>
      <xsl:apply-templates/>
    <xsl:text>}</xsl:text>
    <xsl:value-of select="$gs_newline"/>

    <!-- f.functionType="method" -->
    <xsl:text>f.functionType="globalInit";</xsl:text>
    <xsl:value-of select="$gs_newline"/>
    
    <xsl:apply-templates select="@*" mode="gs_function_attributes"/>
    
    <!-- Global global-init are declared everywhere, outside of the actuall Global class 
      otherwise they will get registered as part of the class
    -->
    <xsl:if test="$gs_object_name = 'Global'">
      <xsl:text>OO.registerMethod(window.Global,f);</xsl:text>
      <xsl:value-of select="$gs_newline"/>
    </xsl:if>
    
    <!-- invocation -->
    <xsl:text>OO.executeGlobalInit(f</xsl:text>
    <xsl:if test="@delay">,<xsl:value-of select="@delay"/></xsl:if>
    <xsl:text>);</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template>

  <!-- ##################################################### extensions ##################################################### -->
  <xsl:template match="javascript:jquery-expression-extension">
    <!-- jQuery.expr[':'].<name> = function() { ... }
      jQuery.expr[':'].<name>.extension = true;
    -->
    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>
    <xsl:variable name="gs_prototype_path"><xsl:text>jQuery.expr[':'].</xsl:text><xsl:value-of select="@name"/></xsl:variable>

    <!-- method -->
    <xsl:value-of select="$gs_prototype_path"/><xsl:text> = function(</xsl:text>
    <xsl:value-of select="xjs:parameters(@parameters, @name)"/>
    <xsl:text>) {</xsl:text>
      <xsl:if test="@override = 'true'"> //(@override=true)</xsl:if>
      <xsl:value-of select="$gs_newline"/>
      <xsl:if test="not(@parameter-checks = 'off' or ancestor::javascript:code[1][@parameter-checks = 'off'])"><xsl:value-of select="xjs:parameter-checks(@parameters, @name, 6, $gs_stage_dev)"/></xsl:if>
      <xsl:apply-templates/>
    <xsl:text>}</xsl:text>
    <xsl:value-of select="$gs_newline"/>

    <xsl:value-of select="$gs_prototype_path"/><xsl:text>.extension = true;</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template>

  <xsl:template match="javascript:jquery-extension">
    <!-- jQuery.fn.<name> = function() { ... }
      jQuery.fn.<name>.extension = true;
    -->
    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>
    <xsl:variable name="gs_prototype_path"><xsl:text>jQuery.fn.</xsl:text><xsl:value-of select="@name"/></xsl:variable>

    <!-- old method if applicable -->
    <xsl:text>var fOriginal=</xsl:text><xsl:value-of select="$gs_prototype_path"/><xsl:text>;</xsl:text>
    <xsl:value-of select="$gs_newline"/>
    
    <!-- method -->
    <xsl:text>var f=</xsl:text>
    <xsl:value-of select="$gs_prototype_path"/><xsl:text> = function(</xsl:text>
    <xsl:value-of select="xjs:parameters(@parameters, @name)"/>
    <xsl:text>) {</xsl:text>
      <xsl:if test="@override = 'true'"> //(@override=true)</xsl:if>
      <xsl:value-of select="$gs_newline"/>
      <xsl:if test="not(@parameter-checks = 'off' or ancestor::javascript:code[1][@parameter-checks = 'off'])">
        <xsl:value-of select="xjs:parameter-checks(@parameters, @name, 6, $gs_stage_dev)"/>
      </xsl:if>
      <xsl:apply-templates/>
    <xsl:text>}</xsl:text>
    <xsl:value-of select="$gs_newline"/>

    <xsl:text>if (fOriginal) f.fOriginal = fOriginal;</xsl:text>
    <xsl:value-of select="$gs_newline"/>

    <xsl:text>f.extension = true;</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template>

  <xsl:template match="javascript:jquery-static-extension">
    <!-- jQuery.<name> = function() { ... } -->
    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>
    <xsl:variable name="gs_prototype_path"><xsl:text>jQuery.</xsl:text><xsl:value-of select="@name"/></xsl:variable>

    <!-- method -->
    <xsl:value-of select="$gs_prototype_path"/><xsl:text> = function(</xsl:text>
    <xsl:value-of select="xjs:parameters(@parameters, @name)"/>
    <xsl:text>) {</xsl:text>
      <xsl:value-of select="$gs_newline"/>
      <xsl:if test="not(@parameter-checks = 'off' or ancestor::javascript:code[1][@parameter-checks = 'off'])">
        <xsl:value-of select="xjs:parameter-checks(@parameters, @name, 6, $gs_stage_dev)"/>
      </xsl:if>
      <xsl:apply-templates/>
    <xsl:text>}</xsl:text>
    <xsl:value-of select="$gs_newline"/>

    <xsl:value-of select="$gs_prototype_path"/><xsl:text>.extension = true;</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template>

  <xsl:template match="javascript:jquery-event-extension">
    <!-- jQuery.Event.<name> = function() { ... } -->
    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>

    <xsl:text>jQuery.Event.prototype.</xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text> = function(</xsl:text>
    <xsl:value-of select="xjs:parameters(@parameters, @name)"/>
    <xsl:text>) {</xsl:text>
      <xsl:value-of select="$gs_newline"/>
      <xsl:if test="not(@parameter-checks = 'off' or ancestor::javascript:code[1][@parameter-checks = 'off'])"><xsl:value-of select="xjs:parameter-checks(@parameters, @name, 6, $gs_stage_dev)"/></xsl:if>
      <xsl:apply-templates/>
    <xsl:text>}</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template>

  <xsl:template match="javascript:javascript-static-extension-property">
    <!-- String.property = value; -->
    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>

    <xsl:value-of select="@object-name"/>
    <xsl:text>.</xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text> = </xsl:text>
    <xsl:value-of select="."/>
    <xsl:text>;</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template>

  <xsl:template match="javascript:javascript-extension|javascript:javascript-static-extension">
    <!-- String.prototype.trim = function() {return this.replace(/^\s+|\s+$/g, '');}
      String.prototype.trim.exec = function(){return false;} //prevent jQuery from iterating over this expansion
    -->
    <xsl:if test="@object-name = 'Object' and not(@name = 'toString')">
      <xsl:text>/* Object extension of [</xsl:text>
      <xsl:value-of select="@name"/>
      <xsl:text>] ignored */</xsl:text>
      <xsl:value-of select="$gs_newline"/>
      <xsl:text>alert('cannot extend the javascript Object because it breaks all for-each() loops including jQuery')</xsl:text>
    </xsl:if>

    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>

    <xsl:if test="not(@object-name = 'Object' and not(@name = 'toString'))">
      <xsl:variable name="gs_function_name">
        <xsl:value-of select="@object-name"/>
        <xsl:if test="not(self::javascript:javascript-static-extension)"><xsl:text>.prototype</xsl:text></xsl:if>
        <xsl:text>.</xsl:text>
        <xsl:value-of select="@name"/>
      </xsl:variable>

      <xsl:if test="not(@override)">
        <xsl:text>if (typeof </xsl:text>
        <xsl:value-of select="$gs_function_name"/>
        <xsl:text> !== 'function') {</xsl:text>
        <xsl:value-of select="$gs_newline"/>
      </xsl:if>
        <xsl:value-of select="$gs_function_name"/>
        <xsl:text> = function(</xsl:text>
        <xsl:value-of select="xjs:parameters(@parameters, $gs_function_name)"/>
        <xsl:text>) {</xsl:text>
          <xsl:value-of select="$gs_newline"/>
          <xsl:if test="not(@parameter-checks = 'off' or ancestor::javascript:code[1][@parameter-checks = 'off'])"><xsl:value-of select="xjs:parameter-checks(@parameters, $gs_function_name, 6, $gs_stage_dev)"/></xsl:if>
          <xsl:apply-templates/>
        <xsl:text>}</xsl:text>
        <xsl:value-of select="$gs_newline"/>

        <xsl:value-of select="$gs_function_name"/>
        <xsl:text>.exec = function(){return true;}</xsl:text>
        <xsl:value-of select="$gs_newline"/>
      <xsl:if test="not(@override)">}</xsl:if>
      <xsl:value-of select="$gs_newline"/>
    </xsl:if>

    <xsl:if test="@override = 'true'">
      <xsl:value-of select="$gs_prototype_path"/><xsl:text>.override = true;</xsl:text>
      <xsl:value-of select="$gs_newline"/>
    </xsl:if>

    <xsl:value-of select="$gs_newline"/>
  </xsl:template>
  
  <!-- ##################################################### javascript:object ##################################################### -->
  <xsl:template match="javascript:object|javascript:namespace">
    <!-- function Thing() {
        ...
      }
      Thing.notDefinedClass = true;
      Thing.inheritanceOrder = 7;
      OO.registerDerivedClass(Thing, window.BaseThing);

      [Thing.prototype.method = function (){...}]

      OO.registerClass(Thing);
      Thing.notDefinedClass = false;
      Thing.definedClass = true;
    -->

    <!-- potential Class__ container and its base classes
      @name = inherit|name
      $gs_inheritance_order will be zero for non-class objects
    -->
    <xsl:param name="gs_class" select="parent::javascript:code/parent::class:*"/>

    <xsl:variable name="gs_inheritance_order" select="database:base-class-count($gs_class)"/>

    <!-- inherit the class name for the objects if no name directly given -->
    <xsl:variable name="gs_object_name_fragment">
      <xsl:choose>
        <xsl:when test="not(@name) or @name = '' or @name = 'inherit'"><xsl:value-of select="local-name($gs_class)"/></xsl:when>
        <xsl:otherwise><xsl:value-of select="@name"/></xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    <xsl:variable name="gs_object_name" select="string($gs_object_name_fragment)"/>

    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words">
      <xsl:with-param name="gs_name" select="$gs_object_name"/>
    </xsl:apply-templates>

    <xsl:if test="not($gs_object_name)">
      <xsl:text>if (window.console) {window.console.error("object without an @name");}</xsl:text>
    </xsl:if>
    <xsl:if test="$gs_object_name">
      <xsl:text>function </xsl:text>
      <xsl:value-of select="$gs_object_name"/>
      <xsl:text>(</xsl:text>
      <xsl:value-of select="xjs:parameters(javascript:init/@parameters, $gs_object_name)"/>
      <xsl:text>) {</xsl:text>
        <xsl:value-of select="$gs_newline"/>

        <!-- link to the Derived constructor function
          the this.constructor points to the BaseObject
          we do not want the prototype chain initialisation to re-write these properties below
            this.classDerivedConstructor=function User() {...};
          bIsCreateForInheritance: instanciate the object anyway, but without init
        -->
        <xsl:text>  if (!OO.isCreateForInheritance(arguments)) {</xsl:text>
        <xsl:value-of select="$gs_newline"/>

        <xsl:if test="not(javascript:init/@parameter-checks = 'off' or ancestor::javascript:code[1][@parameter-checks = 'off'])">
          <xsl:value-of select="xjs:parameter-checks(javascript:init/@parameters, $gs_object_name, 6, $gs_stage_dev)"/>
          <xsl:value-of select="$gs_newline"/>
        </xsl:if>

        <!-- useful for Debugging templates -->
        <xsl:apply-templates select="." mode="gs_javascript_before_method"/>

        <!-- cpfs(f) uses non-standard method history functions -->
        <xsl:if test="$gs_stage_dev">
          <xsl:text>      </xsl:text>
          <xsl:text>Function.cpfs(window.</xsl:text>
          <xsl:value-of select="$gs_object_name"/>
          <xsl:text>);</xsl:text>
          <xsl:value-of select="$gs_newline"/>
        </xsl:if>
      
        <xsl:apply-templates select="@requires"/>
        
        <!-- derived constructor only: top level construction information -->
        <xsl:text>    if (!this.classDerivedConstructor) {</xsl:text><xsl:value-of select="$gs_newline"/>
          <xsl:text>      this.classDerivedConstructor=</xsl:text><xsl:value-of select="$gs_object_name"/><xsl:text>;</xsl:text><xsl:value-of select="$gs_newline"/>
          <xsl:value-of select="$gs_newline"/>

          <!-- class:*/@is-singleton indicates that we only instanciate ONE of these Classes for the client
            this is usually because there is only one concept of this object
            e.g. the Server, HTMLWebpage, Database
            it CAN have multiple displays
            make this singleton available through the static_property singleton
              <Function>.singleton
            OO.setSingleton() will only warn if the singleton is already available
            singleton will always point to the FIRST object of this type
          -->
          <xsl:if test="$gs_class/@is-singleton">
            <xsl:text>    OO.setSingleton(this);</xsl:text>
            <xsl:value-of select="$gs_newline"/>
          </xsl:if>
        <xsl:text>    }</xsl:text>
        <xsl:value-of select="$gs_newline"/>

        <!-- TODO: component-of
          HTMLWebpage.Clipboard.copy()
        -->
        <!-- xsl:if test="$gs_class/@component-of">
          <xsl:value-of select="$gs_class/@component-of"/>
          <xsl:text>.</xsl:text>
          <xsl:value-of select="$gs_object_name"/>
          <xsl:text>= new </xsl:text>
          <xsl:value-of select="$gs_object_name"/>
          <xsl:text>();</xsl:text>
          <xsl:value-of select="$gs_newline"/>
        </xsl:if -->

        <!-- chain prototype constructor if present
          it MUST be non-present on BaseClasses otherwise the prototype chain will return the wrong value
          DatabaseObject.apply(this, arguments);
          TODO: create a warning if javascript:init chaining is on and the first arguments do not match the arguments of the base
        -->
        <xsl:if test="not(javascript:init/@chain = 'false') and $gs_class/class:*">
          <xsl:value-of select="$gs_newline"/>
          <xsl:text>      </xsl:text>
          <xsl:text>var oChainResult = OO.chainMethod(</xsl:text>
          <xsl:value-of select="$gs_object_name"/>
          <xsl:text>, this, undefined, arguments);</xsl:text>
          <xsl:value-of select="$gs_newline"/>
        </xsl:if>

        <!-- this.properties = blah -->
        <xsl:apply-templates select="javascript:object-property" mode="gs_in_object_init"/>

        <!-- object initialisation script in to the constructor -->
        <xsl:apply-templates select="javascript:init" mode="gs_in_object_init"/>

        <!-- /if (!bIsCreateForInheritance) -->
        <xsl:text>  }</xsl:text>
        <xsl:value-of select="$gs_newline"/>
      <xsl:text>}</xsl:text>
      <xsl:value-of select="$gs_newline"/>
      
      <xsl:value-of select="$gs_object_name"/>
      <xsl:text>.notDefinedClass=true;</xsl:text>
      <xsl:value-of select="$gs_newline"/>
      
      <!-- OO.inheritFrom(fDerivedClass, fBaseClass);
        non-gs-class singular inheritance, e.g. direct JavaScript objects like Array,Date,etc.
      -->
      <xsl:if test="string(@inherits-javascript-primitive)">
        <xsl:if test="@inherits-javascript-primitive = 'Object'">//</xsl:if>
        <xsl:text>OO.inheritFrom(</xsl:text>
        <xsl:value-of select="$gs_object_name"/>
        <xsl:text>, window.</xsl:text>
        <xsl:value-of select="@inherits-javascript-primitive"/>
        <xsl:text>);</xsl:text>
        <xsl:value-of select="$gs_newline"/>
      </xsl:if>

      <xsl:if test="$gs_class">
        <!-- TODO: ancestor class relation
          OO.registerComponent(HTMLWebpage, Toggle);
        -->
        <!-- xsl:variable name="gs_super_super_class" select="$gs_class/parent::class:*"/>
        <xsl:variable name="gs_super_super_classname" select="local-name($gs_super_super_class)"/>
        <xsl:variable name="gs_super_super_class_object" select="$gs_super_super_class/javascript:code/javascript:object[@name='inherit']"/>
        <xsl:if test="$gs_gs_class_prototype and $gs_super_super_class_object">
          <xsl:text>OO.registerComponent(</xsl:text>
          <xsl:value-of select="$gs_super_super_classname"/>,<xsl:value-of select="$gs_object_name"/>
          <xsl:text>);</xsl:text>
          <xsl:value-of select="$gs_newline"/>
        </xsl:if -->

        <!-- javascript xsd model -->
        <!-- xsl:apply-templates select="$gs_class/xsd:schema[not(@name)]" mode="gs_javascript_xsd_model">
          <xsl:with-param name="gs_object_name" select="$gs_object_name"/>
        </xsl:apply-templates -->

        <!-- so that inheritance can report ordering -->
        <xsl:value-of select="$gs_object_name"/><xsl:text>.inheritanceOrder = </xsl:text>
        <xsl:value-of select="$gs_inheritance_order"/>
        <xsl:text>;</xsl:text>
        <xsl:value-of select="$gs_newline"/>

        <!-- User.htmlContainer='ul' -->
        <xsl:apply-templates select="$gs_class/@*[namespace-uri()='']" mode="gs_javascript_class_attributes">
          <xsl:with-param name="gs_object_name" select="$gs_object_name"/>
        </xsl:apply-templates>

        <!-- registerDerivedClass(fDerivedClass, fBaseClass);
          MULTIPLE inheritance
          inherit only from class:* that have javascript:objects @name=inherit
          necessary inheritance here: new Base()
          so that prototype.methods can be declared on the oBase
          
          we sort in reverse inheritance-depth order so that any copy inheritance happens on the smallest class
          with the exception of library-wrappers like JQueryClass
        -->
        <xsl:apply-templates select="$gs_class/class:*/javascript:code/javascript:object[not(@name) or @name = 'inherit']" mode="gs_inheritance_render">
          <xsl:with-param name="gs_class" select="$gs_class"/>
          <xsl:with-param name="gs_object_name" select="$gs_object_name"/>
          <xsl:sort select="../../@is-library-wrapper" order="descending"/>
          <xsl:sort select="database:base-class-count(../..)" data-type="number" order="descending"/>
        </xsl:apply-templates>

        <!-- javascript controller template functions 
          using this.updateWithControllerTemplate()
        -->
        <xsl:apply-templates select="$gs_class/xsl:stylesheet[str:boolean(@controller)]/xsl:template" mode="gs_javascript_xsl_templates">
          <xsl:with-param name="gs_object_name" select="$gs_object_name"/>
        </xsl:apply-templates>
      </xsl:if>
      
      <xsl:apply-templates select="$gs_class/repository:friends/class:*" mode="gs_javascript_friends">
        <xsl:with-param name="gs_object_name" select="$gs_object_name"/>
      </xsl:apply-templates>

      <!-- prototype methods on the .prototype and all others including global-init 
        we @select=* because we do not want all the text
      -->
      <xsl:apply-templates select="javascript:*">
        <xsl:with-param name="gs_object_name" select="$gs_object_name"/>
      </xsl:apply-templates>

      <!-- OO.registerClass([Class]); -->
      <xsl:text>OO.registerClass(</xsl:text>
      <xsl:value-of select="$gs_object_name"/>
      <xsl:text>);</xsl:text>
      <xsl:value-of select="$gs_newline"/>

      <xsl:value-of select="$gs_newline"/>
      <xsl:value-of select="$gs_newline"/>
    </xsl:if>
  </xsl:template>

  <xsl:template match="javascript:object/javascript:object">
    <xsl:comment>sub-objects [<xsl:value-of select="../@xml:id"/>/<xsl:value-of select="@xml:id"/>] not supported yet</xsl:comment>
  </xsl:template>

  <xsl:template match="*" mode="gs_context_menu_views"/>
  <xsl:template match="*" mode="gs_context_menu_controllers"/>

  <xsl:template match="@base-class-count|@name|@is-library-wrapper" mode="gs_javascript_class_attributes"/>

  <xsl:template match="@*" mode="gs_javascript_class_attributes">
    <xsl:param name="gs_object_name"/>
    <xsl:text>OO.addCP(</xsl:text>
    <xsl:value-of select="$gs_object_name"/>
    <xsl:text>,"</xsl:text><xsl:value-of select="name()"/><xsl:text>"</xsl:text>
    <xsl:text>,</xsl:text>
    <xsl:text>"</xsl:text><xsl:value-of select="." disable-output-escaping="yes"/><xsl:text>"</xsl:text>
    <xsl:text>);</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template>

  <xsl:template match="javascript:object" mode="gs_inheritance_render_base_list">
    <xsl:value-of select="local-name(../parent::class:*)"/>
    <xsl:if test="not(position() = last())">,</xsl:if>
  </xsl:template>

  <xsl:template match="javascript:object" mode="gs_inheritance_render_init_chain">
    <xsl:value-of select="local-name(../parent::class:*)"/>
    <xsl:text>.apply(this, arguments);</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template>
 
  <xsl:template match="class:*" mode="gs_javascript_friends">
    <xsl:param name="gs_object_name"/>
    
    <xsl:if test="$gs_stage_dev">
      <!-- @friends ship must be reciprocated by target class in-case one is instanciated before the other -->
      <xsl:text>OO.addFriend(</xsl:text>
      <xsl:value-of select="$gs_object_name"/>
      <xsl:text>,window.</xsl:text>
      <xsl:value-of select="local-name()"/>
      <xsl:text>);</xsl:text>
      <xsl:value-of select="$gs_newline"/>
    </xsl:if>
  </xsl:template>

  <xsl:template match="javascript:object" mode="gs_inheritance_render">
    <!-- javascript:object's in the base class(es) of this $gs_class
      acts upon $gs_class/class:*/javascript:code/javascript:object
      implicitly causes the prototype inheritance with OO.inheritFrom(DatabaseQuery, Options);
      OO.registerDerivedClass(window.User, window.DatabaseObject);
      TODO: what if the $gs_gs_class_prototype is not created yet? ORDER!
    -->
    <xsl:param name="gs_class"/>
    <xsl:param name="gs_object_name"/>

    <xsl:variable name="gs_base_class" select="ancestor::class:*[1]"/>

    <xsl:text>OO.registerDerivedClass(</xsl:text>
    <xsl:value-of select="$gs_object_name"/>
    <xsl:text>, window.</xsl:text><xsl:value-of select="local-name($gs_base_class)"/>
    <xsl:text>);</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template>

  <xsl:template match="javascript:object/javascript:init" mode="gs_in_object_init">
    <!-- standard gs_in_object_init behavior: include the text of the method
      this is overridden in javascript_gs.xsl templates to include things like arguments checking
    -->
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="javascript:object/javascript:init"/>

  <xsl:template match="javascript:enum">
    <xsl:param name="gs_object_name"/>

    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>

    <xsl:value-of select="$gs_object_name"/><xsl:text>.</xsl:text><xsl:value-of select="@name"/><xsl:text> = {</xsl:text>
    <xsl:apply-templates/>
    <xsl:text>};</xsl:text>
  </xsl:template>

  <xsl:template match="javascript:conversion" mode="gs_function_name">
    <xsl:value-of select="concat('to', @to-type)"/>
  </xsl:template>

  <xsl:template match="javascript:enum/javascript:value">
    <xsl:text>"</xsl:text><xsl:value-of select="@name"/><xsl:text>"</xsl:text>:<xsl:value-of select="."/><xsl:text>,</xsl:text>
  </xsl:template>

  <xsl:template match="javascript:object/javascript:object-property"/>

  <xsl:template match="javascript:object/javascript:object-property" mode="gs_in_object_init">
    <!-- new object[name] = value
      new for every object
      declared within the javascript:init
    -->
    <xsl:param name="gs_object_name"/>

    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>

    <xsl:text>this.</xsl:text>
    <xsl:value-of select="@name"/>
    <xsl:text> = </xsl:text>
    <xsl:if test="not(node())">undefined</xsl:if>
    <xsl:apply-templates/>
    <xsl:text>;</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template>

  <xsl:template match="javascript:object/javascript:fixed-prototype-property">
    <!-- object.prototype.name = value;
      useful when the value needs to be available on the prototype and thus the inherited prototype for other objects
    -->
    <xsl:param name="gs_object_name"/>

    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>

    <xsl:variable name="gs_prototype_path">
      <xsl:value-of select="$gs_object_name"/><xsl:text>.prototype.</xsl:text><xsl:value-of select="@name"/>
    </xsl:variable>

    <xsl:value-of select="$gs_prototype_path"/>
    <xsl:text> = </xsl:text>
    <xsl:if test="not(node())">undefined</xsl:if>
    <xsl:apply-templates/>
    <xsl:text>;</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template>

  <xsl:template match="*|@*|text()" mode="gs_check_javascript_reserved_words">
    <!-- not client side compatable: is included in the HTTP JS handler -->
  </xsl:template>

  <xsl:template match="*" mode="gs_javascript_function_init"/>
  <xsl:template match="*" mode="gs_javascript_after_function"/>

  <xsl:template match="@name|@parameters|@xml:id|@is-protected|@parameter-checks" mode="gs_function_attributes"/>
  <xsl:template match="javascript:static-method/@*|javascript:conversion/@*" mode="gs_function_attributes"/>

  <xsl:template match="@*" mode="gs_function_attributes">
    <xsl:param name="gs_object_name"/>
    
    <xsl:text>OO.addMP(f,"</xsl:text>
    <xsl:value-of select="name()"/><xsl:text>"</xsl:text>
    <xsl:text>,</xsl:text>
    <xsl:text>"</xsl:text><xsl:value-of select="." disable-output-escaping="yes"/><xsl:text>"</xsl:text>
    <xsl:text>);</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template>

  <xsl:template match="javascript:event-handler" mode="gs_javascript_function_init">
    <xsl:variable name="gs_action" select="string(@action)"/>
    <xsl:variable name="gs_content" select="string(.)"/>
    <xsl:if test="not($gs_content) and $gs_action and ../javascript:method[@name = $gs_action]">
      <xsl:text>      </xsl:text>
      <xsl:text>var oDirectRunResult = this.</xsl:text>
      <xsl:value-of select="$gs_action"/>
      <xsl:text>.apply(this, arguments);</xsl:text>
      <xsl:value-of select="$gs_newline"/>
    </xsl:if>
  </xsl:template>

  <xsl:template match="javascript:static-event-handler" mode="gs_javascript_function_init">
    <xsl:variable name="gs_action" select="string(@action)"/>
    <xsl:variable name="gs_content" select="string(.)"/>
    <xsl:if test="not($gs_content) and $gs_action and ../javascript:static-method[@name = $gs_action]">
      <xsl:text>      </xsl:text>
      <xsl:text>var oDirectRunResult = this.</xsl:text>
      <xsl:value-of select="$gs_action"/>
      <xsl:text>.apply(this, arguments);</xsl:text>
      <xsl:value-of select="$gs_newline"/>
    </xsl:if>
  </xsl:template>

  <xsl:template match="javascript:*" mode="gs_function_name">
    <xsl:value-of select="@name"/>
  </xsl:template>

  <xsl:template match="javascript:event-handler|javascript:static-event-handler" mode="gs_function_name">
    <!-- @event, [@action, @chain=false]
      User.prototype.f_@event[_@action] = function() {
        var oChainResult     = OO.chainMethod(this ,...);  //@chain
        var oDirectRunResult = this[@action](...); //direct @action

        ...
      }
    -->
    <xsl:text>f_</xsl:text>
    <xsl:value-of select="@event"/>
    <xsl:if test="string(@action)">_<xsl:value-of select="@action"/></xsl:if>
  </xsl:template>

  <xsl:template match="javascript:method|javascript:conversion|javascript:event-handler|javascript:capability">
    <!--
      if ((object.prototype.name !== undefined) && window.console) window.console.warn(this, "function already defined on blah");
      object.prototype.name = function name(sParam, [oParam, bParam = true], ...) {
        [if (object.baseClass.prototype.name instanceof Function) object.baseClass.prototype.name.apply(this, arguments);]
        ...
      }
      capabilities return true if the class / object can-do. see also static-capability
    -->
    <xsl:param name="gs_object_name"/>

    <xsl:variable name="gs_function_name">
      <xsl:apply-templates select="." mode="gs_function_name"/>
    </xsl:variable>

    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>

    <xsl:variable name="gs_prototype_path">
      <xsl:value-of select="$gs_object_name"/><xsl:text>.prototype.</xsl:text><xsl:value-of select="$gs_function_name"/>
    </xsl:variable>

    <!-- function definition -->
    <xsl:text>var f=</xsl:text>
    <xsl:value-of select="$gs_prototype_path"/>
    <xsl:text>=function</xsl:text>
    <xsl:text> </xsl:text><xsl:value-of select="$gs_function_name"/>
    <xsl:text>(</xsl:text>
    <xsl:value-of select="xjs:parameters(@parameters, $gs_function_name)"/>
    <xsl:text>) {</xsl:text>
      <xsl:value-of select="$gs_newline"/>
      
      <!-- function additions -->
      <!-- xsl:text>//function injections by framework: can be switched off in production</xsl:text>
      <xsl:value-of select="$gs_newline"/ -->
      <xsl:apply-templates select="." mode="gs_javascript_before_method"/>
      
      <!-- cpfs(f, protected) uses non-standard method history functions -->
      <xsl:if test="$gs_stage_dev">
        <xsl:text>      </xsl:text>
        <xsl:text>Function.cpfs(</xsl:text>
        <xsl:value-of select="$gs_prototype_path"/>
        <xsl:text>,</xsl:text>
        <xsl:if test="@is-protected = 'true'">true</xsl:if>
        <xsl:if test="not(@is-protected = 'true')">false</xsl:if>
        <xsl:text>);</xsl:text>
        <xsl:value-of select="$gs_newline"/>
      </xsl:if>
      
      <xsl:apply-templates select="@requires"/>

      <xsl:apply-templates select="." mode="gs_javascript_function_init"/>

      <xsl:if test="@chain = 'true'">
        <xsl:text>      </xsl:text>
        <xsl:text>var fF=</xsl:text><xsl:value-of select="$gs_object_name"/><xsl:text>.prototype.</xsl:text><xsl:value-of select="$gs_function_name"/><xsl:text>;</xsl:text>
        <xsl:value-of select="$gs_newline"/>
        <xsl:text>      </xsl:text>
        <xsl:text>var oChainResult = OO.chainMethod(fF.classDerivedConstructor, this, fF.functionName(), arguments);</xsl:text>
        <xsl:value-of select="$gs_newline"/>
      </xsl:if>
      <xsl:if test="not(@parameter-checks = 'off' or ancestor::javascript:code[1][@parameter-checks = 'off'])">
        <xsl:value-of select="xjs:parameter-checks(@parameters, $gs_function_name, 6, $gs_stage_dev)"/>
      </xsl:if>

      <!-- function body -->
      <xsl:apply-templates/>
    <xsl:text>}</xsl:text>
    <xsl:value-of select="$gs_newline"/>

    <!-- f.functionType="method" -->
    <xsl:text>f.functionType="</xsl:text>
    <xsl:apply-templates select="." mode="gs_dash_to_camel">
      <xsl:with-param name="gs_string" select="local-name()"/>
    </xsl:apply-templates>
    <xsl:text>";</xsl:text>
    <xsl:value-of select="$gs_newline"/>
    
    <!-- LIVE: client still needs to know private methods so can ignore them in introspection -->
    <xsl:if test="@is-protected = 'true'">
      <xsl:text>f.isProtected=true;</xsl:text>
      <xsl:value-of select="$gs_newline"/>
    </xsl:if>

    <xsl:apply-templates select="@*" mode="gs_function_attributes"/>

    <xsl:apply-templates select="." mode="gs_javascript_after_function"/>
  </xsl:template>
  
  <xsl:template match="javascript:*/@requires">
    <xsl:param name="gs_requires" select="string(.)"/>
    
    <xsl:if test="$gs_stage_dev">
      <xsl:variable name="gs_requires_part_fragment">
        <xsl:value-of select="substring-before($gs_requires, ',')"/>
        <xsl:if test="not(contains($gs_requires, ','))"><xsl:value-of select="$gs_requires"/></xsl:if>
      </xsl:variable>
      <xsl:variable name="gs_requires_part" select="normalize-space($gs_requires_part_fragment)"/>
      
      <xsl:if test="$gs_requires_part">
        <xsl:text>      </xsl:text>
        <xsl:text>if ((window.</xsl:text>
        <xsl:value-of select="$gs_requires_part"/>
        <xsl:text> === undefined)) Debug.error(this, "[window.</xsl:text>
        <xsl:value-of select="$gs_requires_part"/>
        <xsl:text>] required for [</xsl:text>
        <xsl:value-of select="../@name"/>
        <xsl:text>]");</xsl:text>
        <xsl:value-of select="$gs_newline"/>
      </xsl:if>
      
      <xsl:if test="contains($gs_requires, ',')">
        <xsl:apply-templates select=".">
          <xsl:with-param name="gs_requires" select="substring-after($gs_requires, ',')"/>
        </xsl:apply-templates>
      </xsl:if>
    </xsl:if>
  </xsl:template>

  <xsl:template match="javascript:object/javascript:static-method|javascript:namespace/javascript:static-method|javascript:object/javascript:static-event-handler|javascript:object/javascript:static-capability">
    <!-- User.login = function login() {...}
      User.prototype.login = User.login;
    -->
    <xsl:param name="gs_object_name"/>
    
    <xsl:variable name="gs_function_name">
      <xsl:apply-templates select="." mode="gs_function_name"/>
    </xsl:variable>

    <xsl:variable name="gs_prototype_path">
      <xsl:value-of select="$gs_object_name"/><xsl:text>.</xsl:text><xsl:value-of select="$gs_function_name"/>
    </xsl:variable>

    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>

    <xsl:text>var f=</xsl:text>
    <xsl:value-of select="$gs_prototype_path"/>
    <xsl:text> = function</xsl:text>
    <xsl:text> </xsl:text><xsl:value-of select="$gs_function_name"/>
    <xsl:text>(</xsl:text>
    <xsl:value-of select="xjs:parameters(@parameters, $gs_function_name)"/>
    <xsl:text>) {</xsl:text>
      <xsl:value-of select="$gs_newline"/>
      
      <xsl:apply-templates select="." mode="gs_javascript_before_method"/>

      <!-- cpfs(f, protected) uses non-standard method history functions -->
      <xsl:if test="$gs_stage_dev">
        <xsl:text>      </xsl:text>
        <xsl:text>Function.cpfs(</xsl:text>
        <xsl:value-of select="$gs_prototype_path"/>
        <xsl:text>,</xsl:text>
        <xsl:if test="@is-protected = 'true'">true</xsl:if>
        <xsl:if test="not(@is-protected = 'true')">false</xsl:if>
        <xsl:text>);</xsl:text>
        <xsl:value-of select="$gs_newline"/>
      </xsl:if>
      
      <xsl:apply-templates select="@requires"/>

      <xsl:apply-templates select="." mode="gs_javascript_function_init"/>
      
      <xsl:if test="not(@parameter-checks = 'off' or ancestor::javascript:code[1][@parameter-checks = 'off'])">
        <xsl:value-of select="xjs:parameter-checks(@parameters, $gs_function_name, 6, $gs_stage_dev)"/>
      </xsl:if>
      
      <xsl:apply-templates/>
    <xsl:text>}</xsl:text>
    <xsl:value-of select="$gs_newline"/>

    <!-- LIVE: client still needs to know private methods so can ignore them in introspection -->
    <xsl:if test="@is-protected = 'true'">
      <xsl:text>f.isProtected=true;</xsl:text>
      <xsl:value-of select="$gs_newline"/>
    </xsl:if>

    <!-- make it available also on the prototype -->
    <!-- xsl:value-of select="$gs_object_name"/><xsl:text>.prototype.</xsl:text><xsl:value-of select="$gs_function_name"/>
    <xsl:text>=f;</xsl:text>
    <xsl:value-of select="$gs_newline"/ -->

    <!-- f.functionType="method" -->
    <xsl:text>f.functionType="</xsl:text>
    <xsl:apply-templates select="." mode="gs_dash_to_camel">
      <xsl:with-param name="gs_string" select="local-name()"/>
    </xsl:apply-templates>
    <xsl:text>";</xsl:text>
    <xsl:value-of select="$gs_newline"/>

    <xsl:apply-templates select="@*" mode="gs_function_attributes"/>

    <xsl:apply-templates select="." mode="gs_javascript_after_function"/>
  </xsl:template>

  <xsl:template match="javascript:object/javascript:static-property|javascript:namespace/javascript:static-property">
    <!-- object.name = value
      setTimeout(object.name = value, 0); //in the case of non-literals
    -->
    <xsl:param name="gs_object_name"/>
    
    <xsl:variable name="gs_property_path">
      <xsl:value-of select="$gs_object_name"/><xsl:text>.</xsl:text><xsl:value-of select="@name"/>
    </xsl:variable>

    <xsl:apply-templates select="." mode="gs_check_javascript_reserved_words"/>

    <xsl:variable name="gs_property_value" select="normalize-space(.)"/>
    <xsl:variable name="gs_is_literal" select="not($gs_property_value) or (starts-with($gs_property_value, '$') or starts-with($gs_property_value, '/') or starts-with($gs_property_value, '{') or starts-with($gs_property_value, '[') or starts-with($gs_property_value, $gs_quote) or starts-with($gs_property_value, $gs_apos) or $gs_property_value = 'true' or $gs_property_value = 'false' or (translate($gs_property_value, concat($gs_uppercase, '_'), '') = '') or (translate($gs_property_value, '0123456789.', '') = '') or $gs_property_value = 'new Object()' or $gs_property_value = 'jQuery()' or $gs_property_value = 'new Array()' or $gs_property_value = 'new Cache()')"/>
    <xsl:variable name="gs_delay" select="$gs_property_value and not(@delay = 'false') and not($gs_is_literal)"/>

    <xsl:if test="$gs_delay">
      <xsl:text>//delayed because it seems to be a non-literal</xsl:text>
      <xsl:value-of select="$gs_newline"/>
      <xsl:text>setTimeout(function(){</xsl:text>
      <xsl:value-of select="$gs_newline"/>
    </xsl:if>
      <xsl:value-of select="$gs_property_path"/>
      <xsl:text> = </xsl:text>
      <xsl:apply-templates/> <!-- maintain the layout of the value as it may span several lines (not value-of) -->
      <xsl:if test="not($gs_property_value)">undefined</xsl:if>
      <xsl:text>;</xsl:text>
      <xsl:value-of select="$gs_newline"/>
    <xsl:if test="$gs_delay">
      <xsl:text>}, 0);</xsl:text>
      <xsl:value-of select="$gs_newline"/>
    </xsl:if>
  </xsl:template>

  <xsl:template match="xsd:schema" mode="gs_javascript_xsd_model">
    <xsl:param name="gs_object_name"/>
  
    <!-- User.model = jQuery('<xsd:schema>...</xsd:schema>'); -->
    <xsl:value-of select="$gs_object_name"/><xsl:text>.model = jQuery('</xsl:text>
    <!-- TODO: .model = jQuery('...') should we be setting up a separate introspection MI? -->
    <!-- xsl:copy-of select="xsd:complexType[1]/xsd:sequence[1]"/ -->
    <xsl:text>');</xsl:text>
    <xsl:value-of select="$gs_newline"/>
  </xsl:template>

  <!-- ############################################### editor mode javascript children ############################################### -->
  <xsl:template match="javascript:object" mode="editor">
    <pre>
      <xsl:apply-templates select="."/>
    </pre>
  </xsl:template>

  <xsl:template match="javascript:dependency" mode="editor">
    <li><xsl:value-of select="@uri"/></li>
  </xsl:template>

  <!-- ############################################### introspection ############################################### -->
  <xsl:template match="javascript:object" mode="eventhandler" meta:interface-template="yes">
    <xsl:param name="gs_html_identifier_class"/>

    <!-- we always want a record of those objects involved -->
    <li class="{$gs_html_identifier_class} gs-semantic-only-markup">
      <ul class="CSS__VerticalMenu gs-interface-mode-default gs-semantic-only-markup">
        <xsl:if test="not(*)">
          <li class="gs-empty-message">no event handlers</li>
        </xsl:if>
        <xsl:apply-templates mode="gs_view_render">
          <xsl:with-param name="gs_interface_mode" select="'eventhandler'"/>
        </xsl:apply-templates>
        <li class="gs-add-message"><a href="#">add</a> an event-handler</li>
      </ul>
    </li>
  </xsl:template>

  <xsl:template match="javascript:event-handler" mode="eventhandler" meta:interface-template="yes">
    <xsl:param name="gs_html_identifier_class"/>
    <xsl:param name="gs_interface_mode"/>

    <li class="{$gs_html_identifier_class} gs-xsl-javascript-function">
      <xsl:apply-templates select="." mode="gs_meta_data_standard_groups">
        <xsl:with-param name="gs_interface_mode" select="$gs_interface_mode"/>
      </xsl:apply-templates>
      <xsl:text>f_</xsl:text>
      <xsl:value-of select="@event"/>
      <xsl:if test="@action">_<xsl:value-of select="@action"/></xsl:if>
      <xsl:if test="@selector"> (<xsl:value-of select="@selector" disable-output-escaping="yes"/>)</xsl:if>
    </li>
  </xsl:template>
</xsl:stylesheet>
