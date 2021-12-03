<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/javascript/2006">
  <javascript:object>
    <javascript:static-method name="startedLoading" parameters="j1DisplayObject"><![CDATA[
      //create and record
      var jNewAbsoluteLoadingDIV = JOO.createElement("div", {class:"CSS__Overlay gs-exclusivity-sibling gs-interface-mode-placeholder gs-ajax-loading-message gs-ajax-loading"});
      j1DisplayObject.o.jNewAbsoluteLoadingDIV = jNewAbsoluteLoadingDIV;

      //append to the parent because this is likely to be hidden
      //also we want to be in the correct hierarchy for gs-exclusivity-* and positioning
      jNewAbsoluteLoadingDIV.insertAfter(j1DisplayObject);

      //optional because sometimes the caller will position in other ways
      if (j1DisplayObject.allVisible()) {
        jNewAbsoluteLoadingDIV.offset(j1DisplayObject.offset()); //absolute position
      } else {
        //using base library jQuery show() and hide()
        //because we do not want to trigger any AJAX loading!
        j1DisplayObject.showDirect();
        if (j1DisplayObject.allHidden()) Debug.error(Overlay, "an ancestor or j1LoaderPosition is not visible. tried to temporarily show positioning element but failed");
        else jNewAbsoluteLoadingDIV.offset(j1DisplayObject.offset()); //absolute position
        j1DisplayObject.hideDirect();
      }

      //trigger the exclusive sibling
      jNewAbsoluteLoadingDIV.show();

      return jNewAbsoluteLoadingDIV;
    ]]></javascript:static-method>

    <javascript:static-method name="finishedLoading" parameters="j1DisplayObject"><![CDATA[
      if ((j1DisplayObject.o.jNewAbsoluteLoadingDIV !== undefined)) {
        j1DisplayObject.o.jNewAbsoluteLoadingDIV.remove();
        delete j1DisplayObject.o.jNewAbsoluteLoadingDIV;
      }
    ]]></javascript:static-method>
  </javascript:object>
</javascript:code>