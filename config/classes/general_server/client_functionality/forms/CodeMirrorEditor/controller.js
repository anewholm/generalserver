<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/javascript/2006">
  <!-- these dependencies will get rendered outside the script tags -->
  <javascript:dependency uri="/resources/shared/codemirror/codemirror-3.20/lib/codemirror.js"/>
  <javascript:dependency uri="/resources/shared/codemirror/codemirror-3.20/addon/fold/foldcode.js"/>
  <javascript:dependency uri="/resources/shared/codemirror/codemirror-3.20/mode/javascript/javascript.js"/>
  <javascript:dependency uri="/resources/shared/codemirror/codemirror-3.20/mode/xml/xml.js"/>
  <javascript:dependency uri="/resources/shared/codemirror/gs-xml.js"/>
  <javascript:dependency uri="/resources/shared/codemirror/codemirror-3.1/addon/format/formatting.js"/>

  <javascript:object>
    <javascript:static-property delay="false" name="oCodemirrorOptions"><![CDATA[
      {
        mode: "gs-xml",
        lineNumbers:  false,
        lineWrapping: false,
        viewportMargin: Infinity //bad performance effects for large documents! CodeMirror progrssively renders
      }
    ]]></javascript:static-property>

    <javascript:static-property delay="false" name="oAPIOptions"><![CDATA[
      {
        // "schema":"root-only",            //returns jDataChildren
        copyQueryAttributes:"no",
        createMetaContextAttributes:"none",
        nodeMask:           false,               //prevent standard limiting node-mask
        hardlinkOutput:     "placeholder",       //create <xml:hardlink target-id="...">s
        databaseQueryOutput:"placeholder",
        dataRenderOutput:   "placeholder"
      }
    ]]></javascript:static-property>

    <javascript:method name="displayAdded_xpathajax" parameters="j1DisplayObject" requires="CodeMirror" chain="true"><![CDATA[
      //this object that can govern multiple interface:CodeMirrorEditor jElement instances simultaneously
      var self = this;
      //layout
      var jTextarea    = j1DisplayObject.children("textarea");
      var sEditorXPath = j1DisplayObject.find(".gs-ajax-xpath").text();
      
      if (jTextarea.length) {
        j1DisplayObject.o.editor = CodeMirror.fromTextArea(jTextarea.get(0), CodeMirrorEditor.oCodemirrorOptions);
        if (j1DisplayObject.o.editor) {
          j1DisplayObject.o.editor.displayObject = j1DisplayObject;
          //AJAX auto-load from xpath
          if (sEditorXPath) this.loadXML(j1DisplayObject, sEditorXPath);
          else this.setError(j1DisplayObject, "gs-ajax-xpath is blank, no data for CodeMirror");
        } else this.setError(j1DisplayObject, "failed to instanciate CodeMirro instance");
      } else this.setError(j1DisplayObject, "CodeMirror has no textarea to link to");
    ]]></javascript:method>

    <javascript:event-handler event="presubmit" action="updateFormElement" parameters="j1DisplayObject"><![CDATA[
      //we explicitly declare the handler here otherwise "presubmit" will not be added to the window events
      this.updateFormElement(j1DisplayObject);
    ]]></javascript:event-handler>

    <javascript:event-handler event="click" action="setBreakpoint" parameters="j1DisplayObject" requires="User,Database"><![CDATA[
      //------------------- lineHandle:
      //  window.CodeMirror.CodeMirror.Line:
      //    height: 1
      //    parent: LeafChunk
      //    stateAfter: Object
      //      cc: Array[0]
      //      context: Object
      //      indented: 0
      //      startOfLine: false
      //      stringStartCol: 144
      //      tagName: null
      //      tagStart: null
      //      tokenize: function inText(stream, state) {
      //      type: "endTag"
      //    styles: Array[37] 1, 16, "tag", 17, null, 22, "attribute", 23, null, ...
      //    text: "<javascript:code xmlns="http://general_server.org/xmlnamespaces/javascript/2006" >"
      var self           = this;
      var oEditor        = j1DisplayObject.editor;
      var oPosition      = oEditor.getCursor(); //returns {line, ch}
      var iLine          = oPosition.line;
      var oLineHandle    = oEditor.getLineHandle(iLine);
      var aXMLIds        = oLineHandle.text.match(/ meta:xpath-to-node="([^"]+)"/);
      var sXmlId         = null;

      if (aXMLIds && aXMLIds.length >= 2) {
        sXmlId = aXMLIds[1];
        User.setBreakpoint(
          sXmlId,
          function(oXMLDoc) {self.breakpointSet(oEditor, sXmlId, iLine);},
          function(sMessage){self.setContentError(oEditor, sMessage);}
        );
      }
    ]]></javascript:event-handler>

    <javascript:method name="breakpointSet" is-protected="true" parameters="oEditor, sXPath, iLine"><![CDATA[
      oEditor.setBookmark({line:iLine, ch:0});
      oEditor.addLineClass(iLine, "background", "gs-breakpoint");
      //oEditor.setGutterMarker(iLine, gutterID, DOM);
    ]]></javascript:method>

    <javascript:method name="loadXML" is-protected="true" parameters="j1DisplayObject, s1EditorXPath" requires="Database"><![CDATA[
      var self = this;

      //get XML contents
      //using AJAX to create an XML document instead of a HTML document
      //to preserve CDATA
      Database.query( //CodeMirror AJAX request
        s1EditorXPath,
        CodeMirrorEditor.oAPIOptions,
        function(jqDataChildren) {
          self.setContent(j1DisplayObject, jqDataChildren);
          j1DisplayObject.addClass("gs-ajax-loaded");
          j1DisplayObject.trigger("editorloaded");
        },
        function(oAJAXResult) {
          var sMessage = (oAJAXResult ? oAJAXResult.statusText : "statusText not found on error response object");
          self.setError(j1DisplayObject, sMessage);
        }
      );
    ]]></javascript:method>

    <javascript:method name="setError" is-protected="true" parameters="j1DisplayObject, sError" chain="true"><![CDATA[
      if (j1DisplayObject.o.editor) {
        j1DisplayObject.o.editor.setValue("<div class=\"gs-warning\">" + sError + "</div>");
        Debug.error(this, "CodeMirrorEditor " + sError);
      }
    ]]></javascript:method>

    <javascript:method name="setContent" parameters="j1DisplayObject, (XHTMLString) sContent"><![CDATA[
      var self = this;
      var jElement, jTextarea;
      var oEditor = j1DisplayObject.o.editor;

      //replace encoded meta:cdataenc elements with CDATA sections
      //http://www.w3schools.com/dom/dom_node.asp
      /*
      var oGSCDATAEncNode, oNewCDATANode, oParentNode;
      var vGSCDATAEncNodes = oXMLDoc.getElementsByTagName('gs:cdataenc');

      while (vGSCDATAEncNodes.length) {
        oGSCDATAEncNode = vGSCDATAEncNodes.item(0);
        oNewCDATANode   = oXMLDoc.createCDATASection(oGSCDATAEncNode.textContent);
        oParentNode     = oGSCDATAEncNode.parentNode;
        oParentNode.replaceChild(oNewCDATANode, oGSCDATAEncNode);

      }

      */

      if (oEditor) oEditor.setValue(sContent);
      this.refresh(j1DisplayObject);
    ]]></javascript:method>

    <javascript:method name="formatContent" parameters="j1DisplayObject"><![CDATA[
      //autoFormatRange is provided by a separate plugin in the dependencies of this class
      var oEditor = j1DisplayObject.o.editor;
      if (oEditor && oEditor.autoFormatRange) {
        oEditor.autoFormatRange({line:0, ch:0}, {line:oEditor.lineCount()});
        oEditor.setCursor(0);
      }
    ]]></javascript:method>
    
    <javascript:method name="updateFormElement" is-protected="true" parameters="j1DisplayObject"><![CDATA[
      //update the textarea. this will also happen onsubmit
      //we do this manually because CodeMirror may have issues
      //CM.save(...) sets .value on the textarea, not its contents
      var jTextarea = j1DisplayObject.children("textarea");
      if (jTextarea) jTextarea.effectiveVal(j1DisplayObject.o.editor.getValue());
    ]]></javascript:method>

    <javascript:method name="refresh" is-protected="true" parameters="j1DisplayObject"><![CDATA[
      //on a timeout to give the CM reaction to content changes etc
      if (j1DisplayObject.o.editor) {
        this.setTimeout(function(j1DisplayObject){
          j1DisplayObject.o.editor.refresh();
        }, 0, j1DisplayObject);
      }
    ]]></javascript:method>
  </javascript:object>
</javascript:code>
