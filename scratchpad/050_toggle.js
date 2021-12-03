<javascript:code xmlns:repository="http://general_server.org/xmlnamespaces/repository/2006">
  <javascript:raw><![CDATA[
    //----------------------------------------------------------- common
    //arrays of expanded objects
    var item_click_expanded                   = [],
        item_click_exclusive_sibling_expanded = [],
        item_hover_exclusive_global_expanded  = [],
        item_click_exclusive_global_expanded  = [];

    var item_hover_exclusive_global_closing_timer;

    function gs_item_contents(jItem) {
      return jItem.hasClass('item_contents') ? jItem : jItem.find('.item_contents:first');
    }

    //----------------------------------------------------- clicking events
    //can optionally be called directly with a null, fCallback
    function item_click(event, fCallback) {
        var bExpanded = toggle_item(event, fCallback);
        if (bExpanded) item_click_expanded.push(this);
        else           item_click_expanded.remove(this);

        hideAll_item_click_exclusive_global();
        hideAll_item_hover_exclusive_global();

        /* supress bubble up */
        if (event) event.stopImmediatePropagation();
        return false;
    }

    function item_click_exclusive_sibling(event, fCallback) {
        //only one sibling can be expanded at a time (not global)
        var self     = this;
        var jTarget  = gs_item_contents($(this));
        if (jTarget.filter(":hidden").length) {
            show_item.call(this, event, fCallback);
            //close siblings
            $(this).siblings().each(function(){
                hide_item.apply(this);
                item_click_exclusive_sibling_expanded.remove(this);
            });
            item_click_exclusive_sibling_expanded.push(this);
        } else {
            hide_item.call(this, event, fCallback);
            item_click_expanded.remove(this);
        }

        /* supress bubble up */
        if (event) event.stopImmediatePropagation();
        return false;
    }

    function item_hover_exclusive_global(event, fCallback) {
        //global singular item_contents (hover over menus)
        //only show this if it is not already shown
        if ($.inArray(this, item_hover_exclusive_global_expanded) == -1) {
            var self    = this;
            var jTarget = gs_item_contents($(this));
            show_item.call(this, event, fCallback);
            $(document.body).addClass("has-item-hover-exclusive-global-expanded");

            //close anything that is not an ancestor of this
            var elementtoclose, ancestor;
            for (var i = 0; i < item_hover_exclusive_global_expanded.length; i++) {
                ancestor       = this;
                elementtoclose = item_hover_exclusive_global_expanded[i];
                while (ancestor && ancestor != elementtoclose) ancestor = ancestor.parentNode;
                if (!ancestor) {
                    hide_item.apply(elementtoclose);
                    item_hover_exclusive_global_expanded.remove(elementtoclose);
                }
            }
            item_hover_exclusive_global_expanded.push(this);
        }
        if (item_hover_exclusive_global_closing_timer) clearTimeout(item_hover_exclusive_global_closing_timer); //prevent closing if in process

        /* supress bubble up */
        if (event) event.stopImmediatePropagation();
        return false;
    }

    function item_click_exclusive_global(event, fCallback) {
        //global singular item_contents (hover over menus)
        //only show this if it is not already shown
        if ($.inArray(this, item_click_exclusive_global_expanded) == -1) {
            var self    = this;
            var jTarget = gs_item_contents($(this));
            show_item.call(this, event, fCallback);
            $(document.body).addClass("has-item-click-exclusive-global-expanded");

            //close anything that is not an ancestor of this
            var elementtoclose, ancestor;
            for (var i = 0; i < item_click_exclusive_global_expanded.length; i++) {
                ancestor       = this;
                elementtoclose = item_click_exclusive_global_expanded[i];
                while (ancestor && ancestor != elementtoclose) ancestor = ancestor.parentNode;
                if (!ancestor) {
                    hide_item.apply(elementtoclose);
                    item_click_exclusive_global_expanded.remove(elementtoclose);
                }
            }
            item_click_exclusive_global_expanded.push(this);
        }

        /* supress bubble up */
        if (event) event.stopImmediatePropagation();
        return false;
    }

    //----------------------------------------------------- basic show_item hide toggle functionality
    function toggle_item(event, fCallback) {
        var jTarget = gs_item_contents($(this));
        var bExpanded;

        if (jTarget.filter(":hidden").length) {
            show_item.call(this, event, fCallback);
            bExpanded = true;
        } else {
            hide_item.call(this, event, fCallback);
            bExpanded = false;
        }

        return bExpanded;
    }

    function show_item(event, fCallback) {
        var jTarget = gs_item_contents($(this));
        var style   = 'slide';

        //if this node has no children then do nothing
        if (jTarget.filter(":hidden").length) {
            //style settings
            if ($(this).hasClass("item_fade"))    style = 'fade';
            if ($(this).hasClass("item_fast"))    style = 'fast';
            if ($(this).hasClass("item_instant")) style = 'instant';

            //classes
            $(this).addClass("expanded");
            $(this).removeClass("compact");
            if (event) {
              $(event.target).addClass("expanded");
              $(event.target).removeClass("compact");
            }

            //events
            $(this).trigger("expanded");

            //visibility
            switch (style) {
              case 'fade':    {jTarget.fadeIn(500, fCallback); break;} //"fast" = 200, "slow" = 600 milliseconds
              case 'fast':    {jTarget.show("fast"); break;}
              case 'instant': {jTarget.show(); break;}
              default: jTarget.slideDown("fast", fCallback);
            }
        }
    }

    function hide_item(event, fCallback) {
        var jTarget = gs_item_contents($(this));
        var style   = 'slide';

        //if this node has no children then do nothing
        if (jTarget.filter(":visible").length) {
            //style settings
            if ($(this).hasClass("item_fade"))    style = 'fade';
            if ($(this).hasClass("item_fast"))    style = 'fast';
            if ($(this).hasClass("item_instant")) style = 'instant';

            //classes
            $(this).removeClass("expanded");
            $(this).addClass("compact");
            if (event) {
              $(event.target).removeClass("expanded");
              $(event.target).addClass("compact");
            }

            //events
            $(this).trigger("compacted");

            //visibility
            switch (style) {
              case 'fade':    {jTarget.fadeOut(300, fCallback); break;} //"fast" = 200, "slow" = 600 milliseconds
              case 'fast':    {jTarget.hide('fast'); break;}
              case 'instant': {jTarget.hide(); break;}
              default: jTarget.slideUp("fast", fCallback);
            }
        }
    }

    //----------------------------------------------------------- global hide and show functions
    function expandAll() {
        alert('not implemented');
    }
    function hideAll() {
        hideAll_item_click();
        hideAll_item_click_exclusive_sibling();
        hideAll_item_hover_exclusive_global();
    }
    function hideAll_item_click() {
      for (var i = 0; i < item_click_expanded.length; i++) hide_item.apply(item_click_expanded[i]);
      item_click_expanded = [];
    }
    function hideAll_item_click_exclusive_sibling() {
      for (var i = 0; i < item_click_exclusive_sibling_expanded.length; i++) hide_item.apply(item_click_exclusive_sibling_expanded[i]);
      item_click_exclusive_sibling_expanded = [];
    }
    function hideAll_item_hover_exclusive_global() {
      for (var i = 0; i < item_hover_exclusive_global_expanded.length; i++) hide_item.apply(item_hover_exclusive_global_expanded[i]);
      item_hover_exclusive_global_expanded = [];
      $(document.body).removeClass("has-item-hover-exclusive-global-expanded");
    }
    function hideAll_item_click_exclusive_global() {
      for (var i = 0; i < item_click_exclusive_global_expanded.length; i++) hide_item.apply(item_click_exclusive_global_expanded[i]);
      item_click_exclusive_global_expanded = [];
      $(document.body).removeClass("has-item-click-exclusive-global-expanded");
    }

    //---------------------------------------------------------- setup
    function setupOnce_Toggles(event) {
      Debug.cinfo("setupOnce_Toggles()");

      //attach the click events for show/hide
      $(document).on("click",     ".item_click", null, item_click);
      $(document).on("click",     ".item_click_exclusive_sibling", null, item_click_exclusive_sibling);
      $(document).on("mouseover", ".item_hover_exclusive_global",  null, item_hover_exclusive_global);
      $(document).on("click",     ".item_click_exclusive_global",  null, item_click_exclusive_global);

      //hide all of the menus if the mouse is over the document or is clicked (IE fucks up the document.mouseover)
      $(document).click(hideAll_item_hover_exclusive_global);
      $(document).click(hideAll_item_click_exclusive_global);
      if (!window.isie) $(document).mouseover(function (){
          if (item_hover_exclusive_global_closing_timer) clearTimeout(item_hover_exclusive_global_closing_timer);
          item_hover_exclusive_global_closing_timer = setTimeout(hideAll_item_hover_exclusive_global, 1000);
      });
    }
    //only attachs to document which doesnt depend on content changes
    $(document).ready(setupOnce_Toggles);
  ]]></javascript:raw>
</javascript:code>