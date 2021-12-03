<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/javascript/2006">
  <javascript:object>
    <javascript:static-property name="sDebugFlag">'LOG_REQUEST_DETAIL'</javascript:static-property>
    <javascript:static-property name="oAjaxStandardOptions"><![CDATA[
      {
        context: this, //this context for callbacks
        cache:   true  //false: adds a timestamp to the URL to force HTTP request
      }
    ]]></javascript:static-property>

    <javascript:static-method name="debugAJAXOptions" parameters="oOptions"><![CDATA[
      oOptions.url = oOptions.url.setUrlParameter("configuration-flags", HTTP.sDebugFlag, String.prototype.commaAdd);
    ]]></javascript:static-method>

    <javascript:static-method name="ajax" parameters="oOptions"><![CDATA[
      //https://api.jquery.com/jQuery.ajax/
      //allow the browser to cache calls
      //TODO: strip interfaceMode and re-apply on top level XML results on receipt
      Debug.debugAJAXOptions(oOptions);

      //some defaults
      if (oOptions.url === undefined) oOptions.url = "/";
      
      //some checks
      if (oOptions.url.contains("#")) Debug.warn(this, "# in URL [" + oOptions.url + "]will be ignored"); //STAGE-DEV
      
      jQuery.ajax(oOptions);
    ]]></javascript:static-method>
  </javascript:object>
</javascript:code>