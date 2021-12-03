<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/javascript/2006">
  <javascript:object>
    <javascript:method name="displayAdded_default" parameters="oDispayObject" chain="true"><![CDATA[
      this.errorCount = this.liErrorCount(oDispayObject); //global for this session
    ]]></javascript:method>

    <javascript:method name="liErrorCount" is-protected="true" parameters="oDispayObject"><![CDATA[
      return oDispayObject.find("#gs_errors li").length;
    ]]></javascript:method>

    <javascript:event-handler event="servernodechanged" interface-modes="all" parameters="j1DisplayObject, jqPrimaryNodes, [fCallback]"><![CDATA[
      //this object has changed in the database
      //call DatabaseElement.prototype.changed with a callBack because it will AJAX refresh the HTML
      var self        = this;
      return DatabaseElement.prototype.f_servernodechanged.call(this, j1DisplayObject, jqPrimaryNodes, function(){self.checkErrors(j1DisplayObject);});
    ]]></javascript:event-handler>

    <javascript:method name="checkErrors" parameters="jElement"><![CDATA[
      //this check happens *after* DatabaseElement.prototype.changed has refreshed the HTML
      /*
      NOT_COMPLETE("");
      var iErrorCount = this.liErrorCount(this.elements);
      var iNewErros   = iErrorCount - this.errorCount;
      if (iNewErros) {
        alert('User changed: (' + iNewErros + ') new errors!');
        this.errorCount = iErrorCount;
      }
      */
    ]]></javascript:method>

    <javascript:static-method name="setBreakpoint" parameters="sXPath, fCallbackSuccess, [fCallbackFailure]"><![CDATA[
      //TODO: these functions should be exposed automatically from the user xmlns extensions
      //and User.NAMESPACE_ALIAS will not then be necessary
      var sDBFunctionNS      = User.NAMESPACE_ALIAS; //TODO: use the proper namespace
      var sDBFunctionName    = User.setBreakpoint.camelToDash();
      var aParams            = Array.prototype.slice.call(arguments, 0, arguments.length - 2);
      var sFunctionSignature = sDBFunctionNS + ":" + sDBFunctionName + "(<" + aParams.length + " params>)";

      OO.requireClass("Database", sFunctionSignature + " requires the Database Class for the API call");
      Database.queryWithFunction(
        sDBFunctionNS, sDBFunctionName, aParams,
        fCallbackSuccess,
        fCallbackFailure
      );
    ]]></javascript:static-method>
  </javascript:object>
</javascript:code>