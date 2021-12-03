<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/javascript/2006">
  <javascript:dependency uri="/resources/shared/jquery/plugins/splitter/js/jquery.splitter-0.14.0.js"/>

  <javascript:object>
    <javascript:event-handler event="resize" action="setFrameHeights"/>
    <javascript:event-handler event="ready" action="setFrameHeights"/>

    <javascript:method name="setFrameHeights" is-protected="true" parameters="j1DisplayObject"><![CDATA[
      var iMainFrameHeight;
      var jHeadFrame     = j1DisplayObject.children(".gs-name-frame-head");
      var jFooterFrame   = j1DisplayObject.children(".gs-name-frame-footer");
      var jMainFrameSet  = j1DisplayObject.children(".gs-name-verticalframeset-main");

      iMainFrameHeight = j1DisplayObject.height() - jHeadFrame.height() - jFooterFrame.height() - 4;
      jMainFrameSet.height(iMainFrameHeight.toString());
    ]]></javascript:method>

    <javascript:method name="setContents" parameters="j1DisplayObject, uContents, [oParams, sTitle]"><![CDATA[
      var jDetailFrame   = j1DisplayObject.find("> .gs-name-verticalframeset-main > .gs-name-frame-detail");
      var jDetailObjects = jDetailFrame.children(Object);

      if (jDetailObjects.length) {
        //for example an MDI with setContents() would then request tab add
        //setContents() will only run on objects that implement it
        //TODO: check that an Object does implement setContents()
        jDetailObjects.setContents(uContents, oParams, sTitle);
      } else Debug.warn(this, "failed to locate the detail Frame or an Object that can setContents()");
    ]]></javascript:method>
  </javascript:object>
</javascript:code>