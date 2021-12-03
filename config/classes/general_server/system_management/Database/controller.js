<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/javascript/2006">
  <javascript:object>
    <javascript:static-property name="sDebugFlag">'LOG_DATA_RENDER'</javascript:static-property>
    <javascript:static-property name="bRequestInProgress" description="set to true at ajax request start, and to false at end">false</javascript:static-property>
    <javascript:static-property name="NAMESPACE">"http://general_server.org/xmlnamespaces/database/2006"</javascript:static-property>
    <javascript:static-property name="NAMESPACE_ALIAS">"database"</javascript:static-property>

    <javascript:static-method name="debugAJAXOptions" parameters="oOptions"><![CDATA[
      oOptions.url = oOptions.url.setUrlParameter("configuration-flags", Database.sDebugFlag, String.prototype.commaAdd);
    ]]></javascript:static-method>

    <!-- ########################################## helper functions ########################################## -->
    <!-- javascript:static-method name="containerObject" parameters="oDatabaseElement, fSuccessCallback, [fFailureCallback]"><![CDATA[
      return Database.queryWithFunction(
        Database.NAMESPACE_ALIAS, Database.containerObject.camelToDash(), [oDatabaseElement],
        fCallbackSuccess,
        fCallbackFailure
      );
    ]]></javascript:static-method -->

    <javascript:static-method name="ancestorPath" parameters="oDatabaseElement, sInterfaceMode, fSuccessCallback, [fFailureCallback, oOptions = {}]"><![CDATA[
      //setup
      oOptions.nodeMask      = oDatabaseElement.xpathToNode.ancestorOrSelf();     //singular direct ancestor path
      oOptions.interfaceMode = sInterfaceMode;

      return Database.query(
        XPathString.document(), // "/", which the API will translate to the root of the current DB
        oOptions,
        fSuccessCallback,
        fFailureCallback
      );
    ]]></javascript:static-method>

    <!-- ########################################## base query functions (using QUERY-STRING GET) ##########################################
      Used to AJAX return DOM XML only.
      Usually the JavaScript will then traverse the DOM to analyse the results
      e.g. ancestorPath
    -->
    <javascript:static-method name="queryWithFunction" parameters="sDBFunctionNS, sDBFunctionName, aParams, fSuccessCallback, [fFailureCallback]"><![CDATA[
      var sFunctionString = sDBFunctionNS + ":" + sDBFunctionName + "(";
      //TODO: SECURITY: concatenating strings for entry in to a database: be careful
      for (var i = 0; i < aParams.length; i++) {
        oItem = aParams[i];
        if (i) sFunctionString += ",";
        if (oItem instanceof DatabaseElement) oItem = oItem.xpathToNode;
        sItem = oItem.toString();
        sFunctionString += sItem;
      }
      sFunctionString += ")";

      Database.query(
        sFunctionString,
        null,
        fSuccessCallback,
        fFailureCallback
      );
    ]]></javascript:static-method>

    <javascript:static-method name="queryObjectWithViewTemplate" parameters="oSelect, oOptions, sTemplateMode, fSuccessCallback, [fFailureCallback]"><![CDATA[
      //server-side XML view execution
      //**** output still should be XML
      //for HTML use the HTMLWebpage.load() function and load.xsl which conditionally transforms also
      //e.g. listen-server-events
      var ret;
      if      (!oSelect)                            Debug.error(this, "DatabaseElement required");
      else if (!oSelect instanceof DatabaseElement) {
        Debug.error(this, "oSelect not instanceof DatabaseElement");
        Debug.log(this, this, true);
      } else if (oSelect && !oSelect.xpathToNode)   {
        Debug.error(this, "DatabaseElement xpathToNode missing");
        Debug.log(this, this, true);
      } else {
        if (!oOptions) oOptions = {};
        oOptions.interfaceMode  = sTemplateMode;
        ret = Database.query(oSelect.xpathToNode, oOptions, fSuccessCallback, fFailureCallback);
      }

      return ret;
    ]]></javascript:static-method>

    <javascript:static-method name="query" parameters="(String) sQueryXPath, oOptions, fSuccessCallback, [fFailureCallback]"><![CDATA[
      //this is a generic function that accepts ANY xpath statement and queries it
      if (!oOptions) oOptions = {};
      Database.bRequestInProgress = true;

      var oAJAXOptions = {
        url:       "/api/" + sQueryXPath.toURLEncoded(),
        data:      Options.serialiseURLEncoded(oOptions),
        type:      (oOptions.method ? oOptions.method : "GET"),
        context:   this, //the this for the callback functions
        cache:     oOptions.cache,
        success:   function(hResponseDOM,  sStatus){Database.querySuccess.call(this, hResponseDOM,  sStatus, fSuccessCallback);},
        error:     function(oError, sStatus)       {Database.queryFailure.call(this, oError, sStatus, fFailureCallback);},
        dataType:  (oOptions.dataType ? oOptions.dataType : "xml")
      };
      HTTP.ajax(oAJAXOptions);
    ]]></javascript:static-method>

    <javascript:static-method name="querySuccess" parameters="hResponseDOM, sStatus, fCallback"><![CDATA[
      //return jQuery: these elements are not in the main DOM
      Database.bRequestInProgress = false;
      return fCallback(Database.dataChildrenFromResponseDOM(hResponseDOM), hResponseDOM, sStatus);
    ]]></javascript:static-method>

    <javascript:static-method name="queryFailure" parameters="oError, sStatus, [fCallback]"><![CDATA[
      Database.bRequestInProgress = false;
      if (fCallback) fCallback(oError, sStatus);
      else if (OO.classExists("Debug")) Debug.log(this, oError);
    ]]></javascript:static-method>

    <javascript:static-method name="dataChildrenFromResponseDOM" parameters="hResponseDOM"><![CDATA[
      //return jQuery: these elements are not in the main DOM
      //there can be different root names, e.g. meta:ajax, meta:root
      var jqDataChildren = new jQuery(hResponseDOM).firstChild().children("gs\\:data").children();
      if (!jqDataChildren.length) {
        Debug.error(this, "roots not found in hResponseDOM");
        Debug.log(this, hResponseDOM);
      }
      return jqDataChildren;
    ]]></javascript:static-method>

    <!-- ########################################## base update functions (using FORM POST) ##########################################
      Used to AJAX update the DB
      302 Found result returned as always
      use listeners to cause object updates to see results
    -->
    <javascript:static-method name="updateObjectWithControllerTemplate" parameters="dbSelect, s1TemplateMode, [oData = {}, fSuccessCallback, fFailureCallback]"><![CDATA[
      oData.meta_xpathToSelect = dbSelect;       //will be serialised to the .xpathToNode value
      oData.meta_interfaceMode = s1TemplateMode;

      return Database.update("class-command", oData, fSuccessCallback, fFailureCallback);
    ]]></javascript:static-method>

    <javascript:static-method name="update" parameters="sDatabaseFunctionName, [oFormData = {}, fSuccessCallback, fFailureCallback]"><![CDATA[
      //the fact that POST form data is included will cause GS to use the
      //  [POST update request] message interpretation
      //which ignores the URL and returns a 302 Found to the URL which is un-important to update AJAX
      //TODO: the AJAX will then process the GET though, so we need to stop that!

      //META data for the class-action
      oFormData.meta_xpathToXsdDataProcessing = "none";
      oFormData.meta_domMethod                = sDatabaseFunctionName;

      Database.bRequestInProgress = true;
      var oAJAXOptions = {
        url:       "/",
        data:      Options.serialiseURLEncoded(oFormData),
        type:      "POST",
        context:   this, //the this for the callback functions
        cache:     oFormData.cache,
        success:   function(hData,  sStatus){Database.updateSuccess(hData,  sStatus, fSuccessCallback);},
        error:     function(oError, sStatus){Database.updateFailure(oError, sStatus, fFailureCallback);},
        dataType:  "xml",
        //introduced in jQuery 1.5 http://api.jquery.com/jquery.ajax/
        /*
        statusCode: {
          404: function(){alert(404);},
          302: function(){alert(302);} //prevent following of 302 Found from the POST MI
        }
        */
      };
      HTTP.ajax(oAJAXOptions);
    ]]></javascript:static-method>

    <javascript:static-method name="updateSuccess" parameters="hData, sStatus, [fCallback]"><![CDATA[
      Database.bRequestInProgress = false;
      if (fCallback) fCallback(hData, sStatus);
    ]]></javascript:static-method>

    <javascript:static-method name="updateFailure" parameters="oError, sStatus, [fCallback]"><![CDATA[
      Database.bRequestInProgress = false;
      if (fCallback) fCallback(oError, sStatus);
      else if (OO.classExists("Debug")) Debug.log(this, oError);
    ]]></javascript:static-method>

    <!-- ########################################## specific known POST update types ########################################## -->
    <javascript:static-method name="copyChild" parameters="oSelect, oDestination, [fSuccessCallback, fFailureCallback]"><![CDATA[
      return Database.update("copy-child", {"meta:select": oSelect, "meta:destination": oDestination}, fSuccessCallback, fFailureCallback);
    ]]></javascript:static-method>

    <javascript:static-method name="moveChild" parameters="oSelect, oDestination, [fSuccessCallback, fFailureCallback]"><![CDATA[
      return Database.update("move-child", {"meta:select": oSelect, "meta:destination": oDestination}, fSuccessCallback, fFailureCallback);
    ]]></javascript:static-method>

    <javascript:static-method name="hardlinkChild" parameters="oSelect, oDestination, [fSuccessCallback, fFailureCallback]"><![CDATA[
      return Database.update("hardlink-child", {"meta:select": oSelect, "meta:destination": oDestination}, fSuccessCallback, fFailureCallback);
    ]]></javascript:static-method>

    <javascript:static-method name="removeNode" parameters="oSelect, [fSuccessCallback, fFailureCallback]"><![CDATA[
      return Database.update("remove-node", {"meta:select": oSelect}, fSuccessCallback, fFailureCallback);
    ]]></javascript:static-method>

    <javascript:static-method name="renameNode" parameters="oSelect, sNewName, [fSuccessCallback, fFailureCallback]"><![CDATA[
      return Database.update("rename-node", {"meta:select": oSelect, "new-name": sNewName}, fSuccessCallback, fFailureCallback);
    ]]></javascript:static-method>
  </javascript:object>
</javascript:code>
