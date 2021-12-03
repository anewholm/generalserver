<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006">
  <javascript:object>
    <javascript:static-property name="aVariableTypes">JS_BASE_CHECK_TYPES</javascript:static-property>
    <javascript:static-method name="registerDerivedClass" parameters="fDerivedClass" parameter-checks="off"><![CDATA[
      //we unshift to place the more advanced VariableTypes at the front
      //so they can override the base types
      VariableType.aVariableTypes.unshift(fDerivedClass);
    ]]></javascript:static-method>
  </javascript:object>
</javascript:code>