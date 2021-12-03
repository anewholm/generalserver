<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/javascript/2006">
  <javascript:object>
    <javascript:method name="canHandleEvent" parameters="eEvent, jqCurrentDisplayObject, fEventHandler"><![CDATA[
      //Menus should not handle the selection events on their menu-items
      //but should handle SubMenu hovers and clicks
      //if the fEventHandler is declared directly on the Menu Class hierarchy then handle it
      //otherwise, it is inherited and we let it go
      //this is only really important for the IDE context-menu because it allows common BaseObject methods
      return (Menu === fEventHandler.classDerivedConstructor || fEventHandler.classDerivedConstructor.isDerivedFrom(Menu));
    ]]></javascript:method>
    
    <javascript:event-handler event="mouseover" action="nothing"><![CDATA[
      //suppression only
      return false;
    ]]></javascript:event-handler>

    <javascript:event-handler event="click" action="nothing"><![CDATA[
      //suppression only
      return false;
    ]]></javascript:event-handler>

    <javascript:method name="toggleCSSDashPathDOFromEventFunctionElement" parameters="j1DisplayObject, s1DashCSSPath, [fCallback, bForceReload]"><![CDATA[
      //copied here otherwise the canHandleEvent() will ignore it
      return DatabaseElement.prototype.toggleCSSDashPathDOFromEventFunctionElement.apply(this, arguments);
    ]]></javascript:method>

    <javascript:event-handler event="mouseover"><![CDATA[
      //no need to do mouseout because these are global exclusive shows
      //TODO: introduced delayed load for AJAX SubMenu
      if (Event.currentEvent().targetElement().is("li")) this.showLISubMenu(Event.currentEvent().targetElement());
    ]]></javascript:event-handler>

    <javascript:method name="showLISubMenu" parameters="j1TargetLI"><![CDATA[
      //show first Class__SubMenu
      j1TargetLI.children(SubMenu).show();
    ]]></javascript:method>
  </javascript:object>
</javascript:code>