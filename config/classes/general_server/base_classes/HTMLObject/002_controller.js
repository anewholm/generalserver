<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/javascript/2006">
  <javascript:object>
    <!-- this object carrys out:
      provides HTML object methods
      linkage from #HTML-element to attached object
    -->
    <javascript:static-property name="aSelected">new Array()</javascript:static-property>

    <javascript:method name="clearStylesheetCache" is-protected="true" requires="HTMLWebpage"><![CDATA[
      return HTMLWebpage.clearStylesheetCache();
    ]]></javascript:method>

    <javascript:event-handler event="click" action="properties" requires="Debug"><![CDATA[
      Debug.log(this, this, true); //force (in case Debug.only, Debug.func)
      if (Event.currentEvent().firstStatedCSSFunctionElement()) 
        Event.currentEvent().firstStatedCSSFunctionElement().temptext("properties (console)");
    ]]></javascript:event-handler>

    <javascript:method name="loadProperties" is-protected="true" parameters="jq1DisplayObject, [bReport = false]"><![CDATA[
      var self = this;
      var oObject;
      var sNamespacePrefix;
      var jqMetaParent, jqMetaGroups, jqMetaGroup, jqProperty;
      var sContent, aPropertiesLine, sXML;
      var sDashName, sValue, bFunctionCallOnly;

      if (jq1DisplayObject.isSingularAbstractElement()) {
        //-------------------------------- abstract it is in the head
        //e.g. a script or style element in the <html:head>
        //SHOULD contain an xml object initiator for this:
        /* GSProperties:
          <properties>   <= jqMetaParent
            <div class="gs-meta-data gs-namespace-prefix-gs gs-load-as-object">...</div>
            <div class="gs-meta-data gs-namespace-prefix-database gs-load-as-properties">...</div>
          </properties>
        */
        //further [spurious] elements can follow but will be ignored
        //an AJAXHTMLLoader can sometimes appear within the script element during refresh
        //TODO: how to handle */ in the XML?
        bReport  = true;
        sContent = jq1DisplayObject.html(); //text() does not work on <script>
        if (sContent) {
          aPropertiesLine = sContent.match(/^\s*\/\* GSProperties: (.+) \*\//);
          if (aPropertiesLine && aPropertiesLine.length > 1) {
            sXML = aPropertiesLine[1];
            jqMetaParent = new jQuery(sXML);
            if (!jqMetaParent.length) Debug.warn(self, "root child not found in:\n" + sXML)
          } else Debug.warn(this, "new isSingularAbstractElement() (in the <head>) has incorrect format (GSProperties:...)");
        } else Debug.warn(this, "new isSingularAbstractElement() (in the <head>) has no content");
        if (!jqMetaParent) {
          Debug.log(this, jq1DisplayObject.parent(), true);
          jqMetaParent = jq1DisplayObject;
        }
      } else {
        //-------------------------------- normal body HTML load
        //direct <div @class=DatabaseObject><div @class=gs-meta-data
        jqMetaParent = jq1DisplayObject;
      }

      if (jqMetaParent.isNonXHTMLNamespace()) {
        //-------------------------------- non-HTML namespace element: load @attributes
        //e.g. <database:query @data=* @node-mask=. />
        //ALWAYS on the passed jq1DisplayObject
        jq1DisplayObject.attributes().each(function(){
          jqProperty = new jQuery(this);
          Options.set(jq1DisplayObject, jqProperty.tagName(), jqProperty.val());
        });
        //however, we still need a valid ID
        this.xmlID = jq1DisplayObject.ID;
      } else {
        //-------------------------------- load normal HTML DIV.gs-meta-data groups
        //gs-display-specific      places the properties on the jq1DisplayObject (in self.displays)
        //  AND then
        //gs-load-as-<Object-name> creates a new <Object-name> and puts the properties there, e.g. DatabaseQuery
        //gs-load-as-properties    puts the properties directly on self!
        jqMetaGroups = jqMetaParent.children(".gs-meta-data");
        //XSchema.sAdditionalMetaLocations points also to the form inputs that contain the context values like destination
        if (this.classDerivedConstructor.sAdditionalMetaLocations) {
          jqMetaGroups = jqMetaGroups.add(jqMetaParent.find(this.classDerivedConstructor.sAdditionalMetaLocations));
        }
        jqMetaGroups.each(function(){
          jqMetaGroup = new jQuery(this);

          //displays[]... specific?
          //  display data goes on the data() {object}
          if (jqMetaGroup.hasClass("gs-display-specific")) oObject = jq1DisplayObject;
          else                                             oObject = self;

          //xml:id => xmlID
          bIncludeNamespace = jqMetaGroup.hasClass("gs-include-namespace");
          sNamespacePrefix  = (bIncludeNamespace ? jqMetaGroup.firstClassGroup1(/^gs-namespace-prefix-([^ ]+)/g) : undefined);

          //[displays[]].object[]... ?
          bFunctionCallOnly = jqMetaGroup.hasClass("gs-load-as-function-calls");
          if (!bFunctionCallOnly && !jqMetaGroup.hasClass("gs-load-as-properties")) {
            //allow an explicit gs-group-name
            //OR default to the namespace prefix
            sObjectType = jqMetaGroup.firstClassGroup1(/^gs-load-as-([^ ]+)/g);
            if (!sObjectType) sObjectType = "Options";

            sGroupName = jqMetaGroup.firstClassGroup1(/^gs-group-([^ ]+)/g);
            if (!sGroupName) sGroupName = sNamespacePrefix || jqMetaGroup.firstClassGroup1(/^gs-namespace-prefix-([^ ]+)/g);

            if (sGroupName) {
              oGroupObject = new window[sObjectType]();
              oObject[sGroupName.dashToCamel()] = oGroupObject;
              oObject = oGroupObject;
            } else Debug.warn(self, "group-name not resolved for [" + self.functionName() + "]", bReport);
          }

          //-------------------------------- Options.set()
          jqMetaGroup.children().each(function(){
            jqProperty = new jQuery(this);
            switch (jqProperty.tagName()) {
              case "input": {sDashName = jqProperty.attr("name");  sValue = jqProperty.attr("value"); break;}
              default:      {sDashName = jqProperty.attr("class"); sValue = jqProperty.text();}
            }
            if (bIncludeNamespace) sDashName = sNamespacePrefix + ':' + sDashName; //id => xml:id
            Options.set(oObject, sDashName, sValue, bFunctionCallOnly);
          });
        });
        //deleting the MetaGroups will mean that copy operations will loose meta-data on attachToObject()
        //jqMetaGroups.filter(":not(.gs-persist)").remove();
      }
    ]]></javascript:method>

    <javascript:method name="addElement" parameters="jq1Element"><![CDATA[
      //DO NOT override  addElement(...)
      //USE              displayAdded_<display mode>(jq1Element) instead (@chain=true)
      //defaults to      displayAdded_default(jq1Element) NO interface
      //ALSO always runs interfaceModeAddElement(jq1Element)
      var iInitialElementCount, sInterfaceModeAddElementFuncName;
      var iDash, sInterfaceModeString;

      //-------------------------------- create new singular display for this.jqDisplays
      //note that jq1Element.cssInterfaceMode() will return "default" if it is empty
      jq1Element.addClass("gs-display-" + this.jqDisplays.length); //zero based, not-updated
      jq1Element.object(this);
      this.jqDisplays = this.jqDisplays.add(jq1Element);

      //-------------------------------- remove any invalid elements
      if (iInitialElementCount = this.jqDisplays.length) {
        this.removeInvalidDisplays();
        if (this.jqDisplays.length != iInitialElementCount) {
          Debug.warn(this, "invalid elements removed");
          if (this.jqDisplays.length == 0) Debug.warn(this, "no valid elements left!");
        }
      }

      this.loadProperties(jq1Element);

      return jq1Element;
    ]]></javascript:method>

    <javascript:method name="setError" parameters="j1DisplayObject, sError"><![CDATA[
      var jqDIV = new jQuery("div", {class:"gs-warning"}).text(sError);
      j1DisplayObject.html(jqDIV);
      Debug.error(this, sError);
    ]]></javascript:method>

    <javascript:method name="unselect"><![CDATA[
      HTMLObject.aSelected.remove(this);
      this.jqDisplays.removeClass("selected");
      this.triggerAllDisplays("unselect");
    ]]></javascript:method>

    <javascript:method name="select" parameters="j1DisplayObject, ..."><![CDATA[
      var self = this;

      //multi-select aware
      //TODO: HTMLObject.aSelected should be a jSelected
      if (!Event.currentEvent().ctrlKey) {
        HTMLObject.aSelected.eachEx(function() {
          if (this != self) this.unselect();
        });
      }

      //select this one
      this.jqDisplays.addClass("selected");
      if (HTMLObject.aSelected.contains(this)) {
        //this is a re-select
        if (OO.classExists("Debug") && OO.classExists("HTMLWebpage")) {
          this.clearStylesheetCache();
          this.reload_css();
          this.reload_javascript();
        }
      } else HTMLObject.aSelected.push(this);

      //including the event here so that non-user events don't trigger the context menu
      Event.currentEvent().currentDisplayObject().trigger("select");
    ]]></javascript:method>

    <javascript:method name="trigger" parameters="uEventToTrigger, [aExtraParameters, fCallback]"><![CDATA[
      return this.triggerAllDisplays.apply(this, arguments);
    ]]></javascript:method>

    <javascript:method name="triggerAllDisplays" parameters="uEventToTrigger, [aExtraParameters, fCallback]"><![CDATA[
      return new JQueryClass(this.jqDisplays).trigger(uEventToTrigger, aExtraParameters, fCallback);
    ]]></javascript:method>
    
    <javascript:event-handler event="servernodechanged" interface-modes="all" parameters="j1DisplayObject, jqPrimaryNodes, [fCallback]"><![CDATA[
      //update the jElement list to remove those that are no longer in the document
      this.removeInvalidDisplays();
      this.jqDisplays.addClass("gs-changed");
      if (fCallback) fCallback();
    ]]></javascript:event-handler>

    <!-- ###################################### dynamic HTML helpers #################################### -->
    <javascript:method name="newIcon" requires="LinkedServer.singleton" parameters="sName, [sGroup, oProperties = new Object()]"><![CDATA[
      oProperties.src = LinkedServer.singleton.uri.slashAdd("resources/shared/images");
      if (sGroup) oProperties.src = oProperties.src.slashAdd(sGroup);
      oProperties.src = oProperties.src.slashAdd(sName + ".png");
      return new JOO("<img/>", oProperties);
    ]]></javascript:method>
  </javascript:object>
</javascript:code>