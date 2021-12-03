<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" name="object orientation">
  <javascript:namespace name="OO">
    <javascript:define name="GS_OBJECT_ATTACHED_CLASS">"gs-object-attached"</javascript:define>
    <javascript:define name="GS_OBJECT_ATTACHED_FILTER">"." + GS_OBJECT_ATTACHED_CLASS</javascript:define>

    <javascript:static-property name="oLinkage"><![CDATA[new Object()]]></javascript:static-property>
    <javascript:static-property name="iUniqueObjectID">1</javascript:static-property>
    <javascript:static-property name="bInitialised">false</javascript:static-property>
    <javascript:static-property name="IntrospectionClass">new Object()</javascript:static-property>

    <!-- ####################################### properties ####################################### -->
    <javascript:static-method name="addMP" parameters="fMethod, s1DashName, sValue, [bForceImmediate = false]" parameter-checks="off"><![CDATA[
      //addMethodProperty()
      var oValue, sCamelName = s1DashName.dashToCamel();
      if (fMethod[sCamelName] === undefined) {
        if (OO.classExists("VariableType")) oValue = Function.convertClassAttributeValueNatural(fMethod.functionName(), s1DashName, sValue);
        else oValue = sValue;
        fMethod[sCamelName] = oValue;
      }
    ]]></javascript:static-method>

    <javascript:static-method name="addCP" parameters="fClass, s1DashName, sValue, [bForceImmediate = false]" parameter-checks="off"><![CDATA[
      //addClassProperty()
      var sCamelName = s1DashName.dashToCamel();
      if ((fClass[sCamelName] === undefined)) fClass[sCamelName] = Function.convertClassAttributeValueNatural(fClass.functionName(), s1DashName, sValue);
    ]]></javascript:static-method>

    <!-- ####################################### requirements introspection ####################################### -->
    <javascript:static-method name="classExists" parameters="s1ClassName" parameter-checks="off"><![CDATA[
      var fClass = window[s1ClassName];
      return (fClass instanceof Function && fClass.definedClass ? fClass : undefined);
    ]]></javascript:static-method>

    <javascript:static-method name="requireClass" parameters="s1ClassName, [sReason]"><![CDATA[
      var bExists = OO.classExists(s1ClassName);
      if (!bExists) {
        if      (window.Debug instanceof Function) Debug.error(this, sReason);
        else if (window.console)                   console.error(sReason);
        else                                       alert(sReason);
      }
      return bExists;
    ]]></javascript:static-method>

    <javascript:static-method name="requireSingleton" parameters="s1ClassName, [sReason]"><![CDATA[
      var bExists = OO.classExists(s1ClassName);
      var bSingletonExists = (window[s1ClassName].singleton);
      var sErrorMessage;
      if (!bSingletonExists) {
        sErrorMessage = s1ClassName + ".singleton required for [" + sReason + "]";
        if      (window.Debug instanceof Function) Debug.error(this, sErrorMessage);
        else if (window.console)                   console.error(sErrorMessage);
        else                                       alert(sErrorMessage);
      }
      return bSingletonExists;
    ]]></javascript:static-method>

    <javascript:static-method name="requireCapability" parameters="fFunction, [s1Reason]"><![CDATA[
      var bExists = fFunction && fFunction();
      if (!bExists) {
        if      (window.Debug instanceof Function) Debug.error(this, s1Reason);
        else if (window.console)                   console.error(s1Reason);
        else                                       alert(s1Reason);
      }
      return bExists;
    ]]></javascript:static-method>

    <javascript:static-method name="setSingleton" parameters="oObject"><![CDATA[
      //a singleton is when only one of this object type is *usually* be instanciated
      //and thus can be accessed through <Constructor>.singleton
      //so, only one Server()
      //but, potentially with multiple display points on the page
      //but only one IDX identifier (server)
      //it is NOT an error to instanciate several. for example:
      //  a webpage that manages Servers
      //the singleton will point to the *first* object in this case
      var fClassFunction;
      if (!oObject) Debug.error(this, "Singleton object null for [" + fClassFunction.functionName() + "]");
      else {
        if (!oObject.classDerivedConstructor) Debug.warn(this, "setting Singleton without oObject.classDerivedConstructor");
        fClassFunction = (oObject.classDerivedConstructor ? oObject.classDerivedConstructor : oObject.constructor);
        //NOT an error to create several
        if (fClassFunction.singleton) Debug.warn(this, "Singleton already set on [" + fClassFunction.functionName() + "] (not an issue necessarily)");
        else {
          fClassFunction.singleton = oObject;

        }
      }
    ]]></javascript:static-method>

    <javascript:static-method name="registerComponent" parameters="fHostClass, fComponentClass"><![CDATA[
      if (fHostClass instanceof Function && fComponentClass instanceof Function) {
        fHostClass.componentClass = fComponentClass;
      } else {
        Debug.error(this, "OO.registerComponent() failed");
        Debug.log(this, fComponentClass);
        Debug.log(this, fHostClass);
      }
    ]]></javascript:static-method>

    <javascript:static-method name="registerClass" parameters="fClass" parameter-checks="off"><![CDATA[
      var sX;
      
      //inheritance tree (one-level of)
      if (!this.derivedClasses) this.derivedClasses = new Array();
      if (!this.baseClasses)    this.baseClasses    = new Array();

      //Class lookup
      OO.IntrospectionClass[fClass.functionName()] = fClass;
      
      //register own properties only
      //avoid properties like classDerivedConstructor which points to a Function
      //TODO: move to getOwnPropertyNames() later on (LESS support now)
      //https://developer.mozilla.org/en-US/docs/Web/JavaScript/Enumerability_and_ownership_of_properties
      for (sX in fClass.prototype) {
        if (fClass.prototype.hasOwnProperty(sX))
          OO.registerMethod(fClass, fClass.prototype[sX]);
      }
      for (sX in fClass) {
        if (fClass.hasOwnProperty(sX))
          OO.registerMethod(fClass, fClass[sX], true);
      }
      
      //class properties
      fClass.classDerivedConstructor = fClass;
      fClass.noIteration             = true;
      fClass.definedClass            = true;
      fClass.notDefinedClass         = false;
    ]]></javascript:static-method>

    <javascript:static-method name="registerMethod" parameters="fClass, fMethod, [bIsStatic]" parameter-checks="off"><![CDATA[
      //to subclass Intorspection, use the pseudo:
      //  @inherits-javascript-primitive=OO.Introspection*
      var sFType, sFName, sFClass, fFClass, fInvokeMethod;
      
      //there may be Function properties that point to external Class methods or classes
      //avoid properties like classDerivedConstructor which points to a Function
      if (fMethod instanceof Function && fMethod.functionType) {
        if (fMethod.classDerivedConstructor) {
          if (!fMethod.isClone && window.console) console.warn("[" + fMethod.functionName() + "] method ignored because already has a classDerivedConstructor")
        } else {
          //method properties
          fMethod.classDerivedConstructor = fClass;
          fMethod.noIteration             = true;
          if (bIsStatic) fMethod.isStatic = true;
          
          if ( fMethod.functionType          //method, static-method, event-handler
            && !fMethod.isProtected          //not externally available therefore no need to register
          ) {
            //------------------------------ OO.Introspection*() classes
            //can serve also as a prototype object 
            //that redirects all function calls to invoked
            sFType = fMethod.functionType;
            sFClass = 'Introspection' + sFType.capitalise();
            
            //The function-type is a Class, e.g. function IntrospectionMethod() {}
            //this ensures that the IntrospectionMethod() can be instantiated as a prototype
            fFClass = OO[sFClass];
            if (!(fFClass instanceof Function)) {
              fFClass = OO[sFClass]           = new Function("//" + sFClass + "() class");
              fFClass.functionNameHint        = sFClass;
              fFClass.classDerivedConstructor = fFClass;
            }
            
            //expando
            sFName        = fMethod.functionName();
            if (fMethod.event) sFName = fMethod.event;
            fInvokeMethod = fFClass.prototype[sFName];
            
            if ( sFName != "invoked"     //avoid the obvious infinite loop
              && sFName != "constructor" //static, native
              && sFName != "toString"    //native
              && !fInvokeMethod          //[native functions], e.g. toString will aready be present
            ) { 
              fInvokeMethod = new Function("return this.invoked('" + sFName + "', arguments);");
              fInvokeMethod.classDerivedConstructor = fFClass;
              fInvokeMethod.functionNameHint        = sFName;
              fFClass.prototype[sFName]             = fInvokeMethod;
            }
            
            //------------------------------ OO.Introspection*() names to methods
            //used for static event lookup
            if (fInvokeMethod) {
              if (!fInvokeMethod.aMethods) fInvokeMethod.aMethods = new Array();
              fInvokeMethod.aMethods.push(fMethod);
            }
          }
        }
      }
    ]]></javascript:static-method>
    
    <!-- ####################################### inheritance and method chains ####################################### -->
    <javascript:static-method name="isInbuiltObject" parameters="fFunction" parameter-checks="off"><![CDATA[
      //built-in, builtin
      return (JS_PRIMITIVES.contains(fFunction)       //String,Boolean,Number
          || JS_BUILTIN_OBJECTS.contains(fFunction)   //Object,Array,Date,Function,RegExp
          || HTML_BUILTIN_OBJECTS.contains(fFunction) //Document,Node
      );
    ]]></javascript:static-method>

    <javascript:static-method name="isCreateForInheritance" debug="off" parameters="oArgs" parameter-checks="off"><![CDATA[
      //arguments is an object, not an Array!
      return (oArgs[0] === OO.isCreateForInheritance);
    ]]></javascript:static-method>

    <javascript:static-method name="chainMethod" parameter-checks="off" parameters="fCurrentDerivedClass, oOriginalObject, [sMethodName, oArguments]"><![CDATA[
      //var oChainResult = OO.chainMethod(fClass, this, fThisMethod, arguments);
      //the primary method is already being called. we only need to check the bases
      //returns the first defined chain result only
      //baseClasses can be multiple so we do not rely on the prototype object chain
      var oChainResult, oThisChainResult, fFunctionOnPrototype;
      var aBaseClasses = fCurrentDerivedClass.baseClasses;
      
      if (aBaseClasses instanceof Array) {
        aBaseClasses.eachBackwards(function(){
          oThisChainResult     = undefined;
          //if sMethodName is undefined then we are in javascript:init
          fFunctionOnPrototype = ((sMethodName !== undefined) ? this.prototype[sMethodName] : this);
          if (fFunctionOnPrototype instanceof Function) {
            //this call may, in turn, call OO.chainMethod()
            //BUT it would send through the base-class method signature
            oThisChainResult = fFunctionOnPrototype.apply(oOriginalObject, oArguments);
          } else {
            //there is no implementation of sMethodName on this base-class
            //BUT its base-class in turn may contain sMethodName
            oThisChainResult = OO.chainMethod(this, oOriginalObject, sMethodName, oArguments);
          }
          //accept the LAST valid chain result, overwriting previous valid results
          if (oThisChainResult !== undefined) oChainResult = oThisChainResult;
        });
      }

      return oChainResult;
    ]]></javascript:static-method>

    <javascript:static-method name="registerDerivedClass" parameters="fDerivedClass, fBaseClass" parameter-checks="off"><![CDATA[
      //registration of GS Classes with inheritance
      //use OO.isCreateForInheritance(arguments) in the constructor to determine if the call is for inheritance or standard instanciation
      //and tell the fBaseClass it has a new fDerivedClass which leads to registration
      var oPrototypeObject;

      if (window.console) {
        //do not use Debug here because it may have failed to instanciate
        if (fDerivedClass === undefined)                console.error("registerDerivedClass(): fDerivedClass null");
        if (fBaseClass    === undefined)                console.error("registerDerivedClass(): fBaseClass null");
        if (!(fDerivedClass instanceof Function))       console.error("registerDerivedClass(): fDerivedClass not a Function");
        if (!(fBaseClass    instanceof Function))       console.error("registerDerivedClass(): fBaseClass not a Function");
      }

      if (fBaseClass && fDerivedClass) {
        OO.inheritFrom(fDerivedClass, fBaseClass);

        //run static registerDerivedClass() on the direct class
        //note that static method are also registered on the prototype
        if (fBaseClass.registerDerivedClass) fBaseClass.registerDerivedClass(fDerivedClass);
      }
    ]]></javascript:static-method>

    <javascript:static-method name="addFriend" parameters="fContainer, [fFriend]" parameter-checks="off"><![CDATA[
      //fFriend may not be defined yet
      //@friends must be reciprocated
      if ((fFriend !== undefined)) {
        if (!fContainer.aFriends) fContainer.aFriends = new Array();
        fContainer.aFriends.push(fFriend);
        if (!fFriend.aFriends) fFriend.aFriends = new Array();
        fFriend.aFriends.push(fContainer);
      }
    ]]></javascript:static-method>
    
    <javascript:static-method name="inheritFrom" parameters="fDerivedClass, fBaseClass, [bForceImmediate = false]" parameter-checks="off"><![CDATA[
      var oInheritanceObject;
      var sX, fBaseFunc, sFuncs, sBaseName;

      if (window.console) {
        //do not use Debug here because it may have failed to instanciate
        if (fDerivedClass === undefined)          console.error("registerDerivedClass(): fDerivedClass null");
        if (fBaseClass    === undefined)          console.error("registerDerivedClass(): fBaseClass null");
        if (!(fDerivedClass instanceof Function)) console.error("registerDerivedClass(): fDerivedClass not a Function");
        if (!(fBaseClass    instanceof Function)) console.error("registerDerivedClass(): fBaseClass not a Function");
      }

      if (fBaseClass === Object) console.warn("ignoring request to inherit from Object");
      else {
        sBaseName = (fBaseClass.functionName ? fBaseClass.functionName() : "[an Object]");
        if (fDerivedClass.inherited) {
          //-------------------- prototype copy inherit
          //JS only supports 1 prototype, so we copy the funcs this time
          //PERFORMANCE: good idea to put the larger inheritance first
          sFuncs = "";
          if (window.console) console.info(fDerivedClass.functionName() + " [" + fDerivedClass.inheritanceOrder + "] (copy inherits) => " + sBaseName+ " [" + fBaseClass.inheritanceOrder + "/" + fBaseClass.inheritanceOrderSize + "]");
          if (fBaseClass.definedClass === true) {
            for (sX in fBaseClass.prototype) {
              fBaseFunc = fBaseClass.prototype[sX];
              if (fBaseFunc instanceof Function) {
                //avoid classDerivedConstructor etc. which also point to Functions but are not actual functions
                if (fBaseFunc.functionType) {
                  if (!fDerivedClass.prototype[sX]) {
                    //Function.clone() will set:
                    //  fClone.functionNameHint
                    //  fClone.functionType
                    //  fClone.noIteration
                    //  fClone.classDerivedConstructor
                    //unfortunately Funtion.name is read-only
                    //so the new clone will have the original classDerivedConstructor
                    fDerivedClass.prototype[sX] = fBaseFunc.clone();
                    sFuncs = sFuncs.commaAdd(sX + "()");
                  } else {
                    //so it is already overriding this func. ignore the conflict, allow the override
                    //if (window.console) window.console.warn("base class method [" + sBaseName+ "->" + sX + "()] defined already on [" + fDerivedClass.functionName() + "]");
                  }
                }
              }
            }
          } else if (window.console) window.console.error("base class [" + sBaseName + "] not defined yet for multiple inheritance");

          if (window.console) console.info("  [" + sFuncs + "]");
        } else {
          //-------------------- prototype assignment
          //Array, Object, Date, String, Math are Javascript in-built objects
          //our GS objects accept a OO.isCreateForInheritance to indicate the context
          if (window.console) console.info(fDerivedClass.functionName() + " [" + fDerivedClass.inheritanceOrder + "] (inherits) => " + sBaseName+ " [" + fBaseClass.inheritanceOrder + "/" + fBaseClass.inheritanceOrderSize + "]");
          if      (OO.isInbuiltObject(fBaseClass)) oInheritanceObject = new fBaseClass();  //e.g. new Array()
          else if (fBaseClass instanceof Function) oInheritanceObject = new fBaseClass(OO.isCreateForInheritance);
          //this CAN be avoided
          //prototypes should always be new Functions() 
          //otherwise their methods can be overwritten when shared
          else if (window.console) console.error("cloneing non-Function inheritance request");
          fDerivedClass.prototype = oInheritanceObject;
          fDerivedClass.inherited = true;
        }

        //update reference arrays
        if ((fDerivedClass.baseClasses === undefined)) fDerivedClass.baseClasses = new Array(fBaseClass);
        else fDerivedClass.baseClasses.push(fBaseClass);
        if ((fBaseClass.derivedClasses === undefined)) fBaseClass.derivedClasses = new Array(fDerivedClass);
        else fBaseClass.derivedClasses.push(fDerivedClass);
      }
    ]]></javascript:static-method>

    <javascript:static-method name="executeGlobalInit" parameters="fFunc, bDelay" parameter-checks="off"><![CDATA[
      var sMessage, sClassName, sFunctionName;
      
      sFunctionName = (fFunc.functionName ? fFunc.functionName() + "()" : "global-init");
      sClassName    = (fFunc.classDerivedConstructor ? fFunc.classDerivedConstructor.functionName() : "Unknown");
      
      if (bDelay) {
        if (window.jQuery) new jQuery(document).ready(function(){executeGlobalInit(fFunc);});
        else if (window.console) window.console.error("cannot queue [" + sFunctionName + "] because jQuery not available");
      } else {
        sMessage         = fFunc();
        fFunc.sMessage   = sMessage;
        fFunc.iInitOrder = Global.iInitOrder++;
        sMessage = sClassName + "::" + sFunctionName + ": " + (sMessage ? sMessage : "ok");
        if (window.console) window.console.info(sMessage);
      }
      
      return sMessage;
    ]]></javascript:static-method>

    <!-- ########################################## generic .Object attachement ########################################## -->
    <javascript:static-method name="fromIDX" parameters="s1XmlID"><![CDATA[
      //IDX => array(objects)
      //Array lookup will return undefined, not null
      return (s1XmlID ? OO.oLinkage[s1XmlID] : undefined);
    ]]></javascript:static-method>

    <javascript:static-method name="detachFromObjects" parameters="jqElements"><![CDATA[
      var iObject, jqObjects;
      jqObjects  = jqElements.filter(".Object" + GS_OBJECT_ATTACHED_FILTER);
      iObject    = jqObjects.length;

      while (iObject) OO.detachFromObject(jqObjects.eq(--iObject));
    ]]></javascript:static-method>
    
    <javascript:static-method name="checkDisplayObjectValidities" parameters="jqDetachInvalid"><![CDATA[
      var iObject = jqDetachInvalid.length;
      while (iObject) OO.checkDisplayObjectValidity(jqDetachInvalid.eq(--iObject));
    ]]></javascript:static-method>

    <javascript:static-method name="attachToObjects" parameters="jqElements"><![CDATA[
      var jq1DisplayObject, iObject, jqObjects;
      var jqDOMs, aDOMs = new Array();

      //instanciate in reverse DOM order
      //so that inner objects get instanciated first
      //and outer objects can then immediately access their child DatabaseObjects through thC()
      jqObjects = jqElements.filter(".Object:not(" + GS_OBJECT_ATTACHED_FILTER + ")");
      iObject   = jqObjects.length;

      while (iObject) {
        jq1DisplayObject = OO.attachToObject(jqObjects.eq(--iObject));
        if (jq1DisplayObject) aDOMs.push(jq1DisplayObject.get(0));
      }
      jqDOMs = new jQuery(aDOMs);

      //run functions after all new Objects instanciated
      //displayAdded_*()
      //some objects require access to others
      jqDOMs.eachEq(OO.runDisplayAdded); //first argument will be the array item (and this)
      
      //objects in DOM order
      jqDOMs.reverse();
      
      //events
      jqDOMs.eachEq(jQuery.prototype.trigger, DEFAULT, "newobject");
      
      //return the objects in DOM order (see reverse above)
      return new JOO(jqDOMs);
    ]]></javascript:static-method>

    <javascript:static-method name="runDisplayAdded" parameters="jq1DisplayObject, ..." parameter-checks="off"><![CDATA[
      var oRet, oObject, sFuncName;

      if (oObject = jq1DisplayObject.object()) {
        sFuncName = "displayAdded_" + (jq1DisplayObject.cssInterfaceMode() ? jq1DisplayObject.cssInterfaceMode() : "default");
        if (oObject[sFuncName] instanceof Function)
          oRet = oObject[sFuncName](new JOO(jq1DisplayObject));
      }

      return oRet;
    ]]></javascript:static-method>

    <javascript:static-method name="checkDisplayObjectValidity" parameters="jqDisplayObject"><![CDATA[
      //TODO: jqDisplayObject.remove();
    ]]></javascript:static-method>

    <javascript:static-method name="detachFromObject" parameters="jq1Element"><![CDATA[
      //TODO: j1Element.remove();
    ]]></javascript:static-method>

    <javascript:static-method name="attachToObject" parameters="jq1Element"><![CDATA[
      var oObject, sXmlID, aClassNames, sClassName, fClassFunction;

      sXmlID         = jq1Element.cssXPathToNode(); //potentially zero, e.g. for transient non-xml-id objects
      aClassNames    = jq1Element.classes(/^Class__(.+)/, 1);
      oObject        = (sXmlID ? OO.fromIDX(sXmlID) : undefined);

      ]]><javascript:stage-dev><![CDATA[
        //pre-checks
        if (aClassNames.length == 0) {
          Debug.warn(this, "failed to create DatabaseObjects because cannot find class name in style");
          Debug.log(this, jq1Element, true);
        }
        if (aClassNames.length > 1) {
          Debug.warn(this, "creation limited to 1 Class__ per element at the moment because there is only one Mulition object per DatabaseElement.fromIDX([id]) " + aClassNames);
          Debug.log(this, jq1Element, true);
        }
        if (jq1Element.parent().length == 0) {
          Debug.error(this, "cannot attachToObject() without a parent()");
          Debug.log(this, jq1Element, true);
        }
      ]]></javascript:stage-dev><![CDATA[

      //register attached now just in case any of the init functions trigger an OO.attachToObjects()
      jq1Element.addClass(GS_OBJECT_ATTACHED_CLASS);

      for (i = 0; i < aClassNames.length; i++) {
        //create or add
        //using oLinkage for our Multiton pattern
        //http://en.wikipedia.org/wiki/Multiton_pattern
        //xml:id is optional for DatabaseObjects
        //TODO: IF there are more than one un-related objects on this element then they need to both be instanciated
        sClassName = aClassNames[i];

        if (oObject) {
          //will auto run removeInvalidDisplays() first
          if (!oObject.hasValidDisplays()) {
            //previous possible single element DatabaseElement now defunct
            //completely recreate a new one
            delete oObject;
            oObject = null;
            //delete OO.oLinkage[sXmlID]; //not necessary. will be populated below
          } else {
            //multiple element DatabaseElement with several valid instances
            oObject.addElement(jq1Element);
          }
        }

        if (!oObject) {
          fClassFunction = window[sClassName];
          //e.g. Class__HTTP will not have an equivalent JS object
          if (fClassFunction instanceof Function) {
            //create new, and run the init()
            oObject = new fClassFunction();
            if (oObject.addElement instanceof Function) oObject.addElement(jq1Element);
            else Debug.warning(this, "Object has no addElement() to record the new display");

            //abstract indicates that the object is NOT tied to it's xml:id
            //so we create a new object everytime
            //e.g. MultiView, CodeMirrorEditor
            if (fClassFunction.abstract) sActualID = OO.registerObject(oObject);
            else                         sActualID = OO.registerObject(oObject, sXmlID);
            if (sXmlID != sActualID) jq1Element.cssXPathToNode(sActualID); //so hC() can find it!
          } else Debug.error(this, "Class [" + sClassName + "] not found");
        }
      } //for aClassNames
      
      ]]><javascript:stage-dev><![CDATA[
        //post-checks
        if (jq1Element.data().Object === undefined) {
          Debug.error(this, "attach failed to populate the DOM data() Object owner linkage. this is usually done in DisplayObject.init");
          Debug.log(this, jq1Element, true);
        } 
        else if ((oObject === undefined)) {
          Debug.error(this, "attach failed to create Object");
          Debug.log(this, jq1Element, true);
        }
        else if ((oObject.jqDisplays === undefined)) {
          Debug.error(this, "Object has no display store");
          Debug.log(this, jq1Element, true);
        }
        else if (!oObject.jqDisplays.length) {
          Debug.error(this, "Object display store empty");
          Debug.log(this, jq1Element, true);
        }
      ]]></javascript:stage-dev><![CDATA[
      
      return jq1Element;
    ]]></javascript:static-method>

    <javascript:static-method name="registerObject" parameters="oObject, [sXmlId]"><![CDATA[
      var sActualID = (sXmlId ? sXmlId : "noXmlID_" + OO.iUniqueObjectID);
      OO.oLinkage[sActualID] = oObject;
      oObject.iUniqueObjectID = OO.iUniqueObjectID;
      OO.iUniqueObjectID++;
      return sActualID;
    ]]></javascript:static-method>

    <javascript:static-method name="listObjects" parameters="[fClass, bIncludeDerivedClasses = true, bIncludeAbstract = true]"><![CDATA[
      //JOO(Frame) => JOO(".Class__Frame") is also possible
      //TODO: bIncludeAbstract (move HTMLObject => BaseObject first)
      var sXmlID, oObject, aObjects = new Array();
      
      for (sXmlID in OO.oLinkage) {
        oObject = OO.oLinkage[sXmlID];
        if (oObject) {
          if ( (!fClass)
            || (oObject.classDerivedConstructor === fClass)
            || (bIncludeDerivedClasses && oObject instanceof fClass)
          ) {
            //Debug.log(this, "OO.fromIDX('" + sXmlID + "').highlight():");
            //Debug.log(this, oObject);
            aObjects.push(oObject);
          }
        }
      }

      return aObjects;
    ]]></javascript:static-method>
  </javascript:namespace>
  
  <javascript:namespace name="Global">
    <!-- used for holding Global global-init -->
    <javascript:static-property name="iInitOrder">0</javascript:static-property>
  </javascript:namespace>
</javascript:code>
