<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006">
  <javascript:object>
    <javascript:static-property name="sDebugFlag">'XML_DEBUG_XPATH_EXPR'</javascript:static-property>
    <!-- isTypeHungarianNotation() goes first because the static_property will cause a type check including this class -->
    <javascript:static-method name="isTypeHungarianNotation" parameters="s1Name" parameter-checks="off"><![CDATA[
      return (s1Name[0] == "x");
    ]]></javascript:static-method>

    <javascript:static-method name="isTypeNaturalNotation" parameters="s1NameCamel" parameter-checks="off"><![CDATA[
      return s1NameCamel.isSingular() && s1NameCamel.match(/^xpath|^destination$|^select$|^condition$|^transform$|^nodeMask|^data$/);
    ]]></javascript:static-method>

    <javascript:static-method name="istype" parameters="uValue" parameter-checks="off"><![CDATA[
      return (uValue instanceof XPathString);
    ]]></javascript:static-method>

    <javascript:static-property name="TRUE" description="the xpath true() function">"true()"</javascript:static-property>
    <javascript:static-property name="FALSE" description="the xpath false() function">"false()"</javascript:static-property>

    <javascript:static-method name="debugAJAXOptions" parameters="oOptions"><![CDATA[
      oOptions.url = oOptions.url.setUrlParameter("configuration-flags", XPathString.sDebugFlag, String.prototype.commaAdd);
    ]]></javascript:static-method>

    <!-- ######################################################## constructors ######################################################## -->
    <javascript:init parameters="[uXPath]"><![CDATA[
      //---------------------------------------------------------- static
      //like jQuery can be called statically XPathString(...) or new XPathString(...)
      var bNew = (this instanceof XPathString);      //Function called with the new operator
      if (!bNew) return new XPathString(uXPath);

      //---------------------------------------------------------- new this
      if      (String.istype(uXPath))  
        this.xpath = uXPath.trim();
      else if (uXPath instanceof Function && uXPath.definedClass) 
        //interface:ContextMenu
        //TODO: analyse the @elements?
        this.xpath = this.classElement(uXPath);
      else if (arguments.length === 0 || (uXPath === undefined))
        this.xpath = "";
      else if (uXPath === null) 
        Debug.error(this, "null string sent through to XPathString init");
      else if (uXPath instanceof Object && uXPath.toString instanceof Function) 
        this.xpath = uXPath.toString().trim();
      else 
        Debug.error(this, "XPathString initialiser not understood");
    ]]></javascript:init>

    <javascript:static-method name="document"><![CDATA[
      return new XPathString("/");
    ]]></javascript:static-method>

    <javascript:static-method name="root"><![CDATA[
      return new XPathString("/object:Server");
    ]]></javascript:static-method>

    <javascript:static-method name="self"><![CDATA[
      return new XPathString(".");
    ]]></javascript:static-method>

    <javascript:static-method name="xmlID" parameters="sXmlID"><![CDATA[
      if (!sXmlID) Debug.error(XPathString, "XPathString::xmlID() sXmlID is empty");
      return new XPathString("id(" + sXmlID.quote() + ")");
    ]]></javascript:static-method>

    <javascript:static-method name="fromString" parameters="sString"><![CDATA[
      return new XPathString(sString.quote());
    ]]></javascript:static-method>

    <javascript:static-method name="fromBoolean" parameters="bBool"><![CDATA[
      return new XPathString(bBool ? XPathString.TRUE : XPathString.FALSE);
    ]]></javascript:static-method>

    <javascript:static-method name="xpath" parameters="oXPath"><![CDATA[
      return new XPathString(oXPath);
    ]]></javascript:static-method>

    <javascript:static-method name="nodeset" parameters="sXPath"><![CDATA[
      return XPathString.xpath(sXPath);
    ]]></javascript:static-method>

    <javascript:static-method name="fromNumber" parameters="iNumber"><![CDATA[
      return new XPathString(iNumber);
    ]]></javascript:static-method>

    <javascript:static-method name="databaseClass" parameters="oClassName"><![CDATA[
      var oXPath, sClassName;

      if (!oClassName) Debug.error(XPathString, "XPathString::class() oClassName is empty");
      else {
        if (oClassName instanceof Object && oClassName.definedClass) sClassName = oClassName.functionName();
        else sClassName = oClassName;
        oXPath = new XPathString("~" + sClassName);
      }

      return oXPath;
    ]]></javascript:static-method>

    <!-- ######################################################## methods ######################################################## -->
    <javascript:method name="eq" parameters="(XPathString) xPathString"><![CDATA[
      return (xPathString && this.xpath === xPathString.xpath);
    ]]></javascript:method>
    
    <javascript:method name="containsParameter"><![CDATA[
      return this.xpath.contains("$");
    ]]></javascript:method>
    

    <javascript:method name="usageCheck" is-protected="true"><![CDATA[
      if (this == window) Debug.warn(this, "XPathString::usageCheck() jQuery is probably trying to serialize() this object, use toString() first");
      return this != window;
    ]]></javascript:method>

    <javascript:method name="databaseXStylesheet"><![CDATA[
      //returns the stand-alone server-side xsl inherited xstylesheet for the Class
      //e.g. database:xstylesheet('User',true(),false())
      var sStandalone     = XPathString.TRUE;
      var sClientSideOnly = XPathString.FALSE;
      return new XPathString("database:class-xstylesheet(" + this.databaseClasses().toString() + "," + sStandalone + "," + sClientSideOnly + ")");
    ]]></javascript:method>

    <javascript:method name="databaseClasses"><![CDATA[
      return new XPathString("database:classes(" + this.xpath + ")");
    ]]></javascript:method>

    <javascript:method name="nextStep" is-protected="true"><![CDATA[
      //just in case the xpath is empty
      return (!this.xpath || this.xpath.endsWith("/") ? "" : "/");
    ]]></javascript:method>

    <javascript:method name="filter" parameters="sWhat"><![CDATA[
      return "[" + sWhat + "]";
    ]]></javascript:method>

    <javascript:method name="classElement" parameters="fClass"><![CDATA[
      return "object:" + fClass.functionName();
    ]]></javascript:method>

    <javascript:conversion to-type="String"><![CDATA[
      return this.xpath;
    ]]></javascript:conversion>

    <javascript:method name="valueOf"><![CDATA[
      return this.toString();
    ]]></javascript:method>

    <javascript:conversion to-type="AbsoluteURL"><![CDATA[
      //adds a root slash if not already there, e.g.
      //  id('idx_456') => /id('idx_456')
      //  object:Server/object:Thing => /object:Server/object:Thing
      return this.toString().slashPrepend().toURLEncoded();
    ]]></javascript:conversion>

    <!-- ############################### collections ############################### -->
    <javascript:method name="or" parameters="xWhat"><![CDATA[
      //just in case the xpath is empty
      return (this.xpath ? this.xpath + "|" + xWhat.xpath : xWhat.xpath);
    ]]></javascript:method>

    <javascript:method name="isMultiple"><![CDATA[
      return this.xpath.contains("|");
    ]]></javascript:method>

    <!-- ############################### axis ############################### -->
    <javascript:method name="children" parameters="[sType]"><![CDATA[
      if (this.usageCheck()) {
        var sNewXPath = this.xpath + this.nextStep() + (sType === undefined ? "*" : sType );
        return new XPathString(sNewXPath);
      }
    ]]></javascript:method>

    <javascript:method name="ancestorsOrSelf" parameters="[sType]"><![CDATA[
      if (this.usageCheck()) {
        var sNewXPath = this.xpath + this.nextStep() + "ancestors-or-self::" + (sType === undefined ? "*" : sType );
        return new XPathString(sNewXPath);
      }
    ]]></javascript:method>

    <javascript:method name="ancestorOrSelf" parameters="[sType]"><![CDATA[
      if (this.usageCheck()) {
        var sNewXPath = this.xpath + this.nextStep() + "ancestor-or-self::" + (sType === undefined ? "*" : sType );
        return new XPathString(sNewXPath);
      }
    ]]></javascript:method>

    <javascript:method name="ancestors" parameters="[sType]"><![CDATA[
      if (this.usageCheck()) {
        var sNewXPath = this.xpath + this.nextStep() + "ancestors::" + (sType === undefined ? "*" : sType );
        return new XPathString(sNewXPath);
      }
    ]]></javascript:method>

    <javascript:method name="ancestor" parameters="[sType]"><![CDATA[
      if (this.usageCheck()) {
        var sNewXPath = this.xpath + this.nextStep() + "ancestor::" + (sType === undefined ? "*" : sType );
        return new XPathString(sNewXPath);
      }
    ]]></javascript:method>

    <javascript:method name="parent"><![CDATA[
      if (this.usageCheck()) {
        var sNewXPath = this.xpath + this.nextStep() + "..";
        return new XPathString(sNewXPath);
      }
    ]]></javascript:method>

    <!-- ############################### predicates ############################### -->
    <javascript:method name="first"><![CDATA[
      if (this.usageCheck()) {
        var sNewXPath = this.xpath + this.filter("1");
        return new XPathString(sNewXPath);
      }
    ]]></javascript:method>

    <javascript:method name="not" parameters="sWhat"><![CDATA[
      if (this.usageCheck()) {
        var sNewXPath = this.xpath + this.filter("not(" + sWhat + ")");
        return new XPathString(sNewXPath);
      }
    ]]></javascript:method>

    <javascript:method name="is" parameters="sAttributeName"><![CDATA[
      if (this.usageCheck()) {
        var sNewXPath = this.xpath + this.filter("str:boolean(@" + sAttributeName + ")");
        return new XPathString(sNewXPath);
      }
    ]]></javascript:method>

    <javascript:method name="isNot" parameters="sAttributeName"><![CDATA[
      if (this.usageCheck()) {
        var sNewXPath = this.xpath + this.filter("str:not(@" + sAttributeName + ")");
        return new XPathString(sNewXPath);
      }
    ]]></javascript:method>

    <javascript:method name="hasChildren" parameters="[sType]"><![CDATA[
      if (this.usageCheck()) {
        var sNewXPath = this.xpath + this.filter(sType === undefined ? "*" : sType );
        return new XPathString(sNewXPath);
      }
    ]]></javascript:method>
  </javascript:object>
</javascript:code>
