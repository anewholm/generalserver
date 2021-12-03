<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006">
  <javascript:object>
    <!-- https://www.w3.org/TR/CSS21/grammar.html#scanner -->
    <javascript:static-property name="rxClausesSplitRegEx"><![CDATA[/ *, */]]></javascript:static-property>
    <javascript:static-property name="rxStepsSplitRegEx"><![CDATA[/[ >]+/]]></javascript:static-property>
    <javascript:static-property name="rxStepClassRegEx"><![CDATA[/\.([-_a-zA-Z0-9]+)/]]></javascript:static-property>
    <javascript:static-property name="rxParentRegEx"><![CDATA[/[> ]*[^ ]+$/]]></javascript:static-property>

    <!-- ######################################################## constructors ######################################################## -->
    <javascript:init parameters="[uCSSPath]" chain="true"><![CDATA[
      //do NOT inherit from String because conversions to String will then be turned off
      //native non-generic String functions will not work on String derived classes

      //---------------------------------------------------------- static
      //like jQuery can be called statically CSSPathString(...) or new CSSPathString(...)
      var bNew = (this instanceof CSSPathString);      //Function called with the new operator
      if (!bNew) return new CSSPathString(uCSSPath);

      //---------------------------------------------------------- new this
      if      (String.istype(uCSSPath))  
        this.csspath = uCSSPath.trim();
      else if (uCSSPath instanceof Function && uCSSPath.functionName()) 
        //interface:ContextMenu
        //TODO: analyse the @elements?
        this.csspath = CSSPathString.gsClass(uCSSPath);
      else if (arguments.length === 0 || (uCSSPath === undefined))
        this.csspath = "";
      else if (uCSSPath === null) 
        Debug.error(this, "null string sent through to CSSPathString init");
      else if (uCSSPath instanceof window.RegExp) 
        this.csspath = uCSSPath.toCSSPathString();
      else if (uCSSPath instanceof Object && uCSSPath.toString instanceof Function) 
        this.csspath = uCSSPath.toString().trim();
      else 
        Debug.error(this, "CSSPathString initialiser not understood");
    ]]></javascript:init>

    <javascript:static-method name="fromGSClass" parameters="fGSClass, [bClassNameOnly = false]"><![CDATA[
      return new CSSPathString(bClassNameOnly ? CSSPathString.gsClassName(fGSClass) : CSSPathString.gsClass(fGSClass));
    ]]></javascript:static-method>

    <javascript:static-method name="fromClass" parameters="s1ClassName"><![CDATA[
      return new CSSPathString(CSSPathString.cssClass(s1ClassName));
    ]]></javascript:static-method>

    <javascript:static-method name="fromID" parameters="s1ID"><![CDATA[
      return new CSSPathString(CSSPathString.cssID(s1ID));
    ]]></javascript:static-method>
    
    <javascript:static-method name="fromDashClassList" parameters="sDashString"><![CDATA[
      return sDashString.split("-").toCSSPathString();
    ]]></javascript:static-method>

    <javascript:static-method name="fromUnderscoreClassList" parameters="sUnderscoreString"><![CDATA[
      return sUnderscoreString.split("_").toCSSPathString();
    ]]></javascript:static-method>

    <!-- ############################### conversion ############################### -->
    <javascript:conversion to-type="String"><![CDATA[
      return this.csspath;
    ]]></javascript:conversion>

    <javascript:conversion to-type="JQueryNativeValue"><![CDATA[
      return this.toString();
    ]]></javascript:conversion>

    <!-- ######################################################## methods ######################################################## -->
    <javascript:method name="eq" parameters="(CSSPathString) cCSSPath"><![CDATA[
      return (this.csspath == cCSSPath.csspath);
    ]]></javascript:method>
    
    <javascript:static-method name="isTypeHungarianNotation" parameters="s1Name" parameter-checks="off"><![CDATA[
      return (s1Name[0] == "c");
    ]]></javascript:static-method>

    <javascript:static-method name="isTypeNaturalNotation" parameters="s1NameCamel" parameter-checks="off"><![CDATA[
      return s1NameCamel.isSingular() && s1NameCamel.startsWith("css");
    ]]></javascript:static-method>

    <javascript:static-method name="istype" parameters="uValue" parameter-checks="off"><![CDATA[
      return (uValue instanceof CSSPathString);
    ]]></javascript:static-method>

    <javascript:method name="valueOf"><![CDATA[
      return this.toString();
    ]]></javascript:method>

    <javascript:static-method name="cssClass" parameters="sClassName"><![CDATA[
      return "." + sClassName;
    ]]></javascript:static-method>

    <!-- ######################################################## private ######################################################## -->
    <javascript:method name="usageCheck" is-protected="true"><![CDATA[
      if (this == window) Debug.warn(this, "CSSPathString::usageCheck() jQuery is probably trying to serialize() this object, use toString() first");
      return this != window;
    ]]></javascript:method>

    <javascript:method name="nextStep" is-protected="true" parameters="[bSelf = false, bDirect = !bSelf]"><![CDATA[
      var sNextStep = "";
      if (!bSelf && bDirect && !this.csspath.trim().endsWith(">")) sNextStep += " >";
      if (!bSelf) sNextStep += " ";
      return sNextStep;
    ]]></javascript:method>

    <javascript:method name="functionCall" is-protected="true" parameters="sFunctionName, ..."><![CDATA[
      return sFunctionName + "(" + Array.prototype.slice.call(arguments,1).join() + ")";
    ]]></javascript:method>

    <javascript:method name="filter" is-protected="true" parameters="sWhat, ..."><![CDATA[
      return ":" + (arguments.length > 1 ? this.functionCall.apply(this, arguments) : sWhat);
    ]]></javascript:method>

    <javascript:static-method name="gsClassName" is-protected="true" parameters="fGSClass"><![CDATA[
      //<Class> => CSS__<Class>
      //Menu    => CSS__Menu
      //we use the CSS classname because that takes in to account inheritance
      var sCSSClassName;
      if      (fGSClass === Object) sCSSClassName = "Object";
      else if (!fGSClass.definedClass) Debug.error(this, "fClass requires definedClass");
      else sCSSClassName = "CSS__" + fGSClass.functionName();
      return sCSSClassName;
    ]]></javascript:static-method>

    <javascript:static-method name="gsClass" is-protected="true" parameters="fGSClass"><![CDATA[
      return this.cssClass(this.gsClassName(fGSClass));
    ]]></javascript:static-method>

    <javascript:static-method name="cssID" is-protected="true" parameters="sID"><![CDATA[
      return "#" + sID;
    ]]></javascript:static-method>

    <javascript:static-method name="cssElement" is-protected="true" parameters="sTagName"><![CDATA[
      return sTagName;
    ]]></javascript:static-method>

    <javascript:method name="clauses" is-protected="true"><![CDATA[
      return this.csspath.split(CSSPathString.rxClausesSplitRegEx);
    ]]></javascript:method>

    <javascript:method name="lastStep" is-protected="true"><![CDATA[
      var sLastStep, aSteps;
      if (aSteps = this.steps()) sLastStep = aSteps[aSteps.length-1];
      return sLastStep;
    ]]></javascript:method>

    <javascript:method name="steps" is-protected="true"><![CDATA[
      var aSteps;
      if (this.isMultiple()) Debug.error(this, "cannot return steps on multiple clause CSS path [" + this.csspath + "]");
      else aSteps = this.csspath.split(CSSPathString.rxStepsSplitRegEx);
      return aSteps;
    ]]></javascript:method>

    <javascript:method name="lastClassName" is-protected="true"><![CDATA[
      var sLastClassName, sLastStep;
      if (sLastStep = this.lastStep()) sLastClassName = this.classInStep(sLastStep);
      return sLastClassName;
    ]]></javascript:method>

    <javascript:method name="classInStep" is-protected="true" parameters="sStep"><![CDATA[
      var sClassName, aMatches;

      if (aMatches = CSSPathString.rxStepClassRegEx.exec(sStep)) {
        if (aMatches.length == 2) sClassName = aMatches[1];
      }

      return sClassName;
    ]]></javascript:method>

    <!-- ############################### collections ############################### -->
    <javascript:method name="or" parameters="cWhat"><![CDATA[
      //just in case the csspath is empty
      return (this.csspath ? this.csspath + "," + cWhat.csspath : cWhat.csspath);
    ]]></javascript:method>

    <javascript:method name="isMultiple"><![CDATA[
      return (this.clauses().length > 1);
    ]]></javascript:method>

    <!-- ############################### axis ############################### -->
    <javascript:method name="children" parameters="[sClassName]"><![CDATA[
      var cNewCSSPath;

      if (this.usageCheck()) {
        if (this.isMultiple()) Debug.error(this, "cannot traverse axis on a multiple clause CSS path");
        else cNewCSSPath = new CSSPathString(this.csspath + this.nextStep() + ((sClassName !== undefined) ? CSSPathString.cssClass(sClassName) : "*" ));
      }

      return cNewCSSPath;
    ]]></javascript:method>

    <javascript:method name="self" parameters="s1ClassName"><![CDATA[
      var cNewCSSPath;

      if (this.usageCheck()) {
        if (this.isMultiple()) Debug.error(this, "cannot traverse axis on a multiple clause CSS path");
        else cNewCSSPath = new CSSPathString(this.csspath + CSSPathString.cssClass(sClassName));
      }

      return cNewCSSPath;
    ]]></javascript:method>

    <javascript:method name="parent"><![CDATA[
      var cNewCSSPath;

      if (this.usageCheck()) {
        if (this.isMultiple()) Debug.error(this, "cannot traverse axis on a multiple clause CSS path");
        else cNewCSSPath = new CSSPathString(this.csspath.replace(CSSPathString.rxParentRegEx, ""));
      }

      return cNewCSSPath;
    ]]></javascript:method>

    <!-- ############################### perogatives ############################### -->
    <javascript:method name="withDefaultInterfaceMode"><![CDATA[
      return this.withInterfaceMode("default");
    ]]></javascript:method>

    <javascript:method name="withInterfaceMode" parameters="s1InterfaceMode"><![CDATA[
      var cNewCSSPath;

      if (this.usageCheck()) {
        if (this.isMultiple()) Debug.error(this, "cannot traverse axis on a multiple clause CSS path");
        else cNewCSSPath = new CSSPathString(this.csspath + ".gs-interface-mode-" + s1InterfaceMode);
      }

      return cNewCSSPath;
    ]]></javascript:method>

    <javascript:method name="withName" parameters="uLocalName, [s1Name, bSelf = true]"><![CDATA[
      //.withName("localName")   => .gs-name-localName
      //.withName(Frame)         => .gs-name-frame
      //.withName("verticalframeset") => .gs-name-verticalframeset
      //.withName(Frame, "head") => .gs-name-frame-head
      var sNameClass, cNewCSSPath;

      if (this.usageCheck()) {
        if (this.isMultiple()) Debug.error(this, "cannot traverse axis on a multiple clause CSS path");
        else {
          sNameClass = ".gs-name-";
          if (Class.istype(uLocalName)) sNameClass += uLocalName.functionName().toLowerCase();
          else sNameClass += uLocalName.toString();
          if (s1Name) sNameClass += "-" + s1Name;
          
          cNewCSSPath = new CSSPathString(this.csspath + this.nextStep(bSelf) + sNameClass);
        }
      }

      return cNewCSSPath;
    ]]></javascript:method>

    <javascript:method name="first"><![CDATA[
      if (this.usageCheck()) {
        var sCSSPath = this.csspath + this.filter("first");
        return new CSSPathString(sCSSPath);
      }
    ]]></javascript:method>

    <javascript:method name="regex" parameters="s1RegEx, [sAttribute = 'class']"><![CDATA[
      if (this.usageCheck()) {
        var sCSSPath = this.csspath + this.filter("regex", sAttribute, s1RegEx);
        return new CSSPathString(sCSSPath);
      }
    ]]></javascript:method>

    <javascript:method name="not" parameters="(String) sWhat"><![CDATA[
      if (this.usageCheck()) {
        var sCSSPath = this.csspath + this.filter("not", sWhat);
        return new CSSPathString(sCSSPath);
      }
    ]]></javascript:method>

    <javascript:method name="is" parameters="sAttributeName"><![CDATA[
      if (this.usageCheck()) {
        var sCSSPath = this.csspath + this.filter(sAttributeName);
        return new CSSPathString(sCSSPath);
      }
    ]]></javascript:method>

    <javascript:method name="isNot" parameters="sAttributeName"><![CDATA[
      if (this.usageCheck()) {
        var sCSSPath = this.csspath + this.not(sAttributeName);
        return new CSSPathString(sCSSPath);
      }
    ]]></javascript:method>
  </javascript:object>
</javascript:code>