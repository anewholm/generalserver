<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/javascript/2006">
  <javascript:object>
    <javascript:method name="displayAdded_default" parameters="j1DisplayObject" chain="true"><![CDATA[
      //TODO: client-side xpath query and XSLT abstraction library
      //dynamic content form HTML:
      //  replace this with a generic contents replacement based on xpath
      //  as with the server-side interfaces system
      //  <object:Thing att="{../xpath-thing}"
      //NOTE: CSS path selectors CANNOT select the parent
      //  which is often the requirement here (e.g. Class__ContextMenu)
      //  and we want homogeneity of tools
      //
      //<div class=title>{ancestor::/@title}</div>
      //  http://dev.abiss.gr/sarissa/
      //  https://github.com/ilinsky/jquery-xpath/blob/master/README.md
      var jTitle       = j1DisplayObject.children(".gs-title");
      var jContainerDO = j1DisplayObject.parent().closest(Object);
      if (jContainerDO.length) jTitle.text(jContainerDO.cssClassName());
    ]]></javascript:method>

    <javascript:static-method name="startedLoading" parameters="j1DisplayObject"><![CDATA[
      var jAbsoluteLoadingDIV = Overlay.startedLoading(j1DisplayObject);
      jAbsoluteLoadingDIV.positionToMouse();
      return jAbsoluteLoadingDIV;
    ]]></javascript:static-method>

    <javascript:static-method name="finishedLoading" parameters="j1DisplayObject"><![CDATA[
      return Overlay.finishedLoading.apply(this, arguments);
    ]]></javascript:static-method>
  </javascript:object>
</javascript:code>