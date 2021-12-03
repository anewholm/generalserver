<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" name="controller" controller="true">
  <javascript:object>
    <javascript:init parameters="[sCacheName, oObject]"><![CDATA[
      if (sCacheName) this[sCacheName] = oObject;
    ]]></javascript:init>

    <javascript:method name="clear"><![CDATA[
      for (var i in this) if (!i.noIteration) delete this[i];
    ]]></javascript:method>
  </javascript:object>
</javascript:code>