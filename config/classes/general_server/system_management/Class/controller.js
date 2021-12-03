<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/javascript/2006">
  <javascript:object>
    <javascript:init parameters="[FClass]"><![CDATA[
      //new Class()
      this.fClass = FClass;
    ]]></javascript:init>
    
    <javascript:method name="displayAdded_default" parameters="j1DisplayObject" chain="true"><![CDATA[
      //...
      //this.fClass = ...
    ]]></javascript:method>
    
    <javascript:static-method name="istype" parameter-checks="off" parameters="uValue"><![CDATA[
      //all registerClass() have definedClass === true
      //apart from Object which is a pseudo-class that should not have expandos
      return ((uValue instanceof Function && uValue.definedClass === true) || uValue === Object);
    ]]></javascript:static-method>

    <javascript:static-method name="isTypeHungarianNotation" parameters="s1Name" parameter-checks="off"><![CDATA[
      return (s1Name[0] == "F");
    ]]></javascript:static-method>

    <javascript:static-method name="isTypeNaturalNotation" parameters="s1NameCamel" parameter-checks="off"><![CDATA[
      //don't grab class-name
      return s1NameCamel.isSingular() && s1NameCamel.startsWith("class") && !s1NameCamel.endsWith("Name");
    ]]></javascript:static-method>

    <javascript:conversion to-type="JQueryNativeValue"><![CDATA[
      return CSSPathString.fromGSClass(this.fClass).toString();
    ]]></javascript:conversion>

    <javascript:conversion to-type="String"><![CDATA[
      return (this.fClass ? this.fClass.functionName() : undefined);
    ]]></javascript:conversion>

    <javascript:event-handler event="submitfailure" chain="true"><![CDATA[
      alert('oops says the Class object');
      return;
    ]]></javascript:event-handler>
  </javascript:object>
</javascript:code>