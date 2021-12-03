<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/javascript/2006" parameter-checks="off">
  <javascript:object debug="off">
    <javascript:static-property name="bAlertedOnce">false</javascript:static-property>
    <javascript:static-property name="onlyClass">new Array()</javascript:static-property>
    <!-- this static property is at the end of the script because it contains function pointers -->
    <!-- javascript:static-property name="aFuncs">new Array(Debug.warn, Debug.error)</javascript:static-property -->
    <javascript:static-property name="bMoveEvents">true</javascript:static-property>

    <javascript:static-method name="help"><![CDATA[
      var fFuncName, fFunc;
      
      console.info("Server logging classes:");
      for (fFuncName in window) {
        fFunc = window[fFuncName];
        if (fFunc && fFunc instanceof Object && fFunc.sDebugFlag)
          console.info('[' + fFunc.name + '] => [' + fFunc.sDebugFlag + ']');
      }
    ]]></javascript:static-method>
    
    <!-- #################################### output control settings #################################### -->
    <javascript:static-method name="only" parameters="..."><![CDATA[
      var aOldValue = Debug.onlyClass;
      var fFunc;

      Debug.onlyClass = new Array();
      for (var i = 0; i < arguments.length; i++) {
        fFunc = arguments[i];
        if (fFunc) {
          Debug.onlyClass.push(fFunc);
          if (fFunc.debugMessage) Debug.info(this, fFunc.debugMessage, true);
          if (fFunc.sDebugFlag)   Debug.info(this, '[' + fFunc.sDebugFlag + '] server logging switched on', true);
        } else {
          if (window.console && window.console.error && window.console.log) {
            window.console.error("parameter not a function");
            window.console.log(fFunc);
          }
        }
      }

      return aOldValue;
    ]]></javascript:static-method>

    <javascript:static-method name="any" parameters="[bSuppressConfirm = false]"><![CDATA[
      Debug.onlyClass = new Array();
      if (!bSuppressConfirm && window.console && window.console.info) window.console.info("reporting objects reset to all");
      return null;
    ]]></javascript:static-method>

    <javascript:static-method name="off" parameters="[bSuppressConfirm = false]"><![CDATA[
      var aOldValue = Debug.aFuncs;
      Debug.aFuncs = new Array(Debug.warn, Debug.error);
      if (!bSuppressConfirm && window.console && window.console.info) window.console.info("functions reset to warnings and errors only");
      return aOldValue;
    ]]></javascript:static-method>

    <javascript:static-method name="all"><![CDATA[
      //doesnot return anything: console use only
      Debug.on(
        Debug.info,
        Debug.method,
        Debug.group,
        Debug.groupend,
        Debug.error,
        Debug.warn,
        Debug.log);
    ]]></javascript:static-method>

    <javascript:static-method name="on" parameters="..."><![CDATA[
      var aOldValue = Debug.aFuncs;
      var fFunc;
      //Debug.on([Debug.warn]) will not output any confirmations

      if (arguments.length == 1 && arguments[0] instanceof Array) {
        //Debug.on([Debug.x, Debug.y, ...])
        Debug.aFuncs = arguments[0];
      } else if (arguments.length) {
        //Debug.on(Debug.x, Debug.y, ...)
        Debug.aFuncs = new Array();
        for (var i = 0; i < arguments.length; i++) {
          fFunc = arguments[i];
          if (fFunc && typeof fFunc === "function") {
            Debug.aFuncs.push(fFunc);
          } else {
            if (window.console && window.console.error && window.console.log) {
              window.console.error("parameter not a function");
              window.console.log(fFunc);
            }
          }
        }
      } else {
        //Debug.on()
        //default: warn and error
        Debug.aFuncs = new Array(Debug.warn, Debug.error);
        Debug.consoleOutput(this, "error", "Debug.<funcs> list required (cumulative)");
      }

      return aOldValue;
    ]]></javascript:static-method>

    <javascript:static-method name="pause"><![CDATA[
      debugger; //Debug.pause();
    ]]></javascript:static-method>

    <javascript:static-method name="debugAJAXOptions" parameters="oOptions"><![CDATA[
      Debug.onlyClass.eachEx(function(oOptions){
        if (this.debugAJAXOptions instanceof Function) this.debugAJAXOptions(oOptions); //in-place url replacement
      }, DEFAULT, oOptions);
      return oOptions;
    ]]></javascript:static-method>


    <!-- #################################### output control info #################################### -->
    <javascript:static-method name="isFunc" parameters="sType"><![CDATA[
      //we always allow log
      return !sType || !Debug[sType] 
        || sType == "log" 
        || (!Debug.aFuncs || Debug.aFuncs.contains(Debug[sType]));
    ]]></javascript:static-method>

    <javascript:static-method name="isEmpty"><![CDATA[
      return (Debug.onlyClass.length == 0);
    ]]></javascript:static-method>

    <javascript:static-method name="isOnly" parameters="uObject"><![CDATA[
      //global functions can be debugged with Debug.only(window)
      //specific objects can be debugged with Debug.only(gsQuery('idx_4096'))
      //whole classes can be debugged with    Debug.only(XSchema)
      var bObjectIncluded = false, bClassIncluded = false, bSuppressAllDebug = false;

      if (!Debug.isEmpty()) {
        bObjectIncluded   = Debug.onlyClass.contains(uObject);
        bClassIncluded    = (uObject.classDerivedConstructor && Debug.onlyClass.contains(uObject.classDerivedConstructor));
      }
      bSuppressAllDebug = (uObject && uObject.classDerivedConstructor && uObject.classDerivedConstructor.suppressAllDebug);

      return (bClassIncluded || bObjectIncluded) && !bSuppressAllDebug;
    ]]></javascript:static-method>

    <javascript:static-method name="consoleOutput" parameters="oObject, sType, [sMessage, bForce = false]"><![CDATA[
      var bAnnounced = false;
      var fConsoleFunc;

      if (window.console) {
        fConsoleFunc = window.console[sType];
        if (bForce || (
            (Debug.isEmpty() || Debug.isOnly(oObject)) 
          && Debug.isFunc(sType)
        )) {
          if (fConsoleFunc) {
            fConsoleFunc.call(window.console, sMessage);
            bAnnounced = true;
          }
        }
      }
      return bAnnounced;
    ]]></javascript:static-method>

    <javascript:static-method name="frameInfo"><![CDATA[
      var sPre = '[';
      if (document.title) sPre += document.title;
      else if (window.location.pathname) {
        //get the last part of the URL, e.g. ...(index.xsl)
        var aMatches = window.location.pathname.match(/[^\/]*$/g);
        var sMatch   = aMatches[0];
        sPre += (sMatch.length ? sMatch : '/');
      }
      else sPre += "(un-named window)";
      sPre += "]: ";
      return sPre;
    ]]></javascript:static-method>

    <javascript:static-method name="alertOnce" parameters="sMessage"><![CDATA[
      if (!bAlertedOnce) {bAlertedOnce = true;alert(sMessage);}
    ]]></javascript:static-method>

    <javascript:static-method name="serialise" parameters="oArg"><![CDATA[
      var sArg;

      if      (oArg === null)              sArg = "(null)";
      else if (oArg === undefined)         sArg = "(undefined)";
      else if (typeof oArg === "function") sArg = oArg.functionName();
      else if (oArg instanceof window.DatabaseElement)  sArg = oArg.functionName() + (oArg.xmlID ? " [" + oArg.xmlID + "]" : "");
      else if (oArg instanceof window.BaseObject) sArg = oArg.functionName();
      else if (oArg instanceof window.jQuery)     {
        if      (oArg.length == 0)         sArg = "$()";
        else if (oArg.length == 1)         sArg = "$(" + oArg.tagName() + ")";
        else                               sArg = "$(" + oArg.length + " objects)";
      } else if (oArg instanceof window.jQuery.Event) sArg = "Event(" + oArg.type + ")";
      else if (oArg == document)           sArg = "[the Document]";
      else if (oArg == window)             sArg = "[the Window]";
      else if (typeof oArg === "object") {
        sArg = "[object " + oArg.constructor.functionName() + "]";
        /*
        var sThisArg, sComma = "";
        sArg = "{";
        for (i in oArg) {
          sThisArg = (oArg[i] && oArg[i].toString ? oArg[i].toString() : "(unknown)" );
          sArg    += sComma + i + ":" + sThisArg;
          sComma   = ", ";
        }
        sArg += "}";
        */
      } else                                 sArg = oArg.toString();

      return sArg;
    ]]></javascript:static-method>

    <!-- #################################### debug functions #################################### -->
    <javascript:static-method name="debug" parameters="oObject, sMessage, [bForce = false]"><![CDATA[
      return Debug.consoleOutput(oObject, "debug", sMessage, bForce);
    ]]></javascript:static-method>

    <javascript:static-method name="info" parameters="oObject, sMessage, [bForce = false]"><![CDATA[
      return Debug.consoleOutput(oObject, "info", sMessage, bForce);
    ]]></javascript:static-method>

    <javascript:static-method name="method" parameters="oObject,fCurrentBaseClass,sMethodName,sParameters,oArguments"><![CDATA[
      var sMessage;
      var sObjectName = Debug.serialise(oObject);
      var sBaseClass  = fCurrentBaseClass.functionName();
      var bSameObject = (oObject && oObject.classDerivedConstructor === fCurrentBaseClass);
      var sArg, aArgs = new Array();

      NOT_CURRENTLY_USED("");

      /*
      for (var i = 0; i < oArguments.length; i++) {
        sArg = Debug.serialise(oArguments[i]);
        if (sArg && sArg.length > 30) sArg = sArg.substring(0, 30) + "...";
        aArgs.push(sArg);
      }

      sMessage = sObjectName + (bSameObject ? "" : " (" + sBaseClass + ")") + "::" + (sMethodName ? sMethodName : "(noname)") + "(" + sParameters + ") [" + aArgs.toString() + "]";
      Debug.consoleOutpmoveEventsut(oObject, "info", sMessage);
      for (var i = 0; i < oArguments.length; i++) {
        if (oArguments[i] instanceof Object) Debug.consoleOutput(oObject, "log", oArguments[i].toString());
      }
      */
    ]]></javascript:static-method>

    <javascript:static-method name="callback" parameters="..."><![CDATA[
      var bAnnounced = false;
      if (window.console) {
        //window.console.info("Debug.callback() finished with:");
        //window.console.log(arguments);
        bAnnounced = true;
      }
      return bAnnounced;
    ]]></javascript:static-method>

    <javascript:static-method name="log" parameters="oObject, uReportObject, [bForce = false]"><![CDATA[
      var bAnnounced = false;
      if (bForce || (Debug.isOnly(oObject) && Debug.isFunc("log"))) {
        if (window.console) {
          window.console.log(uReportObject);
          bAnnounced = true;
        }
      }
      return bAnnounced;
    ]]></javascript:static-method>

    <javascript:static-method name="warn" parameters="oObject, sMessage, [bForce = false]"><![CDATA[
      return Debug.consoleOutput(oObject, "warn", sMessage, bForce);
    ]]></javascript:static-method>

    <javascript:static-method name="error" parameters="oObject, sMessage, [bForce = false]"><![CDATA[
      Debug.consoleOutput(oObject, "error", sMessage, bForce);
      Debug.pause(); //debugger
      return true;
    ]]></javascript:static-method>

    <javascript:static-method name="group" parameters="oObject, [sMessage, bForce = false]"><![CDATA[
      return Debug.consoleOutput(oObject, "group", sMessage, bForce);
    ]]></javascript:static-method>

    <javascript:static-method name="groupend" parameters="oObject, [sMessage, bForce = false]"><![CDATA[
      return Debug.consoleOutput(oObject, "groupEnd", sMessage, bForce);
    ]]></javascript:static-method>

    <!-- this static property is at the end of the script because it contains function pointers
      normally it would dela set but we do not want the startup script to output all info so we det @delay=false
    -->
    <javascript:static-property name="aFuncs" delay="false">new Array(Debug.warn, Debug.error)</javascript:static-property>
  </javascript:object>
</javascript:code>
