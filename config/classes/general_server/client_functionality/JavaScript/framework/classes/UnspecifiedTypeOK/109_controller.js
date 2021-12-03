<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006">
  <javascript:object>
    <javascript:static-method name="istype" parameter-checks="off" parameters="uValue"><![CDATA[
      //type has not been defined and that is ok
      //this is different to:
      //  AnyVariableType: which indicates the type has been defined and can hold anything
      //  undefined: type unknown and not ok
      return true;
    ]]></javascript:static-method>

    <javascript:static-method name="isTypeHungarianNotation" parameters="s1Name" parameter-checks="off"><![CDATA[
      //commented because this is a Type and thus will be checked here anyway
      //if (window.console) window.console("there can be no UnspecifiedTypeOK in Hungarian Notation, only u = AnyVariableType");
      return false;
    ]]></javascript:static-method>

    <javascript:static-method name="isTypeNaturalNotation" parameters="s1NameCamel" parameter-checks="off"><![CDATA[
      return false;
    ]]></javascript:static-method>
  </javascript:object>
</javascript:code>