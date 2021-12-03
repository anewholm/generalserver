<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006">
  <javascript:object>
    <javascript:static-method name="istype" parameter-checks="off" parameters="uValue"><![CDATA[
      var bCorrectType = false;
      switch (typeof uValue) {
        case "string":
        case "number":
        case "boolean":
          bCorrectType = true;
      }
      return bCorrectType;
    ]]></javascript:static-method>

    <javascript:static-method name="isTypeHungarianNotation" parameters="s1Name" parameter-checks="off"><![CDATA[
      return s1Name.isAllUpperCase();
    ]]></javascript:static-method>

    <javascript:static-method name="isTypeNaturalNotation" parameters="s1NameCamel" parameter-checks="off"><![CDATA[
      return s1NameCamel.isAllUpperCase();
    ]]></javascript:static-method>
  </javascript:object>
</javascript:code>