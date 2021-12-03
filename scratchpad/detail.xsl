<xsl:stylesheet xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns:object="http://general_server.org/xmlnamespaces/object/2006" xmlns:css="http://general_server.org/xmlnamespaces/css/2006" xmlns:javascript="http://general_server.org/xmlnamespaces/javascript/2006" xmlns="http://www.w3.org/1999/xhtml" xmlns:html="http://www.w3.org/1999/xhtml" xmlns:server="http://general_server.org/xmlnamespaces/server/2006" xmlns:DB="http://general_server.org/xmlnamespaces/DB/2006" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:gs="http://general_server.org/xmlnamespaces/general_server/2006" xmlns:class="http://general_server.org/xmlnamespaces/class/2006" xmlns:dyn="http://exslt.org/dynamic" xmlns:xmlsecurity="http://general_server.org/xmlnamespaces/xmlsecurity/2006" version="1.0">
  <!-- TODO: remove this test_include.xsl is to test server side transform filesystem path -> xpath translations -->
  <xsl:include href="../shared/client-side-transforms/classes.xsl"/>

  <gs:stylesheet_settings system_menu="on" copyright="on" classes-injection="all"/>

  <!-- ******************************** data ******************************** -->
  <!-- note that the <node> XML content is retrieved using an AJAX API call to create an client side XML document with CDATA
       this node call is purely for the attributes and children analysis -->
  <!-- gs:query name="full" database="config" system-transform="full">id($request/gs:query/gs:xmlid)/ancestor::*[$default_database/repository:classes]</gs:query -->
  <gs:query name="node" database="config" system-transform="self_and_immediate_children">id($request/gs:query/gs:xmlid) | dyn:evaluate($request/gs:query/gs:xpath)</gs:query>
  <gs:query name="owner" database="config" system-transform="all_with_suppress_html_loading_elements" optional="yes">/gs:root/object:Server/repository:users/object:User[@id = id($request/gs:query/gs:xmlid)/@xmlsecurity:ownerid]</gs:query>

  <!-- ******************************** attribute form processing ******************************** -->
  <gs:form name="add attribute" action="set-attribute" select="id($request/gs:query/gs:xmlid) | dyn:evaluate($request/gs:query/gs:xpath)" attribute_name="gs:attribute_name" new_value="gs:value"/>
  <gs:form name="change" action="set-attribute" select="id($request/gs:query/gs:xmlid) | dyn:evaluate($request/gs:query/gs:xpath)" attribute_name="gs:attribute_name" new_value="gs:new_value"/>
  <gs:form name="delete" action="remove" select="id($request/gs:query/gs:xmlid) | dyn:evaluate($request/gs:query/gs:xpath)" attribute_name="gs:attribute_name"/>

  <!-- ******************************** node form processing ******************************** -->
  <gs:form name="delete node" action="remove-node" select="id($request/gs:query/gs:xmlid)"/>
  <gs:form name="add child" action="move-child" destination="id($request/gs:query/gs:xmlid) | dyn:evaluate($request/gs:query/gs:xpath)" transform-with="this"/>
  <xsl:template match="gs:form" mode="add_child">
    <xsl:element name="{gs:node_name}"><xsl:value-of select="gs:content"/></xsl:element>
  </xsl:template>
  <!-- NOTE: this one not used anymore because of xml:id and other analysis needed together:
  gs:form name="save xml" action="replace-node"
    select="DB:parse-xml(gs:xml)"
    destination="id($request/gs:query/gs:xmlid)" / -->
  <gs:form name="save node" action="replace-node" destination="id($request/gs:query/gs:xmlid) | dyn:evaluate($request/gs:query/gs:xpath)" xml="gs:xml"/>

  <xsl:variable name="query" select="/gs:root/gs:input/gs:query"/>
  <xsl:variable name="xpath_to_node">
    <xsl:if test="$query/gs:xmlid">id('<xsl:value-of select="$query/gs:xmlid"/>')</xsl:if>
    <xsl:if test="$query/gs:xpath"><xsl:value-of select="$query/gs:xpath"/></xsl:if>
  </xsl:variable>
  <xsl:variable name="this_node" select="/gs:root/gs:data/gs:node/*"/>
  <xsl:variable name="action">
    <xsl:text>detail?</xsl:text>
    <xsl:if test="$query/gs:xmlid">xmlid=<xsl:value-of select="$query/gs:xmlid"/></xsl:if>
    <xsl:if test="$query/gs:xpath">xpath=<xsl:value-of select="$query/gs:xpath"/></xsl:if>
  </xsl:variable>

  <xsl:template match="gs:root">
    <html>
      <head>
        <!-- CodeMirror -->
        <xsl:variable name="code-mirror-version" select="'3.20'"/>
        <link rel="stylesheet" href="/resources/shared/codemirror-{$code-mirror-version}/lib/codemirror.css"/>
        <script src="/resources/shared/codemirror-{$code-mirror-version}/lib/codemirror.js"/>
        <script src="/resources/shared/codemirror-{$code-mirror-version}/addon/fold/foldcode.js"/>
        <script src="/resources/shared/codemirror-{$code-mirror-version}/mode/javascript/javascript.js"/>
        <script src="/resources/shared/codemirror-{$code-mirror-version}/mode/xml/xml.js"/>

        <!-- main details CSS -->
        <link rel="stylesheet" type="text/css" href="admin.css"/>
        <css:stylesheet xmlns="http://general_server.org/xmlnamespaces/css/2006" type="text/css">
          <css:raw><![CDATA[
            body {
              padding:0 10px;
            }
            .test_output {display:none;font-size:10px;color:#3377dd;}
            .permission_bit {padding-right:5px;}
            #gs_form_menu input {
              display:block;
              width:90px;
            }
            #xmledit {width:100%;height:200px;}

            .CodeMirror {
              width:100%;
              background-color:#dddddd;
              font-style:italic; /* loading... */
              padding-bottom:20px;
            }
            .CodeMirror-scroll {
              overflow-y: auto;
              overflow-x: auto;
            }
            .CodeMirror.cm-loaded {
              background-color:#ffffff;
              font-style:normal;
            }
            .CodeMirror-foldmarker {
              color: blue;
              text-shadow: #b9f 1px 1px 2px, #b9f -1px -1px 2px, #b9f 1px -1px 2px, #b9f -1px 1px 2px;
              font-family: arial;
              line-height: .3;
              cursor: pointer;
            }
          ]]></css:raw>
        </css:stylesheet>

        <javascript:raw list_name="editor loader"><![CDATA[
          var oEditor, jEditor, jEditorTextArea, sEditorXPath;

          function setEditorDocument(oXMLDoc) {
            //http://www.w3schools.com/dom/dom_node.asp
            //replace encoded gs:cdataenc elements with CDATA sections
            /*
            var oGSCDATAEncNode, oNewCDATANode, oParentNode;
            var vGSCDATAEncNodes = oXMLDoc.getElementsByTagName('gs:cdataenc');
            cgroup('[' + vGSCDATAEncNodes.length + '] gs:cdataenc found in xml edit area');
            while (vGSCDATAEncNodes.length) {
              oGSCDATAEncNode = vGSCDATAEncNodes.item(0);
              oNewCDATANode   = oXMLDoc.createCDATASection(oGSCDATAEncNode.textContent);
              oParentNode     = oGSCDATAEncNode.parentNode;
              oParentNode.replaceChild(oNewCDATANode, oGSCDATAEncNode);
              cinfo('replaced gs:cdataenc with CDATA element');
            }
            cgroupend();
            */

            //serialise
            var oSerialiser = new XMLSerializer();
            var oRoot, oSingleNode;
            var sXML;

            if (oRoot = oXMLDoc.firstChild) {
              if (oSingleNode = oRoot.firstChild) {
                sXML = oSerialiser.serializeToString(oSingleNode);
                oEditor.setValue(sXML);
              }
            }

            //mark complete
            $(".CodeMirror").addClass("cm-loaded");
            $("#xmledit_submit").removeAttr("disabled");
            oEditor.loadedXML = true;
          }

          function setEditorHeight() {
            var browserHeight = document.documentElement.clientHeight;
            var spaceHeight   = browserHeight -58;
            if (oEditor) {
              oEditor.getWrapperElement().style.height = spaceHeight + 'px';
              oEditor.refresh();
            }
          }

          function loadXMLEditor() {
            //load editor
            jEditor          = $(".codemirror");
            jEditorTextArea  = jEditor.find("textarea");
            sEditorXPath     = jEditor.find(".gs_ajax_xpath").text();

            oEditor = CodeMirror.fromTextArea(jEditorTextArea.get(0), {
              mode: "text/xml",
              lineNumbers: false,
              lineWrapping: false
            });
            oEditor.loadedXML = false;

            //height management
            setEditorHeight();
            $(window).resize(setEditorHeight);

            //get XML contents
            //using AJAX to create an XML document instead of a HTML document
            //to preserve CDATA
            $.ajax({
              type: "GET",
              dataType: "xml",
              url: "/api/" + sEditorXPath,
              cache: false,
              success: setEditorDocument,
              error: function(msg) {alert('oops, failed to get XML content for the editor.');}
            });
          }

          $(document).ready(function(){
            //load the editor on demand
            //also because we want to render it properly
            $(document).bind("tabsactivate", function(event, ui) {
              if (ui.newPanel.attr("id") == "tabs-xmleditor" && !oEditor) loadXMLEditor();
            });

            $(document).bind("save", function(event){
              //NOTE: the submit button will be disabled if the editor is not loaded yet
              $("#xmledit_submit").click();
            });
          });


          $(window).keypress(function (event) {
            //simply disables save event for chrome
            if (!(event.which == 115 && (navigator.platform.match("Mac") ? event.metaKey : event.ctrlKey)) && !(event.which == 19)) return true;
            event.preventDefault();
            return false;
          });

          // used to process the cmd+s and ctrl+s events
          $(document).keydown(function (event) {
            if (event.which == 83 && (navigator.platform.match("Mac") ? event.metaKey : event.ctrlKey)) {
              event.preventDefault();
              $(document).trigger("save");
              return false;
            }
          });
        ]]></javascript:raw>
      </head>

      <body>
        <div class="notabs">
          <ul>
            <li><a href="#tabs-main"><xsl:value-of select="name($this_node)"/></a></li>
            <li><a href="#tabs-properties">properties</a></li>
            <li><a href="#tabs-xmleditor">xml editor</a></li>
          </ul>

          <div id="tabs-main">
            <xsl:apply-templates select="gs:data/gs:node/*" mode="full"/>
          </div>

          <div id="tabs-xmleditor">
            <form id="savexml" action="{$action}" method="post"><div>
              <div class="codemirror">
                <textarea name="xml">
                  loading...
                </textarea>
                <span class="xtransport gs_ajax_xpath"><xsl:value-of select="$xpath_to_node"/></span>
              </div>

              <div id="gs_form_menu">
                <input id="xmledit_submit" name="submit" disabled="1" value="save node" type="submit"/>
                <input name="submit" value="delete node" type="submit"/>
              </div>
            </div></form>
          </div>

          <div id="tabs-properties">
            <h3>attributes</h3>
            <table><xsl:apply-templates select="gs:data/gs:node/*/@*" mode="general_attributes"/></table>
            <form action="{$action}" method="post"><div>
              <input class="textfield_small" name="attribute_name" value=""/>
              <input class="textfield_small" name="value" value=""/>
              <input name="submit" value="add attribute" type="submit"/>
            </div></form>

            <h3>children</h3>
            <ul id="tree"><xsl:apply-templates select="gs:data/gs:node/*/*"/></ul>
            <form action="{$action}" method="post"><div>
              <input class="textfield_small" name="node_name" value=""/>
              <input class="textfield_small" name="content" value=""/>
              <input name="submit" value="add child" type="submit"/>
            </div></form>

            <h3>security</h3>
            <xsl:apply-templates select="gs:data/gs:owner/object:User" mode="security"/>
            <ul>
              <xsl:apply-templates select="gs:data/gs:node/*/@*[substring-before(name(), ':') = 'xmlsecurity']" mode="security"/>
            </ul>
          </div>
        </div>
      </body>
    </html>
  </xsl:template>

  <xsl:template match="/gs:root/gs:input|/gs:root/gs:session"/>

  <!-- **************************************** attributes **************************************** -->
  <xsl:template match="gs:node/*/@*" mode="general_attributes">
    <tr>
      <td><xsl:value-of select="name()"/></td>
      <td>
        <form class="inline" action="{$action}" method="post"><div class="inline">
          <input type="hidden" name="attribute_name" value="{name()}"/>
          <input class="textfield" name="new_value" value="{.}"/>
          <input name="submit" value="change" type="submit"/>
        </div></form>
        <form class="inline" action="{$action}" method="post"><div class="inline">
          <input type="hidden" name="attribute_name" value="{name()}"/>
          <input name="submit" value="delete" type="submit"/>
        </div></form>
      </td>
    </tr>
  </xsl:template>

  <xsl:template match="gs:data/gs:node/*/@*[substring-before(name(), ':') = 'xmlsecurity']" mode="general_attributes">
    <!-- only way to get Firefox to recognise attribute namespaces -->
  </xsl:template>

  <!-- **************************************** children **************************************** -->
  <xsl:template match="gs:node/*/*">
    <li class="{local-name()}_li {substring-before(name(), ':')}">
      <div class="details {local-name()}_details">
        <xsl:if test="contains(name(), ':')">
          <span class="namespace_prefix"><xsl:value-of select="substring-before(name(), ':')"/><xsl:text>:</xsl:text></span>
        </xsl:if>
        <span class="name"><xsl:value-of select="local-name()"/></span>
      </div>
    </li>
  </xsl:template>

  <!-- **************************************** test output **************************************** -->
  <xsl:template match="gs:test_output">
    <div class="test_output"><xsl:apply-templates/></div>
  </xsl:template>

  <!-- **************************************** security **************************************** -->
  <xsl:template match="gs:data/gs:node/*/@*[substring-before(name(), ':') = 'xmlsecurity']" mode="security">
    <!-- only way to get Firefox to recognise attribute namespaces -->
    <li><xsl:value-of select="local-name()"/>: <xsl:value-of select="."/></li>
  </xsl:template>

  <xsl:template match="gs:owner/object:User" mode="security">
    <img src="/resources/shared/images/icons/{@tree_icon}.png"/><xsl:value-of select="@name"/>
  </xsl:template>

  <xsl:template match="gs:data/gs:node/*/@*[name() = 'xmlsecurity:permissions']" mode="security">
    <!-- only way to get Firefox to recognise attribute namespaces -->
    <li>
      <ul>
        <li>owner: <xsl:call-template name="unix_permission"><xsl:with-param name="digit" select="substring(., 1, 1)"/></xsl:call-template></li>
        <li>group: <xsl:call-template name="unix_permission"><xsl:with-param name="digit" select="substring(., 2, 1)"/></xsl:call-template></li>
        <li>other: <xsl:call-template name="unix_permission"><xsl:with-param name="digit" select="substring(., 3, 1)"/></xsl:call-template></li>
      </ul>
    </li>
  </xsl:template>

  <xsl:template name="unix_permission">
    <xsl:param name="digit"/>
    <xsl:if test="floor($digit div 4) mod 2 = 1"><span class="permission_bit">read</span></xsl:if>
    <xsl:if test="floor($digit div 2) mod 2 = 1"><span class="permission_bit">write</span></xsl:if>
    <xsl:if test="floor($digit div 1) mod 2 = 1"><span class="permission_bit">execute</span></xsl:if>
  </xsl:template>
</xsl:stylesheet>