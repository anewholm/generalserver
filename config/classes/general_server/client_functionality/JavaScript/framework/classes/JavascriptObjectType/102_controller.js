<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006">
  <javascript:object>
    <javascript:static-method name="istype" parameters="uValue" parameter-checks="off"><![CDATA[
      //we do not extend Object because that underlying breaks libraries that use these base Object
      return (uValue instanceof Object);
    ]]></javascript:static-method>

    <javascript:static-method name="isTypeHungarianNotation" parameters="s1Name" parameter-checks="off"><![CDATA[
      return (s1Name[0] == "o");
    ]]></javascript:static-method>

    <javascript:static-method name="isTypeNaturalNotation" parameters="s1NameCamel" parameter-checks="off"><![CDATA[
      return false;
    ]]></javascript:static-method>
  </javascript:object>
</javascript:code>