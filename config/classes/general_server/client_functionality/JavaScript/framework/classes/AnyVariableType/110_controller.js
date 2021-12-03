<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006">
  <!-- IMPORTANT to register this class last in the types chain -->
  <javascript:object>
    <javascript:static-method name="istype" parameter-checks="off" parameters="uValue"><![CDATA[
      return true;
    ]]></javascript:static-method>

    <javascript:static-method name="isTypeHungarianNotation" parameters="s1Name" parameter-checks="off"><![CDATA[
      return (s1Name[0] == "u");
    ]]></javascript:static-method>

    <javascript:static-method name="isTypeNaturalNotation" parameters="s1NameCamel" parameter-checks="off"><![CDATA[
      return false;
    ]]></javascript:static-method>
  </javascript:object>
</javascript:code>