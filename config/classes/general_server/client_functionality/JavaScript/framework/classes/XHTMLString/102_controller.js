<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006">
  <javascript:object>
    <!-- isTypeHungarianNotation() goes first because the static_property will cause a type check including this class -->
    <javascript:static-method name="isTypeHungarianNotation" parameters="s1Name" parameter-checks="off"><![CDATA[
      return (s1Name.substr(0,5) == "xhtml");
    ]]></javascript:static-method>

    <javascript:static-method name="isTypeNaturalNotation" parameters="s1NameCamel" parameter-checks="off"><![CDATA[
      return s1NameCamel.isSingular() && s1NameCamel.match(/^xhtml/);
    ]]></javascript:static-method>

    <javascript:static-method name="istype" parameters="uValue" parameter-checks="off"><![CDATA[
      return (uValue instanceof XHTMLString);
    ]]></javascript:static-method>

    <!-- ######################################################## constructors ######################################################## -->
    <javascript:init parameters="[uXHTML]"><![CDATA[
      //---------------------------------------------------------- static
      //like jQuery can be called statically XHTMLString(...) or new XHTMLString(...)
      var bNew = (this instanceof XHTMLString);      //Function called with the new operator
      if (!bNew) return new XHTMLString(uXHTML);

      this.xhtml = uXHTML.trim();
    ]]></javascript:init>

    <!-- ######################################################## conversion ######################################################## -->
    <javascript:conversion to-type="String"><![CDATA[
      return this.xhtml;
    ]]></javascript:conversion>

    <javascript:conversion to-type="JOO"><![CDATA[
      //simply adds a root slash if not already there, e.g.
      //  id('idx_456') => /id('idx_456')
      //  object:Server/object:Thing => /object:Server/object:Thing
      return new JOO(this.toString());
    ]]></javascript:conversion>
  </javascript:object>
</javascript:code>