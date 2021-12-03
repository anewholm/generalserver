<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/javascript/2006">
  <javascript:object>
    <javascript:static-property name="sDebugFlag">'XSLT_TRACE_APPLY_TEMPLATES,XSLT_TRACE_TEMPLATES,XSLT_TRACE_CALL_TEMPLATE'</javascript:static-property>
    <javascript:static-method name="debugAJAXOptions" parameters="oOptions"><![CDATA[
      oOptions.url = oOptions.url.setUrlParameter("configuration-flags", XSLTemplate.sDebugFlag, String.prototype.commaAdd);
    ]]></javascript:static-method>

    <javascript:method name="canHandleEvent" parameters="eEvent, jqCurrentDisplayObject, fEventHandler"><![CDATA[
      //XSLTempate commands are often menu-items
      return (jqCurrentDisplayObject.cssInterfaceMode() != "controller");
    ]]></javascript:method>
  </javascript:object>
</javascript:code>