<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/javascript/2006">
  <javascript:object>
    <!-- REQUIRED object so that it can instanciate DatabaseObject for control of javascript:code LIs -->

    <javascript:static-event-handler event="ready">
      //see associated CSS classes
      new JOO(".js_show").show();
      new JOO(".js_hide").show();
      new JOO(".js_remove").remove();
    </javascript:static-event-handler>
  </javascript:object>

  <javascript:global-function name="help" debug="false"><![CDATA[
    //help               //show this help" +
    //gsQuery().everything(); //show all the current General Server obejcts available" +
    //..."
  ]]></javascript:global-function>
</javascript:code>