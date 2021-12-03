<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/javascript/2006">
  <javascript:object>
    <javascript:static-method name="startedLoading" parameters="j1DisplayObject"><![CDATA[
      //called from the AJAX init, so SubMenu does not exist yet
      var jAbsoluteLoadingDIV = Overlay.startedLoading.apply(this, arguments);
      SubMenu.setPositionContainer(jAbsoluteLoadingDIV);
      return jAbsoluteLoadingDIV;
    ]]></javascript:static-method>

    <javascript:static-method name="finishedLoading" parameters="j1DisplayObject"><![CDATA[
      //called from the AJAX finish
      return Overlay.finishedLoading.apply(this, arguments);
    ]]></javascript:static-method>

    <javascript:event-handler event="shown" parameters="j1DisplayObject"><![CDATA[
      SubMenu.setPositionContainer(j1DisplayObject);
      //jContextMenu.scrollIntoView(); //TODO: fix this scrollIntoView
    ]]></javascript:event-handler>

    <javascript:static-method name="setPositionContainer" parameters="j1Element, [oOptions = {}]"><![CDATA[
      //SubMenu appears in a different place depending on the aspect of the parent Menu
      var jOffestElement   = j1Element.offsetParent();
      var jParentElement   = j1Element.parent();
      var jContainerMenu   = jParentElement.closest(Menu); //Menu takes in to account inheritance, e.g. ContextMenu
      var bParentHasLayout = jOffestElement.is(jParentElement);
      var iOffestTop       = (bParentHasLayout ? 0 : jParentElement.position().top);
      var iOffestLeft      = (bParentHasLayout ? 0 : jParentElement.position().left);
      var fContainerOrientation = (jContainerMenu.length ? jContainerMenu.setting("orientation") : "vertical");
      
      switch (fContainerOrientation) {
        case window.HorizontalMenu: {
          //SubMenu position is directly under the parent MenuItem and aligned to the left of it
          //false indicates do not include the margins
          //mouseout occurs at the beginning of the margin
          j1Element.position({top:iOffestTop + jParentElement.outerHeight(false) - 1, left:iOffestLeft});
          break;
        }
        case window.VerticalMenu: {
          //SubMenu position is directly right of the parent MenuItem and aligned to the top of it
          //false indicates do not include the margins
          //mouseout occurs at the beginning of the margin
          j1Element.position({top:iOffestTop, left:iOffestLeft + jParentElement.outerWidth(false) - 1});
          break;
        }
        default: {
          Debug.warn(SubMenu, "unable to determine the orientation of this SubMenu container [" + fContainerOrientation + "]");
        }
      }

      return j1Element;
    ]]></javascript:static-method>
  </javascript:object>
</javascript:code>