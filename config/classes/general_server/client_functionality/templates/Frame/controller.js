<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006" xmlns="http://general_server.org/xmlnamespaces/javascript/2006">
  <javascript:dependency uri="/resources/shared/jquery/plugins/splitter/js/jquery.splitter-0.14.0.js"/>

  <javascript:object>
    <javascript:event-handler event="shown" parameters="j1DisplayObject"><![CDATA[
      var iScrollTop;
      
      if (Event.currentEvent().targetElement().hasClass(window.ContextMenu)) {
        if (iScrollTop = j1DisplayObject.scrollTop()) {
          //Frame content is scrolled
          //we need to show context menu without loosing scroll
          j1DisplayObject.css('position', 'relative');
          j1DisplayObject.css('top',      -iScrollTop + 'px');
        }

        //context-menu is a CHILD of this frame
        //make it all visible
        j1DisplayObject.addClass("gs-has-contextmenu");
        this.aOldOverflow = [j1DisplayObject.css('overflow'), j1DisplayObject.css('overflow-x'), j1DisplayObject.css('overflow-y')];
        j1DisplayObject.css('overflow',   'visible');
        j1DisplayObject.css('overflow-x', 'visible');
        j1DisplayObject.css('overflow-y', 'visible');
      }
    ]]></javascript:event-handler>

    <javascript:event-handler event="hidden" parameters="j1DisplayObject"><![CDATA[
      if (Event.currentEvent().targetElement().hasClass(window.ContextMenu)) {
        //sets overflow to scroll
        j1DisplayObject.removeClass("gs-has-contextmenu");
        
        //removes any manual scroll position setting
        if (this.aOldOverflow) {
          j1DisplayObject.css('overflow',   this.aOldOverflow[0]);
          j1DisplayObject.css('overflow-x', this.aOldOverflow[1]);
          j1DisplayObject.css('overflow-y', this.aOldOverflow[2]);
        }
        j1DisplayObject.css('position', '');
        j1DisplayObject.css('top',      '');
      }
    ]]></javascript:event-handler>
  </javascript:object>
</javascript:code>