<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" name="controller" controller="true">
  <javascript:object>
    <javascript:static-method name="set" parameters="oObject, s1DashName, sValue, [bFunctionCallOnly = false]"><![CDATA[
      var sObjectName, sNamespace, s1DashLocalName, s1CamelName;
      var oExistingValue, uValue;

      if (sValue === null) {
        Debug.warn(this, "attempt to set a null Option for [" + s1DashName + "]");
      } else {
        //remove namespace prefixes
        //EXCEPT xml (xml:id => xmlID)
        //namespaces should be loaded in to separate objects
        //however, in the case of XSchema meta-data form inputs they also need to be namespaced in the input @name
        sNamespace      = s1DashName.substringBefore(':', ''); //default, if not present, is a blank string
        s1DashLocalName = (sNamespace && sNamespace != "xml" ? s1DashName.substringAfter(':') : s1DashName);
        s1CamelName     = s1DashLocalName.dashToCamel();

        //only processes string values to objects
        //oObject is sent through in case the conversion needs context
        uValue = Function.convertClassAttributeValueNatural("Options.set", s1DashName, sValue, oObject);

        //we need to store jQuery data on its data {object}
        if      (oObject instanceof jQuery) oObject = oObject.data();
        else if (JOO.istype(oObject))       oObject = oObject.o; //o = data() 
        sObjectName = oObject.constructor.functionName();
        
        if (oExistingValue = oObject[s1CamelName]) {
          //--------------------------------- existing value or function
          if (oExistingValue instanceof Function 
            && !(uValue instanceof Function)
          ) {
            //we only CALL functions when the type is not a Function:
            //valid:
            //  @show=slow
            //problems because natural-type is a Function also:
            //  @class=XSchema()
            //  classDerivedConstructor=User()
            //causes issues because they look like methods on the object
            //properties are set and reset on shared objects, especially shared AJAXHTMLLoader
            //we do not want to set a Function VALUE and then run it the second time
            //e.g. func-start-notify=SubMenu
            oExistingValue.call(oObject, uValue);
          }
          else if (bFunctionCallOnly) {
            Debug.error(this, "accessor [" + s1CamelName + "](" + sValue + ") not found");
          } 
          else if (!uValue.eq) {
            if ((typeof oExistingValue === typeof uValue) && (typeof oExistingValue == 'string' || typeof oExistingValue == 'number')) {
              Debug.warn(oObject, "auto-setting property type has conflict but no equality function [eq] [" + sObjectName + "." + s1CamelName + " (" + s1CamelName + ")]:" + oExistingValue + "] => [" + uValue + "]");
            } else {
              Debug.warn(oObject, "auto-setting property type has conflict but no equality function [eq] [" + sObjectName + "." + s1CamelName + " (" + s1CamelName + ")]: (" + typeof oExistingValue + ") => (" + typeof uValue + ")");
              Debug.log(oObject, oExistingValue, true);
              Debug.log(oObject, uValue, true);
            }
            Debug.log(oObject, oObject, true);
          } 
          else if (!uValue.eq(oExistingValue)) {
            if ((typeof oExistingValue === typeof uValue) && (typeof oExistingValue == 'string' || typeof oExistingValue == 'number')) {
              Debug.warn(oObject, "auto-setting property conflict on [" + sObjectName + "." + s1CamelName + " (" + s1CamelName + ")]: [" + oExistingValue + "] => [" + uValue + "]");
            } else {
              Debug.warn(oObject, "auto-setting property conflict on [" + sObjectName + "." + s1CamelName + " (" + s1CamelName + ")]: (" + typeof oExistingValue + ") => (" + typeof uValue + ")");
              Debug.log(oObject, oExistingValue, true);
              Debug.log(oObject, uValue, true);
            }
            Debug.log(oObject, oObject, true);
          }
        } else {
          //--------------------------------- no existing expando
          if (bFunctionCallOnly) Debug.error(this, "accessor [" + s1CamelName + "](" + sValue + ") not found");
          else {
            oObject[s1CamelName] = uValue;
          }
        }
      }
    ]]></javascript:static-method>

    <javascript:static-method name="serialiseURLEncoded" parameters="oObject"><![CDATA[
      var oNewOptions = new Object();
      var sExpando, s1Name, oValue, sValue;

      for (sExpando in oObject) {
        oValue = oObject[sExpando];
        if (oValue !== undefined && oValue !== null) {
          //user-defined functions on oObject are ignored
          if (!(oValue instanceof Object && oValue.noIteration === true)) { 
            s1Name   = sExpando.camelToDash(); //meta_xpathToNode => meta:xpath-to-node
            sValue   = (oValue.toURLEncoded instanceof Function ? oValue.toURLEncoded() : oValue.toString());
            oNewOptions[s1Name] = sValue;
          }
        }
      }

      return oNewOptions;
    ]]></javascript:static-method>
  </javascript:object>
</javascript:code>