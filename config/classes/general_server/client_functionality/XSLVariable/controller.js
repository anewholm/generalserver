<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/javascript/2006">
  <javascript:object>
    <javascript:static-property name="sDebugFlag">'XSLT_TRACE_VARIABLES'</javascript:static-property>
    <javascript:static-method name="debugAJAXOptions" parameters="oOptions"><![CDATA[
      oOptions.url = oOptions.url.setUrlParameter("configuration-flags", XSLVariable.sDebugFlag, String.prototype.commaAdd);
    ]]></javascript:static-method>
  </javascript:object>
</javascript:code>